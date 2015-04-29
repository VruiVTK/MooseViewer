// VTK includes
#include <ExternalVTKWidget.h>
#include <vtkActor.h>
#include <vtkAppendPolyData.h>
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
#include <vtkSMPContourGrid.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <vtkSpanSpace.h>
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
  vtkRenderer* ren = this->externalVTKWidget->AddRenderer();
  ren->AddActor(this->actor);

  this->compositeFilter = vtkSmartPointer<vtkCompositeDataGeometryFilter>::New();
  this->mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  this->mapper->SetInputConnection(this->compositeFilter->GetOutputPort());
  this->mapper->SetScalarVisibility(1);
  this->mapper->SetScalarModeToUsePointFieldData();
  this->actor->SetMapper(this->mapper);

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
  this->contourMapper = vtkSmartPointer<vtkCompositePolyDataMapper>::New();
  this->contourMapper->SetScalarVisibility(1);
  this->contourMapper->SetScalarModeToUsePointFieldData();
  this->contourMapper->SetColorModeToMapScalars();
  this->contourMapper->SetLookupTable(this->lut);
  this->contourActor = vtkSmartPointer<vtkActor>::New();
  this->contourActor->SetMapper(this->contourMapper);
  ren->AddActor(this->contourActor);

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
  ren->AddVolume(this->actorVolume);

  vtkNew<vtkSpanSpace> aSpanTree;
  aSpanTree->SetResolution(100);
  vtkNew<vtkSpanSpace> bSpanTree;
  bSpanTree->SetResolution(100);
  vtkNew<vtkSpanSpace> cSpanTree;
  cSpanTree->SetResolution(100);
  this->aContour = vtkSmartPointer<vtkSMPContourGrid>::New();
  this->aContour->ComputeScalarsOn();
  this->aContour->UseScalarTreeOn();
  this->aContour->GenerateTrianglesOff();
  this->aContour->MergePiecesOff();
  this->aContour->SetScalarTree(aSpanTree.GetPointer());
  this->aContourMapper = vtkSmartPointer<vtkCompositePolyDataMapper>::New();
  this->aContourMapper->SetInputConnection(this->aContour->GetOutputPort());
  this->aContourMapper->ScalarVisibilityOn();
  this->aContourMapper->SetColorModeToMapScalars();
  this->aContourMapper->SetScalarModeToUsePointData();
  this->actorAContour = vtkSmartPointer<vtkActor>::New();
  this->actorAContour->SetMapper(this->aContourMapper);
  ren->AddVolume(this->actorAContour);
  this->bContour = vtkSmartPointer<vtkSMPContourGrid>::New();
  this->bContour->ComputeScalarsOn();
  this->bContour->UseScalarTreeOn();
  this->bContour->GenerateTrianglesOff();
  this->bContour->MergePiecesOff();
  this->bContour->SetScalarTree(bSpanTree.GetPointer());
  this->bContourMapper = vtkSmartPointer<vtkCompositePolyDataMapper>::New();
  this->bContourMapper->SetInputConnection(this->bContour->GetOutputPort());
  this->bContourMapper->ScalarVisibilityOn();
  this->bContourMapper->SetColorModeToMapScalars();
  this->bContourMapper->SetScalarModeToUsePointData();
  this->actorBContour = vtkSmartPointer<vtkActor>::New();
  this->actorBContour->SetMapper(this->bContourMapper);
  ren->AddVolume(this->actorBContour);
  this->cContour = vtkSmartPointer<vtkSMPContourGrid>::New();
  this->cContour->ComputeScalarsOn();
  this->cContour->UseScalarTreeOn();
  this->cContour->GenerateTrianglesOff();
  this->cContour->MergePiecesOff();
  this->cContour->SetScalarTree(cSpanTree.GetPointer());
  this->cContourMapper = vtkSmartPointer<vtkCompositePolyDataMapper>::New();
  this->cContourMapper->SetInputConnection(this->cContour->GetOutputPort());
  this->cContourMapper->ScalarVisibilityOn();
  this->cContourMapper->SetColorModeToMapScalars();
  this->cContourMapper->SetScalarModeToUsePointData();
  this->actorCContour = vtkSmartPointer<vtkActor>::New();
  this->actorCContour->SetMapper(this->cContourMapper);
  ren->AddVolume(this->actorCContour);

  this->framerate = vtkSmartPointer<vtkTextActor>::New();
  this->framerate->GetTextProperty()->SetJustificationToLeft();
  this->framerate->GetTextProperty()->SetVerticalJustificationToTop();
  this->framerate->GetTextProperty()->SetFontSize(12);
  this->framerate->SetTextScaleModeToViewport();
  vtkCoordinate *fpsCoord = this->framerate->GetPositionCoordinate();
  fpsCoord->SetCoordinateSystemToNormalizedDisplay();
  fpsCoord->SetValue(0, 0.999);
  ren->AddActor2D(this->framerate);
}

//----------------------------------------------------------------------------
MooseViewer::DataItem::~DataItem(void)
{
}

