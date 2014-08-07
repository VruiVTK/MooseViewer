#ifndef _MOOSEVIEWER_H
#define _MOOSEVIEWER_H

// OpenGL/Motif includes
#include <GL/gl.h>

// Vrui includes
#include <GL/GLObject.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/Slider.h>
#include <GLMotif/TextField.h>
#include <GLMotif/ToggleButton.h>
#include <Vrui/Application.h>

// VTK includes
#include <vtkSmartPointer.h>

// STL includes
#include <vector>
#include <string>

/* Forward Declarations */
namespace GLMotif
{
  class Popup;
  class PopupMenu;
  class SubMenu;
}

class BaseLocator;
class ClippingPlane;
class ExternalVTKWidget;
class vtkActor;
class vtkCompositeDataGeometryFilter;
class vtkExodusIIReader;
class vtkLight;

class MooseViewer:public Vrui::Application,public GLObject
{
/* Embedded classes: */
  typedef std::vector<BaseLocator*> BaseLocatorList;
private:
  struct DataItem : public GLObject::DataItem
  {
  /* Elements */
  public:
    /* VTK components */
    vtkSmartPointer<ExternalVTKWidget> externalVTKWidget;
    vtkSmartPointer<vtkActor> actor;
    vtkSmartPointer<vtkActor> actorOutline;
    vtkSmartPointer<vtkCompositeDataGeometryFilter> compositeFilter;
    vtkSmartPointer<vtkLight> flashlight;

    /* Constructor and destructor*/
    DataItem(void);
    virtual ~DataItem(void);
  };

  /* vtkExodusIIReader */
  vtkSmartPointer<vtkExodusIIReader> reader;

  /* Elements: */
  GLMotif::PopupMenu* mainMenu; // The program's main menu
  GLMotif::PopupMenu* createMainMenu(void);
  GLMotif::Popup* createRepresentationMenu(void);
  GLMotif::Popup* createAnalysisToolsMenu(void);
  GLMotif::Popup* createVariablesMenu(void);
  GLMotif::Popup* createColorByVariablesMenu(void);
  GLMotif::PopupWindow* renderingDialog;
  GLMotif::PopupWindow* createRenderingDialog(void);
  GLMotif::TextField* opacityValue;

  /* Update the menus */
  void updateVariablesMenu(void);
  void updateColorByVariablesMenu(void);

  /* Variables submenu */
  GLMotif::SubMenu* variablesMenu;
  GLMotif::SubMenu* colorByVariablesMenu;

  /* Variables vector */
  std::vector<std::string> variables;

  /* Name of file to load */
  char* FileName;

  /* Opacity value */
  double Opacity;

  /* Representation Type */
  int RepresentationType;

  /* bounds */
  double* DataBounds;

  /* First Frame */
  bool FirstFrame;

  /* Data Center */
  Vrui::Point Center;

  /* Data Radius  */
  Vrui::Scalar Radius;

  BaseLocatorList baseLocators;

  /* Analysis Tools */
  int analysisTool;

  /* Clipping Plane */
  ClippingPlane * ClippingPlanes;
  int NumberOfClippingPlanes;

  /* Flashlight position and direction */
  int * FlashlightSwitch;
  double * FlashlightPosition;
  double * FlashlightDirection;

  /* Outline visible */
  bool Outline;

  /* Constructors and destructors: */
public:
  MooseViewer(int& argc,char**& argv);
  virtual ~MooseViewer(void);

  /* Methods to set/get the filename to read */
  void setFileName(const char* name);
  const char* getFileName(void);

  /* Clipping Planes */
  ClippingPlane * getClippingPlanes(void);
  int getNumberOfClippingPlanes(void);

  /* Get Flashlight position and direction */
  int * getFlashlightSwitch(void);
  double * getFlashlightPosition(void);
  double * getFlashlightDirection(void);

  /* Methods to manage render context */
  virtual void initContext(GLContextData& contextData) const;
  virtual void display(GLContextData& contextData) const;
  virtual void frame(void);

  /* Callback methods */
  void centerDisplayCallback(Misc::CallbackData* cbData);
  void opacitySliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
  void changeRepresentationCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void showRenderingDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void changeAnalysisToolsCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void changeVariablesCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void changeColorByVariablesCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);

  virtual void toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData);
  virtual void toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData);
};

#endif //_MOOSEVIEWER_H
