#ifndef MVOUTLINE_H
#define MVOUTLINE_H

#include "vvAsyncGLObject.h"

#include <vtkNew.h>
#include <vtkSmartPointer.h>

class vtkActor;
class vtkCompositePolyDataMapper;
class vtkDataObject;
class vtkOutlineFilter;

/**
 * @brief The mvOutline class renders an outline of the dataset bounds.
 */
class mvOutline : public vvAsyncGLObject
{
public:
  using Superclass = vvAsyncGLObject;

  struct DataItem : public Superclass::DataItem
  {
    DataItem();

    // Rendering pipeline:
    vtkNew<vtkCompositePolyDataMapper> mapper;
    vtkNew<vtkActor> actor;
  };

  mvOutline();
  ~mvOutline();

  // vvGLObjectAPI:
  void initVvContext(vvContextState &mvContext,
                     GLContextData &contextData) const override;
  void syncContextState(const vvApplicationState &appState,
                        const vvContextState &contextState,
                        GLContextData &contextData) const override;

  /**
   * Toggle visibility of the contour props on/off.
   */
  bool visible() const { return m_visible; }
  void setVisible(bool visible) { m_visible = visible; }

private: // vvAsyncGLObject virtual API:
  void configureDataPipeline(const vvApplicationState &state) override;
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
