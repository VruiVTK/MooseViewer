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
void mvOutline::configureDataPipeline(const mvApplicationState &state)
{
  m_filter->SetInputDataObject(state.reader().dataObject());
}

//------------------------------------------------------------------------------
bool mvOutline::dataPipelineNeedsUpdate() const
{
  return
      m_filter->GetInputDataObject(0, 0) &&
      m_visible &&
      (!m_appData ||
       m_appData->GetMTime() < m_filter->GetMTime());
}

//------------------------------------------------------------------------------
void mvOutline::executeDataPipeline() const
{
  m_filter->Update();
}

//------------------------------------------------------------------------------
void mvOutline::retrieveDataPipelineResult()
{
  vtkDataObject *dObj = m_filter->GetOutputDataObject(0);
  m_appData.TakeReference(dObj->NewInstance());
  m_appData->ShallowCopy(dObj);
}

//------------------------------------------------------------------------------
void mvOutline::syncContextState(const mvApplicationState &appState,
                                 const mvContextState &contextState,
                                 GLContextData &contextData) const
{
  this->Superclass::syncContextState(appState, contextState, contextData);

  DataItem *dataItem = contextData.retrieveDataItem<DataItem>(this);
  assert(dataItem);

  dataItem->mapper->SetInputDataObject(m_appData);

  dataItem->actor->SetVisibility((m_visible && m_appData) ? 1 : 0);
}
