#ifndef MVOUTLINE_H
#define MVOUTLINE_H

#include "mvAsyncGLObject.h"

#include <vtkNew.h>
#include <vtkSmartPointer.h>

class vtkActor;
class vtkCompositePolyDataMapper;
class vtkDataObject;
class vtkOutlineFilter;

/**
 * @brief The mvOutline class renders an outline of the dataset bounds.
 */
class mvOutline : public mvAsyncGLObject
{
public:
  typedef mvAsyncGLObject Superclass;

  struct DataItem : public Superclass::DataItem
  {
    DataItem();

    // Rendering pipeline:
    vtkNew<vtkCompositePolyDataMapper> mapper;
    vtkNew<vtkActor> actor;
  };

  mvOutline();
  ~mvOutline();

  // mvGLObjectAPI:
  void initMvContext(mvContextState &mvContext,
                     GLContextData &contextData) const override;
  void syncContextState(const mvApplicationState &appState,
                        const mvContextState &contextState,
                        GLContextData &contextData) const override;

  /**
   * Toggle visibility of the contour props on/off.
   */
  bool visible() const { return m_visible; }
  void setVisible(bool visible) { m_visible = visible; }

private: // mvAsyncGLObject virtual API:
  void configureDataPipeline(const mvApplicationState &state) override;
  bool dataPipelineNeedsUpdate() const override;
  void executeDataPipeline() const override;
  void retrieveDataPipelineResult() override;
  std::string progressLabel() const override { return "Updating Outline"; }

private:
  // Not implemented -- disable copy:
  mvOutline(const mvOutline&);
  mvOutline& operator=(const mvOutline&);

private:
  // Data pipeline:
  vtkNew<vtkOutlineFilter> m_filter;

  // Renderable data:
  vtkSmartPointer<vtkDataObject> m_appData;

  bool m_visible;
};

#endif // MVOUTLINE_H
