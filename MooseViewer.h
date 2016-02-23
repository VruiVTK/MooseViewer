#ifndef _MOOSEVIEWER_H
#define _MOOSEVIEWER_H

// MooseViewer includes
#include "mvApplicationState.h"

// OpenGL/Motif includes
#include <GL/gl.h>

// Vrui includes
#include <GL/GLObject.h>
#include <GLMotif/ListBox.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RadioBox.h>
#include <GLMotif/Slider.h>
#include <GLMotif/TextField.h>
#include <GLMotif/ToggleButton.h>
#include <Misc/Timer.h>
#include <Vrui/Application.h>

// VTK includes
#include <vtkSmartPointer.h>
#include <vtkTimeStamp.h>

// STL includes
#include <vector>
#include <set>
#include <string>

/* Forward Declarations */
namespace GLMotif
{
  class Popup;
  class PopupMenu;
  class SubMenu;
}

class AnimationDialog;
class Contours;
class TransferFunction1D;
class mvContours;
class mvReader;
class VariablesDialog;
class vtkDataArray;
class vtkLookupTable;

class MooseViewer:public Vrui::Application,public GLObject
{
private:
  mvApplicationState m_state;

  /* Hints for widgets: */
  std::string widgetHintsFile;

  /* Elements: */
  GLMotif::PopupMenu* mainMenu; // The program's main menu
  GLMotif::PopupMenu* createMainMenu(void);
  GLMotif::Popup* createRepresentationMenu(void);
  GLMotif::Popup* createAnalysisToolsMenu(void);
  GLMotif::Popup* createColorByVariablesMenu(void);
  GLMotif::Popup*  createColorMapSubMenu(void);
  GLMotif::PopupWindow* renderingDialog;
  GLMotif::PopupWindow* createRenderingDialog(void);
  GLMotif::TextField* opacityValue;

  /* Update the menus */
  void updateVariablesDialog(void);
  void updateColorByVariablesMenu(void);

  /* Variables dialog */
  VariablesDialog *variablesDialog;

  GLMotif::SubMenu* colorByVariablesMenu;

  /* Color editor dialog */
  TransferFunction1D* ColorEditor;

  /** Cached copy of the last colormap data. Used to skip colormap updates when
   *  nothing actually changes. */
  double *m_colorMapCache;

  /* Animation dialog */
  AnimationDialog* AnimationControl;

  /* Draw histogram */
  float* Histogram;
  vtkTimeStamp HistogramMTime;
  void updateHistogram(void);

  /* Contours dialog */
  Contours* ContoursDialog;

  /* Volume visible */
  GLMotif::TextField* sampleValue;
  GLMotif::TextField* radiusValue;
  GLMotif::TextField* exponentValue;

  /* Custom scalar range */
  double ScalarRange[2];

  /* Constructors and destructors: */
public:
  MooseViewer(int& argc,char**& argv);
  virtual ~MooseViewer(void);

  void Initialize();

  /* vtkExodusIIReader */
  mvReader& reader() { return m_state.reader(); }
  const mvReader& reader() const { return m_state.reader(); }

  /* Methods to set/get the filename to read */
  void setFileName(const std::string &name);
  const std::string &getFileName(void);

  /* Methods to set/get the widget hints file */
  void setWidgetHintsFile(const std::string &whFile);
  const std::string& getWidgetHintsFile(void);

  void setShowFPS(bool show);
  bool getShowFPS() const;

  /* Animation */
  bool IsPlaying;
  bool Loop;

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

  /* Custom scalar range */
  void setScalarMinimum(double min);
  void setScalarMaximum(double max);

  /* Does the work of centering the display. */
  void centerDisplay() const;

  /* Callback methods */
  void toggleFPSCallback(Misc::CallbackData* cbData);
  void centerDisplayCallback(Misc::CallbackData*);
  void opacitySliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
  void sampleSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
  void radiusSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
  void exponentSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
  void showVariableDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void changeRepresentationCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void showRenderingDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void showColorEditorDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void showContoursDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void showAnimationDialogCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void changeAnalysisToolsCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void changeVariablesCallback(GLMotif::ListBox::SelectionChangedCallbackData* callBackData);
  void changeColorByVariablesCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
  void changeColorMapCallback(GLMotif::RadioBox::ValueChangedCallbackData* callBackData);
  void alphaChangedCallback(Misc::CallbackData* callBackData);
  void colorMapChangedCallback(Misc::CallbackData* callBackData);
  void updateColorMap(void);
  void contourValueChangedCallback(Misc::CallbackData* callBackData);
  void updateScalarRange(void);
};

#endif //_MOOSEVIEWER_H
