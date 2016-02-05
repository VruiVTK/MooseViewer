#ifndef MVCONTEXTSTATE_H
#define MVCONTEXTSTATE_H

#include <GL/GLObject.h>

// VTK includes
#include <vtkNew.h>

// Forward declarations
class ExternalVTKWidget;
class vtkActor;
class vtkCompositeDataGeometryFilter;
class vtkCompositePolyDataMapper;
class vtkExternalOpenGLRenderer;
class vtkPolyDataMapper;

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

  // These need to be refactored into mvGLObject context data.
  vtkNew<vtkActor> actor;
  vtkNew<vtkCompositeDataGeometryFilter> compositeFilter;
  vtkNew<vtkPolyDataMapper> mapper;

private:
  // VTK components
  vtkNew<vtkExternalOpenGLRenderer> m_renderer;
  vtkNew<ExternalVTKWidget> m_widget;
};

#endif // MVCONTEXTSTATE_H
