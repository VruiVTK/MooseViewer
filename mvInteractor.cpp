#include "mvInteractor.h"

#include <Vrui/Tool.h>
#include <Vrui/Vrui.h>

//------------------------------------------------------------------------------
mvInteractor::mvInteractor()
  : m_state(NoInteraction),
    m_tool(nullptr)
{
  Vrui::ToolManager *toolMgr = Vrui::getToolManager();

  toolMgr->getToolCreationCallbacks().add(
        const_cast<mvInteractor*>(this), &mvInteractor::_cb_toolCreation);
  toolMgr->getToolDestructionCallbacks().add(
        const_cast<mvInteractor*>(this), &mvInteractor::_cb_toolDestruction);
}

//------------------------------------------------------------------------------
mvInteractor::~mvInteractor()
{
  Vrui::ToolManager *toolMgr = Vrui::getToolManager();

  toolMgr->getToolCreationCallbacks().remove(
        const_cast<mvInteractor*>(this), &mvInteractor::_cb_toolCreation);
  toolMgr->getToolDestructionCallbacks().remove(
        const_cast<mvInteractor*>(this), &mvInteractor::_cb_toolDestruction);

  if (m_tool)
    {
    this->unbindTool(m_tool);
    }
}

//------------------------------------------------------------------------------
void mvInteractor::_cb_move(mvInteractorTool::MoveEvent *e)
{
  if (this->isInteracting())
    {
    Vrui::NavTrackerState lastInverse = Geometry::invert(m_current);
    m_current = e->transform;
    m_delta = m_current * lastInverse;
    }
}

//------------------------------------------------------------------------------
void mvInteractor::_cb_press(mvInteractorTool::PressEvent *e)
{
  // Ignore events if we're already interacting:
  if (this->isInteracting())
    {
    return;
    }

  switch (e->button)
    {
    case 0:
      m_state = Translating;
      break;

    case 1:
      m_state = Rotating;
      break;

    default:
      reset();
      return;
    }

  m_current = e->transform;
  m_delta = Vrui::NavTrackerState::identity;
}

//------------------------------------------------------------------------------
void mvInteractor::_cb_release(mvInteractorTool::ReleaseEvent *e)
{
  // Ignore events for other interactions:
  switch (e->button)
    {
    case 0:
      if (m_state != Translating)
        {
        return;
        }
      break;

    case 1:
      if (m_state != Rotating)
        {
        return;
        }
      break;

    default:
      return;
    }

  reset();
}

//------------------------------------------------------------------------------
void mvInteractor::
_cb_toolCreation(Vrui::ToolManager::ToolCreationCallbackData *cbd)
{
  if (ToolType *tool = dynamic_cast<ToolType*>(cbd->tool))
    {
    this->bindTool(tool);
    }
}

//------------------------------------------------------------------------------
void mvInteractor::
_cb_toolDestruction(Vrui::ToolManager::ToolCreationCallbackData *cbd)
{
  if (ToolType *tool = dynamic_cast<ToolType*>(cbd->tool))
    {
    this->unbindTool(tool);
    }
}

//------------------------------------------------------------------------------
void mvInteractor::reset()
{
  m_state = NoInteraction;
  m_current = Vrui::NavTrackerState::identity;
  m_delta = Vrui::NavTrackerState::identity;
}

//------------------------------------------------------------------------------
void mvInteractor::bindTool(mvInteractor::ToolType *tool)
{
  if (!m_tool)
    {
    tool->getMoveCallbacks().add(this, &mvInteractor::_cb_move);
    tool->getPressCallbacks().add(this, &mvInteractor::_cb_press);
    tool->getReleaseCallbacks().add(this, &mvInteractor::_cb_release);
    m_tool = tool;
    }
}

//------------------------------------------------------------------------------
void mvInteractor::unbindTool(mvInteractor::ToolType *tool)
{
  if (m_tool && m_tool == tool)
    {
    tool->getMoveCallbacks().remove(this, &mvInteractor::_cb_move);
    tool->getPressCallbacks().remove(this, &mvInteractor::_cb_press);
    tool->getReleaseCallbacks().remove(this, &mvInteractor::_cb_release);
    }
}
