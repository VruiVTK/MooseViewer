// MooseViewer includes
#include "DataItem.h"

// VTK includes
#include <ExternalVTKWidget.h>
#include <vtkActor.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkLight.h>
#include <vtkProperty.h>

//----------------------------------------------------------------------------
MooseViewer::DataItem::DataItem(void)
{
  /* Initialize VTK renderwindow and renderer */
  this->externalVTKWidget = vtkSmartPointer<ExternalVTKWidget>::New();
  this->actor = vtkSmartPointer<vtkActor>::New();
  this->actor->GetProperty()->SetEdgeColor(1.0, 1.0, 1.0);
  this->externalVTKWidget->GetRenderer()->AddActor(this->actor);

  this->compositeFilter = vtkSmartPointer<vtkCompositeDataGeometryFilter>::New();

  this->flashlight = vtkSmartPointer<vtkLight>::New();
  this->flashlight->SwitchOff();
  this->flashlight->SetLightTypeToHeadlight();
  this->flashlight->SetColor(0.0, 1.0, 1.0);
  this->flashlight->SetConeAngle(10);
  this->flashlight->SetPositional(true);
  this->externalVTKWidget->GetRenderer()->AddLight(this->flashlight);

  this->actorOutline = vtkSmartPointer<vtkActor>::New();
  this->actorOutline->GetProperty()->SetColor(1.0, 1.0, 1.0);
  this->externalVTKWidget->GetRenderer()->AddActor(this->actorOutline);
}

//----------------------------------------------------------------------------
MooseViewer::DataItem::~DataItem(void)
{
}

