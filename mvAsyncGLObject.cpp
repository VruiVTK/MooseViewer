#include "mvAsyncGLObject.h"

#include <Vrui/Vrui.h>

#include "mvApplicationState.h"
#include "mvProgress.h"
#include "mvProgressCookie.h"

#include <cassert>
#include <chrono>
#include <iostream>

//------------------------------------------------------------------------------
mvAsyncGLObject::mvAsyncGLObject()
  : m_cookie(nullptr)
{
}

//------------------------------------------------------------------------------
mvAsyncGLObject::~mvAsyncGLObject()
{
  // Don't free cookie -- mvProgress owns these and will clean them up.
  // If mvAsyncGLObjects start being deleted before process exit, we'll
  // to remove cookies from mvProgress from the cookie's dtor.

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

      // Clean up the progress monitor:
      assert("Cookie exists." && m_cookie != nullptr);
      appState.progress().removeEntry(m_cookie);
      m_cookie = nullptr;
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
    assert("Cookie cleaned up." && m_cookie == nullptr);
    m_cookie = appState.progress().addEntry(this->progressLabel());

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
