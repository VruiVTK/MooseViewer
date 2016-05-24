#ifndef MVINTERACTOR_H
#define MVINTERACTOR_H

#include "mvInteractorTool.h"

#include <Vrui/Geometry.h>
#include <Vrui/ToolManager.h>

#include <Geometry/OrthogonalTransformation.h>

/**
 * @brief The mvInteractor class captures input events from VRUI for
 * MooseViewer.
 *
 * This class (and the associated mvInteractorTool) can be used to monitor
 * input events from VRUI. Important methods are:
 *
 * - state(): Indicates what type of interaction, if any, is occurring.
 * - isInteracting(): Convenience for state() != NoInteraction.
 * - current(): Returns the current transformation object.
 * - delta(): Returns the difference between this frame and the last frame's
 *            transformations.
 */
class mvInteractor
{
public:
  using ToolType = mvInteractorTool;

  mvInteractor();
  ~mvInteractor();

  enum State
    {
    Uninitialized,
    NoInteraction, /// No interaction requested at this time.
    Translating, /// A translation event has been requested.
    Rotating /// A rotation event has been requested.
    };

  // Initializes internal state. Must be called after VRUI is initialized.
  void init();

  /**
   * @return True if there is an mvInteractorTool bound to this interactor.
   */
  bool isBound() const { return m_tool != nullptr; }

  /**
   * @return The current interaction state. See State.
   */
  State state() const { return m_state; }

  /**
   * @return True if any interactions are requested.
   */
  bool isInteracting() const { return m_state != NoInteraction; }

  /**
   * @return The current 6-DOF tracker state.
   */
  const Vrui::NavTrackerState &current() const { return m_current; }

  /**
   * @return The change in tracker state since the last frame.
   */
  const Vrui::NavTrackerState &delta() const { return m_delta; }

  // Tool callbacks:
  void _cb_move(ToolType::MoveEvent *e);
  void _cb_press(ToolType::PressEvent *e);
  void _cb_release(ToolType::ReleaseEvent *e);

  // Callback for binding:
  void _cb_toolCreation(Vrui::ToolManager::ToolCreationCallbackData *cbd);
  void _cb_toolDestruction(Vrui::ToolManager::ToolCreationCallbackData *cbd);

private:
  void reset();
  void bindTool(ToolType *tool);
  void unbindTool(ToolType *tool);

private:
  // Not implemented -- disable copy:
  mvInteractor(const mvInteractor&);
  mvInteractor& operator=(const mvInteractor&);

private:
  State m_state;
  ToolType *m_tool;

  Vrui::NavTrackerState m_current;
  Vrui::NavTrackerState m_delta;
};

#endif // MVINTERACTOR_H
