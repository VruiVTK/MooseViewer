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
mvGeometry::DataItem::DataItem()
{
  this->mapper->SetScalarVisibility(1);
  this->actor->GetProperty()->SetEdgeColor(1., 1., 1.);
  this->actor->SetMapper(this->mapper.Get());
}

//------------------------------------------------------------------------------
mvGeometry::mvGeometry()
  : m_opacity(1.0),
    m_representation(Surface),
    m_visible(true)
{
}

//------------------------------------------------------------------------------
mvGeometry::~mvGeometry()
{
}

//------------------------------------------------------------------------------
void mvGeometry::initMvContext(mvContextState &mvContext,
                               GLContextData &contextData) const
{
  this->Superclass::initMvContext(mvContext, contextData);

  assert("Duplicate context initialization detected!" &&
         !contextData.retrieveDataItem<DataItem>(this));

  DataItem *dataItem = new DataItem;
  contextData.addDataItem(this, dataItem);

  mvContext.renderer().AddActor(dataItem->actor.GetPointer());
}

//------------------------------------------------------------------------------
void mvGeometry::configureDataPipeline(const mvApplicationState &state)
{
  m_filter->SetInputDataObject(state.reader().dataObject());
}

//------------------------------------------------------------------------------
bool mvGeometry::dataPipelineNeedsUpdate() const
{
  return
      m_filter->GetInputDataObject(0, 0) &&
      m_visible &&
      m_representation != NoGeometry &&
      (!m_appData ||
       m_appData->GetMTime() < m_filter->GetMTime());
}

//------------------------------------------------------------------------------
void mvGeometry::executeDataPipeline() const
{
  m_filter->Update();
}

//------------------------------------------------------------------------------
void mvGeometry::retrieveDataPipelineResult()
{
  vtkDataObject *dObj = m_filter->GetOutputDataObject(0);
  m_appData.TakeReference(dObj->NewInstance());
  m_appData->ShallowCopy(dObj);
}

//------------------------------------------------------------------------------
void mvGeometry::syncContextState(const mvApplicationState &appState,
                                  const mvContextState &contextState,
                                  GLContextData &contextData) const
{
  this->Superclass::syncContextState(appState, contextState, contextData);

  DataItem *dataItem = contextData.retrieveDataItem<DataItem>(this);
  assert(dataItem);

  dataItem->mapper->SetInputDataObject(m_appData);

  // Only modify the filter if the colorByArray is loaded.
  auto metaData = appState.reader().variableMetaData(appState.colorByArray());
  if (metaData.valid())
    {
    switch (metaData.location)
    {
    case mvReader::VariableMetaData::Location::PointData:
      dataItem->mapper->SetScalarModeToUsePointFieldData();
      break;

    case mvReader::VariableMetaData::Location::CellData:
      dataItem->mapper->SetScalarModeToUseCellFieldData();
      break;

    case mvReader::VariableMetaData::Location::FieldData:
      dataItem->mapper->SetScalarModeToUseFieldData();
      break;

    default:
      break;
    }

    dataItem->mapper->SelectColorArray(appState.colorByArray().c_str());
    dataItem->mapper->SetScalarRange(metaData.range);
    dataItem->mapper->SetLookupTable(&appState.colorMap());
    }

  bool vis = m_visible; // repr == NoGeometry overrides this.
  switch (m_representation)
    {
    case mvGeometry::NoGeometry:
      vis = false;
      break;
    case mvGeometry::Points:
      dataItem->actor->GetProperty()->SetRepresentation(VTK_POINTS);
      dataItem->actor->GetProperty()->EdgeVisibilityOff();
      break;
    case mvGeometry::Wireframe:
      dataItem->actor->GetProperty()->SetRepresentation(VTK_WIREFRAME);
      dataItem->actor->GetProperty()->EdgeVisibilityOff();
      break;
    case mvGeometry::Surface:
      dataItem->actor->GetProperty()->SetRepresentation(VTK_SURFACE);
      dataItem->actor->GetProperty()->EdgeVisibilityOff();
      break;
    case mvGeometry::SurfaceWithEdges:
      dataItem->actor->GetProperty()->SetRepresentation(VTK_SURFACE);
      dataItem->actor->GetProperty()->EdgeVisibilityOn();
      break;
    }

  if (!m_appData)
    {
    vis = false;
    }

  dataItem->actor->SetVisibility(vis ? 1 : 0);
  dataItem->actor->GetProperty()->SetOpacity(m_opacity);
}

//------------------------------------------------------------------------------
double mvGeometry::opacity() const
{
  return m_opacity;
}

//------------------------------------------------------------------------------
void mvGeometry::setOpacity(double o)
{
  m_opacity = o;
}

//------------------------------------------------------------------------------
mvGeometry::Representation mvGeometry::representation() const
{
  return m_representation;
}

//------------------------------------------------------------------------------
void mvGeometry::setRepresentation(Representation repr)
{
  m_representation = repr;
}
