#ifndef MVCONTOURS_H
#define MVCONTOURS_H

#include "mvGLObject.h"

#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <vector>

class vtkActor;
class vtkCompositePolyDataMapper;
class vtkDataObject;
class vtkSMPContourGrid;
class vtkSpanSpace;

/**
 * @brief The mvContours class implements contouring.
 *
 * mvContours renders a series of contours (see contourValues()) cut from the
 * current colorByArray using the application's colormap.
 *
 * Note that contour values are specified in the range [0, 255], and are mapped
 * to the current scalar array's data range prior to contouring.
 */
class mvContours : public mvGLObject
{
public:
  struct DataItem : public mvGLObject::DataItem
  {
    DataItem();

    // Renderable data:
    vtkSmartPointer<vtkDataObject> data;

    // Rendering pipeline:
    vtkNew<vtkCompositePolyDataMapper> mapper;
    vtkNew<vtkActor> actor;
  };

  mvContours();
  ~mvContours();

  // mvGLObjectAPI:
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

  /**
   * The scalar values to generate isosurfaces from. Note that these contour
   * values are specified in the range [0, 255], and are mapped to the current
   * scalar array's data range prior to contouring.
   */
  std::vector<double> contourValues() const;
  void setContourValues(const std::vector<double> &contourValues);

private:
  // Not implemented -- disable copy:
  mvContours(const mvContours&);
  mvContours& operator=(const mvContours&);

private:
  // Contouring:
  vtkNew<vtkSpanSpace> m_scalarTree;
  vtkNew<vtkSMPContourGrid> m_contour;

  // Contour values:
  std::vector<double> m_contourValues;

  // Render settings:
  bool m_visible;
};

#endif // MVCONTOURS_H
