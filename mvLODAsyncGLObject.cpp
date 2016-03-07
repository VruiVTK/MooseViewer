#include "mvLODAsyncGLObject.h"
#include "mvAsyncGLObject.h"

#include <GL/GLContextData.h>

#include <Vrui/Vrui.h>

#include "mvApplicationState.h"
#include "mvContextState.h"
#include "mvProgress.h"
#include "mvProgressCookie.h"

#include <vtkTimerLog.h>

#include <cassert>
#include <chrono>
#include <iostream>
#include <sstream>
#include <type_traits>

namespace {
template <typename T>
void deleteLODArray(mvLODAsyncGLObject::LODArray<T*> &a)
{
  for (T *o: a)
    {
    delete o;
    }
  a.fill(nullptr);
}

std::ostream& operator<<(std::ostream &str,
                         mvLODAsyncGLObject::LevelOfDetail lod)
{
  switch (lod)
    {
    case mvLODAsyncGLObject::LevelOfDetail::Hint:
      return (str << "Hint");
    case mvLODAsyncGLObject::LevelOfDetail::LoRes:
      return (str << "LoRes");
    case mvLODAsyncGLObject::LevelOfDetail::HiRes:
      return (str << "HiRes");
    default:
      return (str << "Invalid");
    }
}

template <typename T>
std::ostream& operator<<(std::ostream &str,
                         const mvLODAsyncGLObject::LODIteratorImpl<T> &iter)
{
  return (str << static_cast<mvLODAsyncGLObject::LevelOfDetail>(iter));
}

} // end anon namespace

//------------------------------------------------------------------------------
mvLODAsyncGLObject::DataPipeline::~DataPipeline()
{
}

//------------------------------------------------------------------------------
mvLODAsyncGLObject::RenderPipeline::~RenderPipeline()
{
}

//------------------------------------------------------------------------------
mvLODAsyncGLObject::DataItem::~DataItem()
{
  deleteLODArray(renderPipelines);
}

//------------------------------------------------------------------------------
mvLODAsyncGLObject::mvLODAsyncGLObject()
  : m_benchmark(false)
{
}

//------------------------------------------------------------------------------
mvLODAsyncGLObject::~mvLODAsyncGLObject()
{
  delete m_objState;
}

//------------------------------------------------------------------------------
void mvLODAsyncGLObject::init(const mvApplicationState &)
{
  m_objState = this->createObjectState();
  assert("createObjectState() result valid." && m_objState);

  for (auto lod = this->bestToFast(/*skipInvalid =*/ false); lod; ++lod)
    {
    DataPipeline *dp = this->createDataPipeline(lod);
    if (dp)
      {
      lod->status = LODStatus::OutOfDate;
      lod->dataPipeline = dp;
      lod->result = this->createLODData(lod);
      assert("createLODData result valid." && lod->result);
      }
    }
}

//------------------------------------------------------------------------------
void mvLODAsyncGLObject::initMvContext(mvContextState &contextState,
                                       GLContextData &contextData) const
{
  this->Superclass::initMvContext(contextState, contextData);

  assert("Duplicate context initialization detected!" &&
         !contextData.retrieveDataItem<DataItem>(this));

  DataItem *dataItem = new DataItem;
  contextData.addDataItem(this, dataItem);

  for (auto lod = this->bestToFast(); lod; ++lod)
    {
    RenderPipeline *rp = this->createRenderPipeline(lod);
    assert("createRenderPipeline(lod) result valid" && rp);
    rp->init(*m_objState, contextState);
    dataItem->renderPipelines[lod] = rp;
    }
}

//------------------------------------------------------------------------------
void mvLODAsyncGLObject::syncApplicationState(const mvApplicationState &state)
{
  m_objState->update(state);

  // Grab the best detail data pipeline.
  auto lod = this->bestToFast();

  // Iterate through all LODs and complete any pending updates. This prevents
  // progress cookies from lingering if a `better` LOD finishes before a
  // 'faster' one:
  for (; lod; ++lod)
    {
    if (lod->status == LODStatus::Updating)
      {
      assert("Thread has been started." && lod->monitor.valid());

      // Check state:
      std::chrono::seconds now(0);
      std::future_status fState = lod->monitor.wait_for(now);

      // We should never use deferred execution:
      assert("Always async." && fState != std::future_status::deferred);

      if (fState == std::future_status::ready)
        { // Update the result if done:
        lod->monitor.get(); // Reset thread state
        lod->dataPipeline->exportResult(*lod->result);
        lod->status = LODStatus::UpToDate;
        assert("Cookie created." && lod->cookie != nullptr);
        state.progress().removeEntry(lod->cookie);
        lod->cookie = nullptr;
        }
      }
    }

  // Traverse from best-to-fast, searching for the first up-to-date LOD
  // that can be made live.
  for (lod.jumpToBest(); lod; ++lod)
    {
    // Reconfigure and test any pipelines that are marked up-to-date:
    if (lod->status == LODStatus::UpToDate)
      {
      lod->dataPipeline->configure(*m_objState, state);
      if (lod->dataPipeline->needsUpdate(*m_objState, *lod->result))
        {
        lod->status = LODStatus::OutOfDate;
        }
      }

    // If the current pipeline is up-to-date, don't bother checking the
    // lower detailed ones.
    if (lod->status == LODStatus::UpToDate)
      {
      break;
      }
    }

  // At this point, lod is pointing at either the up-to-date, live copy
  // of the best LOD available, or has exhausted available LODs (all are
  // updating or out-of-date).

  // Make the iterator point at the first LOD that's better than the current
  // best:
  if (lod)
    { // We're pointing at the best up-to-date pipeline.
    --lod; // Move to the next higher quality LOD:
    }
  else
    { // There is no up-to-date LOD. Jump to the fastest LOD:
    lod.jumpToFast();
    }

  // Now check all LODs that are better than what is currently shown.
  for (; lod; --lod)
    {
    if (lod->status == LODStatus::OutOfDate)
      {
      lod->dataPipeline->configure(*m_objState, state);
      if (lod->dataPipeline->needsUpdate(*m_objState, *lod->result))
        { // If an update is needed, execute the pipeline
        if (lod->dataPipeline->forceSynchronousUpdates())
          { // Run immediately:
          this->executeWrapper(lod, lod->dataPipeline);
          lod->dataPipeline->exportResult(*lod->result);
          lod->status = LODStatus::UpToDate;
          }
        else
          { // Run in background:
          assert("Cookie cleaned up." && lod->cookie == nullptr);

          std::ostringstream progLabel;
          progLabel << "Updating " << this->progressLabel() << " ("
                    << lod << ")";
          lod->cookie = state.progress().addEntry(progLabel.str());
          assert("Cookie assigned." && lod->cookie != nullptr);

          lod->monitor = std::async(std::launch::async,
                                    &mvLODAsyncGLObject::executeWrapper,
                                    this, lod, lod->dataPipeline);
          lod->status = LODStatus::Updating;
          }
        }
      else
        {
        // LOD is suddenly and inexplicably up-to-date! Isn't that magical...
        lod->status = LODStatus::UpToDate;
        }
      }
    }
}

//------------------------------------------------------------------------------
void mvLODAsyncGLObject::syncContextState(const mvApplicationState &appState,
                                          const mvContextState &contextState,
                                          GLContextData &contextData) const
{
  this->Superclass::syncContextState(appState, contextState, contextData);

  DataItem *dataItem = contextData.retrieveDataItem<DataItem>(this);
  assert(dataItem);

  // Show the best up-to-date LOD:
  bool liveSet = false;
  for (const_iterator lod = this->bestToFast(); lod; ++lod)
    {
    RenderPipeline *rp = dataItem->renderPipeline(lod);

    if (!liveSet && lod->status == LODStatus::UpToDate)
      {
      rp->update(*m_objState, appState, contextState, *lod->result);
      liveSet = true;
      }
    else
      {
      rp->disable();
      }
    }
}

//------------------------------------------------------------------------------
void mvLODAsyncGLObject::executeWrapper(LevelOfDetail lod,
                                        mvLODAsyncGLObject::DataPipeline *p)
{
  vtkTimerLog *log = nullptr;
  if (m_benchmark)
    {
    std::ostringstream out;
    out << "Updating " << this->progressLabel() << " (" << lod << ").\n";
    std::cerr << out.str();

    log = vtkTimerLog::New();
    log->StartTimer();
    }

  p->execute();

  if (log != nullptr)
    {
    log->StopTimer();
    std::ostringstream out;
    out << this->progressLabel() << " (" << lod << ") ready! ("
        << log->GetElapsedTime() << "s)\n";
    std::cerr << out.str();
    log->Delete();
    }

  Vrui::requestUpdate();
}

//------------------------------------------------------------------------------
mvLODAsyncGLObject::DataPipelineManager::~DataPipelineManager()
{
  // Ensure background thread is finished:
  if (monitor.valid())
    {
    monitor.wait();
    }

  delete dataPipeline;
  delete result;
  // Don't free cookie -- these are owned by mvProgress.
}
