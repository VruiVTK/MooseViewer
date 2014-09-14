#ifndef _MOOSEVIEWER_H
#define _MOOSEVIEWER_H

// OpenGL/Motif includes
#include <GL/gl.h>

// Vrui includes
#include <GL/GLObject.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RadioBox.h>
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

class AnimationDialog;
class BaseLocator;
class ClippingPlane;
class Contours;
class Isosurfaces;
class TransferFunction1D;
class vtkDataArray;
class vtkExodusIIReader;
class vtkLookupTable;

class MooseViewer:public Vrui::Application,public GLObject
{
/* Embedded classes: */
  typedef std::vector<BaseLocator*> BaseLocatorList;
private:
  struct DataItem;

  /* Elements: */
  GLMotif::PopupMenu* mainMenu; // The program's main menu
  GLMotif::PopupMenu* createMainMenu(void);
  GLMotif::Popup* createRepresentationMenu(void);
  GLMotif::Popup* createAnalysisToolsMenu(void);
  GLMotif::Popup* createVariablesMenu(void);
  GLMotif::Popup* createColorByVariablesMenu(void);
  GLMotif::Popup*  createColorMapSubMenu(void);
  GLMotif::PopupWindow* renderingDialog;
  GLMotif::PopupWindow* createRenderingDialog(void);
  GLMotif::TextField* opacityValue;

  /* Update the menus */
  void updateVariablesMenu(void);
  void updateColorByVariablesMenu(void);
  std::string getSelectedColorByArrayName(void) const;
  vtkSmartPointer<vtkDataArray> getSelectedArray(int & type) const;

  /* Variables submenu */
  GLMotif::SubMenu* variablesMenu;
  GLMotif::SubMenu* colorByVariablesMenu;

  /* Variables vector */
  std::vector<std::string> variables;

  /* Name of file to load */
  char* FileName;

  /* SmartVolumeMapper Requested RenderMode */
  int RequestedRenderMode;

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

  /* Color editor dialog */
  TransferFunction1D* ColorEditor;

  /* Color Transfer function */
  double * ColorMap;

  /* Animation dialog */
  AnimationDialog* AnimationControl;

  /* Draw histogram */
  float* Histogram;
  void updateHistogram(void);

  /* Contours dialog */
  Contours* ContoursDialog;
  bool ContourVisible;
  std::vector<double> ContourValues;

  /* Isosurfaces dialog */
  float aIsosurface;
  float bIsosurface;
  float cIsosurface;
  bool AIsosurface;
  bool BIsosurface;
  bool CIsosurface;
  double* IsosurfaceColormap;
  vtkSmartPointer<vtkLookupTable> isosurfaceLUT;
  Isosurfaces* isosurfacesDialog;

  /* Volume visible */
  bool Volume;
  GLMotif::TextField* sampleValue;
  GLMotif::TextField* radiusValue;
  GLMotif::TextField* exponentValue;
  double GaussianSplatterRadius;
  double GaussianSplatterExp;
  double GaussianSplatterDims;

  /* Constructors and destructors: */
public:
  MooseViewer(int& argc,char**& argv);
  virtual ~MooseViewer(void);

  /* vtkExodusIIReader */
  vtkSmartPointer<vtkExodusIIReader> reader;

  /* Methods to set/get the filename to read */
  void setFileName(const char* name);
  const char* getFileName(void);

  /* Animation */
  bool IsPlaying;
  bool Loop;

  /* Clipping Planes */
  ClippingPlane * getClippingPlanes(void);
  int getNumberOfClippingPlanes(void);

  /* Get Flashlight position and direction */
  int * getFlashlightSwitch(void);
  double * getFlashlightPosition(void);
  double * getFlashlightDirection(void);

  /* Methods to set/get the requested render mode */
  void setRequestedRenderMode(int mode);
  int getRequestedRenderMode(void) const;

  /* Contours */
  std::vector<double> getContourValues();
  void setContourVisible(bool visible);

  /* Histogram */
  float * getHistogram();

  /* Methods to manage render context */
  virtual void initContext(GLContextData& contextData) const;
  virtual void display(GLContextData& contextData) const;
  virtual void frame(void);

  /* Isosurface */
  void setIsosurfaceColorMapChanged(bool SliceColorMapChanged);
  void updateIsosurfaceColorMap(double* SliceColormap);
  void setAIsosurface(float aIsosurface);
  void setBIsosurface(float bIsosurface);
  void setCIsosurface(float cIsosurface);
  void showAIsosurface(bool AIsosurface);
  void showBIsosurface(bool BIsosurface);
  void showCIsosurface(bool CIsosurface);


  /* Callback methods */
  void centerDisplayCallback(Misc::CallbackData* cbData);
  void opacitySliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
  void sampleSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
  void radiusSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
  void exponentSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
  void changeRepresentationCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void showRenderingDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void showColorEditorDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void showContoursDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void showAnimationDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void showIsosurfacesDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void changeAnalysisToolsCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void changeVariablesCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void changeColorByVariablesCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void changeColorMapCallback(GLMotif::RadioBox::ValueChangedCallbackData* callBackData);
  void alphaChangedCallback(Misc::CallbackData* callBackData);
  void colorMapChangedCallback(Misc::CallbackData* callBackData);
  void updateAlpha(void);
  void updateColorMap(void);
  void contourValueChangedCallback(Misc::CallbackData* callBackData);

  virtual void toolCreationCallback(Vrui::ToolManager::ToolCreationCallbackData* cbData);
  virtual void toolDestructionCallback(Vrui::ToolManager::ToolDestructionCallbackData* cbData);
};

#endif //_MOOSEVIEWER_H
