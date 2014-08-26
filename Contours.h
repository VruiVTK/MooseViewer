#ifndef CONTOURS_INCLUDED
#define CONTOURS_INCLUDED

// STD includes
#include <vector>
#include <algorithm>

/* Vrui includes */
#include <GL/GLColorMap.h>
#include <GLMotif/Blind.h>
#include <GLMotif/Popup.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RadioBox.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Slider.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/TextField.h>
#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>

#include "MooseViewer.h"

// begin Forward Declarations
class ScalarWidget;
// end Forward Declarations

class Contours: public GLMotif::PopupWindow {
public:
    MooseViewer * mooseViewer;

    Contours(MooseViewer * _MooseViewer);
    virtual ~Contours(void);
    void sliderCallback(GLMotif::Slider::ValueChangedCallbackData * callBackData);
    void toggleSelectCallback(GLMotif::ToggleButton::ValueChangedCallbackData * callBackData);
    Misc::CallbackList& getAlphaChangedCallbacks(void);
    std::vector<double> getContourValues(void);
    void setHistogram(float* hist);
private:
    ScalarWidget* alphaComponent;
    GLMotif::Blind * colorPane;
    GLMotif::Slider * colorSliders[3];
    GLMotif::TextField* xSliceValue;
    GLMotif::TextField* ySliceValue;
    GLMotif::TextField* zSliceValue;
    void controlPointChangedCallback(Misc::CallbackData * callbackData);
    GLMotif::RowColumn * createButtonBox(GLMotif::RowColumn * & colorMapDialog);
    void alphaControlPointChangedCallback(Misc::CallbackData* _callbackData);
    void createContoursDialog(const GLMotif::StyleSheet & styleSheet);
    void createAlphaComponent(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn*& colorMapDialog);
    void createXContours(GLMotif::RowColumn * & xyzContoursRowColumn, const GLMotif::StyleSheet & styleSheet);
    void createXYZContours(const GLMotif::StyleSheet & styleSheet, GLMotif::RowColumn * & colorMapDialog);
    void createYContours(GLMotif::RowColumn * & xyzContoursRowColumn, const GLMotif::StyleSheet & styleSheet);
    void createZContours(GLMotif::RowColumn * & xyzContoursRowColumn, const GLMotif::StyleSheet & styleSheet);
    void initialize(void);
    void removeControlPointCallback(Misc::CallbackData * callbackData);
};

#endif
