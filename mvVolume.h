#ifndef MVVOLUME_H
#define MVVOLUME_H

#include "mvAsyncGLObject.h"

#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <string>

class vtkCellDataToPointData;
class vtkCheckerboardSplatter;
class vtkColorTransferFunction;
class vtkDataObject;
class vtkPiecewiseFunction;
class vtkSmartVolumeMapper;
class vtkVolume;
class vtkVolumeProperty;

/**
 * @brief The mvVolume class renders the dataset as a volume.
 */
class mvVolume : public mvAsyncGLObject
{
public:
  typedef mvAsyncGLObject Superclass;

  struct DataItem : public Superclass::DataItem
  {
    DataItem();

    // Rendering pipeline:
    vtkNew<vtkSmartVolumeMapper> mapper;
    vtkNew<vtkVolume> actor;
  };

  mvVolume();
  ~mvVolume();

  // mvGLObjectAPI:
  void initMvContext(mvContextState &mvContext,
                     GLContextData &contextData) const override;
  void syncContextState(const mvApplicationState &appState,
                        const mvContextState &contextState,
                        GLContextData &contextData) const override;

  /**
   * Toggle visibility of the props on/off.
   */
  bool visible() const { return m_visible; }
  void setVisible(bool visible) { m_visible = visible; }

  /**
   * The requested render mode (see vtkSmartVolumeMapper).
   */
  int requestedRenderMode() const;
  void setRequestedRenderMode(int mode);

  double splatRadius() const;
  void setSplatRadius(double splatRadius);

  double splatExponent() const;
  void setSplatExponent(double splatExponent);

  double splatDimensions() const;
  void setSplatDimensions(double splatDimensions);

private: // mvAsyncGLObject virtual API:
  void configureDataPipeline(const mvApplicationState &state) override;
  bool dataPipelineNeedsUpdate() const override;
  void executeDataPipeline() const override;
  void retrieveDataPipelineResult() override;

private:
  // Not implemented -- disable copy:
  mvVolume(const mvVolume&);
  mvVolume& operator=(const mvVolume&);

private:
  // Data pipeline:
  vtkNew<vtkCellDataToPointData> m_cell2point;
  vtkNew<vtkCheckerboardSplatter> m_splat;

  // Renderable data:
  vtkSmartPointer<vtkDataObject> m_appData;

  // Rendering configuration
  vtkNew<vtkColorTransferFunction> m_colorFunction;
  vtkNew<vtkPiecewiseFunction> m_opacityFunction;
  vtkNew<vtkVolumeProperty> m_volumeProperty;

  int m_requestedRenderMode;
  bool m_visible;

  double m_splatDimensions;
  double m_splatExponent;
  double m_splatRadius;
};

#endif // MVVOLUME_H
