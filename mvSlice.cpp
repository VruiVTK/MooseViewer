#include "mvSlice.h"

#include <GL/GLContextData.h>

#include <Geometry/Rotation.h>

#include <vtkActor.h>
#include <vtkCompositePolyDataMapper.h>
#include <vtkCutter.h>
#include <vtkExternalOpenGLRenderer.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkSampleImplicitFunctionFilter.h>
#include <vtkSMPContourGrid.h>
#include <vtkUnstructuredGrid.h>

#include "mvApplicationState.h"
#include "mvContextState.h"
#include "mvInteractor.h"
#include "mvReader.h"

#include <algorithm>
#include <iostream>

//------------------------------------------------------------------------------
mvSlice::DataItem::DataItem()
{
  this->hintMapper->ScalarVisibilityOff();

  this->hintActor->SetMapper(this->hintMapper.Get());
  this->hintActor->GetProperty()->SetColor(1., 1., 1.);
  this->hintActor->GetProperty()->SetOpacity(0.25);

  this->mapper->SetScalarVisibility(1);
  this->mapper->SetScalarModeToDefault();
  this->mapper->SetColorModeToMapScalars();
  this->mapper->UseLookupTableScalarRangeOn();

  this->actor->SetMapper(this->mapper.GetPointer());
}

//------------------------------------------------------------------------------
mvSlice::mvSlice()
  : m_visible(false)
{
  m_plane.normal = {1., 1., 1.};
  m_plane.origin = {0., 0., 0.};

  m_hintCutter->GenerateTrianglesOn();
  m_hintCutter->GenerateCutScalarsOff();
  m_hintCutter->SetNumberOfContours(1);
  m_hintCutter->SetValue(0, 0.);
  m_hintCutter->SetInputData(m_hintBox.Get());
  m_hintCutter->SetCutFunction(m_hintFunction.Get());

  m_addPlane->SetImplicitFunction(m_function.Get());
  m_addPlane->ComputeGradientsOff();

  m_cutter->SetInputConnection(m_addPlane->GetOutputPort());
  m_cutter->GenerateTrianglesOn();
  m_cutter->ComputeScalarsOn();
  m_cutter->SetNumberOfContours(1);
  m_cutter->SetValue(0, 0.);

  // BUG: These options cause artifacts with the SMP filter. Reported as VTK
  // bug 15969.
//  m_contour->SetScalarTree(m_scalarTree.GetPointer());
//  m_contour->UseScalarTreeOn();
//  m_contour->MergePiecesOn();
  m_cutter->MergePiecesOff();
}

//------------------------------------------------------------------------------
mvSlice::~mvSlice()
{
}

//------------------------------------------------------------------------------
void mvSlice::initMvContext(mvContextState &mvContext,
                            GLContextData &contextData) const
{
  this->Superclass::initMvContext(mvContext, contextData);

  assert("Duplicate context initialization detected!" &&
         !contextData.retrieveDataItem<DataItem>(this));

  DataItem *dataItem = new DataItem;
  contextData.addDataItem(this, dataItem);

  dataItem->hintMapper->SetInputConnection(m_hintCutter->GetOutputPort());
  mvContext.renderer().AddActor(dataItem->hintActor.Get());

  mvContext.renderer().AddActor(dataItem->actor.Get());
}

//------------------------------------------------------------------------------
void mvSlice::frame(const mvApplicationState &state)
{
  // Update the hinter if needed:
  if (m_visible && state.interactor().isInteracting())
    {
    // Update the plane:
    switch (state.interactor().state())
      {
      case mvInteractor::NoInteraction:
        return;

      case mvInteractor::Translating:
        {
        const Vrui::Vector &vec = state.interactor().current().getTranslation();
        m_plane.origin[0] = vec[0];
        m_plane.origin[1] = vec[1];
        m_plane.origin[2] = vec[2];
        }
        break;

      case mvInteractor::Rotating:
        {
        const Vrui::Rotation &rot = state.interactor().delta().getRotation();
        Vrui::Vector n(m_plane.normal.data());
        n = rot.transform(n);
        std::copy(n.getComponents(), n.getComponents() + 3,
                  m_plane.normal.begin());
        }
        break;

      default:
        std::cerr << "Unknown interaction state: "
                  << state.interactor().state() << "\n";
        return;
      }

    // Cut a plane from a simple image data:
    m_hintBox->SetExtent(0, 1, 0, 1, 0, 1);
    m_hintBox->SetOrigin(const_cast<double*>(
                           state.reader().bounds().GetMinPoint()));
    m_hintBox->SetSpacing(state.reader().bounds().GetLength(0),
                          state.reader().bounds().GetLength(1),
                          state.reader().bounds().GetLength(2));

    // Setup the plane:
    m_hintFunction->SetNormal(m_plane.normal.data());
    m_hintFunction->SetOrigin(m_plane.origin.data());

    // Always update here; otherwise the update will be triggered from the
    // (potentially threaded) rendering code on-demand.
    m_hintCutter->Update();
    }
}

//------------------------------------------------------------------------------
void mvSlice::configureDataPipeline(const mvApplicationState &state)
{
  m_addPlane->SetInputDataObject(state.reader().dataObject());

  // Sync planes:
  // Casts are for VTK (not const-correct)
  m_function->SetNormal(const_cast<double*>(m_plane.normal.data()));
  m_function->SetOrigin(const_cast<double*>(m_plane.origin.data()));
}

//------------------------------------------------------------------------------
bool mvSlice::dataPipelineNeedsUpdate() const
{
  return
      m_addPlane->GetInputDataObject(0, 0) &&
      m_visible &&
      (!m_appData ||
       m_appData->GetMTime() < m_addPlane->GetMTime() ||
       m_appData->GetMTime() < m_cutter->GetMTime() ||
       m_appData->GetMTime() < m_function->GetMTime());
}

//------------------------------------------------------------------------------
void mvSlice::executeDataPipeline() const
{
  m_cutter->Update();
}

//------------------------------------------------------------------------------
void mvSlice::retrieveDataPipelineResult()
{
  vtkDataObject *dObj = m_cutter->GetOutputDataObject(0);
  m_appData.TakeReference(dObj->NewInstance());
  m_appData->ShallowCopy(dObj);
}

//------------------------------------------------------------------------------
void mvSlice::syncContextState(const mvApplicationState &appState,
                               const mvContextState &contextState,
                               GLContextData &contextData) const
{
  this->Superclass::syncContextState(appState, contextState, contextData);

  DataItem *dataItem = contextData.retrieveDataItem<DataItem>(this);
  assert(dataItem);

  // Determine what should be drawn:
  auto metaData = appState.reader().variableMetaData(appState.colorByArray());
  enum Target { None, Hint, Full } target = None;
  if (m_visible)
    {
    if (!appState.interactor().isInteracting() &&
        m_appData &&
        metaData.valid())
      {
      target = Full;
      }
    else if (appState.interactor().isInteracting())
      {
      target = Hint;
      }
    }

  // Update visibilities:
  dataItem->hintActor->SetVisibility(target == Hint ? 1 : 0);
  dataItem->actor->SetVisibility(target == Full ? 1 : 0);

  dataItem->mapper->SetInputDataObject(m_appData);

  // Only update state if the color array exists.
  if (metaData.valid())
    {
    dataItem->mapper->SetLookupTable(&appState.colorMap());

    // Point the mapper at the proper scalar array.
    switch (metaData.location)
      {
      case mvReader::VariableMetaData::Location::PointData:
        dataItem->mapper->SetScalarModeToUsePointFieldData();
        dataItem->mapper->SelectColorArray(appState.colorByArray().c_str());
        break;

      case mvReader::VariableMetaData::Location::CellData:
        dataItem->mapper->SetScalarModeToUseCellFieldData();
        dataItem->mapper->SelectColorArray(appState.colorByArray().c_str());
        break;

      case mvReader::VariableMetaData::Location::FieldData:
        dataItem->mapper->SetScalarModeToUseFieldData();
        dataItem->mapper->SelectColorArray(appState.colorByArray().c_str());
        break;

      default:
        break;
      }
    }
}
