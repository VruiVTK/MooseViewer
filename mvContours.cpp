#include "mvContours.h"

#include <GL/GLContextData.h>

#include <vtkActor.h>
#include <vtkCompositeDataSet.h>
#include <vtkCompositePolyDataMapper.h>
#include <vtkExodusIIReader.h>
#include <vtkExternalOpenGLRenderer.h>
#include <vtkLookupTable.h>
#include <vtkSMPContourGrid.h>
#include <vtkSpanSpace.h>
#include <vtkUnstructuredGrid.h>

#include "ArrayLocator.h"
#include "mvApplicationState.h"
#include "mvContextState.h"

//------------------------------------------------------------------------------
mvContours::DataItem::DataItem()
{
  this->mapper->SetScalarVisibility(1);
  this->mapper->SetScalarModeToDefault();
  this->mapper->SetColorModeToMapScalars();
  this->mapper->UseLookupTableScalarRangeOn();

  this->actor->SetMapper(this->mapper.GetPointer());
}

//------------------------------------------------------------------------------
mvContours::mvContours()
  : m_visible(true)
{
  m_scalarTree->SetResolution(100);
  m_contour->ComputeScalarsOn();

  // BUG: These options cause artifacts with the SMP filter. Reported as VTK
  // bug 15969.
//  m_contour->SetScalarTree(m_scalarTree.GetPointer());
//  m_contour->UseScalarTreeOn();
//  m_contour->MergePiecesOn();
  m_contour->MergePiecesOff();
}

//------------------------------------------------------------------------------
mvContours::~mvContours()
{
}

//------------------------------------------------------------------------------
void mvContours::initContext(GLContextData &contextData) const
{
  this->mvGLObject::initContext(contextData);

  assert("Duplicate context initialization detected!" &&
         !contextData.retrieveDataItem<DataItem>(this));

  DataItem *dataItem = new DataItem;
  contextData.addDataItem(this, dataItem);
}

//------------------------------------------------------------------------------
void mvContours::initMvContext(mvContextState &mvContext,
                               GLContextData &contextData) const
{
  this->mvGLObject::initMvContext(mvContext, contextData);

  DataItem *dataItem = contextData.retrieveDataItem<DataItem>(this);
  assert(dataItem);

  mvContext.renderer().AddActor(dataItem->actor.GetPointer());
}

//------------------------------------------------------------------------------
void mvContours::syncApplicationState(const mvApplicationState &state)
{
  this->mvGLObject::syncApplicationState(state);

  // Sync pipeline state.
  m_contour->SetInputConnection(state.reader().GetOutputPort());

  // Use the correct array for contouring:
  switch (state.locator().Association)
    {
    case ArrayLocator::NotFound:
    case ArrayLocator::Invalid:
      break;

    case ArrayLocator::PointData:
      m_contour->SetInputArrayToProcess(
            0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS,
            state.locator().Name.c_str());
      break;

    case ArrayLocator::CellData:
      m_contour->SetInputArrayToProcess(
            0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS,
            state.locator().Name.c_str());
      break;

    case ArrayLocator::FieldData:
      m_contour->SetInputArrayToProcess(
            0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_NONE,
            state.locator().Name.c_str());
      break;
    }

  // Set contour values:
  double min = state.locator().Range[0];
  double spread = state.locator().Range[1] - min;
  m_contour->SetNumberOfContours(m_contourValues.size());
  for (int i = 0; i < m_contourValues.size(); ++i)
    {
    m_contour->SetValue(i, (m_contourValues[i]/255.0) * spread + min);
    }

  // Re-execute if modified.
  m_contour->Update();
}

//------------------------------------------------------------------------------
void mvContours::syncContextState(const mvApplicationState &appState,
                                  const mvContextState &contextState,
                                  GLContextData &contextData) const
{
  this->mvGLObject::syncContextState(appState, contextState, contextData);

  DataItem *dataItem = contextData.retrieveDataItem<DataItem>(this);
  assert(dataItem);

  dataItem->actor->SetVisibility(m_visible ? 1 : 0);

  // TODO compute contours in a background thread.
  if (vtkDataObject *filterOutput = m_contour->GetOutputDataObject(0))
    {
    if (!dataItem->data ||
        dataItem->data->GetMTime() < filterOutput->GetMTime())
      {
      // We intentionally break the pipeline here to allow future async
      // computation of the rendered dataset.
      dataItem->data.TakeReference(filterOutput->NewInstance());
      dataItem->data->DeepCopy(filterOutput);
      }
    }
  else
    {
    dataItem->data = NULL;
    }
  dataItem->mapper->SetInputDataObject(dataItem->data);
  dataItem->mapper->SetLookupTable(&appState.colorMap());

  // Point the mapper at the proper scalar array.
  switch (appState.locator().Association)
    {
    case ArrayLocator::NotFound:
    case ArrayLocator::Invalid:
      break;

    case ArrayLocator::PointData:
      dataItem->mapper->SetScalarModeToUsePointFieldData();
      dataItem->mapper->SelectColorArray(appState.locator().Name.c_str());
      break;

    case ArrayLocator::CellData:
      dataItem->mapper->SetScalarModeToUseCellFieldData();
      dataItem->mapper->SelectColorArray(appState.locator().Name.c_str());
      break;

    case ArrayLocator::FieldData:
      dataItem->mapper->SetScalarModeToUseFieldData();
      dataItem->mapper->SelectColorArray(appState.locator().Name.c_str());
      break;
    }
}

//------------------------------------------------------------------------------
std::vector<double> mvContours::contourValues() const
{
  return m_contourValues;
}

//------------------------------------------------------------------------------
void mvContours::setContourValues(const std::vector<double> &contourValues)
{
  m_contourValues = contourValues;
}
