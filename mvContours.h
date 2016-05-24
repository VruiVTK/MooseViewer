#ifndef MVCONTOURS_H
#define MVCONTOURS_H

#include "vvLODAsyncGLObject.h"

#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <vector>

class vtkActor;
class vtkCompositeDataGeometryFilter;
class vtkDataObject;
class vtkFlyingEdges3D;
class vtkPolyDataMapper;
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
 * @todo Isovalues should use actual scalar range at some point.
 */
class mvContours : public vvLODAsyncGLObject
{
public:
  using Superclass = vvLODAsyncGLObject;

  // Contour State: ------------------------------------------------------------
  struct ContourState : public Superclass::ObjectState
  {
    void update(const vvApplicationState &state) override {}
    std::vector<double> contourValues;
    bool visible{false};
  };

  // LoRes LOD: ----------------------------------------------------------------
  // Use vtkFlyingEdges3D to quickly cut contours from the reduced dataset.
  struct LoResDataPipeline : public Superclass::DataPipeline
  {
    vtkNew<vtkFlyingEdges3D> contour;
    vtkNew<vtkCompositeDataGeometryFilter> geometry;

    LoResDataPipeline();
    void configure(const ObjectState &objState,
                   const vvApplicationState &appState) override;
    bool needsUpdate(const ObjectState &objState,
                     const LODData &result) const override;
    void execute() override;
    void exportResult(LODData &result) const override;
  };

  struct LoResLODData : public Superclass::LODData
  {
    vtkSmartPointer<vtkDataObject> contours;
  };

  struct LoResRenderPipeline : public Superclass::RenderPipeline
  {
    vtkNew<vtkPolyDataMapper> mapper;
    vtkNew<vtkActor> actor;

    LoResRenderPipeline();
    void init(const ObjectState &objState,
              vvContextState &contextState) override;
    void update(const ObjectState &objState,
                const vvApplicationState &appState,
                const vvContextState &contextState,
                const LODData &result) override;
    void disable();
  };

  // HiRes LOD: ----------------------------------------------------------------
  // Use vtkSMPContourGrid to cut contours from the full dataset.
  struct HiResDataPipeline : public Superclass::DataPipeline
  {
    vtkNew<vtkSMPContourGrid> contour;
    vtkNew<vtkCompositeDataGeometryFilter> geometry;

    HiResDataPipeline();
    void configure(const ObjectState &objState,
                   const vvApplicationState &appState) override;
    bool needsUpdate(const ObjectState &objState,
                     const LODData &result) const override;
    void execute() override;
    void exportResult(LODData &result) const override;
  };

  struct HiResLODData : public Superclass::LODData
  {
    vtkSmartPointer<vtkDataObject> contours;
  };

  struct HiResRenderPipeline : public Superclass::RenderPipeline
  {
    vtkNew<vtkPolyDataMapper> mapper;
    vtkNew<vtkActor> actor;

    HiResRenderPipeline();
    void init(const ObjectState &objState,
              vvContextState &contextState) override;
    void update(const ObjectState &objState,
                const vvApplicationState &appState,
                const vvContextState &contextState,
                const LODData &result) override;
    void disable();
  };

  // mvContours API: -----------------------------------------------------------
  mvContours();
  ~mvContours();

  /**
   * Toggle visibility of the contour props on/off.
   */
  bool visible() const { return this->objectState<ContourState>().visible; }
  void setVisible(bool v) { this->objectState<ContourState>().visible = v;}

  /**
   * The scalar values to generate isosurfaces from. Note that these contour
   * values are specified in the range [0, 255], and are mapped to the current
   * scalar array's data range prior to contouring.
   */
  std::vector<double> contourValues() const;
  void setContourValues(const std::vector<double> &contourValues);

private: // vvLODAsyncGLObject API:

  std::string progressLabel() const override { return "Contours"; }

  ObjectState* createObjectState() const override;
  DataPipeline* createDataPipeline(LevelOfDetail lod) const override;
  RenderPipeline* createRenderPipeline(LevelOfDetail lod) const override;
  LODData* createLODData(LevelOfDetail lod) const override;

private:
  // Not implemented -- disable copy:
  mvContours(const mvContours&);
  mvContours& operator=(const mvContours&);
};

//------------------------------------------------------------------------------
inline std::vector<double> mvContours::contourValues() const
{
  return this->objectState<ContourState>().contourValues;
}

//------------------------------------------------------------------------------
inline void mvContours::setContourValues(const std::vector<double> &cV)
{
  this->objectState<ContourState>().contourValues = cV;
}

#endif // MVCONTOURS_H
