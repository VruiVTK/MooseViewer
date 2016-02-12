#ifndef MVFRAMERATE_H
#define MVFRAMERATE_H

#include "mvGLObject.h"

#include <Misc/Timer.h>

#include <vtkNew.h>

#include <vector>

class vtkTextActor;
class vtkTextProperty;

/**
 * @brief The mvFramerate class renders the framerate as a vtkTextActor.
 */
class mvFramerate : public mvGLObject
{
public:
  using Superclass = mvGLObject;

  struct DataItem : public Superclass::DataItem
  {
    DataItem();
    vtkNew<vtkTextActor> actor;
  };

  mvFramerate();
  ~mvFramerate();

  // mvGLObjectAPI:
  void initMvContext(mvContextState &mvContext,
                     GLContextData &contextData) const override;
  void syncApplicationState(const mvApplicationState &state) override;
  void syncContextState(const mvApplicationState &appState,
                        const mvContextState &contextState,
                        GLContextData &contextData) const override;

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

  vtkNew<vtkTextProperty> m_tprop;
  bool m_visible;
};

#endif // MVFRAMERATE_H
