#include "mvGeometry.h"

#include <GL/GLContextData.h>

#include <vtkActor.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkExodusIIReader.h>
#include <vtkExternalOpenGLRenderer.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

#include "ArrayLocator.h"
#include "mvApplicationState.h"
#include "mvContextState.h"

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
void mvGeometry::syncApplicationState(const mvApplicationState &state)
{
  this->Superclass::syncApplicationState(state);

  m_filter->SetInputConnection(state.reader().GetOutputPort());

  m_filter->Update();
}

//------------------------------------------------------------------------------
void mvGeometry::syncContextState(const mvApplicationState &appState,
                                  const mvContextState &contextState,
                                  GLContextData &contextData) const
{
  this->Superclass::syncContextState(appState, contextState, contextData);

  DataItem *dataItem = contextData.retrieveDataItem<DataItem>(this);
  assert(dataItem);

  // TODO compute data in a background thread.
  if (vtkDataObject *appData = m_filter->GetOutputDataObject(0))
    {
    if (!dataItem->data ||
        dataItem->data->GetMTime() < appData->GetMTime())
      {
      // We intentionally break the pipeline here to allow future async
      // computation of the rendered dataset.
      dataItem->data.TakeReference(appData->NewInstance());
      dataItem->data->DeepCopy(appData);
      }
    }
  else
    {
    dataItem->data = NULL;
    }
  dataItem->mapper->SetInputDataObject(dataItem->data);

  switch (appState.locator().Association)
    {
    case ArrayLocator::Invalid:
    case ArrayLocator::NotFound:
      break;

    case ArrayLocator::PointData:
      dataItem->mapper->SetScalarModeToUsePointFieldData();
      break;

    case ArrayLocator::CellData:
      dataItem->mapper->SetScalarModeToUseCellFieldData();
      break;

    case ArrayLocator::FieldData:
      dataItem->mapper->SetScalarModeToUseFieldData();
      break;
    }

  dataItem->mapper->SelectColorArray(appState.locator().Name.c_str());
  dataItem->mapper->SetScalarRange(appState.locator().Range[0],
                                   appState.locator().Range[1]);
  dataItem->mapper->SetLookupTable(&appState.colorMap());

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
