#include "mvFramerate.h"

#include <vtkCoordinate.h>
#include <vtkExternalOpenGLRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include <GL/GLContextData.h>

#include "mvApplicationState.h"
#include "mvContextState.h"

#include <algorithm>
#include <cassert>
#include <sstream>

//------------------------------------------------------------------------------
mvFramerate::DataItem::DataItem()
{
  actor->GetTextProperty()->SetJustificationToLeft();
  actor->GetTextProperty()->SetVerticalJustificationToTop();
  actor->GetTextProperty()->SetFontSize(12);
  actor->SetTextScaleModeToViewport();

  vtkCoordinate *coord = actor->GetPositionCoordinate();
  coord->SetCoordinateSystemToNormalizedDisplay();
  coord->SetValue(0, 0.999);
}

//------------------------------------------------------------------------------
mvFramerate::mvFramerate()
  : m_visible(false)
{
}

//------------------------------------------------------------------------------
mvFramerate::~mvFramerate()
{
}

//------------------------------------------------------------------------------
void mvFramerate::initMvContext(mvContextState &mvContext,
                                GLContextData &contextData) const
{
  this->mvGLObject::initMvContext(mvContext, contextData);

  assert("Duplicate context initialization detected!" &&
         !contextData.retrieveDataItem<DataItem>(this));

  DataItem *dataItem = new DataItem;
  contextData.addDataItem(this, dataItem);

  mvContext.renderer().AddActor2D(dataItem->actor.GetPointer());
}

//------------------------------------------------------------------------------
void mvFramerate::syncApplicationState(const mvApplicationState &state)
{
  this->mvGLObject::syncApplicationState(state);

  const size_t FPSCacheSize = 64;

  m_timer.elapse();
  const double time = m_timer.getTime();

  if (time == 0.) // We just started the timer -- first frame.
    {
    return;
    }
  else if (m_times.size() < FPSCacheSize)
    {
    m_times.push_back(time);
    }
  else
    {
    std::rotate(m_times.begin(), m_times.begin() + 1, m_times.end());
    m_times.back() = time;
    }
}

//------------------------------------------------------------------------------
void mvFramerate::syncContextState(const mvApplicationState &appState,
                                   const mvContextState &contextState,
                                   GLContextData &contextData) const
{
  this->mvGLObject::syncContextState(appState, contextState, contextData);

  DataItem *dataItem = contextData.retrieveDataItem<DataItem>(this);
  assert(dataItem);

  dataItem->actor->SetVisibility(m_visible ? 1 : 0);
  if (m_visible)
    {
    double time = 0.0;
    for (size_t i = 0; i < m_times.size(); ++i)
      {
      time += m_times[i];
      }
    double fps = time > 1e-5 ? m_times.size() / time : 0.;

    std::ostringstream fpsStr;
    fpsStr << "FPS: " << fps;
    dataItem->actor->SetInput(fpsStr.str().c_str());
    }
}
