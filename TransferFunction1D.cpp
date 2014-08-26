#include <iostream>
#include <cmath>

/* Vrui includes */
#include <GL/GLColor.h>
#include <GLMotif/Label.h>
#include <GLMotif/Button.h>
#include <GLMotif/WidgetManager.h>
#include <Misc/File.h>

/* Vrui includes to use the Vrui interface */
#include <Vrui/Vrui.h>

#include "ColorMap.h"
#include "ControlPointChangedCallbackData.h"
#include "RGBAColor.h"
#include "Storage.h"
#include "ScalarWidget.h"
#include "ScalarWidgetControlPointChangedCallbackData.h"
#include "SwatchesWidget.h"
#include "TransferFunction1D.h"
#include "MooseViewer.h"

/*
 * TransferFunction1D - Constructor for TransferFunction1D class.
 * 		extends GLMotif::PopupWindow
 */
TransferFunction1D::TransferFunction1D(MooseViewer * mooseViewer) :
    GLMotif::PopupWindow("TransferFunction1DPopup", Vrui::getWidgetManager(), "Color Editor"), interactive(false) {
    this->mooseViewer = mooseViewer;
    initialize();
}

/*
 * ~TransferFunction1D - Destructor for TransferFunction1D class.
 */
TransferFunction1D::~TransferFunction1D(void) {
}

/*
 * changeAlpha - Change the alpha ramp.
 *
 * parameter ramp - int
 */
void TransferFunction1D::changeAlpha(int ramp) const {
    alphaComponent->create(ramp);
} // end changeAlpha()

/*
 * changeColorMap - Change the color map.
 *
 * parameter colormap - int
 */
void TransferFunction1D::changeColorMap(int colormap) const {
    colorMap->createColorMap(colormap);
} // end changeColorMap()

/*
 * colorSliderCallback - Callback of change to color slider value.
 *
 * parameter _callbackData - Misc::CallbackData*
 */
void TransferFunction1D::colorSliderCallback(Misc::CallbackData* _callbackData) {
    RGBAColor* rgbaColor = new RGBAColor(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < 3; ++i) {
        rgbaColor->setValues(i, float(colorSliders[i]->getValue()));
    }
    rgbaColor->setValues(3, 1.0f);
    colorPane->setBackgroundColor(rgbaColor->getValues());
    colorMap->setControlPointColor(*rgbaColor);
}

/*
 * colorSwatchesWidgetCallback
 *
 * parameter _callbackData - Misc::CallbackData*
 */
void TransferFunction1D::colorSwatchesWidgetCallback(Misc::CallbackData* _callbackData) {
    float* _color = swatchesWidget->getCurrentColor();
    RGBAColor* rgbaColor = new RGBAColor(_color[0], _color[1], _color[2], 1.0f);
    for (int i = 0; i < 3; ++i) {
        colorSliders[i]->setValue(_color[i]);
    }
    colorPane->setBackgroundColor(rgbaColor->getValues());
    colorMap->setControlPointColor(*rgbaColor);
} // end colorSwatchesWidgetCallback()

/*
 * createAlphaComponent - Create alpha component.
 *
 * parameter styleSheet - const GLMotif::StyleSheet&
 * parameter colorMapDialog - GLMotif::RowColumn*&
 */
void TransferFunction1D::createAlphaComponent(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn*& colorMapDialog) {
    alphaComponent = new ScalarWidget("AlphaComponent", colorMapDialog, 3);
    alphaComponent->setBorderWidth(styleSheet.size * 0.5f);
    alphaComponent->setBorderType(GLMotif::Widget::LOWERED);
    alphaComponent->setForegroundColor(GLMotif::Color(0.0f, 1.0f, 0.0f));
    alphaComponent->setMarginWidth(styleSheet.size);
    alphaComponent->setPreferredSize(GLMotif::Vector(styleSheet.fontHeight * 20.0, styleSheet.fontHeight * 10.0, 0.0f));
    alphaComponent->setControlPointSize(styleSheet.size);
    alphaComponent->setControlPointScalar(1.0f);
    alphaComponent->setComponent(3);
//    alphaComponent->setHistogram(this->mooseViewer->getHistogram());
//    alphaComponent->drawHistogram();
    alphaComponent->getControlPointChangedCallbacks().add(this, &TransferFunction1D::alphaControlPointChangedCallback);
} // end createAlphaComponent()

/*
 * createButtonBox - Create a box to hold buttons.
 *
 * parameter colorMapDialog - GLMotif::RowColumn*&
 * return - GLMotif::RowColumn*
 */
GLMotif::RowColumn* TransferFunction1D::createButtonBox(GLMotif::RowColumn*& colorMapDialog) {
    GLMotif::RowColumn* buttonBox = new GLMotif::RowColumn("ButtonBox", colorMapDialog, false);
    buttonBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
    buttonBox->setPacking(GLMotif::RowColumn::PACK_GRID);
    GLMotif::Button* removeControlPointButton = new GLMotif::Button("RemoveControlPointButton", buttonBox, "Remove RGB Point");
    removeControlPointButton->getSelectCallbacks().add(this, &TransferFunction1D::removeControlPointCallback);
    GLMotif::Button* removeAlphaControlPointButton = new GLMotif::Button("RemoveAlphaControlPointButton", buttonBox,
            "Remove Alpha Point");
    removeAlphaControlPointButton->getSelectCallbacks().add(this, &TransferFunction1D::removeAlphaControlPointCallback);
//    GLMotif::ToggleButton* guassianToggleButton = new GLMotif::ToggleButton("GuassianToggleButton", buttonBox, "Gaussian");
//    guassianToggleButton->setToggle(false);
//    guassianToggleButton->getValueChangedCallbacks().add(this, &TransferFunction1D::gaussianToggleButtonCallback);
//    interactiveToggleButton = new GLMotif::ToggleButton("InteractiveToggleButton", buttonBox, "Interactive");
//    interactiveToggleButton->setToggle(false);
//    interactiveToggleButton->getValueChangedCallbacks().add(this, &TransferFunction1D::interactiveToggleButtonCallback);
    return buttonBox;
}

/*
 * createColorEditor - Create color editor.
 *
 * parameter styleSheet - const GLMotif::StyleSheet&
 * parameter colorMapDialog - GLMotif::RowColumn*&
 * return - GLMotif::RowColumn*
 */
GLMotif::RowColumn* TransferFunction1D::createColorEditor(const GLMotif::StyleSheet& styleSheet,
        GLMotif::RowColumn*& colorMapDialog) {
    GLMotif::RowColumn* colorEditor = new GLMotif::RowColumn("ColorEditor", colorMapDialog, false);
    colorEditor->setOrientation(GLMotif::RowColumn::HORIZONTAL);
    createColorSwatchesWidget(styleSheet, colorEditor);
    GLMotif::RowColumn* colorSlidersBox = createColorSliderBox(styleSheet, colorEditor);
    colorSlidersBox->manageChild();
    new GLMotif::Blind("Filler", colorEditor);
    return colorEditor;
}

/*
 * createColorMap - Create color map.
 *
 * parameter styleSheet - const GLMotif::StyleSheet&
 * parameter colorMapDialog - GLMotif::RowColumn*&
 */
void TransferFunction1D::createColorMap(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn*& colorMapDialog) {
    colorMap = new ColorMap("ColorMap", colorMapDialog);
    colorMap->setBorderWidth(styleSheet.size * 0.5f);
    colorMap->setBorderType(GLMotif::Widget::LOWERED);
    colorMap->setForegroundColor(GLMotif::Color(0.0f, 1.0f, 0.0f));
    colorMap->setMarginWidth(styleSheet.size);
    colorMap->setPreferredSize(GLMotif::Vector(styleSheet.fontHeight * 5.0, styleSheet.fontHeight * 5.0, 0.0f));
    colorMap->setControlPointSize(styleSheet.size);
    RGBAColor* rgbaColor = new RGBAColor(1.0f, 0.0f, 0.0f, 1.0f);
    colorMap->setControlPointColor(rgbaColor);
    colorMap->getControlPointChangedCallbacks().add(this, &TransferFunction1D::controlPointChangedCallback);
}

/*
 * createColorMapDialog - Create color map dialog.
 *
 * parameter styleSheet - const GLMotif::StyleSheet&
 */
void TransferFunction1D::createColorMapDialog(const GLMotif::StyleSheet& styleSheet) {
    GLMotif::RowColumn* colorMapDialog = new GLMotif::RowColumn("ColorMapDialog", this, false);
    createColorMap(styleSheet, colorMapDialog);
    GLMotif::RowColumn* buttonBox = createButtonBox(colorMapDialog);
    buttonBox->manageChild();
    createAlphaComponent(styleSheet, colorMapDialog);
    createColorPanel(styleSheet, colorMapDialog);
    GLMotif::RowColumn* colorEditor = createColorEditor(styleSheet, colorMapDialog);
    colorEditor->manageChild();
    colorMapDialog->manageChild();
}

/*
 * createColorPanel - Create color panel.
 *
 * parameter styleSheet - const GLMotif::StyleSheet&
 * parameter colorMapDialog - GLMotif::RowColumn*&
 */
void TransferFunction1D::createColorPanel(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn*& colorMapDialog) {
    GLMotif::RowColumn* colorPanel = new GLMotif::RowColumn("ColorPanel", colorMapDialog, false);
    colorPanel->setOrientation(GLMotif::RowColumn::HORIZONTAL);
    colorPanel->setMarginWidth(styleSheet.size);
    colorPane = new GLMotif::Blind("ColorPane", colorPanel);
    colorPane->setBorderWidth(styleSheet.size * 0.5f);
    colorPane->setBorderType(GLMotif::Widget::LOWERED);
    colorPane->setBackgroundColor(GLMotif::Color(0.5f, 0.5f, 0.5f));
    colorPane->setPreferredSize(GLMotif::Vector(styleSheet.fontHeight * 20.0f, styleSheet.fontHeight * 1.0f, 0.0f));
    colorPanel->manageChild();
}

/*
 * createColorSwatchesWidget
 *
 * parameter styleSheet - const GLMotif::StyleSheet&
 * parameter colorEditor - GLMotif::RowColumn*&
 */
void TransferFunction1D::createColorSwatchesWidget(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn*& colorEditor) {
    swatchesWidget = new SwatchesWidget("SwatchesWidget", colorEditor);
    swatchesWidget->setBorderWidth(styleSheet.size * 0.5f);
    swatchesWidget->setBorderType(GLMotif::Widget::LOWERED);
    swatchesWidget->setForegroundColor(GLMotif::Color(0.0f, 1.0f, 0.0f));
    swatchesWidget->setMarginWidth(styleSheet.size);
    swatchesWidget->setPreferredSize(GLMotif::Vector(styleSheet.fontHeight * 15.0, styleSheet.fontHeight * 5.0, 0.0f));
    swatchesWidget->getColorChangedCallbacks().add(this, &TransferFunction1D::colorSwatchesWidgetCallback);
} // end createColorSwatchesWidget()


/*
 * createColorSlider - Create color slider.
 *
 * parameter title - const char*
 * parameter color - GLMotif::Color
 * parameter styleSheet - const GLMotif::StyleSheet&
 * parameter colorSliderBox - GLMotif::RowColumn*
 * return - GLMotif::Slider*
 */
GLMotif::Slider* TransferFunction1D::createColorSlider(const char* title, GLMotif::Color color,
        const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn* colorSlidersBox) {
    GLMotif::Slider* colorSlider = new GLMotif::Slider(title, colorSlidersBox, GLMotif::Slider::VERTICAL, styleSheet.fontHeight
            * 5.0f);
    colorSlider->setSliderColor(color);
    colorSlider->setValueRange(0.0f, 1.0f, 0.01f);
    colorSlider->setValue(0.5f);
    colorSlider->getValueChangedCallbacks().add(this, &TransferFunction1D::colorSliderCallback);
    return colorSlider;
}

/*
 * createColorSliderBox - Create box to contain color slider bars.
 *
 * parameter styleSheet - const GLMotif::StyleSheet&
 * parameter colorEditor - GLMotif::RowColumn*
 * return - GLMotif::RowColumn*
 */
GLMotif::RowColumn* TransferFunction1D::createColorSliderBox(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn* colorEditor) {
    GLMotif::RowColumn* colorSlidersBox = new GLMotif::RowColumn("ColorSliders", colorEditor, false);
    colorSlidersBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
    colorSlidersBox->setPacking(GLMotif::RowColumn::PACK_GRID);
    createColorSliders(styleSheet, colorSlidersBox);
    return colorSlidersBox;
}

/*
 * createColorSliders - Create RGB color sliders.
 *
 * parameter styleSheet - const GLMotif::StyleSheet&
 * parameter colorSliderBox - GLMotif::RowColumn*
 */
void TransferFunction1D::createColorSliders(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn* colorSlidersBox) {
    colorSliders[0] = createColorSlider("RedSlider", GLMotif::Color(1.0f, 0.0f, 0.0f), styleSheet, colorSlidersBox);
    colorSliders[1] = createColorSlider("GreenSlider", GLMotif::Color(0.0f, 1.0f, 0.0f), styleSheet, colorSlidersBox);
    colorSliders[2] = createColorSlider("BlueSlider", GLMotif::Color(0.0f, 0.0f, 1.0f), styleSheet, colorSlidersBox);
}

/*
 * createTransferFunction1D - Create the transfer function 1D.
 *
 * parameter colorMapCreationType - int
 * parameter rampCreationType - int
 * parameter _minimum - double
 * parameter _maximum - double
 */
void TransferFunction1D::createTransferFunction1D(int colorMapCreationType, int rampCreationType, double _minimum, double _maximum) {
    colorMap->createColorMap(colorMapCreationType, _minimum, _maximum);
    alphaComponent->create(rampCreationType, _minimum, _maximum);
}

/*
 * exportAlpha - Export the alpha.
 *
 * parameter colormap - double*
 */
void TransferFunction1D::exportAlpha(double* colormap) const {
    alphaComponent->exportScalar(colormap);
} // end exportAlpha()

/*
 * exportColorMap - Export the color map.
 *
 * parameter colormap - double*
 */
void TransferFunction1D::exportColorMap(double* colormap) const {
    colorMap->exportColorMap(colormap);
}

/*
 * gaussianToggleButtonCallback
 *
 * parameter callBackData - GLMotif::ToggleButton::ValueChangedCallbackData*
 */
void TransferFunction1D::gaussianToggleButtonCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData) {
    if (strcmp(callBackData->toggle->getName(), "GuassianToggleButton") == 0) {
        alphaComponent->setGaussian(callBackData->set);
    }
    Vrui::requestUpdate();
} // end gaussianToggleButtonCallback()

/*
 * getAlphaChangedCallbacks
 *
 * return - Misc::CallbackList&
 */
Misc::CallbackList& TransferFunction1D::getAlphaChangedCallbacks(void) {
    return alphaComponent->getChangedCallbacks();
} // end getAlphaChangedCallbacks()

/*
 * getColorMap
 *
 * return const ColorMap*
 */
const ColorMap* TransferFunction1D::getColorMap(void) const {
    return colorMap;
} // end getColorMap()

/*
 * getColorMap
 *
 * return - ColorMap*
 */
ColorMap* TransferFunction1D::getColorMap(void) {
    return colorMap;
} // end getColorMap()

/*
 * getColorMapChangedCallbacks
 *
 * return - Misc::CallbackList&
 */
Misc::CallbackList& TransferFunction1D::getColorMapChangedCallbacks(void) {
    return colorMap->getColorMapChangedCallbacks();
} // end getColorMapChangedCallbacks()

/*
 * isDragging
 *
 * return - bool
 */
bool TransferFunction1D::isDragging(void) const {
    return alphaComponent->isDragging();
} // end isDragging()

/*
 * isInteractive
 *
 * return - bool
 */
bool TransferFunction1D::isInteractive(void) {
    return interactive;
} // end isInteractive()

/*
 * setInteractive
 *
 * parameter interactive - bool
 */
void TransferFunction1D::setInteractive(bool interactive) {
    this->interactive = interactive;
    interactiveToggleButton->setToggle(interactive);
} // end setInteractive()

/*
 * getTransferFunction1D - Get the transfer function 1D.
 *
 * return - Storage*
 */
Storage* TransferFunction1D::getTransferFunction1D(void) const {
    return colorMap->getColorMap();
}

/*
 * setTransferFunction1D - Set the transfer function 1D.
 *
 * parameter storage - Storage*
 */
void TransferFunction1D::setTransferFunction1D(Storage* storage) {
    colorMap->setColorMap(storage);
}

/*
 * initialize - Initialize the GUI for the TransferFunction1D class.
 */
void TransferFunction1D::initialize(void) {
    const GLMotif::StyleSheet& styleSheet = *Vrui::getWidgetManager()->getStyleSheet();
    createColorMapDialog(styleSheet);
}

/*
 * interactiveToggleButtonCallback
 *
 * parameter callBackData - GLMotif::ToggleButton::ValueChangedCallbackData*
 */
void TransferFunction1D::interactiveToggleButtonCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData) {
    if (strcmp(callBackData->toggle->getName(), "InteractiveToggleButton") == 0) {
       interactive = callBackData->set;
    }
    Vrui::requestUpdate();
} // end interactiveToggleButtonCallback()

/*
 * removeAlphaControlPointCallback - Remove the current alpha control point.
 *
 * parameter _callbackData - Misc::CallbackData*
 */
void TransferFunction1D::removeAlphaControlPointCallback(Misc::CallbackData* _callbackData) {
    alphaComponent->deleteControlPoint();
} // end removeAlphaControlPointCallback()

/*
 * removeControlPointCallback - Remove the current control point.
 *
 * parameter _callbackData - Misc::CallbackData*
 */
void TransferFunction1D::removeControlPointCallback(Misc::CallbackData* _callbackData) {
    colorMap->deleteControlPoint();
} // end removeControlPointCallback()

/*
 * alphaControlPointChangedCallback - Callback to select a control point.
 *
 * parameter _callbackData - Misc::CallbackData*
 */
void TransferFunction1D::alphaControlPointChangedCallback(Misc::CallbackData* _callbackData) {
    ScalarWidgetControlPointChangedCallbackData* callbackData =
            static_cast<ScalarWidgetControlPointChangedCallbackData*> (_callbackData);
    if (callbackData->getCurrentControlPoint() != 0) {
        float _alpha = alphaComponent->getControlPointScalar();
    }
} // end alphaControlPointChangedCallback()

/*
 * controlPointChangedCallback - Callback to select a control point.
 *
 * parameter _callbackData - Misc::CallbackData*
 */
void TransferFunction1D::controlPointChangedCallback(Misc::CallbackData* _callbackData) {
    ControlPointChangedCallbackData* callbackData = static_cast<ControlPointChangedCallbackData*> (_callbackData);
    if (callbackData->getCurrentControlPoint() != 0) {
        RGBAColor* rgbaColor = colorMap->getControlPointColor();
        colorPane->setBackgroundColor(rgbaColor->getValues());
        for (int i = 0; i < 3; ++i)
            colorSliders[i]->setValue(rgbaColor->getValues(i));
    } else {
        colorPane->setBackgroundColor(GLMotif::Color(0.5f, 0.5f, 0.5f));
        for (int i = 0; i < 3; ++i)
            colorSliders[i]->setValue(0.5);
    }
}

/*
 * setHistogram - Method to set and display the histogram
 */
void TransferFunction1D::setHistogram(float* hist)
{
  this->alphaComponent->setHistogram(hist);
  this->alphaComponent->drawHistogram();
}
