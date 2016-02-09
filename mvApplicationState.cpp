#include "mvApplicationState.h"

#include <vtkExodusIIReader.h>
#include <vtkLookupTable.h>

#include "ArrayLocator.h"
#include "mvContours.h"
#include "mvFramerate.h"
#include "mvGeometry.h"
#include "mvOutline.h"
#include "mvVolume.h"
#include "WidgetHints.h"

mvApplicationState::mvApplicationState()
  : m_colorMap(vtkLookupTable::New()),
    m_contours(new mvContours),
    m_framerate(new mvFramerate),
    m_geometry(new mvGeometry),
    m_locator(new ArrayLocator),
    m_outline(new mvOutline),
    m_reader(vtkExodusIIReader::New()),
    m_widgetHints(new WidgetHints()),
    m_volume(new mvVolume())
{
  m_objects.push_back(m_contours);
  m_objects.push_back(m_framerate);
  m_objects.push_back(m_geometry);
  m_objects.push_back(m_outline);
  m_objects.push_back(m_volume);
}

mvApplicationState::~mvApplicationState()
{
  m_colorMap->Delete();
  delete m_contours;
  delete m_framerate;
  delete m_geometry;
  delete m_locator;
  delete m_outline;
  m_reader->Delete();
  delete m_volume;
  delete m_widgetHints;
}