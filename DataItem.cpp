// MooseViewer includes
#include "DataItem.h"

// VTK includes
#include <ExternalVTKWidget.h>
#include <vtkActor.h>
#include <vtkAppendPolyData.h>
#include <vtkColorTransferFunction.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkContourFilter.h>
#include <vtkGaussianSplatter.h>
#include <vtkLight.h>
#include <vtkLookupTable.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

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

  this->gaussian = vtkSmartPointer<vtkGaussianSplatter>::New();
  this->gaussian->ScalarWarpingOn();
  this->gaussian->NormalWarpingOff();
  this->gaussian->SetRadius(0.05);
  this->gaussian->SetExponentFactor(-1);

  this->mapperVolume =
    vtkSmartPointer<vtkSmartVolumeMapper>::New();
  this->mapperVolume->SetInputConnection(this->gaussian->GetOutputPort());
  this->colorFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
  this->opacityFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();
  vtkSmartPointer<vtkVolumeProperty> volumeProperty =
    vtkSmartPointer<vtkVolumeProperty>::New();
  volumeProperty->SetColor(this->colorFunction);
  volumeProperty->SetScalarOpacity(this->opacityFunction);
  volumeProperty->SetInterpolationTypeToLinear();
  volumeProperty->ShadeOff();
  this->actorVolume = vtkSmartPointer<vtkVolume>::New();
  this->actorVolume->SetMapper(this->mapperVolume);
  this->actorVolume->SetProperty(volumeProperty);
  this->externalVTKWidget->GetRenderer()->AddVolume(this->actorVolume);
}

//----------------------------------------------------------------------------
MooseViewer::DataItem::~DataItem(void)
{
}

