#include "mvAsyncGLObject.h"

#include <Vrui/Vrui.h>

#include <cassert>
#include <chrono>
#include <iostream>

//------------------------------------------------------------------------------
mvAsyncGLObject::mvAsyncGLObject()
{
}

//------------------------------------------------------------------------------
mvAsyncGLObject::~mvAsyncGLObject()
{
  // TODO add an abortDataPipeline virtual to stop calculations on exit.
  // This wouldn't go here (the subclass would already be destroyed), but
  // should go in some mvGLObject::aboutToExit virtual called by
  // mvApplicationState's dtor.
}

//------------------------------------------------------------------------------
void mvAsyncGLObject::syncApplicationState(const mvApplicationState &appState)
{
  this->Superclass::syncApplicationState(appState);

  // Check if an asynch update is in process:
  if (m_monitor.valid())
    {
    // If so, see if the operation has completed:
    std::future_status state = m_monitor.wait_for(std::chrono::milliseconds(0));

    // We should not be using deferred execution:
    assert("Always async." && state != std::future_status::deferred);

    if (state == std::future_status::ready)
      {
      // Clear the monitor:
      m_monitor.get();

      // Sync application state cache:
      this->retrieveDataPipelineResult();
      }
    else
      {
      // Data pipeline has not completed yet.
      return;
      }
    }

  // We only reach this point if there's no update in progress, or if we've just
  // retrieved the last result, so check if we need to update.
  this->configureDataPipeline(appState);
  if (this->dataPipelineNeedsUpdate())
    {
    // Launch background calculation.
    m_monitor = std::async(std::launch::async,
                           &mvAsyncGLObject::internalExecutePipeline, this);
    }
}

//------------------------------------------------------------------------------
void mvAsyncGLObject::internalExecutePipeline() const
{
  this->executeDataPipeline();
  Vrui::requestUpdate();
}
