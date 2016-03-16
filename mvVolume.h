#ifndef MVVOLUME_H
#define MVVOLUME_H

#include "mvLODAsyncGLObject.h"

#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <string>

class vtkColorTransferFunction;
class vtkDataObject;
class vtkGaussianKernel;
class vtkImageData;
class vtkPiecewiseFunction;
class vtkPointInterpolator;
class vtkSmartVolumeMapper;
class vtkVolume;
class vtkVolumeProperty;

/**
 * @brief The mvVolume class renders the dataset as a volume.
 */
class mvVolume : public mvLODAsyncGLObject
{
public:
  using Superclass = mvLODAsyncGLObject;

  // Volume state: -------------------------------------------------------------
  struct VolumeState : public Superclass::ObjectState
  {
    VolumeState();
    void update(const mvApplicationState &state) override {}
    int renderMode;
    bool visible;

    int dimension;
    double radius;
    double sharpness;
  };

  // LoRes LOD: ----------------------------------------------------------------
  // Just render the reduced dataset as-is.
  struct LoResDataPipeline : public Superclass::DataPipeline
  {
    vtkSmartPointer<vtkDataObject> reducedDataObject;

    // pipeline is a no-op
    bool forceSynchronousUpdates() const override { return true; }
    void configure(const ObjectState &, const mvApplicationState &) override;
    bool needsUpdate(const ObjectState &objState,
                     const LODData &result) const override;
    void execute() override {}
    void exportResult(LODData &result) const override;
  };

  // LODData is shared by both LoRes and HiRes LODs.
  struct VolumeLODData : public Superclass::LODData
  {
    vtkSmartPointer<vtkDataObject> volume;
  };

  // RenderPipeline is shared by both LoRes and HiRes LODs.
  struct VolumeRenderPipeline : public Superclass::RenderPipeline
  {
    vtkNew<vtkColorTransferFunction> color;
    vtkNew<vtkPiecewiseFunction> opacity;
    vtkNew<vtkVolumeProperty> property;

    vtkNew<vtkSmartVolumeMapper> mapper;
    vtkNew<vtkVolume> actor;

    VolumeRenderPipeline();
    void init(const ObjectState &objState,
              mvContextState &contextState) override;
    void update(const ObjectState &objState,
                const mvApplicationState &appState,
                const mvContextState &contextState,
                const LODData &result) override;
    void disable();
  };

  // HiRes LOD: ----------------------------------------------------------------
  // Create a higher quality volume from the full dataset.
  struct HiResDataPipeline : public Superclass::DataPipeline
  {
    vtkNew<vtkImageData> seed;
    vtkNew<vtkGaussianKernel> kernel;
    vtkNew<vtkPointInterpolator> filter;

    HiResDataPipeline();
    void configure(const ObjectState &objState,
                   const mvApplicationState &appState) override;
    bool needsUpdate(const ObjectState &objState,
                     const LODData &result) const override;
    void execute() override;
    void exportResult(LODData &result) const override;
  };

  // mvVolumeAPI:
  mvVolume();
  ~mvVolume();

  /**
   * Toggle visibility of the props on/off.
   */
  bool visible() const { return this->objectState<VolumeState>().visible; }
  void setVisible(bool v) { this->objectState<VolumeState>().visible = v; }

  /**
   * The requested render mode (see vtkSmartVolumeMapper).
   */
  int renderMode() const;
  void setRenderMode(int mode);

  /**
   * The radius for point interpolation into the volume.
   * Setting to 0.0 will use the norm of the volume's Spacing vector.
   */
  double radius() const;
  void setRadius(double radius);

  double sharpness() const;
  void setSharpness(double s);

  double dimension() const;
  void setDimension(double d);

private: // mvLODAsyncGLObject virtual API:
  std::string progressLabel() const override { return "Volume"; }

  ObjectState* createObjectState() const override;
  DataPipeline* createDataPipeline(LevelOfDetail lod) const override;
  RenderPipeline* createRenderPipeline(LevelOfDetail lod) const override;
  LODData* createLODData(LevelOfDetail lod) const override;

private:
  // Not implemented -- disable copy:
  mvVolume(const mvVolume&);
  mvVolume& operator=(const mvVolume&);
};

#endif // MVVOLUME_H
