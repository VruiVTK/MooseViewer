#include <iostream>
#include <GL/GLColor.h>
#include <GLMotif/Button.h>
#include <GLMotif/CascadeButton.h>
#include <GLMotif/Label.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/WidgetManager.h>
#include <Misc/File.h>

/* Vrui includes to use the Vrui interface */
#include <Vrui/Vrui.h>

#include "ColorMap.h"
#include "ControlPointChangedCallbackData.h"
#include "RGBAColor.h"
#include "Storage.h"
#include "Isosurfaces.h"
#include "SwatchesWidget.h"

/*
 * Isosurfaces - Constructor for Isosurfaces class.
 *      extends GLMotif::PopupWindow
 */
Isosurfaces::Isosurfaces(double* _isosurfaceColormap, MooseViewer * _MooseViewer) :
    GLMotif::PopupWindow("IsosurfacesPopup", Vrui::getWidgetManager(), "Isosurfaces"), isosurfaceColormap(_isosurfaceColormap),
            mooseViewer(_MooseViewer) {
    initialize();
}

/*
 * ~Isosurfaces - Destructor for Isosurfaces class.
 */
Isosurfaces::~Isosurfaces(void) {
}

/*
 * changeIsosurfacesColorMap - Change the colormap.
 *
 * parameter colormap - int
 */
void Isosurfaces::changeIsosurfacesColorMap(int colormap) const {
    colorMap->createColorMap(colormap);
} // end changeIsosurfacesColorMap()

/*
 * changeIsosurfacesColorMapCallback
 *
 * parameter callBackData - GLMotif::RadioBox::ValueChangedCallbackData *
 */
void Isosurfaces::changeIsosurfacesColorMapCallback(GLMotif::RadioBox::ValueChangedCallbackData * callBackData) {
    /* Set the new isosurfaces colormap */
    int value = callBackData->radioBox->getToggleIndex(callBackData->newSelectedToggle);
    changeIsosurfacesColorMap(value);
    exportIsosurfacesColorMap(isosurfaceColormap);
    mooseViewer->updateIsosurfaceColorMap(isosurfaceColormap);
    Vrui::requestUpdate();
} // end changeIsosurfacesColorMapCallback()


/*
 * colorSliderCallback - Callback of change to color slider value.
 *
 * parameter _callbackData - Misc::CallbackData*
 */
void Isosurfaces::colorSliderCallback(Misc::CallbackData* _callbackData) {
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
void Isosurfaces::colorSwatchesWidgetCallback(Misc::CallbackData* _callbackData) {
    float* _color = swatchesWidget->getCurrentColor();
    RGBAColor* rgbaColor = new RGBAColor(_color[0], _color[1], _color[2], 1.0f);
    for (int i = 0; i < 3; ++i) {
        colorSliders[i]->setValue(_color[i]);
    }
    colorPane->setBackgroundColor(rgbaColor->getValues());
    colorMap->setControlPointColor(*rgbaColor);
} // end colorSwatchesWidgetCallback()

/*
 * controlPointChangedCallback - Callback to select a control point.
 *
 * parameter _callbackData - Misc::CallbackData*
 */
void Isosurfaces::controlPointChangedCallback(Misc::CallbackData* _callbackData) {
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
 * createButtonBox - Create a box to hold buttons.
 *
 * parameter colorMapDialog - GLMotif::RowColumn * &
 * return - GLMotif::RowColumn *
 */
GLMotif::RowColumn * Isosurfaces::createButtonBox(GLMotif::RowColumn * & colorMapDialog) {
    GLMotif::RowColumn * buttonBox = new GLMotif::RowColumn("ButtonBox", colorMapDialog, false);
    buttonBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
    GLMotif::Button * removeControlPointButton =
            new GLMotif::Button("RemoveControlPointButton", buttonBox, "Remove RGB Control Point");
    removeControlPointButton->getSelectCallbacks().add(this, &Isosurfaces::removeControlPointCallback);
    // Selection menu for colormap
    GLMotif::CascadeButton * isosurfaceColorMapSubCascade =
            new GLMotif::CascadeButton("IsosurfaceColorMapSubCascade", buttonBox, "Color Map");
    isosurfaceColorMapSubCascade->setPopup(createIsosurfaceColorMapSubMenu());
    return buttonBox;
}

/*
 * createColorEditor - Create color editor.
 *
 * parameter styleSheet - const GLMotif::StyleSheet&
 * parameter colorMapDialog - GLMotif::RowColumn*&
 * return - GLMotif::RowColumn*
 */
GLMotif::RowColumn* Isosurfaces::createColorEditor(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn*& colorMapDialog) {
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
void Isosurfaces::createColorMap(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn*& colorMapDialog) {
    colorMap = new ColorMap("ColorMap", colorMapDialog);
    colorMap->setBorderWidth(styleSheet.size * 0.5f);
    colorMap->setBorderType(GLMotif::Widget::LOWERED);
    colorMap->setForegroundColor(GLMotif::Color(0.0f, 1.0f, 0.0f));
    colorMap->setMarginWidth(styleSheet.size);
    colorMap->setPreferredSize(GLMotif::Vector(styleSheet.fontHeight * 5.0, styleSheet.fontHeight * 5.0, 0.0f));
    colorMap->setControlPointSize(styleSheet.size);
    RGBAColor* rgbaColor = new RGBAColor(1.0f, 0.0f, 0.0f, 1.0f);
    colorMap->setControlPointColor(rgbaColor);
    colorMap->getControlPointChangedCallbacks().add(this, &Isosurfaces::controlPointChangedCallback);
}

/*
 * createColorMapDialog - Create color map dialog.
 *
 * parameter styleSheet - const GLMotif::StyleSheet&
 */
void Isosurfaces::createColorMapDialog(const GLMotif::StyleSheet& styleSheet) {
    GLMotif::RowColumn* colorMapDialog = new GLMotif::RowColumn("ColorMapDialog", this, false);
    createColorMap(styleSheet, colorMapDialog);
    createColorPanel(styleSheet, colorMapDialog);
    GLMotif::RowColumn* colorEditor = createColorEditor(styleSheet, colorMapDialog);
    colorEditor->manageChild();
    createABCIsosurfaces(styleSheet, colorMapDialog);
    GLMotif::RowColumn* buttonBox = createButtonBox(colorMapDialog);
    buttonBox->manageChild();
    colorMapDialog->manageChild();
}

/*
 * createColorPanel - Create color panel.
 *
 * parameter styleSheet - const GLMotif::StyleSheet&
 * parameter colorMapDialog - GLMotif::RowColumn*&
 */
void Isosurfaces::createColorPanel(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn*& colorMapDialog) {
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
void Isosurfaces::createColorSwatchesWidget(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn*& colorEditor) {
    swatchesWidget = new SwatchesWidget("SwatchesWidget", colorEditor);
    swatchesWidget->setBorderWidth(styleSheet.size * 0.5f);
    swatchesWidget->setBorderType(GLMotif::Widget::LOWERED);
    swatchesWidget->setForegroundColor(GLMotif::Color(0.0f, 1.0f, 0.0f));
    swatchesWidget->setMarginWidth(styleSheet.size);
    swatchesWidget->setPreferredSize(GLMotif::Vector(styleSheet.fontHeight * 15.0, styleSheet.fontHeight * 5.0, 0.0f));
    swatchesWidget->getColorChangedCallbacks().add(this, &Isosurfaces::colorSwatchesWidgetCallback);
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
GLMotif::Slider* Isosurfaces::createColorSlider(const char* title, GLMotif::Color color, const GLMotif::StyleSheet& styleSheet,
        GLMotif::RowColumn* colorSlidersBox) {
    GLMotif::Slider* colorSlider = new GLMotif::Slider(title, colorSlidersBox, GLMotif::Slider::VERTICAL, styleSheet.fontHeight
            * 5.0f);
    colorSlider->setSliderColor(color);
    colorSlider->setValueRange(0.0f, 1.0f, 0.01f);
    colorSlider->setValue(0.5f);
    colorSlider->getValueChangedCallbacks().add(this, &Isosurfaces::colorSliderCallback);
    return colorSlider;
}

/*
 * createColorSliderBox - Create box to contain color slider bars.
 *
 * parameter styleSheet - const GLMotif::StyleSheet&
 * parameter colorEditor - GLMotif::RowColumn*
 * return - GLMotif::RowColumn*
 */
GLMotif::RowColumn* Isosurfaces::createColorSliderBox(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn* colorEditor) {
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
void Isosurfaces::createColorSliders(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn* colorSlidersBox) {
    colorSliders[0] = createColorSlider("RedSlider", GLMotif::Color(1.0f, 0.0f, 0.0f), styleSheet, colorSlidersBox);
    colorSliders[1] = createColorSlider("GreenSlider", GLMotif::Color(0.0f, 1.0f, 0.0f), styleSheet, colorSlidersBox);
    colorSliders[2] = createColorSlider("BlueSlider", GLMotif::Color(0.0f, 0.0f, 1.0f), styleSheet, colorSlidersBox);
}

/*
 * createIsosurfaceColorMapSubMenu - Creates a submenu of the available isosurface color maps.
 *
 * return - GLMotif::Popup *
 */
GLMotif::Popup * Isosurfaces::createIsosurfaceColorMapSubMenu(void) {
    GLMotif::Popup * isosurfaceColorMapSubMenuPopup = new GLMotif::Popup("IsosurfaceColorMapSubMenuPopup", Vrui::getWidgetManager());
    GLMotif::RadioBox * isosurfaceColorMaps = new GLMotif::RadioBox("IsosurfaceColorMaps", isosurfaceColorMapSubMenuPopup, false);
    isosurfaceColorMaps->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
    isosurfaceColorMaps->addToggle("Full Rainbow");
    isosurfaceColorMaps->addToggle("Inverse Full Rainbow");
    isosurfaceColorMaps->addToggle("Rainbow");
    isosurfaceColorMaps->addToggle("Inverse Rainbow");
    isosurfaceColorMaps->addToggle("Cold to Hot");
    isosurfaceColorMaps->addToggle("Hot to Cold");
    isosurfaceColorMaps->addToggle("Black to White");
    isosurfaceColorMaps->addToggle("White to Black");
    isosurfaceColorMaps->addToggle("HSB Hues");
    isosurfaceColorMaps->addToggle("Inverse HSB Hues");
    isosurfaceColorMaps->addToggle("Davinci");
    isosurfaceColorMaps->addToggle("Inverse Davinci");
    isosurfaceColorMaps->addToggle("Seismic");
    isosurfaceColorMaps->addToggle("Inverse Seismic");
    isosurfaceColorMaps->setSelectedToggle(3);
    isosurfaceColorMaps->getValueChangedCallbacks().add(this, &Isosurfaces::changeIsosurfacesColorMapCallback);
    isosurfaceColorMaps->manageChild();
    return isosurfaceColorMapSubMenuPopup;
} // end createIsosurfaceColorMapSubMenu()

/*
 * createAIsosurfaces - Create x isosurface widgets.
 *
 * parameter abcIsosurfacesRowColumn - GLMotif::RowColumn * &
 * parameter styleSheet - const GLMotif::StyleSheet&
 */
void Isosurfaces::createAIsosurfaces(GLMotif::RowColumn * & abcIsosurfacesRowColumn, const GLMotif::StyleSheet & styleSheet) {
    //create x isosurface button and slider+textfield
    GLMotif::ToggleButton * showAIsosurfaceToggle = new GLMotif::ToggleButton("ShowAIsosurfacesToggle", abcIsosurfacesRowColumn, "A");
    showAIsosurfaceToggle->setToggle(false);
    showAIsosurfaceToggle->getValueChangedCallbacks().add(this, &Isosurfaces::toggleSelectCallback);
    showAIsosurfaceToggle->setToggleWidth(0.1f);
    AIsosurfacesValue = new GLMotif::TextField("AIsosurfaceValue", abcIsosurfacesRowColumn, 7);
    AIsosurfacesValue->setPrecision(3);
    AIsosurfacesValue->setValue(0);
    GLMotif::Slider * AIsosurfacesSlider = new GLMotif::Slider("AIsosurfaceSlider", abcIsosurfacesRowColumn, GLMotif::Slider::HORIZONTAL, styleSheet.fontHeight * 10.0f);
    AIsosurfacesSlider->setValueRange(0, 255, 1);
    AIsosurfacesSlider->setValue(0);
    AIsosurfacesSlider->getValueChangedCallbacks().add(this, &Isosurfaces::sliderCallback);
} // end createXIsosurfaces()

/*
 * createABCIsosurfaces - Create x, y and z isosurface widgets.
 *
 * parameter styleSheet - const GLMotif::StyleSheet&
 * parameter colorMapDialog - GLMotif::RowColumn*&
 */
void Isosurfaces::createABCIsosurfaces(const GLMotif::StyleSheet& styleSheet, GLMotif::RowColumn*& colorMapDialog) {
    GLMotif::RowColumn * abcIsosurfacesRowColumn = new GLMotif::RowColumn("ABCIsosurfacesRowColumn", colorMapDialog, false);
    abcIsosurfacesRowColumn->setOrientation(GLMotif::RowColumn::VERTICAL);
    abcIsosurfacesRowColumn->setNumMinorWidgets(GLsizei(3));
    createAIsosurfaces(abcIsosurfacesRowColumn, styleSheet);
    createBIsosurfaces(abcIsosurfacesRowColumn, styleSheet);
    createCIsosurfaces(abcIsosurfacesRowColumn, styleSheet);

    abcIsosurfacesRowColumn->manageChild();
} // end createABCIsosurfaces()createABCIsosurfaces

/*
 * createYIsosurfaces - Create x isosurface widgets.
 *
 * parameter abcIsosurfacesRowColumn - GLMotif::RowColumn * &
 * parameter styleSheet - const GLMotif::StyleSheet&
 */
void Isosurfaces::createBIsosurfaces(GLMotif::RowColumn * & abcIsosurfacesRowColumn, const GLMotif::StyleSheet & styleSheet) {
    //create y isosurface button and slider+textfield
    GLMotif::ToggleButton * showBIsosurfaceToggle = new GLMotif::ToggleButton("ShowBIsosurfacesToggle", abcIsosurfacesRowColumn, "B");
    showBIsosurfaceToggle->setToggle(false);
    showBIsosurfaceToggle->getValueChangedCallbacks().add(this, &Isosurfaces::toggleSelectCallback);
    BIsosurfacesValue = new GLMotif::TextField("BIsosurfaceValue", abcIsosurfacesRowColumn, 7);
    BIsosurfacesValue->setPrecision(3);
    BIsosurfacesValue->setValue(0);
    GLMotif::Slider * BIsosurfacesSlider = new GLMotif::Slider("BIsosurfaceSlider", abcIsosurfacesRowColumn, GLMotif::Slider::HORIZONTAL, styleSheet.fontHeight * 10.0f);
    BIsosurfacesSlider->setValueRange(0, 255, 1);
    BIsosurfacesSlider->setValue(0);
    BIsosurfacesSlider->getValueChangedCallbacks().add(this, &Isosurfaces::sliderCallback);
} // end createYIsosurfaces()

/*
 * createZIsosurfaces - Create x isosurface widgets.
 *
 * parameter abcIsosurfacesRowColumn - GLMotif::RowColumn * &
 * parameter styleSheet - const GLMotif::StyleSheet&
 */
void Isosurfaces::createCIsosurfaces(GLMotif::RowColumn * & abcIsosurfacesRowColumn, const GLMotif::StyleSheet & styleSheet) {
    //create z isosurface button and slider+textfield
    GLMotif::ToggleButton * showCIsosurfacesToggle = new GLMotif::ToggleButton("ShowCIsosurfacesToggle", abcIsosurfacesRowColumn, "C");
    showCIsosurfacesToggle->setToggle(false);
    showCIsosurfacesToggle->getValueChangedCallbacks().add(this, &Isosurfaces::toggleSelectCallback);
    CIsosurfacesValue = new GLMotif::TextField("CIsosurfaceValue", abcIsosurfacesRowColumn, 7);
    CIsosurfacesValue->setPrecision(3);
    CIsosurfacesValue->setValue(0);
    GLMotif::Slider * CIsosurfacesSlider = new GLMotif::Slider("CIsosurfaceSlider", abcIsosurfacesRowColumn, GLMotif::Slider::HORIZONTAL, styleSheet.fontHeight * 10.0f);
    CIsosurfacesSlider->setValueRange(0, 255, 1);
    CIsosurfacesSlider->setValue(0);
    CIsosurfacesSlider->getValueChangedCallbacks().add(this, &Isosurfaces::sliderCallback);
} // end createZIsosurfaces()

/*
 * exportIsosurfacesColorMap - Export the color map.
 *
 * parameter colormap - unsigned char *
 */
void Isosurfaces::exportIsosurfacesColorMap(double* colormap) const {
    colorMap->exportColorMap(colormap);
} // end exportIsosurfacesColorMap()

/*
 * getColorMap - Get the isosurface color map.
 *
 * return - Storage *
 */
Storage * Isosurfaces::getColorMap(void) const {
    return colorMap->getColorMap();
} // end getColorMap()

/*
 * setColorMap - Set the isosurface color map.
 *
 * parameter storage - Storage*
 */
void Isosurfaces::setColorMap(Storage* storage) {
    colorMap->setColorMap(storage);
} // end setColorMap()

/*
 * getIsosurfacesColorMap
 *
 * return const ColorMap *
 */
const ColorMap * Isosurfaces::getIsosurfacesColorMap(void) const {
    return colorMap;
} // end getIsosurfacesColorMap()

/*
 * getIsosurfacesColorMap
 *
 * return - ColorMap *
 */
ColorMap * Isosurfaces::getIsosurfacesColorMap(void) {
    return colorMap;
} // end getIsosurfacesColorMap()

/*
 * setIsosurfacesColorMap - Create the isosurface color map.
 *
 * parameter colorMapCreationType - int
 * parameter _minimum - double
 * parameter _maximum - double
 */
void Isosurfaces::setIsosurfacesColorMap(int colorMapCreationType, double _minimum, double _maximum) {
    colorMap->createColorMap(colorMapCreationType, _minimum, _maximum);
} // end setIsosurfacesColorMap()

/*
 * getIsosurfacesColorMapChangedCallbacks
 *
 * return - Misc::CallbackList &
 */
Misc::CallbackList& Isosurfaces::getIsosurfacesColorMapChangedCallbacks(void) {
    return colorMap->getColorMapChangedCallbacks();
} // end getIsosurfacesColorMapChangedCallbacks()

/*
 * initialize - Initialize the GUI for the Isosurfaces class.
 */
void Isosurfaces::initialize(void) {
    const GLMotif::StyleSheet& styleSheet = *Vrui::getWidgetManager()->getStyleSheet();
    createColorMapDialog(styleSheet);
    getIsosurfacesColorMapChangedCallbacks().add(this, &Isosurfaces::isosurfaceColorMapChangedCallback);

}

/*
 * removeControlPointCallback - Remove the current control point.
 *
 * parameter _callbackData - Misc::CallbackData*
 */
void Isosurfaces::removeControlPointCallback(Misc::CallbackData* _callbackData) {
    colorMap->deleteControlPoint();
} // end removeControlPointCallback()

/*
 * isosurfaceColorMapChangedCallback - Change the isosurface color map.
 *
 * parameter callBackData - Misc::CallbackData *
 */
void Isosurfaces::isosurfaceColorMapChangedCallback(Misc::CallbackData * callBackData) {
    exportIsosurfacesColorMap(isosurfaceColormap);
    mooseViewer->updateIsosurfaceColorMap(isosurfaceColormap);
    Vrui::requestUpdate();
} // end isosurfaceColorMapChangedCallback()

/*
 * sliderCallback
 *
 * parameter callBackData - GLMotif::Slider::ValueChangedCallbackData *
 */
void Isosurfaces::sliderCallback(GLMotif::Slider::ValueChangedCallbackData * callBackData) {
    if (strcmp(callBackData->slider->getName(), "AIsosurfaceSlider") == 0) {
        AIsosurfacesValue->setValue(callBackData->value);
        mooseViewer->setAIsosurface(float(callBackData->value));
        Vrui::requestUpdate();
    }
    if (strcmp(callBackData->slider->getName(), "BIsosurfaceSlider") == 0) {
        BIsosurfacesValue->setValue(callBackData->value);
        mooseViewer->setBIsosurface(float(callBackData->value));
        Vrui::requestUpdate();
    }
    if (strcmp(callBackData->slider->getName(), "CIsosurfaceSlider") == 0) {
        CIsosurfacesValue->setValue(callBackData->value);
        mooseViewer->setCIsosurface(float(callBackData->value));
        Vrui::requestUpdate();
    }
} // end sliderCallback()

/*
 * toggleSelectCallback - Adjust the gui/program state based on which toggle button changed state.
 *
 * parameter callBackData - GLMotif::ToggleButton::ValueChangedCallbackData*
 */
void Isosurfaces::toggleSelectCallback(GLMotif::ToggleButton::ValueChangedCallbackData* callBackData) {
    /* Adjust gui/program state based on which toggle button changed state: */
    if (strcmp(callBackData->toggle->getName(), "ShowAIsosurfacesToggle") == 0) {
        mooseViewer->showAIsosurface(callBackData->set);
    } else if (strcmp(callBackData->toggle->getName(), "ShowBIsosurfacesToggle") == 0) {
        mooseViewer->showBIsosurface(callBackData->set);
    } else if (strcmp(callBackData->toggle->getName(), "ShowCIsosurfacesToggle") == 0) {
        mooseViewer->showCIsosurface(callBackData->set);
    }
    Vrui::requestUpdate();
} // end toggleSelectCallback()
