#ifndef TRASFERFUNCTION1D_INCLUDED
#define TRASFERFUNCTION1D_INCLUDED

/* Vrui includes */
#include <GL/GLColorMap.h>
#include <GLMotif/Blind.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/Slider.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/TextField.h>
#include <GLMotif/ToggleButton.h>
#include <Misc/CallbackData.h>
#include <Misc/CallbackList.h>

// begin Forward Declarations
class ColorMap;
class ScalarWidget;
class Storage;
class SwatchesWidget;
class MooseViewer;

class TransferFunction1D: public GLMotif::PopupWindow {
public:
    TransferFunction1D(MooseViewer * mooseViewer);
    virtual ~TransferFunction1D(void);
    void changeAlpha(int ramp) const;
    void changeColorMap(int colormap) const;
    void createTransferFunction1D(int colorMapCreationType, int rampCreationType, double _minimum, double _maximum);
    void exportAlpha(double* colormap) const;
    void exportColorMap(double* colormap) const;
    Misc::CallbackList& getAlphaChangedCallbacks(void);
    const ColorMap* getColorMap(void) const;
    ColorMap* getColorMap(void);
    Misc::CallbackList& getColorMapChangedCallbacks(void);
    bool isDragging(void) const;
    bool isInteractive(void);
    void setInteractive(bool interactive);
    Storage* getTransferFunction1D(void) const;
    void setTransferFunction1D(Storage* storage);
    void setHistogram(float* hist);
    void setScalarRange(double* range);
private:
    ScalarWidget* alphaComponent;
    ColorMap* colorMap;
    GLMotif::Blind* colorPane;
    GLMotif::Slider* colorSliders[3];
    bool interactive;
    GLMotif::ToggleButton* interactiveToggleButton;
    SwatchesWidget * swatchesWidget;
    MooseViewer * mooseViewer;
    GLMotif::Slider* minSlider;
    GLMotif::TextField* minValue;
    GLMotif::Slider* maxSlider;
    GLMotif::TextField* maxValue;
    void colorMapChangedCallback(Misc::CallbackData* _callbackData);
    void colorSliderCallback(Misc::CallbackData* _callbackData);
    void colorSwatchesWidgetCallback(Misc::CallbackData* _callbackData);
    void createAlphaComponent(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn*& colorMapDialog);
    GLMotif::RowColumn* createButtonBox(GLMotif::RowColumn*& colorMapDialog);
    GLMotif::RowColumn* createColorEditor(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn*& colorMapDialog);
    void createColorMap(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn*& colorMapDialog);
    void createColorMapDialog(const GLMotif::StyleSheet& styleSheet);
    void createColorPanel(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn*& colorMapDialog);
    GLMotif::Slider* createColorSlider(const char* title, GLMotif::Color color, const GLMotif::StyleSheet& styleSheet,
            GLMotif::RowColumn* colorSlidersBox);
    GLMotif::RowColumn* createColorSliderBox(const GLMotif::StyleSheet &styleSheet, GLMotif::RowColumn* colorEditor);
    GLMotif::RowColumn* createRangeSliders(const GLMotif::StyleSheet &styleSheet, GLMotif::RowColumn*& colorEditor);
    void createColorSliders(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn* colorEditor);
    void createColorSwatchesWidget(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn*& colorEditor);
    void gaussianToggleButtonCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
    void initialize(void);
    void interactiveToggleButtonCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData);
    void removeAlphaControlPointCallback(Misc::CallbackData* _callbackData);
    void removeControlPointCallback(Misc::CallbackData* _callbackData);
    void controlPointChangedCallback(Misc::CallbackData* _callbackData);
    void alphaControlPointChangedCallback(Misc::CallbackData* _callbackData);
    void rangeSliderCallback(GLMotif::Slider::ValueChangedCallbackData* cbData);
};

#endif
