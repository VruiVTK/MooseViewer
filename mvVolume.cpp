#include "mvVolume.h"

#include <vtkCellDataToPointData.h>
#include <vtkCheckerboardSplatter.h>
#include <vtkColorTransferFunction.h>
#include <vtkCompositeDataIterator.h>
#include <vtkCompositeDataSet.h>
#include <vtkExodusIIReader.h>
#include <vtkExternalOpenGLRenderer.h>
#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkPiecewiseFunction.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

#include <GL/GLContextData.h>

#include "mvApplicationState.h"
#include "mvContextState.h"

//------------------------------------------------------------------------------
mvVolume::DataItem::DataItem()
{
  this->actor->SetMapper(this->mapper.GetPointer());
}

//------------------------------------------------------------------------------
mvVolume::mvVolume()
  : m_visible(false),
    m_requestedRenderMode(vtkSmartVolumeMapper::GPURenderMode),
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
void mvVolume::syncApplicationState(const mvApplicationState &state)
{
  this->Superclass::syncApplicationState(state);

  // Splat the proper array:
  switch (state.locator().Association)
    {
    case ArrayLocator::NotFound:
    case ArrayLocator::Invalid:
      m_splat->SetInputConnection(state.reader().GetOutputPort());
      m_splat->SetInputArrayToProcess(
            0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "");

      break;

    case ArrayLocator::PointData:
      m_splat->SetInputConnection(state.reader().GetOutputPort());
      m_splat->SetInputArrayToProcess(
            0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS,
            state.locator().Name.c_str());
      break;

    case ArrayLocator::CellData:
      m_cell2point->SetInputConnection(state.reader().GetOutputPort());
      m_splat->SetInputConnection(m_cell2point->GetOutputPort());
      m_splat->SetInputArrayToProcess(
            0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS,
            state.locator().Name.c_str());
      break;

    case ArrayLocator::FieldData:
      std::cerr << "Cannot generate volume from generic field data. Must "
                   "use point/cell data." << std::endl;
      m_splat->SetInputConnection(state.reader().GetOutputPort());
      m_splat->SetInputArrayToProcess(
            0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_POINTS, "");
      break;
    }

  m_splat->SetSampleDimensions(m_splatDimensions, m_splatDimensions,
                               m_splatDimensions);
  m_splat->SetRadius(m_splatRadius * 10.);
  m_splat->SetExponentFactor(m_splatExponent);
  m_splat->Update();
}

//------------------------------------------------------------------------------
void mvVolume::syncContextState(const mvApplicationState &appState,
                                const mvContextState &contextState,
                                GLContextData &contextData) const
{
  this->Superclass::syncContextState(appState, contextState, contextData);

  DataItem *dataItem = contextData.retrieveDataItem<DataItem>(this);
  assert(dataItem);

  dataItem->actor->SetVisibility((m_visible && !appState.locator().Name.empty())
                                 ? 1 : 0);

  if (vtkDataObject *appDS = m_splat->GetOutputDataObject(0))
    {
    if (!dataItem->data ||
        dataItem->data->GetMTime() < appDS->GetMTime())
      {
      // If we have composite data, just extract the first image leaf.
      // TODO At some point the splatter should be reworked to handle
      // composite data.
      if (vtkCompositeDataSet *cds = vtkCompositeDataSet::SafeDownCast(appDS))
        {
        vtkCompositeDataIterator *it = cds->NewIterator();
        it->InitTraversal();
        while (!it->IsDoneWithTraversal())
          {
          if (vtkImageData *image =
              vtkImageData::SafeDownCast(it->GetCurrentDataObject()))
            {
            appDS = image;
            break;
            }
          it->GoToNextItem();
          }
        it->Delete();
        }
      // We intentionally break the pipeline here to allow future async
      // computation of the rendered dataset.
      dataItem->data.TakeReference(appDS->NewInstance());
      dataItem->data->DeepCopy(appDS);
      }
    }
  else
    {
    dataItem->data = NULL;
    }
  dataItem->mapper->SetInputDataObject(dataItem->data);

  // Update color map.
  if (appState.colorMap().GetMTime() > std::min(m_colorFunction->GetMTime(),
                                                m_opacityFunction->GetMTime()))
    {
    m_colorFunction->RemoveAllPoints();
    m_opacityFunction->RemoveAllPoints();
    double step = (appState.locator().Range[1] - appState.locator().Range[0])
                   / 255.0;
    double rgba[4];
    for (vtkIdType i = 0; i < 256; ++i)
      {
      const double x = appState.locator().Range[0] + (i * step);
      appState.colorMap().GetTableValue(i, rgba);
      m_colorFunction->AddRGBPoint(x, rgba[0], rgba[1], rgba[2]);
      m_opacityFunction->AddPoint(x, rgba[3]);
      }
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
