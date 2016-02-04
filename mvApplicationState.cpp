#include "mvApplicationState.h"

#include "ArrayLocator.h"
#include "mvContours.h"
#include "vtkExodusIIReader.h"
#include "WidgetHints.h"

mvApplicationState::mvApplicationState()
  : m_contours(new mvContours),
    m_locator(new ArrayLocator),
    m_reader(vtkExodusIIReader::New()),
    m_widgetHints(new WidgetHints())
{
  m_objects.push_back(m_contours);
}

mvApplicationState::~mvApplicationState()
{
  delete m_contours;
  delete m_locator;
  m_reader->Delete();
  delete m_widgetHints;
}
