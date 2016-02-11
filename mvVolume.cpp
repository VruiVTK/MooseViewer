#include "mvVolume.h"

#include <vtkCellDataToPointData.h>
#include <vtkCheckerboardSplatter.h>
#include <vtkColorTransferFunction.h>
#include <vtkCompositeDataIterator.h>
#include <vtkExternalOpenGLRenderer.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkNew.h>
#include <vtkPiecewiseFunction.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

#include <GL/GLContextData.h>

#include "mvApplicationState.h"
#include "mvContextState.h"
#include "mvReader.h"

//------------------------------------------------------------------------------
mvVolume::DataItem::DataItem()
{
  this->actor->SetMapper(this->mapper.GetPointer());
}

//------------------------------------------------------------------------------
mvVolume::mvVolume()
  : m_visible(false),
    m_requestedRenderMode(vtkSmartVolumeMapper::DefaultRenderMode),
    m_splatDimensions(30),
    m_splatExponent(-1.0),
    m_splatRadius(0.01)
{
  m_splat->ScalarWarpingOn();
  m_splat->NormalWarpingOff();

  m_volumeProperty->SetColor(m_colorFunction.Get());
  m_volumeProperty->SetScalarOpacity(m_opacityFunction.Get());
  m_volumeProperty->SetInterpolationTypeToLinear();
  m_volumeProperty->ShadeOff();
}

//------------------------------------------------------------------------------
mvVolume::~mvVolume()
{
}

//------------------------------------------------------------------------------
void mvVolume::initMvContext(mvContextState &mvContext,
                             GLContextData &contextData) const
{
  this->Superclass::initMvContext(mvContext, contextData);

  assert("Duplicate context initialization detected!" &&
         !contextData.retrieveDataItem<DataItem>(this));

  DataItem *dataItem = new DataItem;
  contextData.addDataItem(this, dataItem);

  dataItem->actor->SetProperty(m_volumeProperty.GetPointer());
  mvContext.renderer().AddVolume(dataItem->actor.GetPointer());
}

//------------------------------------------------------------------------------
void mvVolume::configureDataPipeline(const mvApplicationState &state)
{
  m_splat->SetSampleDimensions(m_splatDimensions, m_splatDimensions,
                               m_splatDimensions);
  m_splat->SetRadius(m_splatRadius * 10.);
  m_splat->SetExponentFactor(m_splatExponent);

  // Only modify the filter if the colorByArray is loaded.
  auto metaData = state.reader().variableMetaData(state.colorByArray());
  if (metaData.valid())
    {
    // Splat the proper array:
    switch (metaData.location)
      {
      case mvReader::VariableMetaData::Location::PointData:
        m_splat->SetInputDataObject(state.reader().dataObject());
        m_splat->SetInputArrayToProcess(
              0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS,
              state.colorByArray().c_str());
        break;

      case mvReader::VariableMetaData::Location::CellData:
        m_cell2point->SetInputDataObject(state.reader().dataObject());
        m_splat->SetInputConnection(m_cell2point->GetOutputPort());
        m_splat->SetInputArrayToProcess(
              0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS,
              state.colorByArray().c_str());
        break;

      case mvReader::VariableMetaData::Location::FieldData:
        std::cerr << "Cannot generate volume from generic field data. Must "
                     "use point/cell data." << std::endl;


      default:
        break;
        m_splat->SetInputDataObject(nullptr);
        m_splat->SetInputArrayToProcess(
              0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "");
        break;

      }
    }
  else
    {
    m_splat->SetInputDataObject(nullptr);
    }
}

//------------------------------------------------------------------------------
bool mvVolume::dataPipelineNeedsUpdate() const
{
  return
      m_splat->GetInputDataObject(0, 0) &&
      m_visible &&
      (!m_appData ||
       m_appData->GetMTime() < m_cell2point->GetMTime() ||
       m_appData->GetMTime() < m_splat->GetMTime());
}

//------------------------------------------------------------------------------
void mvVolume::executeDataPipeline() const
{
  m_splat->Update();
}

//------------------------------------------------------------------------------
void mvVolume::retrieveDataPipelineResult()
{
  vtkDataObject *dObj = m_splat->GetOutputDataObject(0);

  // If we have composite data, just extract the first image leaf.
  // TODO At some point the splatter should be reworked to handle
  // composite data.
  if (vtkCompositeDataSet *cds = vtkCompositeDataSet::SafeDownCast(dObj))
    {
    vtkCompositeDataIterator *it = cds->NewIterator();
    it->InitTraversal();
    while (!it->IsDoneWithTraversal())
      {
      if (vtkImageData *image =
          vtkImageData::SafeDownCast(it->GetCurrentDataObject()))
        {
        dObj = image;
        break;
        }
      it->GoToNextItem();
      }
    it->Delete();
    }

  m_appData.TakeReference(dObj->NewInstance());
  m_appData->ShallowCopy(dObj);
}

//------------------------------------------------------------------------------
void mvVolume::syncContextState(const mvApplicationState &appState,
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
    dataItem->mapper->SetRequestedRenderMode(m_requestedRenderMode);
    dataItem->actor->SetVisibility(m_visible ? 1 : 0);

    // Update color map.
    if (appState.colorMap().GetMTime() >
        std::min(m_colorFunction->GetMTime(), m_opacityFunction->GetMTime()))
      {
      m_colorFunction->RemoveAllPoints();
      m_opacityFunction->RemoveAllPoints();
      double step = (metaData.range[1] - metaData.range[0]) / 255.0;
      double rgba[4];
      for (vtkIdType i = 0; i < 256; ++i)
        {
        const double x = metaData.range[0] + (i * step);
        appState.colorMap().GetTableValue(i, rgba);
        m_colorFunction->AddRGBPoint(x, rgba[0], rgba[1], rgba[2]);
        m_opacityFunction->AddPoint(x, rgba[3]);
        }
      }
    }

  if (!m_appData)
    {
    dataItem->actor->SetVisibility(0);
    }
}

//------------------------------------------------------------------------------
int mvVolume::requestedRenderMode() const
{
  return m_requestedRenderMode;
}

//------------------------------------------------------------------------------
void mvVolume::setRequestedRenderMode(int mode)
{
  m_requestedRenderMode = mode;
}

//------------------------------------------------------------------------------
double mvVolume::splatRadius() const
{
  return m_splatRadius;
}

//------------------------------------------------------------------------------
void mvVolume::setSplatRadius(double radius)
{
  m_splatRadius = radius;
}

//------------------------------------------------------------------------------
double mvVolume::splatExponent() const
{
  return m_splatExponent;
}

//------------------------------------------------------------------------------
void mvVolume::setSplatExponent(double exponent)
{
  m_splatExponent = exponent;
}

//------------------------------------------------------------------------------
double mvVolume::splatDimensions() const
{
  return m_splatDimensions;
}

//------------------------------------------------------------------------------
void mvVolume::setSplatDimensions(double dimensions)
{
  m_splatDimensions = dimensions;
}
