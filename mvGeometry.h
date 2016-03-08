#ifndef MVGEOMETRY_H
#define MVGEOMETRY_H

#include "mvLODAsyncGLObject.h"

#include <vtkNew.h>
#include <vtkSmartPointer.h>

class vtkActor;
class vtkCompositeDataGeometryFilter;
class vtkDataObject;
class vtkPolyDataMapper;

/**
 * @brief The mvGeometry class renders the dataset as polydata.
 */
class mvGeometry : public mvLODAsyncGLObject
{
public:
  using Superclass = mvLODAsyncGLObject;

  enum Representation
    {
    NoGeometry,
    Points,
    Wireframe,
    Surface,
    SurfaceWithEdges
    };

  struct GeometryState : public Superclass::ObjectState
  {
    double opacity{1.};
    Representation representation{Surface};
    bool visible{true};

    void update(const mvApplicationState &state) override {}
  };

  // LoRes LOD: ----------------------------------------------------------------
  // Run vtkCompositeDataGeometryFilter on the reduced dataset:
  struct LoResDataPipeline : public Superclass::DataPipeline
  {
    vtkNew<vtkCompositeDataGeometryFilter> filter;

    // Returns the dataset to use. This is the only difference between the
    // LoRes and HiRes pipelines, so this should save some duplication.
    virtual vtkDataObject* input(const mvApplicationState &state) const;

    void configure(const ObjectState &, const mvApplicationState &) override;
    bool needsUpdate(const ObjectState &objState,
                     const LODData &result) const override;
    void execute() override;
    void exportResult(LODData &result) const override;
  };

  // HiRes LOD: ----------------------------------------------------------------
  // Run vtkCompositeDataGeometryFilter on the full dataset:
  struct HiResDataPipeline : public LoResDataPipeline
  {
    vtkDataObject* input(const mvApplicationState &state) const override;
  };

  // Shared: LODData and RenderPipeline are shared between LoRes and HiRes. ----
  struct GeometryLODData : public Superclass::LODData
  {
    vtkSmartPointer<vtkDataObject> geometry;
  };

  struct GeometryRenderPipeline : public Superclass::RenderPipeline
  {
    vtkNew<vtkPolyDataMapper> mapper;
    vtkNew<vtkActor> actor;

    void init(const ObjectState &objState,
              mvContextState &contextState) override;
    void update(const ObjectState &objState,
                const mvApplicationState &appState,
                const mvContextState &contextState,
                const LODData &result) override;
    void disable();
  };

  // mvGeometry API ------------------------------------------------------------
  mvGeometry();
  ~mvGeometry();

  /**
   * Toggle visibility of the contour props on/off.
   */
  bool visible() const { return this->objectState<GeometryState>().visible; }
  void setVisible(bool v) { this->objectState<GeometryState>().visible = v; }

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

private: // mvAsyncGLObject virtual API:
  std::string progressLabel() const override { return "Geometry"; }

  ObjectState* createObjectState() const override;
  DataPipeline* createDataPipeline(LevelOfDetail lod) const override;
  RenderPipeline* createRenderPipeline(LevelOfDetail lod) const override;
  LODData* createLODData(LevelOfDetail lod) const override;

private:
  // Not implemented -- disable copy:
  mvGeometry(const mvGeometry&);
  mvGeometry& operator=(const mvGeometry&);
};

#endif // MVGEOMETRY_H
