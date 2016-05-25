#include "mvContours.h"

#include <GL/GLContextData.h>

#include <vtkActor.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkExternalOpenGLRenderer.h>
#include <vtkFlyingEdges3D.h>
#include <vtkLookupTable.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkPolyDataMapper.h>
#include <vtkSMPContourGrid.h>
#include <vtkSpanSpace.h>
#include <vtkUnstructuredGrid.h>

#include "mvApplicationState.h"
#include "vvContextState.h"
#include "mvReader.h"

//------------------------------------------------------------------------------
mvContours::LoResDataPipeline::LoResDataPipeline()
{
  this->contour->ComputeNormalsOff();
  this->contour->ComputeGradientsOff();

  this->geometry->SetInputConnection(this->contour->GetOutputPort());
}

//------------------------------------------------------------------------------
void mvContours::LoResDataPipeline::configure(
    const ObjectState &objState, const vvApplicationState &vvState)
{
  const mvApplicationState &appState =
      static_cast<const mvApplicationState &>(vvState);

  // Only modify the filter if the colorByArray is loaded.
  auto metaData = appState.reader().variableMetaData(appState.colorByArray());
  if (!metaData.valid())
    {
    this->contour->SetInputDataObject(nullptr);
    return;
    }

  this->contour->SetInputDataObject(appState.reader().reducedDataObject());

  // Use the correct array for contouring:
  switch (metaData.location)
    {
    case mvReader::VariableMetaData::Location::CellData:
      this->contour->SetInputArrayToProcess(
            0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS,
            appState.colorByArray().c_str());
      break;

    case mvReader::VariableMetaData::Location::PointData:
      this->contour->SetInputArrayToProcess(
            0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS,
            appState.colorByArray().c_str());
      break;

    case mvReader::VariableMetaData::Location::FieldData:
      this->contour->SetInputArrayToProcess(
            0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_NONE,
            appState.colorByArray().c_str());
      break;

    default:
      break;
    }

  // Set contour values:
  const ContourState &state = static_cast<const ContourState&>(objState);
  double min = metaData.range[0];
  double spread = metaData.range[1] - min;
  this->contour->SetNumberOfContours(state.contourValues.size());
  for (int i = 0; i < state.contourValues.size(); ++i)
    {
    this->contour->SetValue(i, (state.contourValues[i]/255.0) * spread + min);
    }
}

//------------------------------------------------------------------------------
bool mvContours::LoResDataPipeline::needsUpdate(const ObjectState &objState,
                                                const LODData &result) const
{
  const ContourState& state = static_cast<const ContourState&>(objState);
  const LoResLODData& data = static_cast<const LoResLODData&>(result);

  return
      state.visible &&
      this->contour->GetInputDataObject(0, 0) &&
      (!data.contours ||
       data.contours->GetMTime() < this->contour->GetMTime() ||
       data.contours->GetMTime() < this->geometry->GetMTime());
}

//------------------------------------------------------------------------------
void mvContours::LoResDataPipeline::execute()
{
  this->geometry->Update();
}

//------------------------------------------------------------------------------
void mvContours::LoResDataPipeline::exportResult(LODData &result) const
{
  LoResLODData& data = static_cast<LoResLODData&>(result);

  vtkDataObject *newContours = this->geometry->GetOutputDataObject(0);
  data.contours.TakeReference(newContours->NewInstance());
  data.contours->ShallowCopy(newContours);
}

//------------------------------------------------------------------------------
mvContours::LoResRenderPipeline::LoResRenderPipeline()
{
  this->mapper->SetScalarVisibility(1);
  this->mapper->SetScalarModeToDefault();
  this->mapper->SetColorModeToMapScalars();
  this->mapper->UseLookupTableScalarRangeOn();

  this->actor->SetMapper(this->mapper.GetPointer());
}

//------------------------------------------------------------------------------
void mvContours::LoResRenderPipeline::init(const ObjectState &objState,
                                           vvContextState &contextState)
{
  contextState.renderer().AddActor(this->actor.Get());
}

//------------------------------------------------------------------------------
void mvContours::LoResRenderPipeline::update(const ObjectState &objState,
                                             const vvApplicationState &vvState,
                                             const vvContextState &contextState,
                                             const LODData &result)
{
  const mvApplicationState &appState =
      static_cast<const mvApplicationState &>(vvState);

  const ContourState& state = static_cast<const ContourState&>(objState);
  const LoResLODData& data = static_cast<const LoResLODData&>(result);

  // Only update state if the color array exists.
  auto metaData = appState.reader().variableMetaData(appState.colorByArray());
  if (!metaData.valid() || !state.visible || !data.contours)
    {
    this->disable();
    return;
    }

  this->mapper->SetInputDataObject(data.contours.Get());
  this->mapper->SetLookupTable(&appState.colorMap());
  this->mapper->SelectColorArray(appState.colorByArray().c_str());

  // Point the mapper at the proper scalar array.
  switch (metaData.location)
    {
    case mvReader::VariableMetaData::Location::PointData:
      this->mapper->SetScalarModeToUsePointFieldData();
      break;

    case mvReader::VariableMetaData::Location::CellData:
      this->mapper->SetScalarModeToUseCellFieldData();
      break;

    case mvReader::VariableMetaData::Location::FieldData:
      this->mapper->SetScalarModeToUseFieldData();
      break;

    default:
      break;
    }

  this->actor->SetVisibility(1);
}

//------------------------------------------------------------------------------
void mvContours::LoResRenderPipeline::disable()
{
  this->actor->SetVisibility(0);
}

//------------------------------------------------------------------------------
mvContours::HiResDataPipeline::HiResDataPipeline()
{
  this->contour->GenerateTrianglesOn();
  this->contour->ComputeScalarsOn();

  // These cause artifacts with the SMPContourGrid filter. Reported as VTK
  // bug 15969.
  this->contour->MergePiecesOff();
  this->contour->UseScalarTreeOff();

  this->geometry->SetInputConnection(this->contour->GetOutputPort());
}

//------------------------------------------------------------------------------
void mvContours::HiResDataPipeline::configure(
    const ObjectState &objState, const vvApplicationState &vvState)
{
  const mvApplicationState &appState =
      static_cast<const mvApplicationState &>(vvState);

  // Only modify the filter if the colorByArray is loaded.
  auto metaData = appState.reader().variableMetaData(appState.colorByArray());
  if (!metaData.valid())
    {
    this->contour->SetInputDataObject(nullptr);
    return;
    }

  this->contour->SetInputDataObject(appState.reader().dataObject());

  // Use the correct array for contouring:
  switch (metaData.location)
    {
    case mvReader::VariableMetaData::Location::CellData:
      this->contour->SetInputArrayToProcess(
            0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS,
            appState.colorByArray().c_str());
      break;

    case mvReader::VariableMetaData::Location::PointData:
      this->contour->SetInputArrayToProcess(
            0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS,
            appState.colorByArray().c_str());
      break;

    case mvReader::VariableMetaData::Location::FieldData:
      this->contour->SetInputArrayToProcess(
            0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_NONE,
            appState.colorByArray().c_str());
      break;

    default:
      break;
    }

  // Set contour values:
  const ContourState &state = static_cast<const ContourState&>(objState);
  double min = metaData.range[0];
  double spread = metaData.range[1] - min;
  this->contour->SetNumberOfContours(state.contourValues.size());
  for (int i = 0; i < state.contourValues.size(); ++i)
    {
    this->contour->SetValue(i, (state.contourValues[i]/255.0) * spread + min);
    }
}

//------------------------------------------------------------------------------
bool mvContours::HiResDataPipeline::needsUpdate(const ObjectState &objState,
                                                const LODData &result) const
{
  const ContourState& state = static_cast<const ContourState&>(objState);
  const HiResLODData& data = static_cast<const HiResLODData&>(result);

  return
      state.visible &&
      this->contour->GetInputDataObject(0, 0) &&
      (!data.contours ||
       data.contours->GetMTime() < this->contour->GetMTime() ||
       data.contours->GetMTime() < this->geometry->GetMTime());
}

//------------------------------------------------------------------------------
void mvContours::HiResDataPipeline::execute()
{
  this->geometry->Update();
}

//------------------------------------------------------------------------------
void mvContours::HiResDataPipeline::exportResult(LODData &result) const
{
  HiResLODData& data = static_cast<HiResLODData&>(result);

  vtkDataObject *newContours = this->geometry->GetOutputDataObject(0);
  data.contours.TakeReference(newContours->NewInstance());
  data.contours->ShallowCopy(newContours);
}

//------------------------------------------------------------------------------
mvContours::HiResRenderPipeline::HiResRenderPipeline()
{
  this->mapper->SetScalarVisibility(1);
  this->mapper->SetScalarModeToDefault();
  this->mapper->SetColorModeToMapScalars();
  this->mapper->UseLookupTableScalarRangeOn();

  this->actor->SetMapper(this->mapper.GetPointer());
}

//------------------------------------------------------------------------------
void mvContours::HiResRenderPipeline::init(const ObjectState &objState,
                                           vvContextState &contextState)
{
  contextState.renderer().AddActor(this->actor.Get());
}

//------------------------------------------------------------------------------
void mvContours::HiResRenderPipeline::update(const ObjectState &objState,
                                             const vvApplicationState &vvState,
                                             const vvContextState &contextState,
                                             const LODData &result)
{
  const mvApplicationState &appState =
      static_cast<const mvApplicationState &>(vvState);

  const ContourState& state = static_cast<const ContourState&>(objState);
  const LoResLODData& data = static_cast<const LoResLODData&>(result);

  // Only update state if the color array exists.
  auto metaData = appState.reader().variableMetaData(appState.colorByArray());
  if (!metaData.valid() || !state.visible || !data.contours)
    {
    this->disable();
    return;
    }

  this->mapper->SetInputDataObject(data.contours.Get());
  this->mapper->SetLookupTable(&appState.colorMap());
  this->mapper->SelectColorArray(appState.colorByArray().c_str());

  // Point the mapper at the proper scalar array.
  switch (metaData.location)
    {
    case mvReader::VariableMetaData::Location::PointData:
      this->mapper->SetScalarModeToUsePointFieldData();
      break;

    case mvReader::VariableMetaData::Location::CellData:
      this->mapper->SetScalarModeToUseCellFieldData();
      break;

    case mvReader::VariableMetaData::Location::FieldData:
      this->mapper->SetScalarModeToUseFieldData();
      break;

    default:
      break;
    }

  this->actor->SetVisibility(1);
}

//------------------------------------------------------------------------------
void mvContours::HiResRenderPipeline::disable()
{
  this->actor->SetVisibility(0);
}

//------------------------------------------------------------------------------
mvContours::mvContours()
{
}

//------------------------------------------------------------------------------
mvContours::~mvContours()
{
}

//------------------------------------------------------------------------------
vvLODAsyncGLObject::ObjectState *mvContours::createObjectState() const
{
  return new ContourState;
}

//------------------------------------------------------------------------------
vvLODAsyncGLObject::DataPipeline *
mvContours::createDataPipeline(LevelOfDetail lod) const
{
  switch (lod)
    {
    case LevelOfDetail::Hint:
      return nullptr;

    case LevelOfDetail::LoRes:
      return new LoResDataPipeline;

    case LevelOfDetail::HiRes:
      return new HiResDataPipeline;

    default:
      return nullptr;
    }
}

//------------------------------------------------------------------------------
vvLODAsyncGLObject::RenderPipeline *
mvContours::createRenderPipeline(LevelOfDetail lod) const
{
  switch (lod)
    {
    case LevelOfDetail::Hint:
      return nullptr;

    case LevelOfDetail::LoRes:
      return new LoResRenderPipeline;

    case LevelOfDetail::HiRes:
      return new HiResRenderPipeline;

    default:
      return nullptr;
    }
}

//------------------------------------------------------------------------------
vvLODAsyncGLObject::LODData* mvContours::createLODData(LevelOfDetail lod) const
{
  switch (lod)
    {
    case LevelOfDetail::Hint:
      return nullptr;

    case LevelOfDetail::LoRes:
      return new LoResLODData;

    case LevelOfDetail::HiRes:
      return new HiResLODData;

    default:
      return nullptr;
    }
}
