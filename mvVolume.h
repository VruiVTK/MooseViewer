#ifndef MVVOLUME_H
#define MVVOLUME_H

#include "vvLODAsyncGLObject.h"

#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <string>

class vtkColorTransferFunction;
class vtkDataObject;
class vtkPiecewiseFunction;
class vtkResampleToImage;
class vtkSmartVolumeMapper;
class vtkVolume;
class vtkVolumeProperty;

/**
 * @brief The mvVolume class renders the dataset as a volume.
 */
class mvVolume : public vvLODAsyncGLObject
{
public:
  using Superclass = vvLODAsyncGLObject;

  // Volume state: -------------------------------------------------------------
  struct VolumeState : public Superclass::ObjectState
  {
    VolumeState();
    void update(const vvApplicationState &state) override {}
    int renderMode;
    bool visible;

    int dimension;
  };

  // LoRes LOD: ----------------------------------------------------------------
  // Just render the reduced dataset as-is.
  struct LoResDataPipeline : public Superclass::DataPipeline
  {
    vtkSmartPointer<vtkDataObject> reducedDataObject;

    // pipeline is a no-op
    bool forceSynchronousUpdates() const override { return true; }
    void configure(const ObjectState &, const vvApplicationState &) override;
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
              vvContextState &contextState) override;
    void update(const ObjectState &objState,
                const vvApplicationState &appState,
                const vvContextState &contextState,
                const LODData &result) override;
    void disable();
  };

  // HiRes LOD: ----------------------------------------------------------------
  // Create a higher quality volume from the full dataset.
  struct HiResDataPipeline : public Superclass::DataPipeline
  {
    vtkNew<vtkResampleToImage> filter;

    HiResDataPipeline();
    void configure(const ObjectState &objState,
                   const vvApplicationState &appState) override;
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

  double dimension() const;
  void setDimension(double d);

private: // vvLODAsyncGLObject virtual API:
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
