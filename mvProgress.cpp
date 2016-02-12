#include "mvProgress.h"

#include <GL/GLContextData.h>

#include <vtkCoordinate.h>
#include <vtkExternalOpenGLRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include "mvApplicationState.h"
#include "mvContextState.h"
#include "mvProgressCookie.h"

#include <algorithm>
#include <cassert>
#include <set>
#include <sstream>

//------------------------------------------------------------------------------
mvProgress::DataItem::DataItem()
{
  actor->SetTextScaleModeToViewport();
  vtkCoordinate *coord = actor->GetPositionCoordinate();
  coord->SetCoordinateSystemToNormalizedDisplay();
  coord->SetValue(0.99, 0.99);
}

//------------------------------------------------------------------------------
mvProgress::DataItem::~DataItem()
{
}

//------------------------------------------------------------------------------
mvProgress::mvProgress()
{
  m_tprop->SetJustificationToRight();
  m_tprop->SetVerticalJustificationToTop();
  m_tprop->SetFontSize(8);
  m_tprop->BoldOn();
  m_tprop->SetColor(1., .2, .2);
  m_tprop->SetBackgroundColor(0.25, 0.25, 0.25);
  m_tprop->SetBackgroundOpacity(0.5);
}

//------------------------------------------------------------------------------
mvProgress::~mvProgress()
{
  if (!m_entries.empty())
    {
    std::cerr << "Cleaning up unfinished progress cookies:\n";
    for (auto cookie : m_entries)
      {
      std::cerr << " -- " << cookie->text() << "\n";
      delete cookie;
      }
    m_entries.clear();
    }
}

//------------------------------------------------------------------------------
mvProgressCookie *mvProgress::addEntry(std::string text)
{
  mvProgressCookie *cookie = new mvProgressCookie(text);
  m_entries.push_back(cookie);
  return cookie;
}

//------------------------------------------------------------------------------
void mvProgress::removeEntry(mvProgressCookie *cookie)
{
  auto newEnd = std::remove(m_entries.begin(), m_entries.end(), cookie);
  assert("Double free detected." && newEnd < m_entries.end());
  m_entries.resize(std::distance(m_entries.begin(), newEnd));
  delete cookie;
}

//------------------------------------------------------------------------------
void mvProgress::initMvContext(mvContextState &mvContext,
                               GLContextData &contextData) const
{
  this->Superclass::initMvContext(mvContext, contextData);

  assert("Duplicate context initialization detected!" &&
         !contextData.retrieveDataItem<DataItem>(this));

  DataItem *dataItem = new DataItem;
  contextData.addDataItem(this, dataItem);

  dataItem->actor->SetTextProperty(m_tprop.Get());
  mvContext.renderer().AddActor2D(dataItem->actor.Get());
}

//------------------------------------------------------------------------------
void mvProgress::syncApplicationState(const mvApplicationState &state)
{
  this->Superclass::syncApplicationState(state);

  if (m_entries.empty())
    {
    m_text.clear();
    return;
    }

  std::ostringstream str;
  str << "Update(s) in progress:\n";
  for (const auto cookie : m_entries)
    {
    str << "  - " << cookie->text() << "\n";
    }
  m_text = str.str();
  // Pop off the last newline:
  m_text.pop_back();
}

//------------------------------------------------------------------------------
void mvProgress::syncContextState(const mvApplicationState &appState,
                                  const mvContextState &contextState,
                                  GLContextData &contextData) const
{
  this->Superclass::syncContextState(appState, contextState, contextData);

  DataItem *dataItem = contextData.retrieveDataItem<DataItem>(this);
  assert(dataItem);

  if (m_text.empty())
    {
    dataItem->actor->SetVisibility(0);
    return;
    }

  dataItem->actor->SetInput(m_text.c_str());
  dataItem->actor->SetVisibility(1);
}
