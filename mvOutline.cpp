#include "mvOutline.h"

#include <GL/GLContextData.h>

#include "vtkActor.h"
#include "vtkCompositePolyDataMapper.h"
#include "vtkDataObject.h"
#include "vtkExternalOpenGLRenderer.h"
#include "vtkMultiBlockDataSet.h"
#include "vtkOutlineFilter.h"
#include "vtkProperty.h"

#include "mvApplicationState.h"
#include "mvContextState.h"
#include "mvReader.h"

//------------------------------------------------------------------------------
mvOutline::DataItem::DataItem()
{
  actor->SetMapper(mapper.Get());
  actor->GetProperty()->SetColor(1, 1, 1);
}

//------------------------------------------------------------------------------
mvOutline::mvOutline()
  : m_visible(true)
{
}

//------------------------------------------------------------------------------
mvOutline::~mvOutline()
{
}

//------------------------------------------------------------------------------
void mvOutline::initMvContext(mvContextState &mvContext,
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
void mvOutline::syncApplicationState(const mvApplicationState &state)
{
  this->Superclass::syncApplicationState(state);

  m_filter->SetInputDataObject(state.reader().dataObject());
  m_filter->Update();
}

//------------------------------------------------------------------------------
void mvOutline::syncContextState(const mvApplicationState &appState,
                                 const mvContextState &contextState,
                                 GLContextData &contextData) const
{
  this->Superclass::syncContextState(appState, contextState, contextData);

  DataItem *dataItem = contextData.retrieveDataItem<DataItem>(this);
  assert(dataItem);

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

  dataItem->actor->SetVisibility(m_visible ? 1 : 0);
}
