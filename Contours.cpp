#include <iostream>
//#include <GL/GLColor.h>
#include <GLMotif/Button.h>
#include <GLMotif/CascadeButton.h>
#include <GLMotif/Label.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/WidgetManager.h>
#include <Misc/File.h>

/* Vrui includes to use the Vrui interface */
#include <Vrui/Vrui.h>

#include "ControlPointChangedCallbackData.h"
#include "Contours.h"
#include "ScalarWidget.h"
#include "ScalarWidgetControlPointChangedCallbackData.h"
#include "ScalarWidgetStorage.h"

/*
 * Contours - Constructor for Contours class.
 * 		extends GLMotif::PopupWindow
 */
Contours::Contours(MooseViewer * _MooseViewer) :
    GLMotif::PopupWindow("ContoursPopup", Vrui::getWidgetManager(), "Contours"),
            mooseViewer(_MooseViewer) {
    initialize();
}

/*
 * ~Contours - Destructor for Contours class.
 */
Contours::~Contours(void) {
}

/*
 * controlPointChangedCallback - Callback to select a control point.
 *
 * parameter _callbackData - Misc::CallbackData*
 */
void Contours::controlPointChangedCallback(Misc::CallbackData* _callbackData) {
}

/*
 * createButtonBox - Create a box to hold buttons.
 *
 * parameter colorMapDialog - GLMotif::RowColumn * &
 * return - GLMotif::RowColumn *
 */
GLMotif::RowColumn * Contours::createButtonBox(GLMotif::RowColumn * & colorMapDialog) {
    GLMotif::RowColumn * buttonBox = new GLMotif::RowColumn("ButtonBox", colorMapDialog, false);
    buttonBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
    GLMotif::ToggleButton * showContoursToggle = new GLMotif::ToggleButton(
      "ShowContoursToggle", buttonBox, "Show Contours");
    showContoursToggle->setToggle(true);
    showContoursToggle->getValueChangedCallbacks().add(this, &Contours::toggleSelectCallback);
    showContoursToggle->setToggleWidth(0.1f);
    GLMotif::Button * removeControlPointButton =
            new GLMotif::Button("RemoveControlPointButton", buttonBox, "Remove Control Point");
    removeControlPointButton->getSelectCallbacks().add(this, &Contours::removeControlPointCallback);
    return buttonBox;
}

/*
 * createAlphaComponent - Create alpha component.
 *
 * parameter styleSheet - const GLMotif::StyleSheet&
 * parameter colorMapDialog - GLMotif::RowColumn*&
 */
void Contours::createAlphaComponent(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn*& contoursDialog) {
    alphaComponent = new ScalarWidget("AlphaComponent", contoursDialog, 3);
    alphaComponent->setBorderWidth(styleSheet.size * 0.5f);
    alphaComponent->setBorderType(GLMotif::Widget::LOWERED);
    alphaComponent->setForegroundColor(GLMotif::Color(0.0f, 1.0f, 0.0f));
    alphaComponent->setMarginWidth(styleSheet.size);
    alphaComponent->setPreferredSize(GLMotif::Vector(styleSheet.fontHeight * 20.0, styleSheet.fontHeight * 10.0, 0.0f));
    alphaComponent->setControlPointSize(styleSheet.size);
    alphaComponent->setControlPointScalar(1.0f);
    alphaComponent->getControlPointChangedCallbacks().add(this, &Contours::alphaControlPointChangedCallback);
    alphaComponent->setHistogram(this->mooseViewer->getHistogram());
    alphaComponent->useAs1DWidget(true);
    alphaComponent->setComponent(3);
    alphaComponent->drawHistogram();
} // end createAlphaComponent()

/*
 * createContoursDialog - Create color map dialog.
 *
 * parameter styleSheet - const GLMotif::StyleSheet&
 */
void Contours::createContoursDialog(const GLMotif::StyleSheet& styleSheet) {
    GLMotif::RowColumn* contoursDialog =
      new GLMotif::RowColumn("ContoursDialog", this, false);
    GLMotif::Label * contourLabel = new GLMotif::Label(
      "ContourLabel", contoursDialog, "Contours");
    contourLabel->setString("Contour");
    createAlphaComponent(styleSheet, contoursDialog);
    GLMotif::RowColumn* buttonBox = createButtonBox(contoursDialog);
    buttonBox->manageChild();
//    GLMotif::Label * sliceContourLabel = new GLMotif::Label(
//      "SliceContourLabel", contoursDialog, "SliceContours");
//    sliceContourLabel->setString("Slice Contours");
//    createXYZContours(styleSheet, contoursDialog);
    contoursDialog->manageChild();
}

/*
 * createXContours - Create x slice widgets.
 *
 * parameter xyzContoursRowColumn - GLMotif::RowColumn * &
 * parameter styleSheet - const GLMotif::StyleSheet&
 */
void Contours::createXContours(GLMotif::RowColumn * & xyzContoursRowColumn, const GLMotif::StyleSheet & styleSheet) {
    //create x slice button and slider+textfield
//    GLMotif::ToggleButton * showXSliceToggle = new GLMotif::ToggleButton("ShowXSliceToggle", xyzContoursRowColumn, "X");
//    showXSliceToggle->setToggle(false);
//    showXSliceToggle->getValueChangedCallbacks().add(this, &Contours::toggleSelectCallback);
//    showXSliceToggle->setToggleWidth(0.1f);
//    xSliceValue = new GLMotif::TextField("XSliceValue", xyzContoursRowColumn, 7);
//    xSliceValue->setPrecision(3);
//    xSliceValue->setValue(0.0);
//    GLMotif::Slider * xSliceSlider = new GLMotif::Slider("XSliceSlider", xyzContoursRowColumn, GLMotif::Slider::HORIZONTAL, styleSheet.fontHeight * 10.0f);
//    xSliceSlider->setValueRange(0.0, float(mooseViewer->getWidth() - 1), 1.0);
//    xSliceSlider->setValue(0.0);
//    xSliceSlider->getValueChangedCallbacks().add(this, &Contours::sliderCallback);
} // end createXContours()

/*
 * createXYZContours - Create x, y and z slice widgets.
 *
 * parameter styleSheet - const GLMotif::StyleSheet&
 * parameter colorMapDialog - GLMotif::RowColumn*&
 */
void Contours::createXYZContours(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn*& colorMapDialog) {
    GLMotif::RowColumn * xyzContoursRowColumn = new GLMotif::RowColumn("XYZContoursRowColumn", colorMapDialog, false);
    xyzContoursRowColumn->setOrientation(GLMotif::RowColumn::VERTICAL);
    xyzContoursRowColumn->setNumMinorWidgets(GLsizei(3));
    createXContours(xyzContoursRowColumn, styleSheet);
    createYContours(xyzContoursRowColumn, styleSheet);
    createZContours(xyzContoursRowColumn, styleSheet);

    xyzContoursRowColumn->manageChild();
} // end createXYZContours()createXYZContours

/*
 * createYContours - Create x slice widgets.
 *
 * parameter xyzContoursRowColumn - GLMotif::RowColumn * &
 * parameter styleSheet - const GLMotif::StyleSheet&
 */
void Contours::createYContours(GLMotif::RowColumn * & xyzContoursRowColumn, const GLMotif::StyleSheet & styleSheet) {
//    //create y slice button and slider+textfield
//    GLMotif::ToggleButton * showYSliceToggle = new GLMotif::ToggleButton("ShowYSliceToggle", xyzContoursRowColumn, "Y");
//    showYSliceToggle->setToggle(false);
//    showYSliceToggle->getValueChangedCallbacks().add(this, &Contours::toggleSelectCallback);
//    ySliceValue = new GLMotif::TextField("YSliceValue", xyzContoursRowColumn, 7);
//    ySliceValue->setPrecision(3);
//    ySliceValue->setValue(0.0);
//    GLMotif::Slider * ySliceSlider = new GLMotif::Slider("YSliceSlider", xyzContoursRowColumn, GLMotif::Slider::HORIZONTAL, styleSheet.fontHeight * 10.0f);
//    ySliceSlider->setValueRange(0.0, float(mooseViewer->getLength() - 1), 1.0);
//    ySliceSlider->setValue(0.0);
//    ySliceSlider->getValueChangedCallbacks().add(this, &Contours::sliderCallback);
} // end createYContours()

/*
 * createZContours - Create x slice widgets.
 *
 * parameter xyzContoursRowColumn - GLMotif::RowColumn * &
 * parameter styleSheet - const GLMotif::StyleSheet&
 */
void Contours::createZContours(GLMotif::RowColumn * & xyzContoursRowColumn, const GLMotif::StyleSheet & styleSheet) {
//    //create z slice button and slider+textfield
//    GLMotif::ToggleButton * showZSliceToggle = new GLMotif::ToggleButton("ShowZSliceToggle", xyzContoursRowColumn, "Z");
//    showZSliceToggle->setToggle(false);
//    showZSliceToggle->getValueChangedCallbacks().add(this, &Contours::toggleSelectCallback);
//    zSliceValue = new GLMotif::TextField("ZSliceValue", xyzContoursRowColumn, 7);
//    zSliceValue->setPrecision(3);
//    zSliceValue->setValue(0.0);
//    GLMotif::Slider * zSliceSlider = new GLMotif::Slider("ZSliceSlider", xyzContoursRowColumn, GLMotif::Slider::HORIZONTAL, styleSheet.fontHeight * 10.0f);
//    zSliceSlider->setValueRange(0.0, float(mooseViewer->getHeight() - 1), 1.0);
//    zSliceSlider->setValue(0.0);
//    zSliceSlider->getValueChangedCallbacks().add(this, &Contours::sliderCallback);
} // end createZContours()

/*
 * initialize - Initialize the GUI for the Contours class.
 */
void Contours::initialize(void) {
    const GLMotif::StyleSheet& styleSheet = *Vrui::getWidgetManager()->getStyleSheet();
    createContoursDialog(styleSheet);
}

/*
 * removeControlPointCallback - Remove the current control point.
 *
 * parameter _callbackData - Misc::CallbackData*
 */
void Contours::removeControlPointCallback(Misc::CallbackData* _callbackData) {
    alphaComponent->deleteControlPoint();
} // end removeControlPointCallback()

/*
 * sliderCallback
 *
 * parameter callBackData - GLMotif::Slider::ValueChangedCallbackData *
 */
void Contours::sliderCallback(GLMotif::Slider::ValueChangedCallbackData * callBackData) {
//    if (strcmp(callBackData->slider->getName(), "XSliceSlider") == 0) {
//        xSliceValue->setValue(callBackData->value);
//        mooseViewer->setXContourSlice(int(callBackData->value));
//        Vrui::requestUpdate();
//    }
//    if (strcmp(callBackData->slider->getName(), "YSliceSlider") == 0) {
//        ySliceValue->setValue(callBackData->value);
//        mooseViewer->setYContourSlice(int(callBackData->value));
//        Vrui::requestUpdate();
//    }
//    if (strcmp(callBackData->slider->getName(), "ZSliceSlider") == 0) {
//        zSliceValue->setValue(callBackData->value);
//        mooseViewer->setZContourSlice(int(callBackData->value));
//        Vrui::requestUpdate();
//    }
} // end sliderCallback()

/*
 * toggleSelectCallback - Adjust the gui/program state based on which toggle button changed state.
 *
 * parameter callBackData - GLMotif::ToggleButton::ValueChangedCallbackData*
 */
void Contours::toggleSelectCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData) {
//    /* Adjust gui/program state based on which toggle button changed state: */
//    if (strcmp(callBackData->toggle->getName(), "ShowXSliceToggle") == 0) {
//        mooseViewer->showXContourSlice(callBackData->set);
//    } else if (strcmp(callBackData->toggle->getName(), "ShowYSliceToggle") == 0) {
//        mooseViewer->showYContourSlice(callBackData->set);
//    } else if (strcmp(callBackData->toggle->getName(), "ShowZSliceToggle") == 0) {
//        mooseViewer->showZContourSlice(callBackData->set);
//    }
    if (strcmp(callBackData->toggle->getName(), "ShowContoursToggle") == 0) {
        mooseViewer->setContourVisible(callBackData->set);
    }
    Vrui::requestUpdate();
} // end toggleSelectCallback()

/*
 * alphaControlPointChangedCallback - Callback to select a control point.
 *
 * parameter _callbackData - Misc::CallbackData*
 */
void Contours::alphaControlPointChangedCallback(Misc::CallbackData* _callbackData) {
} // end alphaControlPointChangedCallback()

/*
 * getAlphaChangedCallbacks
 *
 * return - Misc::CallbackList&
 */
Misc::CallbackList& Contours::getAlphaChangedCallbacks(void) {
    return alphaComponent->getChangedCallbacks();
} // end getAlphaChangedCallbacks()

std::vector<double> Contours::getContourValues(void)
{
  return alphaComponent->exportControlPointValues();
}

/*
 * setHistogram - Method to set and display the histogram
 */
void Contours::setHistogram(float* hist)
{
  this->alphaComponent->setHistogram(hist);
  this->alphaComponent->drawHistogram();
}
