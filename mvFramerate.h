#ifndef MVFRAMERATE_H
#define MVFRAMERATE_H

#include "mvGLObject.h"

#include <Misc/Timer.h>

#include <vtkNew.h>

#include <vector>

class vtkTextActor;

/**
 * @brief The mvFramerate class renders the framerate as a vtkTextActor.
 */
class mvFramerate : public mvGLObject
{
public:
  struct DataItem : public mvGLObject::DataItem
  {
    DataItem();
    vtkNew<vtkTextActor> actor;
  };

  mvFramerate();
  ~mvFramerate();

  // mvGLObjectAPI:
  void initContext(GLContextData &contextData) const;
  void initMvContext(mvContextState &mvContext,
                     GLContextData &contextData) const;
  void syncApplicationState(const mvApplicationState &state);
  void syncContextState(const mvApplicationState &appState,
                        const mvContextState &contextState,
                        GLContextData &contextData) const;

  /**
   * Toggle visibility of the contour props on/off.
   */
  bool visible() const { return m_visible; }
  void setVisible(bool visible) { m_visible = visible; }

private:
  // Not implemented -- disable copy:
  mvFramerate(const mvFramerate&);
  mvFramerate& operator=(const mvFramerate&);

private:
  Misc::Timer m_timer;
  std::vector<double> m_times;

  bool m_visible;
};

#endif // MVFRAMERATE_H
