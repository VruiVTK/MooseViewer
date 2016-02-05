#include "mvContextState.h"

#include <ExternalVTKWidget.h>
#include <vtkActor.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkCompositePolyDataMapper.h>
#include <vtkExternalOpenGLRenderer.h>
#include <vtkLight.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkNew.h>

mvContextState::mvContextState()
{
  m_widget->GetRenderWindow()->AddRenderer(m_renderer.GetPointer());
  m_renderer->SetUseDepthPeeling(1);
  m_renderer->SetMaximumNumberOfPeels(4);
  m_renderer->SetOcclusionRatio(0.1);

  // TODO refactor the rest into mvGLObject::DataObjects:

  this->actor->GetProperty()->SetEdgeColor(1.0, 1.0, 1.0);
  m_renderer->AddActor(this->actor.GetPointer());

  this->mapper->SetInputConnection(this->compositeFilter->GetOutputPort());
  this->mapper->SetScalarVisibility(1);
  this->mapper->SetScalarModeToUsePointFieldData();
  this->actor->SetMapper(this->mapper.GetPointer());

  this->actorOutline->GetProperty()->SetColor(1.0, 1.0, 1.0);
  m_renderer->AddActor(this->actorOutline.GetPointer());

  m_colorMap->SetNumberOfColors(256);
  m_colorMap->Build();
  this->mapper->SetLookupTable(m_colorMap.GetPointer());

  this->framerate->GetTextProperty()->SetJustificationToLeft();
  this->framerate->GetTextProperty()->SetVerticalJustificationToTop();
  this->framerate->GetTextProperty()->SetFontSize(12);
  this->framerate->SetTextScaleModeToViewport();
  vtkCoordinate *fpsCoord = this->framerate->GetPositionCoordinate();
  fpsCoord->SetCoordinateSystemToNormalizedDisplay();
  fpsCoord->SetValue(0, 0.999);
  m_renderer->AddActor2D(this->framerate.GetPointer());
}

mvContextState::~mvContextState()
{

}
