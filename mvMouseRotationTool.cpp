#include "mvMouseRotationTool.h"

#include <Vrui/ToolManager.h>
#include <Vrui/Vrui.h>

#include <Geometry/Ray.h>

#include <cassert>

using Misc::CallbackData;
using Misc::CallbackList;
using Vrui::InputDevice;
using Vrui::Point;
using Vrui::Scalar;
using Vrui::Tool;
using Vrui::ToolFactory;
using Vrui::ToolInputAssignment;
using Vrui::ToolManager;
using Vrui::TrackerState;
using Vrui::TransformToolFactory;
using Vrui::Vector;

//------------------------------------------------------------------------------
mvMouseRotationToolFactory::mvMouseRotationToolFactory(ToolManager &mgr)
  : ToolFactory("mvMouseRotationTool", mgr)
{
  layout.setNumValuators(0, true);
  layout.setNumButtons(0, true);

  // Insert into class hierarchy:
  TransformToolFactory *transformToolFactory =
      dynamic_cast<TransformToolFactory*>(mgr.loadClass("TransformTool"));
  assert(transformToolFactory);
  transformToolFactory->addChildClass(this);
	this->addParentClass(transformToolFactory);
}

//------------------------------------------------------------------------------
const char *mvMouseRotationToolFactory::getName() const
{
  return "Mouse Position -> 6-DOF Rotation";
}

//------------------------------------------------------------------------------
Tool* mvMouseRotationToolFactory::createTool(const ToolInputAssignment &i) const
{
  return new mvMouseRotationTool(this, i);
}

//------------------------------------------------------------------------------
void mvMouseRotationToolFactory::destroyTool(Tool *tool) const
{
  delete tool;
}

//------------------------------------------------------------------------------
mvMouseRotationTool::mvMouseRotationTool(const ToolFactory *f,
                                         const ToolInputAssignment &iA)
  : Superclass(f, iA),
    m_rotationOffsetParam(Vrui::getDisplaySize() / 4.),
    m_rotationFactor(Vrui::getDisplaySize() / 4.),
    m_rotationOffset(Vrui::getUiPlane().transform(
                       Vector(0., 0., m_rotationOffsetParam))),
    m_center(Vrui::getDisplayCenter()),
    m_lastPosition(),
    m_currentPosition(),
    m_rotation(TrackerState::Rotation::identity),
    m_factory(f)
{
  this->sourceDevice =
      this->input.getNumButtonSlots() > 0 ? this->getButtonDevice(0)
                                          : this->getValuatorDevice(0);

  // Initialize positions (can't do it in init list above, since sourceDevice
  // wasn't yet set)
  m_lastPosition = m_currentPosition = this->currentPosition();
}

//------------------------------------------------------------------------------
const ToolFactory *mvMouseRotationTool::getFactory() const
{
  return m_factory;
}

//------------------------------------------------------------------------------
void mvMouseRotationTool::frame()
{
  // Update positions:
  m_lastPosition = m_currentPosition;
  m_currentPosition = currentPosition();

  // Current displacement:
  Vector delta = m_currentPosition - m_lastPosition;

  // Angle is the magnitude of the last displacement:
  Scalar angle = Geometry::mag(delta) / m_rotationFactor;

  if (angle != Scalar(0.))
    {
    // Offset:
    Vector offset = m_lastPosition - m_center + m_rotationOffset;

    // Rotation axis is the cross produce of the last offset and the current:
    Vector axis = offset^delta;

    // Update rotation:
    m_rotation.leftMultiply(TrackerState::Rotation(axis, angle));
    }

  this->transformedDevice->setDeviceRay(
        this->sourceDevice->getRay().getDirection(), -Vrui::getUiSize() * 2.0);
  this->transformedDevice->setTransformation(this->currentTransform());
}

//------------------------------------------------------------------------------
Point mvMouseRotationTool::currentPosition() const
{
  // Intersect the device ray with the ui plane:
  Point rayStart = this->sourceDevice->getPosition();
  Vector rayDirection = this->sourceDevice->getRayDirection();

  Point planeCenter = Vrui::getUiPlane().getOrigin();
  Vector planeNormal = Vrui::getUiPlane().getDirection(2);
  Scalar lambda = ((planeCenter - rayStart) * planeNormal) / (rayDirection *
                                                              planeNormal);
  return rayStart + rayDirection * lambda;
}

//------------------------------------------------------------------------------
TrackerState mvMouseRotationTool::currentTransform() const
{
  return TrackerState::rotate(m_rotation);
}
