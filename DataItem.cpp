// MooseViewer includes
#include "DataItem.h"

// VTK includes
#include <ExternalVTKWidget.h>
#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkContourFilter.h>
#include <vtkLight.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
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
  this->mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->mapper->SetInputConnection(this->compositeFilter->GetOutputPort());
  this->mapper->SetScalarVisibility(1);
  this->mapper->SetScalarModeToUsePointFieldData();
  this->actor->SetMapper(this->mapper);

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

  this->lut = vtkSmartPointer<vtkLookupTable>::New();
  this->lut->SetNumberOfColors(256);
  this->lut->Build();
  this->mapper->SetLookupTable(this->lut);

  /* Setting alpha bit planes should be done in VRUI while
   * creating the context */
//  this->externalVTKWidget->GetRenderWindow()->SetAlphaBitPlanes(1);
//  this->externalVTKWidget->GetRenderWindow()->SetMultiSamples(0);

  /* Use depth peeling to enable transparency */
  this->externalVTKWidget->GetRenderer()->SetUseDepthPeeling(1);
  this->externalVTKWidget->GetRenderer()->SetMaximumNumberOfPeels(4);
  this->externalVTKWidget->GetRenderer()->SetOcclusionRatio(0.1);

  this->contours = vtkSmartPointer<vtkAppendPolyData>::New();
  this->contourMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->contourMapper->SetScalarVisibility(1);
  this->contourMapper->SetScalarModeToUsePointFieldData();
  this->contourMapper->SetColorModeToMapScalars();
  this->contourMapper->SetLookupTable(this->lut);
  this->contourActor = vtkSmartPointer<vtkActor>::New();
  this->contourActor->SetMapper(this->contourMapper);
  this->externalVTKWidget->GetRenderer()->AddActor(this->contourActor);
}

//----------------------------------------------------------------------------
MooseViewer::DataItem::~DataItem(void)
{
}

