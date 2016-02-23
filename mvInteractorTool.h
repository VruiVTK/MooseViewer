#ifndef MVINTERACTORTOOL_H
#define MVINTERACTORTOOL_H

#include <Vrui/Tool.h>

#include <Geometry/OrthogonalTransformation.h>

/**
 * @brief The mvInteractorTool class is a Vrui::Tool tightly coupled with
 * mvInteractor.
 *
 * This tool is used to retrieve object translation and rotation requests from
 * the user. It is a 2-button 6-DOF tool. The first button is used for
 * translation and the second is for rotation.
 *
 * When used in a desktop environment, "transformer tools" are needed to
 * convert 2D mouse positions into 6-DOF states. An example configuration that
 * maps the keyboard '1' key to translate and '2' to rotate is:
 *
 * @verbatim
 * # Use 3D position in plane for translation info:
 * section mvInteractorMouseScreen
 *   toolClass MouseTool
 *   bindings ((Mouse, 1))
 * endsection
 *
 * # Compute rotation using mouse position:
 * section mvInteractorMouseRotation
 *   toolClass mvMouseRotationTool
 *   bindings ((Mouse, 2))
 * endsection
 *
 * # Use the above transformer tools to setup the 6-DOF interactions:
 * section mvInteractorToolInstance
 *   toolClass mvInteractorTool
 *   bindings ((mvInteractorMouseScreen, Button0), \
 *             (mvInteractorMouseRotation, Button0))
 * endsection
 * @endverbatim
 *
 * When 6-DOF input devices are available, they may be using directly, just use
 * the mvInteractorTool with the proper bindings.
 */
class mvInteractorTool : public Vrui::Tool
{
public: // Callback data classes:

  // CallbackData for when the input tracker moves:
  class MoveEvent : public Misc::CallbackData
  {
  public:
    MoveEvent(mvInteractorTool *t, const Vrui::NavTrackerState &tState)
      : tool(t), transform(tState) {}

    mvInteractorTool *tool;
    const Vrui::NavTrackerState &transform;
  };

  // CallbackData for button press
  class PressEvent : public Misc::CallbackData
  {
  public:
    PressEvent(mvInteractorTool *t, int bSlot,
               const Vrui::NavTrackerState &tState)
      : tool(t), button(bSlot), transform(tState) {}

    mvInteractorTool *tool;
    int button;
    const Vrui::NavTrackerState &transform;
  };

  // CallbackData for button release
  class ReleaseEvent : public Misc::CallbackData
  {
  public:
    ReleaseEvent(mvInteractorTool *t, int bSlot,
                 const Vrui::NavTrackerState &tState)
      : tool(t), button(bSlot), transform(tState) {}

    mvInteractorTool *tool;
    int button;
    const Vrui::NavTrackerState &transform;
  };

public: // API:

  mvInteractorTool(const Vrui::ToolFactory *f,
                   const Vrui::ToolInputAssignment &iA);

  const Vrui::ToolFactory* getFactory() const override;

  Misc::CallbackList& getMoveCallbacks() { return m_moveCallbacks; }
  Misc::CallbackList& getPressCallbacks() { return m_pressCallbacks; }
  Misc::CallbackList& getReleaseCallbacks() { return m_releaseCallbacks; }

protected:

  Misc::CallbackList m_moveCallbacks;
  Misc::CallbackList m_pressCallbacks;
  Misc::CallbackList m_releaseCallbacks;

  const Vrui::ToolFactory *m_factory;

protected: // Internal API:

  // Returns a new tracker state combining the translation for button 0 and the
  // rotation from button 1.
  Vrui::NavTrackerState trackerState();

public: // Emit events from callback handlers/frame:

  void buttonCallback(int slot,
                      Vrui::InputDevice::ButtonCallbackData *cbd) override;
  void frame() override;
};

// ToolFactory for mvInteractorTool.
class mvInteractorToolFactory: public Vrui::ToolFactory
{
public:
  explicit mvInteractorToolFactory(Vrui::ToolManager &mgr);
  const char* getName() const override;
  const char* getButtonFunction(int slot) const override;
  Vrui::Tool* createTool(const Vrui::ToolInputAssignment &ia) const override;
  void destroyTool(Vrui::Tool *tool) const override;
};

#endif // MVINTERACTORTOOL_H
