#ifndef MVGEOMETRY_H
#define MVGEOMETRY_H

#include "mvGLObject.h"

#include <vtkNew.h>
#include <vtkSmartPointer.h>

class vtkActor;
class vtkCompositeDataGeometryFilter;
class vtkDataObject;
class vtkPolyDataMapper;

/**
 * @brief The mvGeometry class renders the dataset as polydata.
 */
class mvGeometry : public mvGLObject
{
public:
  struct DataItem : public mvGLObject::DataItem
  {
    DataItem();

    // Renderable data:
    vtkSmartPointer<vtkDataObject> data;

    // Rendering pipeline:
    vtkNew<vtkPolyDataMapper> mapper;
    vtkNew<vtkActor> actor;
  };

  enum Representation
    {
    NoGeometry,
    Points,
    Wireframe,
    Surface,
    SurfaceWithEdges
    };

  mvGeometry();
  ~mvGeometry();

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

  /**
   * The actor's opacity.
   */
  double opacity() const;
  void setOpacity(double opacity);

  /**
   * The polydata representation to use.
   */
  Representation representation() const;
  void setRepresentation(Representation representation);

private:
  // Not implemented -- disable copy:
  mvGeometry(const mvGeometry&);
  mvGeometry& operator=(const mvGeometry&);

private:
  vtkNew<vtkCompositeDataGeometryFilter> m_filter;

  double m_opacity;
  Representation m_representation;
  bool m_visible;
};

#endif // MVGEOMETRY_H
