#include "mvGeometry.h"

#include <GL/GLContextData.h>

#include <vtkActor.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkExodusIIReader.h>
#include <vtkExternalOpenGLRenderer.h>
#include <vtkLookupTable.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

#include "mvApplicationState.h"
#include "mvContextState.h"
#include "mvReader.h"


//------------------------------------------------------------------------------
vtkDataObject *
mvGeometry::LoResDataPipeline::input(const mvApplicationState &state) const
{
  return state.reader().reducedDataObject();
}

//------------------------------------------------------------------------------
void mvGeometry::LoResDataPipeline::configure(
    const ObjectState &, const mvApplicationState &appState)
{
  this->filter->SetInputDataObject(this->input(appState));
}

//------------------------------------------------------------------------------
bool mvGeometry::LoResDataPipeline::needsUpdate(const ObjectState &objState,
                                                const LODData &result) const
{
  const GeometryState &state = static_cast<const GeometryState&>(objState);
  const GeometryLODData &data = static_cast<const GeometryLODData&>(result);

  return
      state.visible &&
      state.representation != Representation::NoGeometry &&
      this->filter->GetInputDataObject(0, 0) &&
      (!data.geometry ||
       data.geometry->GetMTime() < this->filter->GetMTime());
}

//------------------------------------------------------------------------------
void mvGeometry::LoResDataPipeline::execute()
{
  this->filter->Update();
}

//------------------------------------------------------------------------------
void mvGeometry::LoResDataPipeline::exportResult(LODData &result) const
{
  GeometryLODData &data = static_cast<GeometryLODData&>(result);

  vtkDataObject *dObj = this->filter->GetOutputDataObject(0);
  data.geometry.TakeReference(dObj->NewInstance());
  data.geometry->ShallowCopy(dObj);
}

//------------------------------------------------------------------------------
vtkDataObject *
mvGeometry::HiResDataPipeline::input(const mvApplicationState &state) const
{
  return state.reader().dataObject();
}

//------------------------------------------------------------------------------
void mvGeometry::GeometryRenderPipeline::init(const ObjectState &,
                                              mvContextState &contextState)
{
  this->mapper->SetScalarVisibility(1);
  this->actor->GetProperty()->SetEdgeColor(1., 1., 1.);
  this->actor->SetMapper(this->mapper.Get());

  contextState.renderer().AddActor(this->actor.Get());
}

//------------------------------------------------------------------------------
void mvGeometry::GeometryRenderPipeline::update(
    const ObjectState &objState, const mvApplicationState &appState,
    const mvContextState &contextState, const LODData &result)
{
  const GeometryState &state = static_cast<const GeometryState&>(objState);
  const GeometryLODData &data = static_cast<const GeometryLODData&>(result);

  if (!state.visible ||
      state.representation == Representation::NoGeometry ||
      !data.geometry)
    {
    this->disable();
    return;
    }

  this->mapper->SetInputDataObject(data.geometry.Get());

  auto metaData = appState.reader().variableMetaData(appState.colorByArray());
  if (metaData.valid())
    {
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

    this->mapper->SelectColorArray(appState.colorByArray().c_str());
    this->mapper->SetScalarRange(metaData.range);
    this->mapper->SetLookupTable(&appState.colorMap());
    }

  switch (state.representation)
    {
    case mvGeometry::NoGeometry:
      // Shouldn't happen, we check for this earlier.
      break;
    case mvGeometry::Points:
      this->actor->GetProperty()->SetRepresentation(VTK_POINTS);
      this->actor->GetProperty()->EdgeVisibilityOff();
      break;
    case mvGeometry::Wireframe:
      this->actor->GetProperty()->SetRepresentation(VTK_WIREFRAME);
      this->actor->GetProperty()->EdgeVisibilityOff();
      break;
    case mvGeometry::Surface:
      this->actor->GetProperty()->SetRepresentation(VTK_SURFACE);
      this->actor->GetProperty()->EdgeVisibilityOff();
      break;
    case mvGeometry::SurfaceWithEdges:
      this->actor->GetProperty()->SetRepresentation(VTK_SURFACE);
      this->actor->GetProperty()->EdgeVisibilityOn();
      break;
    }

  this->actor->GetProperty()->SetOpacity(state.opacity);
  this->actor->SetVisibility(1);
}

//------------------------------------------------------------------------------
void mvGeometry::GeometryRenderPipeline::disable()
{
  this->actor->SetVisibility(0);
}

//------------------------------------------------------------------------------
mvGeometry::mvGeometry()
{
}

//------------------------------------------------------------------------------
mvGeometry::~mvGeometry()
{
}

//------------------------------------------------------------------------------
double mvGeometry::opacity() const
{
  return this->objectState<GeometryState>().opacity;
}

//------------------------------------------------------------------------------
void mvGeometry::setOpacity(double o)
{
  this->objectState<GeometryState>().opacity = o;
}

//------------------------------------------------------------------------------
mvGeometry::Representation mvGeometry::representation() const
{
  return this->objectState<GeometryState>().representation;
}

//------------------------------------------------------------------------------
void mvGeometry::setRepresentation(Representation repr)
{
  this->objectState<GeometryState>().representation = repr;
}

//------------------------------------------------------------------------------
mvLODAsyncGLObject::ObjectState *mvGeometry::createObjectState() const
{
  return new GeometryState;
}

//------------------------------------------------------------------------------
mvLODAsyncGLObject::DataPipeline *
mvGeometry::createDataPipeline(LevelOfDetail lod) const
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
mvLODAsyncGLObject::RenderPipeline *
mvGeometry::createRenderPipeline(LevelOfDetail lod) const
{
  switch (lod)
    {
    case LevelOfDetail::Hint:
      return nullptr;

    case LevelOfDetail::LoRes:
    case LevelOfDetail::HiRes:
      return new GeometryRenderPipeline;

    default:
      return nullptr;
    }
}

//------------------------------------------------------------------------------
mvLODAsyncGLObject::LODData *
mvGeometry::createLODData(LevelOfDetail lod) const
{
  switch (lod)
    {
    case LevelOfDetail::Hint:
      return nullptr;

    case LevelOfDetail::LoRes:
    case LevelOfDetail::HiRes:
      return new GeometryLODData;

    default:
      return nullptr;
    }
}
