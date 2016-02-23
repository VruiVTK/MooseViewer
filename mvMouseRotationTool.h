#ifndef MVMOUSEROTATIONTOOL_H
#define MVMOUSEROTATIONTOOL_H

#include <Vrui/TransformTool.h>

#include <Geometry/OrthogonalTransformation.h>

/**
 * @brief The mvMouseRotationToolclass is a Vrui::TransformTool that
 * converts a 2D mouse position into a 6-DOF rotation event.
 *
 * This essentially a copy of the rotation logic from Vrui::MouseNavigationTool.
 */
class mvMouseRotationTool : public Vrui::TransformTool
{
public:
  using Superclass = Vrui::TransformTool;

  mvMouseRotationTool(const Vrui::ToolFactory *f,
                      const Vrui::ToolInputAssignment &iA);

  const Vrui::ToolFactory* getFactory() const override;
  void frame() override;

protected: // Internal API:

  // Compute relevant positions, etc.
  Vrui::Point currentPosition() const;
  Vrui::TrackerState currentTransform() const;

protected: // State:

  Vrui::Scalar m_rotationOffsetParam; // See MouseNavTool docs.
  Vrui::Scalar m_rotationFactor; // See MouseNavTool docs.
  Vrui::Vector m_rotationOffset; // See MouseNavTool docs.

  Vrui::Point m_center; // Center of the display.
  Vrui::Point m_lastPosition; // Current tool position in UI plane.
  Vrui::Point m_currentPosition; // Current tool position in UI plane.

  Vrui::TrackerState::Rotation m_rotation;

  const Vrui::ToolFactory *m_factory;
};

class mvMouseRotationToolFactory : public Vrui::ToolFactory
{
public:
  explicit mvMouseRotationToolFactory(Vrui::ToolManager &mgr);
  const char* getName() const override;
  Vrui::Tool* createTool(const Vrui::ToolInputAssignment &ia) const override;
  void destroyTool(Vrui::Tool *tool) const override;
};

#endif // MVMOUSEROTATIONTOOL_H
