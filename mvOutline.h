#ifndef MVOUTLINE_H
#define MVOUTLINE_H

#include "mvGLObject.h"

#include <vtkNew.h>
#include <vtkSmartPointer.h>

class vtkActor;
class vtkCompositePolyDataMapper;
class vtkDataObject;
class vtkOutlineFilter;

/**
 * @brief The mvOutline class renders an outline of the dataset bounds.
 */
class mvOutline : public mvGLObject
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

  mvOutline();
  ~mvOutline();

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

private:
  // Not implemented -- disable copy:
  mvOutline(const mvOutline&);
  mvOutline& operator=(const mvOutline&);

private:
  vtkNew<vtkOutlineFilter> m_filter;

  bool m_visible;
};

#endif // MVOUTLINE_H
