#ifndef ISOSURFACES_INCLUDED
#define ISOSURFACES_INCLUDED

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
class ColorMap;
class Storage;
class SwatchesWidget;
// end Forward Declarations

class Isosurfaces: public GLMotif::PopupWindow {
public:
    MooseViewer * mooseViewer;

    Isosurfaces(double* _isosurfaceColormap, MooseViewer * _MooseViewer);
    virtual ~Isosurfaces(void);
    void changeIsosurfacesColorMap(int colormap) const;
    void changeIsosurfacesColorMapCallback(GLMotif::RadioBox::ValueChangedCallbackData * callBackData);
    void exportIsosurfacesColorMap(double* colormap) const;
    Storage * getColorMap(void) const;
    void setColorMap(Storage* storage);
    const ColorMap* getIsosurfacesColorMap(void) const;
    ColorMap* getIsosurfacesColorMap(void);
    void setIsosurfacesColorMap(int colorMapCreationType, double _minimum, double _maximum);
    Misc::CallbackList& getIsosurfacesColorMapChangedCallbacks(void);
    void isosurfaceColorMapChangedCallback(Misc::CallbackData * callBackData);
    void sliderCallback(GLMotif::Slider::ValueChangedCallbackData * callBackData);
    void toggleSelectCallback(GLMotif::ToggleButton::ValueChangedCallbackData * callBackData);
private:
    ColorMap * colorMap;
    GLMotif::Blind * colorPane;
    GLMotif::Slider * colorSliders[3];
    double* isosurfaceColormap;
    SwatchesWidget * swatchesWidget;
    GLMotif::TextField* AIsosurfacesValue;
    GLMotif::TextField* BIsosurfacesValue;
    GLMotif::TextField* CIsosurfacesValue;
    void colorMapChangedCallback(Misc::CallbackData * callbackData);
    void colorSliderCallback(Misc::CallbackData * callbackData);
    void colorSwatchesWidgetCallback(Misc::CallbackData * callbackData);
    void controlPointChangedCallback(Misc::CallbackData * callbackData);
    GLMotif::RowColumn * createButtonBox(GLMotif::RowColumn * & colorMapDialog);
    GLMotif::RowColumn * createColorEditor(const GLMotif::StyleSheet & styleSheet, GLMotif::RowColumn * & colorMapDialog);
    void createColorMap(const GLMotif::StyleSheet & styleSheet, GLMotif::RowColumn * & colorMapDialog);
    void createColorMapDialog(const GLMotif::StyleSheet & styleSheet);
    void createColorPanel(const GLMotif::StyleSheet & styleSheet, GLMotif::RowColumn * & colorMapDialog);
    GLMotif::Slider * createColorSlider(const char * title, GLMotif::Color color, const GLMotif::StyleSheet & styleSheet,
            GLMotif::RowColumn * colorSlidersBox);
    GLMotif::RowColumn * createColorSliderBox(const GLMotif::StyleSheet & styleSheet, GLMotif::RowColumn * colorEditor);
    void createColorSliders(const GLMotif::StyleSheet & styleSheet, GLMotif::RowColumn * colorEditor);
    void createColorSwatchesWidget(const GLMotif::StyleSheet & styleSheet, GLMotif::RowColumn * & colorEditor);
    GLMotif::Popup * createIsosurfaceColorMapSubMenu(void);
    void createAIsosurfaces(GLMotif::RowColumn * & abcIsosurfacesRowColumn, const GLMotif::StyleSheet & styleSheet);
    void createABCIsosurfaces(const GLMotif::StyleSheet & styleSheet, GLMotif::RowColumn * & colorMapDialog);
    void createBIsosurfaces(GLMotif::RowColumn * & abcIsosurfacesRowColumn, const GLMotif::StyleSheet & styleSheet);
    void createCIsosurfaces(GLMotif::RowColumn * & abcIsosurfacesRowColumn, const GLMotif::StyleSheet & styleSheet);
    void initialize(void);
    void removeControlPointCallback(Misc::CallbackData * callbackData);
};

#endif
