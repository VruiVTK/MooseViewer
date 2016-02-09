#ifndef MVCONTEXTSTATE_H
#define MVCONTEXTSTATE_H

#include <GL/GLObject.h>

#include <vtkNew.h>

class ExternalVTKWidget;
class vtkExternalOpenGLRenderer;

/**
 * @brief The mvContextState class holds shared context state.
 *
 * mvContextState holds per-context state that is shared between mvGLObjects.
 */
class mvContextState : public GLObject::DataItem
{
public:
  mvContextState();
  ~mvContextState();

  // These aren't const-correct bc VTK is not const-correct.
  vtkExternalOpenGLRenderer& renderer() const { return *m_renderer.Get(); }
  ExternalVTKWidget& widget() const { return *m_widget.Get(); }

private:
  vtkNew<vtkExternalOpenGLRenderer> m_renderer;
  vtkNew<ExternalVTKWidget> m_widget;
};

#endif // MVCONTEXTSTATE_H
