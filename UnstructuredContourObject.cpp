#include "UnstructuredContourObject.h"

#include <GL/GLContextData.h>

#include <vtkActor.h>
#include <vtkCompositeDataSet.h>
#include <vtkCompositePolyDataMapper.h>
#include <vtkLookupTable.h>
#include <vtkRenderer.h>
#include <vtkSMPContourGrid.h>
#include <vtkSpanSpace.h>
#include <vtkUnstructuredGrid.h>

#include "ArrayLocator.h"

//------------------------------------------------------------------------------
UnstructuredContourObject::DataItem::DataItem()
{
  this->mapper->SetScalarVisibility(1);
  this->mapper->SetScalarModeToDefault();
  this->mapper->SetColorModeToMapScalars();
  this->mapper->UseLookupTableScalarRangeOn();

  this->actor->SetMapper(this->mapper.GetPointer());
}

//------------------------------------------------------------------------------
UnstructuredContourObject::UnstructuredContourObject()
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
UnstructuredContourObject::~UnstructuredContourObject()
{
}

//------------------------------------------------------------------------------
void UnstructuredContourObject::update(const ArrayLocator &locator)
{
  // Sync pipeline state.
  m_contour->SetInputData(m_input);

  // Point the mapper at the proper scalar array.
  if (!m_colorByArrayName.empty())
    {
    switch (locator.Association)
      {
      case ArrayLocator::NotFound:
      case ArrayLocator::Invalid:
        break;

      case ArrayLocator::PointData:
        m_contour->SetInputArrayToProcess(
              0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS,
              m_colorByArrayName.c_str());
        break;

      case ArrayLocator::CellData:
        m_contour->SetInputArrayToProcess(
              0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS,
              m_colorByArrayName.c_str());
        break;

      case ArrayLocator::FieldData:
        m_contour->SetInputArrayToProcess(
              0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_NONE,
              m_colorByArrayName.c_str());
        break;
      }
    }

  // Set contour values:
  m_contour->SetNumberOfContours(m_contourValues.size());
  for (int i = 0; i < m_contourValues.size(); ++i)
    {
    m_contour->SetValue(i, m_contourValues[i]);
    }

  // Re-execute if modified.
  m_contour->Update();
}

//------------------------------------------------------------------------------
void UnstructuredContourObject::initContext(GLContextData &contextData) const
{
  assert("Duplicate context initialization detected!" &&
         !contextData.retrieveDataItem<DataItem>(this));

  DataItem *dataItem = new DataItem;
  contextData.addDataItem(this, dataItem);
}

//------------------------------------------------------------------------------
void UnstructuredContourObject::
initRenderer(GLContextData &contextData, vtkRenderer *renderer) const
{
  DataItem *dataItem = contextData.retrieveDataItem<DataItem>(this);
  assert(dataItem);

  renderer->AddActor(dataItem->actor.GetPointer());
}

//------------------------------------------------------------------------------
void UnstructuredContourObject::setLookupTable(GLContextData &contextData,
                                               vtkLookupTable *lut) const
{
  DataItem *dataItem = contextData.retrieveDataItem<DataItem>(this);
  assert(dataItem);

  dataItem->mapper->SetLookupTable(lut);
}

//------------------------------------------------------------------------------
void UnstructuredContourObject::
syncContext(GLContextData &contextData) const
{
  DataItem *dataItem = contextData.retrieveDataItem<DataItem>(this);
  assert(dataItem);

  if (m_visible)
    {
    dataItem->actor->SetVisibility(1);
    }
  else
    {
    dataItem->actor->SetVisibility(0);
    }

  // Copy the data if the output is newer
  if (vtkDataObject *filterOutput = m_contour->GetOutputDataObject(0))
    {
    if (!dataItem->data ||
        dataItem->data->GetMTime() < filterOutput->GetMTime())
      {
      dataItem->data.TakeReference(filterOutput->NewInstance());
      dataItem->data->DeepCopy(filterOutput);
      dataItem->mapper->SetInputDataObject(dataItem->data);
      }
    }
  else
    {
    dataItem->mapper->SetInputDataObject(NULL);
    }

  // Point the mapper at the proper scalar array.
  ArrayLocator locator(dataItem->data, m_colorByArrayName);
  switch (locator.Association)
    {
    case ArrayLocator::NotFound:
    case ArrayLocator::Invalid:
      break;

    case ArrayLocator::PointData:
      dataItem->mapper->SetScalarModeToUsePointFieldData();
      dataItem->mapper->SelectColorArray(m_colorByArrayName.c_str());
      break;

    case ArrayLocator::CellData:
      dataItem->mapper->SetScalarModeToUseCellFieldData();
      dataItem->mapper->SelectColorArray(m_colorByArrayName.c_str());
      break;

    case ArrayLocator::FieldData:
      dataItem->mapper->SetScalarModeToUseFieldData();
      dataItem->mapper->SelectColorArray(m_colorByArrayName.c_str());
      break;
    }
}

//------------------------------------------------------------------------------
void UnstructuredContourObject::setInput(vtkUnstructuredGrid *grid)
{
  m_input = grid;
}

//------------------------------------------------------------------------------
void UnstructuredContourObject::setInput(vtkCompositeDataSet *cds)
{
  if (cds != m_input.GetPointer())
    {
    vtkCompositeDataIterator *it = cds->NewIterator();
    for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextItem())
      {
      if (!it->GetCurrentDataObject()->IsA("vtkUnstructuredGrid"))
        {
        std::cerr << "Expected all composite dataset leaves to be unstructured "
                     "grids, but found a "
                  << it->GetCurrentDataObject()->GetClassName() << "."
                  << std::endl;
        m_input = NULL;
        return;
        }
      }
    it->Delete();

    m_input = cds;
    }
}

//------------------------------------------------------------------------------
vtkDataObject* UnstructuredContourObject::input() const
{
  return m_input.GetPointer();
}

//------------------------------------------------------------------------------
std::vector<double> UnstructuredContourObject::contourValues() const
{
  return m_contourValues;
}

//------------------------------------------------------------------------------
void UnstructuredContourObject::
setContourValues(const std::vector<double> &contourValues)
{
  m_contourValues = contourValues;
}
