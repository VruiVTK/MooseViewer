#include "mvInteractorTool.h"

#include <Vrui/Vrui.h>

using Misc::CallbackData;
using Misc::CallbackList;
using Vrui::InputDevice;
using Vrui::NavTrackerState;
using Vrui::Tool;
using Vrui::ToolFactory;
using Vrui::ToolInputAssignment;
using Vrui::ToolManager;

//------------------------------------------------------------------------------
mvInteractorToolFactory::mvInteractorToolFactory(ToolManager &mgr)
  : ToolFactory("mvInteractorTool", mgr)
{
  layout.setNumValuators(0);
  layout.setNumButtons(2);
}

//------------------------------------------------------------------------------
const char *mvInteractorToolFactory::getName() const
{
  return "MooseViewer Interactor";
}

//------------------------------------------------------------------------------
const char *mvInteractorToolFactory::getButtonFunction(int slot) const
{
  switch (slot)
    {
    case 0:
      return "Translate Object";
    case 1:
      return "Rotate Object";
    default:
      return nullptr;
    }
}

//------------------------------------------------------------------------------
Tool *mvInteractorToolFactory::createTool(const ToolInputAssignment &ia) const
{
  return new mvInteractorTool(this, ia);
}

//------------------------------------------------------------------------------
void mvInteractorToolFactory::destroyTool(Tool *tool) const
{
  delete tool;
}

//------------------------------------------------------------------------------
mvInteractorTool::mvInteractorTool(const Vrui::ToolFactory *f,
                                   const Vrui::ToolInputAssignment &iA)
  : Tool(f, iA),
    m_factory(f)
{
}

//------------------------------------------------------------------------------
Vrui::NavTrackerState mvInteractorTool::trackerState()
{
  return Vrui::NavTrackerState(
        Vrui::getDeviceTransformation(this->getButtonDevice(0)).getTranslation(),
        Vrui::getDeviceTransformation(this->getButtonDevice(1)).getRotation(),
        Vrui::Scalar(1.));
}

//------------------------------------------------------------------------------
void mvInteractorTool::buttonCallback(int slot,
                                      InputDevice::ButtonCallbackData *cbd)
{
  if (cbd->newButtonState)
    {
    PressEvent e(this, slot, this->trackerState());
    m_pressCallbacks.call(&e);
    }
  else
    {
    ReleaseEvent e(this, slot, this->trackerState());
    m_releaseCallbacks.call(&e);
    }
}

//------------------------------------------------------------------------------
void mvInteractorTool::frame()
{
  MoveEvent e(this, this->trackerState());
  m_moveCallbacks.call(&e);
}

//------------------------------------------------------------------------------
const Vrui::ToolFactory *mvInteractorTool::getFactory() const
{
  return m_factory;
}
