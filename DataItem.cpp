// VTK includes
#include <ExternalVTKWidget.h>
#include <vtkActor.h>
#include <vtkCheckerboardSplatter.h>
#include <vtkColorTransferFunction.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkCompositePolyDataMapper.h>
#include <vtkLight.h>
#include <vtkLookupTable.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkNew.h>

// MooseViewer includes
#include "DataItem.h"

//----------------------------------------------------------------------------
MooseViewer::DataItem::DataItem(void)
{
  /* Initialize VTK renderwindow and renderer */
  this->externalVTKWidget = vtkSmartPointer<ExternalVTKWidget>::New();
  this->actor = vtkSmartPointer<vtkActor>::New();
  this->actor->GetProperty()->SetEdgeColor(1.0, 1.0, 1.0);
  this->renderer = this->externalVTKWidget->AddRenderer();
  this->renderer->AddActor(this->actor);

  this->compositeFilter = vtkSmartPointer<vtkCompositeDataGeometryFilter>::New();
  this->mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->mapper->SetInputConnection(this->compositeFilter->GetOutputPort());
  this->mapper->SetScalarVisibility(1);
  this->mapper->SetScalarModeToUsePointFieldData();
  this->actor->SetMapper(this->mapper);

  this->actorOutline = vtkSmartPointer<vtkActor>::New();
  this->actorOutline->GetProperty()->SetColor(1.0, 1.0, 1.0);
  this->renderer->AddActor(this->actorOutline);

  this->lut = vtkSmartPointer<vtkLookupTable>::New();
  this->lut->SetNumberOfColors(256);
  this->lut->Build();
  this->mapper->SetLookupTable(this->lut);

  /* Setting alpha bit planes should be done in VRUI while
   * creating the context */
//  this->externalVTKWidget->GetRenderWindow()->SetAlphaBitPlanes(1);
//  this->externalVTKWidget->GetRenderWindow()->SetMultiSamples(0);

  /* Use depth peeling to enable transparency */
  this->renderer->SetUseDepthPeeling(1);
  this->renderer->SetMaximumNumberOfPeels(4);
  this->renderer->SetOcclusionRatio(0.1);

  this->gaussian = vtkSmartPointer<vtkCheckerboardSplatter>::New();
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
  this->renderer->AddVolume(this->actorVolume);

  this->framerate = vtkSmartPointer<vtkTextActor>::New();
  this->framerate->GetTextProperty()->SetJustificationToLeft();
  this->framerate->GetTextProperty()->SetVerticalJustificationToTop();
  this->framerate->GetTextProperty()->SetFontSize(12);
  this->framerate->SetTextScaleModeToViewport();
  vtkCoordinate *fpsCoord = this->framerate->GetPositionCoordinate();
  fpsCoord->SetCoordinateSystemToNormalizedDisplay();
  fpsCoord->SetValue(0, 0.999);
  this->renderer->AddActor2D(this->framerate);
}

//----------------------------------------------------------------------------
MooseViewer::DataItem::~DataItem(void)
{
}

