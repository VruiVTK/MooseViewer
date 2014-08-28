#ifndef _DATAITEM_H
#define _DATAITEM_H

// MooseViewer includes
#include "MooseViewer.h"

// VTK includes
#include <vtkSmartPointer.h>

// Forward declarations
class ExternalVTKWidget;
class vtkActor;
class vtkAppendPolyData;
class vtkCompositeDataGeometryFilter;
class vtkContourFilter;
class vtkLight;
class vtkLookupTable;
class vtkPolyDataMapper;

struct MooseViewer::DataItem : public GLObject::DataItem
{
/* Elements */
public:
  /* VTK components */
  vtkSmartPointer<ExternalVTKWidget> externalVTKWidget;
  vtkSmartPointer<vtkActor> actor;
  vtkSmartPointer<vtkActor> actorOutline;
  vtkSmartPointer<vtkCompositeDataGeometryFilter> compositeFilter;
  vtkSmartPointer<vtkPolyDataMapper> mapper;
  vtkSmartPointer<vtkLookupTable> lut;
  vtkSmartPointer<vtkLight> flashlight;

  vtkSmartPointer<vtkAppendPolyData> contours;
  vtkSmartPointer<vtkActor> contourActor;
  vtkSmartPointer<vtkPolyDataMapper> contourMapper;

  /* Constructor and destructor*/
  DataItem(void);
  virtual ~DataItem(void);
};

#endif //_DATAITEM_H
