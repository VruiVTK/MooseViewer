#include "mvApplicationState.h"

#include <vtkExodusIIReader.h>

#include "ArrayLocator.h"
#include "mvContours.h"
#include "mvVolume.h"
#include "WidgetHints.h"

mvApplicationState::mvApplicationState()
  : m_contours(new mvContours),
    m_locator(new ArrayLocator),
    m_reader(vtkExodusIIReader::New()),
    m_widgetHints(new WidgetHints()),
    m_volume(new mvVolume())
{
  m_objects.push_back(m_contours);
  m_objects.push_back(m_volume);
}

mvApplicationState::~mvApplicationState()
{
  delete m_contours;
  delete m_locator;
  m_reader->Delete();
  delete m_volume;
  delete m_widgetHints;
}
