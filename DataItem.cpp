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
  vtkRenderer* ren = this->externalVTKWidget->AddRenderer();
  ren->AddActor(this->actor);

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
  ren->AddLight(this->flashlight);

  this->actorOutline = vtkSmartPointer<vtkActor>::New();
  this->actorOutline->GetProperty()->SetColor(1.0, 1.0, 1.0);
  ren->AddActor(this->actorOutline);

  this->lut = vtkSmartPointer<vtkLookupTable>::New();
  this->lut->SetNumberOfColors(256);
  this->lut->Build();
  this->mapper->SetLookupTable(this->lut);

  /* Setting alpha bit planes should be done in VRUI while
   * creating the context */
//  this->externalVTKWidget->GetRenderWindow()->SetAlphaBitPlanes(1);
//  this->externalVTKWidget->GetRenderWindow()->SetMultiSamples(0);

  /* Use depth peeling to enable transparency */
  ren->SetUseDepthPeeling(1);
  ren->SetMaximumNumberOfPeels(4);
  ren->SetOcclusionRatio(0.1);

  this->contours = vtkSmartPointer<vtkAppendPolyData>::New();
  this->contourMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->contourMapper->SetScalarVisibility(1);
  this->contourMapper->SetScalarModeToUsePointFieldData();
  this->contourMapper->SetColorModeToMapScalars();
  this->contourMapper->SetLookupTable(this->lut);
  this->contourActor = vtkSmartPointer<vtkActor>::New();
  this->contourActor->SetMapper(this->contourMapper);
  ren->AddActor(this->contourActor);

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
  ren->AddVolume(this->actorVolume);

  this->aContour = vtkSmartPointer<vtkContourFilter>::New();
  this->aContour->ComputeScalarsOn();
  this->aContourMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->aContourMapper->SetInputConnection(this->aContour->GetOutputPort());
  this->aContourMapper->SetColorModeToMapScalars();
  this->aContourMapper->ScalarVisibilityOn();
  this->actorAContour = vtkSmartPointer<vtkActor>::New();
  this->actorAContour->SetMapper(this->aContourMapper);
  ren->AddVolume(this->actorAContour);
  this->bContour = vtkSmartPointer<vtkContourFilter>::New();
  this->bContour->ComputeScalarsOn();
  this->bContourMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->bContourMapper->SetInputConnection(this->bContour->GetOutputPort());
  this->bContourMapper->SetColorModeToMapScalars();
  this->bContourMapper->ScalarVisibilityOn();
  this->actorBContour = vtkSmartPointer<vtkActor>::New();
  this->actorBContour->SetMapper(this->bContourMapper);
  ren->AddVolume(this->actorBContour);
  this->cContour = vtkSmartPointer<vtkContourFilter>::New();
  this->cContour->ComputeScalarsOn();
  this->cContourMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->cContourMapper->SetInputConnection(this->cContour->GetOutputPort());
  this->cContourMapper->SetColorModeToMapScalars();
  this->cContourMapper->ScalarVisibilityOn();
  this->actorCContour = vtkSmartPointer<vtkActor>::New();
  this->actorCContour->SetMapper(this->cContourMapper);
  ren->AddVolume(this->actorCContour);
}

//----------------------------------------------------------------------------
MooseViewer::DataItem::~DataItem(void)
{
}

