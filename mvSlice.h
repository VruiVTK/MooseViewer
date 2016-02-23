#ifndef MVSLICE_H
#define MVSLICE_H

#include "mvAsyncGLObject.h"

#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <vector>

class vtkActor;
class vtkCompositePolyDataMapper;
class vtkCutter;
class vtkDataObject;
class vtkImageData;
class vtkPlane;
class vtkPolyDataMapper;
class vtkSampleImplicitFunctionFilter;
class vtkSMPContourGrid;

/**
 * @brief The mvSlice class implements dataset slicing.
 *
 * mvSlice renders a slice (see plane()) cut from the dataset.
 */
class mvSlice : public mvAsyncGLObject
{
public:
  using Superclass = mvAsyncGLObject;

  struct Plane
  {
    Plane() : normal{ 0., 0., 1. }, origin{ 0., 0., 0. } {}
    std::array<double, 3> normal;
    std::array<double, 3> origin;
  };

  struct DataItem : public Superclass::DataItem
  {
    DataItem();

    // Interaction hint:
    vtkNew<vtkPolyDataMapper> hintMapper;
    vtkNew<vtkActor> hintActor;

    // Rendering pipeline:
    vtkNew<vtkCompositePolyDataMapper> mapper;
    vtkNew<vtkActor> actor;
  };

  mvSlice();
  ~mvSlice();

  // mvGLObjectAPI:
  void initMvContext(mvContextState &mvContext,
                     GLContextData &contextData) const override;
  void syncContextState(const mvApplicationState &appState,
                        const mvContextState &contextState,
                        GLContextData &contextData) const override;

  /**
   * Toggle visibility of the slice on/off.
   */
  bool visible() const { return m_visible; }
  void setVisible(bool visible) { m_visible = visible; }

  /**
   * The slice plane.
   */
  const Plane& plane() const { return m_plane; }
  void setPlane(const Plane &p) { m_plane = p; }

private: // mvAsyncGLObject virtual API:
  void frame(const mvApplicationState &state) override;
  void configureDataPipeline(const mvApplicationState &state) override;
  bool dataPipelineNeedsUpdate() const override;
  void executeDataPipeline() const override;
  void retrieveDataPipelineResult() override;
  std::string progressLabel() const override { return "Updating Slice"; }

private:
  // Not implemented -- disable copy:
  mvSlice(const mvSlice&);
  mvSlice& operator=(const mvSlice&);

private:
  // Hint cutter -- fast:
  vtkNew<vtkPlane> m_hintFunction;
  vtkNew<vtkImageData> m_hintBox;
  vtkNew<vtkCutter> m_hintCutter;

  // Data pipeline:
  vtkNew<vtkPlane> m_function;
  vtkNew<vtkSampleImplicitFunctionFilter> m_addPlane;
  vtkNew<vtkSMPContourGrid> m_cutter;

  // Renderable data:
  vtkSmartPointer<vtkDataObject> m_appData;

  // Contour values:
  Plane m_plane;

  // Render settings:
  bool m_visible;
};

#endif // MVSLICES_H
