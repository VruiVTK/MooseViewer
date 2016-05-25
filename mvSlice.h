#ifndef MVSLICE_H
#define MVSLICE_H

#include "vvLODAsyncGLObject.h"

#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <vector>

class vtkActor;
class vtkCompositePolyDataMapper;
class vtkCutter;
class vtkDataObject;
class vtkFlyingEdgesPlaneCutter;
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
class mvSlice : public vvLODAsyncGLObject
{
public:
  using Superclass = vvLODAsyncGLObject;

  struct Plane
  {
    std::array<double, 3> normal{{1., 1., 1.}};
    std::array<double, 3> origin{{0., 0., 0.}};
  };

  // Slice state: --------------------------------------------------------------
  struct SliceState : public Superclass::ObjectState
  {
    void update(const vvApplicationState &appState) override;
    Plane plane;
    bool visible{false};
  };

  // Hint LOD: -----------------------------------------------------------------
  // Renders a simple plane cut from the dataset's boundaries.
  // No scalar lookups.
  struct HintDataPipeline : public Superclass::DataPipeline
  {
    vtkNew<vtkPlane> plane;
    vtkNew<vtkImageData> box;
    vtkNew<vtkCutter> cutter;

    HintDataPipeline();

    // Always show something -- the hint is cheap enough to get away with this.
    bool forceSynchronousUpdates() const override { return true; }

    void configure(const ObjectState &objState,
                   const vvApplicationState &appState) override;
    bool needsUpdate(const ObjectState &objState,
                     const LODData &result) const override;
    void execute() override;
    void exportResult(LODData &result) const override;
  };

  struct HintLODData : public Superclass::LODData
  {
    vtkSmartPointer<vtkDataObject> slice;
  };

  struct HintRenderPipeline : public Superclass::RenderPipeline
  {
    vtkNew<vtkPolyDataMapper> mapper;
    vtkNew<vtkActor> actor;

    HintRenderPipeline();
    void init(const ObjectState &objState,
              vvContextState &contextState) override;
    void update(const ObjectState &objState,
                const vvApplicationState &appState,
                const vvContextState &contextState,
                const LODData &result) override;
    void disable();
  };

  // LoRes LOD: ----------------------------------------------------------------
  // Uses vtkFlyingEdgesPlaneCutter to quickly extract a slice of the reduced
  // dataset.
  struct LoResDataPipeline : public Superclass::DataPipeline
  {
    vtkNew<vtkPlane> plane;
    vtkNew<vtkFlyingEdgesPlaneCutter> cutter;

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
    vtkSmartPointer<vtkDataObject> slice;
  };

  struct LoResRenderPipeline : public Superclass::RenderPipeline
  {
    vtkNew<vtkCompositePolyDataMapper> mapper;
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
  // Uses vtkSMPContourGrid to cut a slice from the full dataset.
  struct HiResDataPipeline : public Superclass::DataPipeline
  {
    vtkNew<vtkPlane> plane;
    vtkNew<vtkSampleImplicitFunctionFilter> addPlane;
    vtkNew<vtkSMPContourGrid> cutter;

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
    vtkSmartPointer<vtkDataObject> slice;
  };

  struct HiResRenderPipeline : public Superclass::RenderPipeline
  {
    vtkNew<vtkCompositePolyDataMapper> mapper;
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

  // mvSlice API: --------------------------------------------------------------
  mvSlice();
  ~mvSlice();

  /**
   * Toggle visibility of the slice on/off.
   */
  bool visible() const;
  void setVisible(bool visible);

  /**
   * The slice plane.
   */
  const Plane& plane() const;
  void setPlane(const Plane &p);

private: // vvLODAsyncGLObject virtual API:

  std::string progressLabel() const { return "Slice"; }

  ObjectState* createObjectState() const override;
  DataPipeline* createDataPipeline(LevelOfDetail lod) const override;
  RenderPipeline* createRenderPipeline(LevelOfDetail lod) const override;
  LODData* createLODData(LevelOfDetail lod) const override;

private:
  // Not implemented -- disable copy:
  mvSlice(const mvSlice&);
  mvSlice& operator=(const mvSlice&);
};

//------------------------------------------------------------------------------
inline bool mvSlice::visible() const
{
  return this->objectState<SliceState>().visible;
}

//------------------------------------------------------------------------------
inline void mvSlice::setVisible(bool v)
{
  this->objectState<SliceState>().visible = v;
}

//------------------------------------------------------------------------------
inline const mvSlice::Plane &mvSlice::plane() const
{
  return this->objectState<SliceState>().plane;
}

//------------------------------------------------------------------------------
inline void mvSlice::setPlane(const mvSlice::Plane &p)
{
  this->objectState<SliceState>().plane = p;
}

#endif // MVSLICES_H
