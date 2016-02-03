#ifndef UNSTRUCTUREDCONTOUROBJECT_H
#define UNSTRUCTUREDCONTOUROBJECT_H

#include <GL/GLObject.h>

#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <string>
#include <vector>

class ArrayLocator;
class vtkActor;
class vtkCompositeDataSet;
class vtkCompositePolyDataMapper;
class vtkDataObject;
class vtkLookupTable;
class vtkRenderer;
class vtkSMPContourGrid;
class vtkSpanSpace;
class vtkUnstructuredGrid;

// GLObject implementation for contouring unstructured grids.
class UnstructuredContourObject : public GLObject
{
public:
  struct DataItem : public GLObject::DataItem
  {
    DataItem();

    // Input data object. This is a deep copy of the contour filter output.
    vtkSmartPointer<vtkDataObject> data;

    // Rendering objects
    vtkNew<vtkCompositePolyDataMapper> mapper;
    vtkNew<vtkActor> actor;
  };

  UnstructuredContourObject();
  ~UnstructuredContourObject();

  // Executes serially for now. Reuses the locator from MooseViewer::frame.
  // Eventually this should re-execute the data pipeline in a background thread,
  // only updating the rendering pipeline once the data is ready.
  void update(const ArrayLocator &locator);

  // GLObject API:
  void initContext(GLContextData &contextData) const;

  // Add any vtk props to the supplied renderer.
  void initRenderer(GLContextData &contextData, vtkRenderer *renderer) const;

  // Set the lookup table.
  void setLookupTable(GLContextData &contextData, vtkLookupTable *lut) const;

  // Sync the GLObject state to the contextData.
  void syncContext(GLContextData &contextData) const;

  void setInput(vtkUnstructuredGrid *grid);
  void setInput(vtkCompositeDataSet *cds);
  vtkDataObject* input() const;

  void setColorByArrayName(const std::string &n) { m_colorByArrayName = n; }
  const std::string& colorByArrayName() const { return m_colorByArrayName; }

  bool visible() const { return m_visible; }
  void setVisible(bool visible) { m_visible = visible; }

  std::vector<double> contourValues() const;
  void setContourValues(const std::vector<double> &contourValues);

private:
  // Not implemented -- disable copy:
  UnstructuredContourObject(const UnstructuredContourObject&);
  UnstructuredContourObject& operator=(const UnstructuredContourObject&);

private:
  // The input unstructured grid:
  vtkSmartPointer<vtkDataObject> m_input;

  // Contouring:
  vtkNew<vtkSpanSpace> m_scalarTree;
  vtkNew<vtkSMPContourGrid> m_contour;

  std::vector<double> m_contourValues;

  // Render settings:
  bool m_visible;
  std::string m_colorByArrayName;
};

#endif // UNSTRUCTUREDCONTOUROBJECT_H
