#include "mvSlice.h"

#include <GL/GLContextData.h>

#include <Geometry/Rotation.h>

#include <vtkActor.h>
#include <vtkCompositePolyDataMapper.h>
#include <vtkCutter.h>
#include <vtkExternalOpenGLRenderer.h>
#include <vtkFlyingEdgesPlaneCutter.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkSampleImplicitFunctionFilter.h>
#include <vtkSMPContourGrid.h>

#include "mvApplicationState.h"
#include "mvContextState.h"
#include "mvInteractor.h"
#include "mvReader.h"

#include <algorithm>
#include <iostream>

//------------------------------------------------------------------------------
void mvSlice::SliceState::update(const mvApplicationState &appState)
{
  if (!this->visible || !appState.interactor().isInteracting())
    {
    return;
    }

  switch (appState.interactor().state())
    {
    case mvInteractor::NoInteraction:
      break;

    case mvInteractor::Translating:
      {
      const Vrui::Vector &v = appState.interactor().current().getTranslation();
      this->plane.origin[0] = v[0];
      this->plane.origin[1] = v[1];
      this->plane.origin[2] = v[2];
      }
      break;

    case mvInteractor::Rotating:
      {
      const Vrui::Rotation &rot = appState.interactor().delta().getRotation();
      Vrui::Vector n(this->plane.normal.data());
      n = rot.transform(n);
      std::copy(n.getComponents(), n.getComponents() + 3,
                this->plane.normal.begin());
      }
      break;

    default:
      std::cerr << "Unknown interaction state: "
                << appState.interactor().state() << "\n";
      break;
    }
}

//------------------------------------------------------------------------------
mvSlice::HintDataPipeline::HintDataPipeline()
{
  this->cutter->SetInputData(this->box.Get());
  this->cutter->SetCutFunction(this->plane.Get());
  this->cutter->GenerateTrianglesOn();
  this->cutter->GenerateCutScalarsOff();
  this->cutter->SetNumberOfContours(1);
  this->cutter->SetValue(0, 0.);
}

//------------------------------------------------------------------------------
void mvSlice::HintDataPipeline::configure(const ObjectState &objState,
                                          const mvApplicationState &appState)
{
  const SliceState& sliceState = static_cast<const SliceState&>(objState);

  // Cut a plane from a simple image data:
  this->box->SetExtent(0, 1, 0, 1, 0, 1);
  this->box->SetOrigin(const_cast<double*>(
                             appState.reader().bounds().GetMinPoint()));
  this->box->SetSpacing(appState.reader().bounds().GetLength(0),
                        appState.reader().bounds().GetLength(1),
                        appState.reader().bounds().GetLength(2));

  // Setup the plane:
  this->plane->SetNormal(const_cast<double*>(sliceState.plane.normal.data()));
  this->plane->SetOrigin(const_cast<double*>(sliceState.plane.origin.data()));
}

//------------------------------------------------------------------------------
bool mvSlice::HintDataPipeline::needsUpdate(const ObjectState &objState,
                                            const LODData &result) const
{
  const SliceState& sliceState = static_cast<const SliceState&>(objState);
  const HintLODData& data = static_cast<const HintLODData&>(result);

  return
      sliceState.visible &&
      (!data.slice.Get() ||
       data.slice->GetMTime() < this->box->GetMTime() ||
       data.slice->GetMTime() < this->plane->GetMTime() ||
       data.slice->GetMTime() < this->cutter->GetMTime());
}

//------------------------------------------------------------------------------
void mvSlice::HintDataPipeline::execute()
{
  this->cutter->Update();
}

//------------------------------------------------------------------------------
void mvSlice::HintDataPipeline::exportResult(LODData &result) const
{
  HintLODData& data = static_cast<HintLODData&>(result);

  vtkDataObject *newSlice = this->cutter->GetOutputDataObject(0);
  data.slice.TakeReference(newSlice->NewInstance());
  data.slice->ShallowCopy(newSlice);
}

//------------------------------------------------------------------------------
mvSlice::HintRenderPipeline::HintRenderPipeline()
{
  this->mapper->ScalarVisibilityOff();

  this->actor->SetMapper(this->mapper.Get());
  this->actor->GetProperty()->SetColor(1., 1., 1.);
  this->actor->GetProperty()->SetOpacity(0.25);
}

//------------------------------------------------------------------------------
void mvSlice::HintRenderPipeline::init(const ObjectState &,
                                       mvContextState &contextState)
{
  contextState.renderer().AddActor(this->actor.Get());
}

//------------------------------------------------------------------------------
void mvSlice::HintRenderPipeline::update(const ObjectState &objState,
                                         const mvApplicationState &,
                                         const mvContextState &,
                                         const LODData &result)
{
  const SliceState& sliceState = static_cast<const SliceState&>(objState);
  const HintLODData& data = static_cast<const HintLODData&>(result);

  if (!sliceState.visible || !data.slice.Get())
    {
    this->disable();
    return;
    }

  this->mapper->SetInputDataObject(data.slice.Get());
  this->actor->VisibilityOn();
}

//------------------------------------------------------------------------------
void mvSlice::HintRenderPipeline::disable()
{
  this->actor->VisibilityOff();
}

//------------------------------------------------------------------------------
mvSlice::LoResDataPipeline::LoResDataPipeline()
{
  this->cutter->SetPlane(this->plane.Get());
  this->cutter->ComputeNormalsOff();
  this->cutter->InterpolateAttributesOn();
}

//------------------------------------------------------------------------------
void mvSlice::LoResDataPipeline::configure(const ObjectState &objState,
                                           const mvApplicationState &appState)
{
  const SliceState& sliceState = static_cast<const SliceState&>(objState);

  // Only support point scalars:
  auto metaData = appState.reader().variableMetaData(appState.colorByArray());
  if (!metaData.valid() ||
      metaData.location != mvReader::VariableMetaData::Location::PointData)
    {
    this->cutter->SetInputDataObject(nullptr);
    return;
    }

  // Use the reduced dataset:
  this->cutter->SetInputDataObject(appState.reader().reducedDataObject());
  this->cutter->SetInputArrayToProcess(0, 0, 0,
                                       vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                       appState.colorByArray().c_str());


  // Casts are for VTK (not const-correct)
  this->plane->SetNormal(const_cast<double*>(sliceState.plane.normal.data()));
  this->plane->SetOrigin(const_cast<double*>(sliceState.plane.origin.data()));
}

//------------------------------------------------------------------------------
bool mvSlice::LoResDataPipeline::needsUpdate(const ObjectState &objState,
                                             const LODData &result) const
{
  const SliceState& sliceState = static_cast<const SliceState&>(objState);
  const LoResLODData& data = static_cast<const LoResLODData&>(result);

  return
      sliceState.visible &&
      this->cutter->GetInputDataObject(0, 0) &&
      (!data.slice ||
       data.slice->GetMTime() < this->plane->GetMTime() ||
       data.slice->GetMTime() < this->cutter->GetMTime());
}

//------------------------------------------------------------------------------
void mvSlice::LoResDataPipeline::execute()
{
  this->cutter->Update();
}

//------------------------------------------------------------------------------
void mvSlice::LoResDataPipeline::exportResult(LODData &result) const
{
  LoResLODData& data = static_cast<LoResLODData&>(result);

  vtkDataObject *newSlice = this->cutter->GetOutputDataObject(0);
  data.slice.TakeReference(newSlice->NewInstance());
  data.slice->ShallowCopy(newSlice);
}

//------------------------------------------------------------------------------
mvSlice::LoResRenderPipeline::LoResRenderPipeline()
{
  this->mapper->SetScalarVisibility(1);
  this->mapper->SetScalarModeToDefault();
  this->mapper->SetColorModeToMapScalars();
  this->mapper->UseLookupTableScalarRangeOn();

  this->actor->SetMapper(this->mapper.Get());
}

//------------------------------------------------------------------------------
void mvSlice::LoResRenderPipeline::init(const ObjectState &objState,
                                        mvContextState &contextState)
{
  contextState.renderer().AddActor(this->actor.Get());
}

//------------------------------------------------------------------------------
void mvSlice::LoResRenderPipeline::update(const ObjectState &objState,
                                          const mvApplicationState &appState,
                                          const mvContextState &contextState,
                                          const LODData &result)
{
  const SliceState& sliceState = static_cast<const SliceState&>(objState);
  const LoResLODData& data = static_cast<const LoResLODData&>(result);

  if (!sliceState.visible || !data.slice.Get())
    {
    this->disable();
    return;
    }

  this->mapper->SetInputDataObject(data.slice.Get());

  auto metaData = appState.reader().variableMetaData(appState.colorByArray());
  if (metaData.valid())
    {
    this->mapper->SetLookupTable(&appState.colorMap());

    // Point the mapper at the proper scalar array.
    switch (metaData.location)
      {
      case mvReader::VariableMetaData::Location::PointData:
        this->mapper->SetScalarModeToUsePointFieldData();
        this->mapper->SelectColorArray(appState.colorByArray().c_str());
        break;

      case mvReader::VariableMetaData::Location::CellData:
        this->mapper->SetScalarModeToUseCellFieldData();
        this->mapper->SelectColorArray(appState.colorByArray().c_str());
        break;

      case mvReader::VariableMetaData::Location::FieldData:
        this->mapper->SetScalarModeToUseFieldData();
        this->mapper->SelectColorArray(appState.colorByArray().c_str());
        break;

      default:
        break;
      }
    }

  this->actor->VisibilityOn();
}

//------------------------------------------------------------------------------
void mvSlice::LoResRenderPipeline::disable()
{
  this->actor->VisibilityOff();
}

//------------------------------------------------------------------------------
mvSlice::HiResDataPipeline::HiResDataPipeline()
{
  this->addPlane->SetImplicitFunction(this->plane.Get());
  this->addPlane->ComputeGradientsOff();
  this->addPlane->SetScalarArrayName("mvSlice Plane");

  this->cutter->SetInputConnection(this->addPlane->GetOutputPort());
  this->cutter->GenerateTrianglesOn();
  this->cutter->ComputeScalarsOn();
  this->cutter->SetNumberOfContours(1);
  this->cutter->SetValue(0, 0.);

  // BUG: These options cause artifacts with the SMP filter. Reported as VTK
  // bug 15969.
//  this->contour->SetScalarTree(m_scalarTree.GetPointer());
//  this->contour->UseScalarTreeOn();
//  this->contour->MergePiecesOn();
  this->cutter->MergePiecesOff();
}

//------------------------------------------------------------------------------
void mvSlice::HiResDataPipeline::configure(const ObjectState &objState,
                                           const mvApplicationState &appState)
{
  const SliceState& sliceState = static_cast<const SliceState&>(objState);

  this->addPlane->SetInputDataObject(appState.reader().dataObject());

  this->cutter->SetInputArrayToProcess(0, 0, 0,
                                       vtkDataObject::FIELD_ASSOCIATION_POINTS,
                                       "mvSlice Plane");

  // Casts are for VTK (not const-correct)
  this->plane->SetNormal(const_cast<double*>(sliceState.plane.normal.data()));
  this->plane->SetOrigin(const_cast<double*>(sliceState.plane.origin.data()));
}

//------------------------------------------------------------------------------
bool mvSlice::HiResDataPipeline::needsUpdate(const ObjectState &objState,
                                             const LODData &result) const
{
  const SliceState& sliceState = static_cast<const SliceState&>(objState);
  const HiResLODData& data = static_cast<const HiResLODData&>(result);

  return
      sliceState.visible &&
      this->addPlane->GetInputDataObject(0, 0) &&
      (!data.slice ||
       data.slice->GetMTime() < this->plane->GetMTime() ||
       data.slice->GetMTime() < this->addPlane->GetMTime() ||
       data.slice->GetMTime() < this->cutter->GetMTime());
}

//------------------------------------------------------------------------------
void mvSlice::HiResDataPipeline::execute()
{
  this->cutter->Update();
}

//------------------------------------------------------------------------------
void mvSlice::HiResDataPipeline::exportResult(LODData &result) const
{
  HiResLODData& data = static_cast<HiResLODData&>(result);

  vtkDataObject *newSlice = this->cutter->GetOutputDataObject(0);
  data.slice.TakeReference(newSlice->NewInstance());
  data.slice->ShallowCopy(newSlice);
}

//------------------------------------------------------------------------------
mvSlice::HiResRenderPipeline::HiResRenderPipeline()
{
  this->mapper->SetScalarVisibility(1);
  this->mapper->SetScalarModeToDefault();
  this->mapper->SetColorModeToMapScalars();
  this->mapper->UseLookupTableScalarRangeOn();

  this->actor->SetMapper(this->mapper.Get());
}

//------------------------------------------------------------------------------
void mvSlice::HiResRenderPipeline::init(const ObjectState &objState,
                                        mvContextState &contextState)
{
  contextState.renderer().AddActor(this->actor.Get());
}

//------------------------------------------------------------------------------
void mvSlice::HiResRenderPipeline::update(const ObjectState &objState,
                                          const mvApplicationState &appState,
                                          const mvContextState &contextState,
                                          const LODData &result)
{
  const SliceState& sliceState = static_cast<const SliceState&>(objState);
  const HiResLODData& data = static_cast<const HiResLODData&>(result);

  if (!sliceState.visible || !data.slice.Get())
    {
    this->disable();
    return;
    }

  this->mapper->SetInputDataObject(data.slice.Get());

  auto metaData = appState.reader().variableMetaData(appState.colorByArray());
  if (metaData.valid())
    {
    this->mapper->SetLookupTable(&appState.colorMap());

    // Point the mapper at the proper scalar array.
    switch (metaData.location)
      {
      case mvReader::VariableMetaData::Location::PointData:
        this->mapper->SetScalarModeToUsePointFieldData();
        this->mapper->SelectColorArray(appState.colorByArray().c_str());
        break;

      case mvReader::VariableMetaData::Location::CellData:
        this->mapper->SetScalarModeToUseCellFieldData();
        this->mapper->SelectColorArray(appState.colorByArray().c_str());
        break;

      case mvReader::VariableMetaData::Location::FieldData:
        this->mapper->SetScalarModeToUseFieldData();
        this->mapper->SelectColorArray(appState.colorByArray().c_str());
        break;

      default:
        break;
      }
    }

  this->actor->VisibilityOn();
}

//------------------------------------------------------------------------------
void mvSlice::HiResRenderPipeline::disable()
{
  this->actor->VisibilityOff();
}

//------------------------------------------------------------------------------
mvSlice::mvSlice()
{
}

//------------------------------------------------------------------------------
mvSlice::~mvSlice()
{
}

//------------------------------------------------------------------------------
mvLODAsyncGLObject::ObjectState *mvSlice::createObjectState() const
{
  return new SliceState;
}

//------------------------------------------------------------------------------
mvLODAsyncGLObject::DataPipeline*
mvSlice::createDataPipeline(LevelOfDetail lod) const
{
  switch (lod)
    {
    case LevelOfDetail::Hint:
      return new HintDataPipeline;

    case LevelOfDetail::LoRes:
      return new LoResDataPipeline;

    case LevelOfDetail::HiRes:
      return new HiResDataPipeline;

    default:
      return nullptr;
    }
}

//------------------------------------------------------------------------------
mvLODAsyncGLObject::RenderPipeline*
mvSlice::createRenderPipeline(LevelOfDetail lod) const
{
  switch (lod)
    {
    case LevelOfDetail::Hint:
      return new HintRenderPipeline;

    case LevelOfDetail::LoRes:
      return new LoResRenderPipeline;

    case LevelOfDetail::HiRes:
      return new HiResRenderPipeline;

    default:
      return nullptr;
    }
}

//------------------------------------------------------------------------------
mvLODAsyncGLObject::LODData*
mvSlice::createLODData(LevelOfDetail lod) const
{
  switch (lod)
    {
    case LevelOfDetail::Hint:
      return new HintLODData;

    case LevelOfDetail::LoRes:
      return new LoResLODData;

    case LevelOfDetail::HiRes:
      return new HiResLODData;

    default:
      return nullptr;
    }
}
