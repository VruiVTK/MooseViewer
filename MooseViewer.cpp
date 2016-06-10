// STL includes
#include <algorithm>
#include <iostream>
#include <sstream>

// Must come before any gl.h include
#include <GL/glew.h>

// VTK includes
#include <ExternalVTKWidget.h>
#include <vtkCellData.h>
#include <vtkCompositeDataIterator.h>
#include <vtkDataSet.h>
#include <vtkLookupTable.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkPointData.h>

// OpenGL/Motif includes
#include <GL/GLContextData.h>
#include <GLMotif/CascadeButton.h>
#include <GLMotif/Menu.h>
#include <GLMotif/Popup.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/RadioBox.h>
#include <GLMotif/Separator.h>
#include <GLMotif/ScrolledListBox.h>
#include <GLMotif/SubMenu.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/WidgetManager.h>

// VRUI includes
#include <Vrui/Application.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Vrui.h>
#include <Vrui/VRWindow.h>
#include <Vrui/WindowProperties.h>

// vtkVRUI includes
#include <vvFramerate.h>

// MooseViewer includes
#include "AnimationDialog.h"
#include "ColorMap.h"
#include "Contours.h"
#include "MooseViewer.h"
#include "mvApplicationState.h"
#include "mvContours.h"
#include "mvGeometry.h"
#include "mvInteractorTool.h"
#include "mvMouseRotationTool.h"
#include "mvOutline.h"
#include "mvReader.h"
#include "mvSlice.h"
#include "mvVolume.h"
#include "ScalarWidget.h"
#include "TransferFunction1D.h"
#include "VariablesDialog.h"
#include "WidgetHints.h"


//----------------------------------------------------------------------------
MooseViewer::MooseViewer(int& argc,char**& argv)
  : Superclass(argc, argv, new mvApplicationState),
    m_mvState(*static_cast<mvApplicationState*>(m_state)),
    colorByVariablesMenu(0),
    ContoursDialog(NULL),
    Histogram(new float[256]),
    IsPlaying(false),
    Loop(false),
    mainMenu(NULL),
    m_colorMapCache(new double[256 * 4]),
    opacityValue(NULL),
    renderingDialog(NULL),
    sampleValue(NULL),
    variablesDialog(0)
{
  std::fill(m_colorMapCache, m_colorMapCache + 4 * 256, -1.); // invalid
  std::fill(this->Histogram, this->Histogram + 256, 0.f);

  this->ScalarRange[0] = 0.0;
  this->ScalarRange[1] = 255.0;

  // Add tool factories:
  Vrui::ToolManager *toolMgr = Vrui::getToolManager();

  Vrui::ToolFactory *factory = new mvInteractorToolFactory(*toolMgr);
  toolMgr->addClass(factory, Vrui::ToolManager::defaultToolFactoryDestructor);

  factory = new mvMouseRotationToolFactory(*toolMgr);
  toolMgr->addClass(factory, Vrui::ToolManager::defaultToolFactoryDestructor);

  // TESTING: Enable benchmarking output:
  m_mvState.reader().setBenchmark(true);
  m_mvState.slice().setBenchmark(true);
  m_mvState.contours().setBenchmark(true);
  m_mvState.volume().setBenchmark(true);
  m_mvState.geometry().setBenchmark(true);
}

//----------------------------------------------------------------------------
MooseViewer::~MooseViewer(void)
{
  delete[] m_colorMapCache;
  delete[] this->Histogram;

  delete this->AnimationControl;
  delete this->ColorEditor;
  delete this->ContoursDialog;
  delete this->mainMenu;
  delete this->renderingDialog;
  delete this->variablesDialog;
}

//----------------------------------------------------------------------------
void MooseViewer::initialize()
{
  this->Superclass::initialize();

  // Start async file read.
  m_mvState.reader().update(m_mvState);

  if (!this->widgetHintsFile.empty())
    {
    m_mvState.widgetHints().loadFile(this->widgetHintsFile);
    }
  else
    {
    m_mvState.widgetHints().reset();
    }

  /* Create the user interface: */
  this->variablesDialog = new VariablesDialog;
  this->updateVariablesDialog();
  this->variablesDialog->getScrolledListBox()->getListBox()->
      getSelectionChangedCallbacks().add(
        this, &MooseViewer::changeVariablesCallback);

  renderingDialog = createRenderingDialog();
  mainMenu=createMainMenu();
  Vrui::setMainMenu(mainMenu);

  /* Initialize the color editor */
  this->ColorEditor = new TransferFunction1D(this);
  this->ColorEditor->createTransferFunction1D(CINVERSE_RAINBOW,
    UP_RAMP, 0.0, 1.0);
  this->ColorEditor->getColorMapChangedCallbacks().add(
    this, &MooseViewer::colorMapChangedCallback);
  this->ColorEditor->getAlphaChangedCallbacks().add(this,
    &MooseViewer::alphaChangedCallback);
  updateColorMap();

  /* Contours */
  this->ContoursDialog = new Contours(this);
  this->ContoursDialog->getAlphaChangedCallbacks().add(this,
    &MooseViewer::contourValueChangedCallback);

  /* Initialize the Animation control */
  this->AnimationControl = new AnimationDialog(this);

  // TODO This is ugly, it'd be great to find a way around this.
  // Force sync reader output:
  while (m_mvState.reader().running(std::chrono::seconds(1)))
    {
    std::cout << "Waiting for initial file read to complete..." << std::endl;
    }
  m_mvState.reader().update(m_mvState); // Update cached data object
}

//----------------------------------------------------------------------------
void MooseViewer::setFileName(const std::string &name)
{
  m_mvState.reader().setFileName(name);
  m_mvState.reader().updateInformation();
}

//----------------------------------------------------------------------------
const std::string& MooseViewer::getFileName(void)
{
  return m_mvState.reader().fileName();
}

//----------------------------------------------------------------------------
void MooseViewer::setWidgetHintsFile(const std::string &whFile)
{
  this->widgetHintsFile = whFile;
}

//----------------------------------------------------------------------------
const std::string& MooseViewer::getWidgetHintsFile()
{
  return this->widgetHintsFile;
}

//----------------------------------------------------------------------------
GLMotif::PopupMenu* MooseViewer::createMainMenu(void)
{
  if (!m_mvState.widgetHints().isEnabled("MainMenu"))
    {
    std::cerr << "Ignoring hint to hide MainMenu." << std::endl;
    }
  m_mvState.widgetHints().pushGroup("MainMenu");

  GLMotif::PopupMenu* mainMenuPopup =
    new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
  mainMenuPopup->setTitle("Main Menu");
  GLMotif::Menu* mainMenu = new GLMotif::Menu("MainMenu",mainMenuPopup,false);

  if (m_mvState.widgetHints().isEnabled("Variables"))
    {
    GLMotif::ToggleButton *showVariablesDialog =
        new GLMotif::ToggleButton("ShowVariablesDialog", mainMenu, "Variables");
    showVariablesDialog->setToggle(false);
    showVariablesDialog->getValueChangedCallbacks().add(
          this, &MooseViewer::showVariableDialogCallback);
    }

  if (m_mvState.widgetHints().isEnabled("ColorBy"))
    {
    GLMotif::CascadeButton* colorByVariablesCascade =
        new GLMotif::CascadeButton("colorByVariablesCascade", mainMenu,
                                   "Color By");
    colorByVariablesCascade->setPopup(createColorByVariablesMenu());
    }

  if (m_mvState.widgetHints().isEnabled("ColorMap"))
    {
    GLMotif::CascadeButton * colorMapSubCascade =
        new GLMotif::CascadeButton("ColorMapSubCascade", mainMenu, "Color Map");
    colorMapSubCascade->setPopup(createColorMapSubMenu());
    }

  if (m_mvState.widgetHints().isEnabled("ColorEditor"))
    {
    GLMotif::ToggleButton * showColorEditorDialog =
        new GLMotif::ToggleButton("ShowColorEditorDialog", mainMenu,
                                  "Color Editor");
    showColorEditorDialog->setToggle(false);
    showColorEditorDialog->getValueChangedCallbacks().add(
          this, &MooseViewer::showColorEditorDialogCallback);
    }

  if (m_mvState.widgetHints().isEnabled("Animation"))
    {
    GLMotif::ToggleButton * showAnimationDialog =
        new GLMotif::ToggleButton("ShowAnimationDialog", mainMenu,
                                  "Animation");
    showAnimationDialog->setToggle(false);
    showAnimationDialog->getValueChangedCallbacks().add(
          this, &MooseViewer::showAnimationDialogCallback);
    }

  if (m_mvState.widgetHints().isEnabled("Representation"))
    {
    GLMotif::CascadeButton* representationCascade =
        new GLMotif::CascadeButton("RepresentationCascade", mainMenu,
                                   "Representation");
    representationCascade->setPopup(createRepresentationMenu());
    }

  if (m_mvState.widgetHints().isEnabled("AnalysisTools"))
    {
    GLMotif::CascadeButton* analysisToolsCascade =
        new GLMotif::CascadeButton("AnalysisToolsCascade", mainMenu,
                                   "Analysis Tools");
    analysisToolsCascade->setPopup(createAnalysisToolsMenu());
    }

  if (m_mvState.widgetHints().isEnabled("Contours"))
    {
    GLMotif::ToggleButton * showContoursDialog = new GLMotif::ToggleButton(
          "ShowContoursDialog", mainMenu, "Contours");
    showContoursDialog->setToggle(false);
    showContoursDialog->getValueChangedCallbacks().add(
          this, &MooseViewer::showContoursDialogCallback);
    }

  if (m_mvState.widgetHints().isEnabled("CenterDisplay"))
    {
    GLMotif::Button* centerDisplayButton =
        new GLMotif::Button("CenterDisplayButton",mainMenu,"Center Display");
    centerDisplayButton->getSelectCallbacks().add(
          this, &MooseViewer::centerDisplayCallback);
    }

  if (m_mvState.widgetHints().isEnabled("ToggleFPS"))
    {
    GLMotif::Button* toggleFPSButton =
        new GLMotif::Button("ToggleFPS",mainMenu,"Toggle FPS");
    toggleFPSButton->getSelectCallbacks().add(
          this, &MooseViewer::toggleFPSCallback);
    }

  if (m_mvState.widgetHints().isEnabled("Rendering"))
    {
    GLMotif::ToggleButton * showRenderingDialog =
        new GLMotif::ToggleButton("ShowRenderingDialog", mainMenu,
                                  "Rendering");
    showRenderingDialog->setToggle(false);
    showRenderingDialog->getValueChangedCallbacks().add(
          this, &MooseViewer::showRenderingDialogCallback);
    }

  mainMenu->manageChild();

  m_mvState.widgetHints().popGroup();

  return mainMenuPopup;
}

//----------------------------------------------------------------------------
GLMotif::Popup* MooseViewer::createRepresentationMenu(void)
{
  m_mvState.widgetHints().pushGroup("Representation");

  const GLMotif::StyleSheet* ss = Vrui::getWidgetManager()->getStyleSheet();

  GLMotif::Popup* representationMenuPopup =
    new GLMotif::Popup("representationMenuPopup", Vrui::getWidgetManager());
  GLMotif::SubMenu* representationMenu = new GLMotif::SubMenu(
    "representationMenu", representationMenuPopup, false);

  if (m_mvState.widgetHints().isEnabled("Outline"))
    {
    GLMotif::ToggleButton* showOutline=new GLMotif::ToggleButton(
          "ShowOutline",representationMenu,"Outline");
    showOutline->getValueChangedCallbacks().add(
          this,&MooseViewer::changeRepresentationCallback);
    showOutline->setToggle(true);
    }

  if (m_mvState.widgetHints().isEnabled("Representations"))
    {
    GLMotif::Label* representation_Label = new GLMotif::Label(
          "Representations", representationMenu,"Representations:");
    }

  GLMotif::RadioBox* representation_RadioBox =
    new GLMotif::RadioBox("Representation RadioBox",representationMenu,true);

  // Set to the default selected option. If left NULL, the first representation
  // is chosen.
  GLMotif::ToggleButton *selected = NULL;

  if (m_mvState.widgetHints().isEnabled("None"))
    {
    GLMotif::ToggleButton* showNone=new GLMotif::ToggleButton(
          "ShowNone",representation_RadioBox,"None");
    showNone->getValueChangedCallbacks().add(
          this,&MooseViewer::changeRepresentationCallback);
    }
  if (m_mvState.widgetHints().isEnabled("Points"))
    {
    GLMotif::ToggleButton* showPoints=new GLMotif::ToggleButton(
          "ShowPoints",representation_RadioBox,"Points");
    showPoints->getValueChangedCallbacks().add(
          this,&MooseViewer::changeRepresentationCallback);
    }
  if (m_mvState.widgetHints().isEnabled("Wireframe"))
    {
    GLMotif::ToggleButton* showWireframe=new GLMotif::ToggleButton(
          "ShowWireframe",representation_RadioBox,"Wireframe");
    showWireframe->getValueChangedCallbacks().add(
          this,&MooseViewer::changeRepresentationCallback);
    }
  if (m_mvState.widgetHints().isEnabled("Surface"))
    {
    GLMotif::ToggleButton* showSurface=new GLMotif::ToggleButton(
          "ShowSurface",representation_RadioBox,"Surface");
    showSurface->getValueChangedCallbacks().add(
          this,&MooseViewer::changeRepresentationCallback);
    selected = showSurface;
    }
  if (m_mvState.widgetHints().isEnabled("SurfaceWithEdges"))
    {
    GLMotif::ToggleButton* showSurfaceWithEdges=new GLMotif::ToggleButton(
          "ShowSurfaceWithEdges",representation_RadioBox,"Surface with Edges");
    showSurfaceWithEdges->getValueChangedCallbacks().add(
          this,&MooseViewer::changeRepresentationCallback);
    }
  if (m_mvState.widgetHints().isEnabled("Volume"))
    {
    GLMotif::ToggleButton* showVolume=new GLMotif::ToggleButton(
          "ShowVolume",representation_RadioBox,"Volume");
    showVolume->getValueChangedCallbacks().add(
          this,&MooseViewer::changeRepresentationCallback);
    }

  representation_RadioBox->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
  if (selected != NULL)
    {
    representation_RadioBox->setSelectedToggle(selected);
    }
  else
    {
    representation_RadioBox->setSelectedToggle(0);
    }

  representationMenu->manageChild();
  m_mvState.widgetHints().popGroup();
  return representationMenuPopup;
}

//----------------------------------------------------------------------------
GLMotif::Popup * MooseViewer::createAnalysisToolsMenu(void)
{
  m_mvState.widgetHints().pushGroup("AnalysisTools");

  const GLMotif::StyleSheet* ss = Vrui::getWidgetManager()->getStyleSheet();

  GLMotif::Popup * analysisToolsMenuPopup = new GLMotif::Popup(
    "analysisToolsMenuPopup", Vrui::getWidgetManager());
  GLMotif::SubMenu* analysisToolsMenu = new GLMotif::SubMenu(
    "representationMenu", analysisToolsMenuPopup, false);

  GLMotif::RadioBox * analysisTools_RadioBox = new GLMotif::RadioBox(
    "analysisTools", analysisToolsMenu, true);

  if (m_mvState.widgetHints().isEnabled("Slice"))
    {
    GLMotif::ToggleButton *showSlice = new GLMotif::ToggleButton(
          "Slice", analysisTools_RadioBox, "Slice");
    showSlice->getValueChangedCallbacks().add(
          this,&MooseViewer::changeAnalysisToolsCallback);
    }

  analysisTools_RadioBox->setSelectionMode(GLMotif::RadioBox::ATMOST_ONE);

  analysisToolsMenu->manageChild();
  m_mvState.widgetHints().popGroup();
  return analysisToolsMenuPopup;
}

//----------------------------------------------------------------------------
GLMotif::Popup* MooseViewer::createColorByVariablesMenu(void)
{
  const GLMotif::StyleSheet* ss = Vrui::getWidgetManager()->getStyleSheet();

  GLMotif::Popup* colorByVariablesMenuPopup =
    new GLMotif::Popup("colorByVariablesMenuPopup", Vrui::getWidgetManager());
  this->colorByVariablesMenu = new GLMotif::SubMenu(
    "colorByVariablesMenu", colorByVariablesMenuPopup, false);

  this->colorByVariablesMenu->manageChild();
  return colorByVariablesMenuPopup;
}

//----------------------------------------------------------------------------
void MooseViewer::updateVariablesDialog(void)
{
  // Add to dialog:
  this->variablesDialog->clearAllVariables();
  for (const auto &var : m_mvState.reader().availableVariables())
    {
    this->variablesDialog->addVariable(var);
    }
}

//----------------------------------------------------------------------------
void MooseViewer::updateColorByVariablesMenu(void)
{
  /* Preserve the selection */
  std::string selectedToggle = m_mvState.colorByArray();

  /* Clear the menu first */
  for (int i = this->colorByVariablesMenu->getNumRows(); i >= 0; --i)
    {
    colorByVariablesMenu->removeWidgets(i);
    }

  if (m_mvState.reader().requestedVariables().size() > 0)
    {
    using GLMotif::RadioBox;
    using GLMotif::ToggleButton;

    RadioBox *box = new RadioBox("Color RadioBox", colorByVariablesMenu);

    int currentIndex = 0;
    int selectedIndex = -1;
    for (const auto &var : m_mvState.reader().requestedVariables())
      {
      ToggleButton *button = new ToggleButton(var.c_str(), box, var.c_str());
      button->getValueChangedCallbacks().add(
        this, &MooseViewer::changeColorByVariablesCallback);
      button->setToggle(false);
      if (!selectedToggle.empty() && (selectedIndex < 0) &&
          (selectedToggle.compare(var) == 0))
        {
        selectedIndex = currentIndex;
        }
      ++currentIndex;
      }

    selectedIndex = selectedIndex > 0 ? selectedIndex : 0;
    box->setSelectedToggle(selectedIndex);
    box->setSelectionMode(RadioBox::ALWAYS_ONE);

    if (ToggleButton *toggle = box->getSelectedToggle())
      {
      m_mvState.setColorByArray(toggle->getName());
      }
    else
      {
      m_mvState.setColorByArray("");
      }

    this->updateScalarRange();
    }
}

//----------------------------------------------------------------------------
GLMotif::Popup* MooseViewer::createColorMapSubMenu(void)
{
  m_mvState.widgetHints().pushGroup("ColorMap");

  GLMotif::Popup * colorMapSubMenuPopup = new GLMotif::Popup(
    "ColorMapSubMenuPopup", Vrui::getWidgetManager());
  GLMotif::RadioBox* colorMaps = new GLMotif::RadioBox(
    "ColorMaps", colorMapSubMenuPopup, false);
  colorMaps->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);

  // This needs to end up pointing at InverseRainbow if enabled, or the first
  // map otherwise:
  int selectedToggle = 0;

  if (m_mvState.widgetHints().isEnabled("FullRainbow"))
    {
    ++selectedToggle;
    colorMaps->addToggle("Full Rainbow");
    }
  if (m_mvState.widgetHints().isEnabled("InverseFullRainbow"))
    {
    ++selectedToggle;
    colorMaps->addToggle("Inverse Full Rainbow");
    }
  if (m_mvState.widgetHints().isEnabled("Rainbow"))
    {
    ++selectedToggle;
    colorMaps->addToggle("Rainbow");
    }
  if (m_mvState.widgetHints().isEnabled("InverseRainbow"))
    {
    colorMaps->addToggle("Inverse Rainbow");
    }
  else
    {
    selectedToggle = 0;
    }
  if (m_mvState.widgetHints().isEnabled("ColdToHot"))
    {
    colorMaps->addToggle("Cold to Hot");
    }
  if (m_mvState.widgetHints().isEnabled("HotToCold"))
    {
    colorMaps->addToggle("Hot to Cold");
    }
  if (m_mvState.widgetHints().isEnabled("BlackToWhite"))
    {
    colorMaps->addToggle("Black to White");
    }
  if (m_mvState.widgetHints().isEnabled("WhiteToBlack"))
    {
    colorMaps->addToggle("White to Black");
    }
  if (m_mvState.widgetHints().isEnabled("HSBHues"))
    {
    colorMaps->addToggle("HSB Hues");
    }
  if (m_mvState.widgetHints().isEnabled("InverseHSBHues"))
    {
    colorMaps->addToggle("Inverse HSB Hues");
    }
  if (m_mvState.widgetHints().isEnabled("Davinci"))
    {
    colorMaps->addToggle("Davinci");
    }
  if (m_mvState.widgetHints().isEnabled("InverseDavinci"))
    {
    colorMaps->addToggle("Inverse Davinci");
    }
  if (m_mvState.widgetHints().isEnabled("Seismic"))
    {
    colorMaps->addToggle("Seismic");
    }
  if (m_mvState.widgetHints().isEnabled("InverseSeismic"))
    {
    colorMaps->addToggle("Inverse Seismic");
    }

  colorMaps->setSelectedToggle(selectedToggle);
  colorMaps->getValueChangedCallbacks().add(this,
    &MooseViewer::changeColorMapCallback);

  colorMaps->manageChild();

  m_mvState.widgetHints().popGroup();

  return colorMapSubMenuPopup;
} // end createColorMapSubMenu()

//----------------------------------------------------------------------------
GLMotif::PopupWindow* MooseViewer::createRenderingDialog(void) {
  const GLMotif::StyleSheet& ss = *Vrui::getWidgetManager()->getStyleSheet();
  GLMotif::PopupWindow * dialogPopup = new GLMotif::PopupWindow(
    "RenderingDialogPopup", Vrui::getWidgetManager(), "Rendering Dialog");
  GLMotif::RowColumn * dialog = new GLMotif::RowColumn(
    "RenderingDialog", dialogPopup, false);
  dialog->setOrientation(GLMotif::RowColumn::VERTICAL);
  dialog->setPacking(GLMotif::RowColumn::PACK_GRID);

  /* Create opacity slider */
  GLMotif::RowColumn * opacityRow = new GLMotif::RowColumn(
    "OpacityRow", dialog, false);
  opacityRow->setOrientation(GLMotif::RowColumn::HORIZONTAL);
  opacityRow->setPacking(GLMotif::RowColumn::PACK_GRID);
  GLMotif::Label* opacityLabel = new GLMotif::Label(
    "OpacityLabel", opacityRow, "Opacity");
  GLMotif::Slider* opacitySlider = new GLMotif::Slider(
    "OpacitySlider", opacityRow, GLMotif::Slider::HORIZONTAL,
    ss.fontHeight*10.0f);
  opacitySlider->setValue(m_mvState.geometry().opacity());
  opacitySlider->setValueRange(0.0, 1.0, 0.1);
  opacitySlider->getValueChangedCallbacks().add(
    this, &MooseViewer::opacitySliderCallback);
  opacityValue = new GLMotif::TextField("OpacityValue", opacityRow, 6);
  opacityValue->setFieldWidth(6);
  opacityValue->setPrecision(3);
  opacityValue->setValue(m_mvState.geometry().opacity());
  opacityRow->manageChild();

  /* Create Volume sampling options sliders */
  GLMotif::RowColumn * sampleRow = new GLMotif::RowColumn(
    "sampleRow", dialog, false);
  sampleRow->setOrientation(GLMotif::RowColumn::HORIZONTAL);
  sampleRow->setPacking(GLMotif::RowColumn::PACK_GRID);
  GLMotif::Label* sampleLabel = new GLMotif::Label(
    "SampleLabel", sampleRow, "Volume Sampling Dimensions");
  GLMotif::Slider* sampleSlider = new GLMotif::Slider(
    "SampleSlider", sampleRow, GLMotif::Slider::HORIZONTAL, ss.fontHeight*10.0f);
  sampleSlider->setValue(m_mvState.volume().sharpness());
  sampleSlider->setValueRange(8.0, 512.0, 8.0);
  sampleSlider->getValueChangedCallbacks().add(
    this, &MooseViewer::sampleSliderCallback);
  sampleValue = new GLMotif::TextField("SampleValue", sampleRow, 6);
  sampleValue->setFieldWidth(6);
  sampleValue->setPrecision(3);
  std::stringstream stringstr;
  stringstr << m_mvState.volume().dimension() << " x "
            << m_mvState.volume().dimension() << " x "
            << m_mvState.volume().dimension();
  sampleValue->setString(stringstr.str().c_str());
  sampleRow->manageChild();

  GLMotif::RowColumn * radiusRow = new GLMotif::RowColumn(
    "RadiusRow", dialog, false);
  radiusRow->setOrientation(GLMotif::RowColumn::HORIZONTAL);
  radiusRow->setPacking(GLMotif::RowColumn::PACK_GRID);
  GLMotif::Label* radiusLabel = new GLMotif::Label(
    "RadiusLabel", radiusRow, "Volume Sampling Radius");
  GLMotif::Slider* radiusSlider = new GLMotif::Slider(
    "RadiusSlider", radiusRow, GLMotif::Slider::HORIZONTAL,
    ss.fontHeight*10.0f);
  radiusSlider->setValue(m_mvState.volume().radius());
  radiusSlider->setValueRange(0.0, 10.0, 0.01);
  radiusSlider->getValueChangedCallbacks().add(
    this, &MooseViewer::radiusSliderCallback);
  radiusValue = new GLMotif::TextField("RadiusValue", radiusRow, 6);
  radiusValue->setFieldWidth(6);
  radiusValue->setPrecision(3);
  if (m_mvState.volume().radius() == 0.)
    {
    radiusValue->setString("Auto");
    }
  else
    {
    radiusValue->setValue(m_mvState.volume().radius());
    }
  radiusRow->manageChild();

  GLMotif::RowColumn * sharpnessRow = new GLMotif::RowColumn(
        "SharpnessRow", dialog, false);
  sharpnessRow->setOrientation(GLMotif::RowColumn::HORIZONTAL);
  sharpnessRow->setPacking(GLMotif::RowColumn::PACK_GRID);
  GLMotif::Label* expLabel = new GLMotif::Label(
        "SharpnessLabel", sharpnessRow, "Volume Sampling Sharpness");
  GLMotif::Slider* sharpnessSlider = new GLMotif::Slider(
        "SharpnessSlider", sharpnessRow, GLMotif::Slider::HORIZONTAL,
        ss.fontHeight*10.0f);
  sharpnessSlider->setValue(m_mvState.volume().sharpness());
  sharpnessSlider->setValueRange(0.1, 5.0, .1);
  sharpnessSlider->getValueChangedCallbacks().add(
        this, &MooseViewer::sharpnessSliderCallback);
  sharpnessValue = new GLMotif::TextField("SharpnessValue", sharpnessRow, 6);
  sharpnessValue->setFieldWidth(6);
  sharpnessValue->setPrecision(3);
  sharpnessValue->setValue(m_mvState.volume().sharpness());
  sharpnessRow->manageChild();

  dialog->manageChild();
  return dialogPopup;
}

//----------------------------------------------------------------------------
void MooseViewer::frame()
{
  // Update internal state:
  m_mvState.reader().update(m_mvState);
  this->updateHistogram();

  this->Superclass::frame();

  // Animation control:
  if (this->IsPlaying)
    {
    int currentTimeStep = m_mvState.reader().timeStep();
    if (currentTimeStep < m_mvState.reader().timeStepRange()[1])
      {
      m_mvState.reader().setTimeStep(currentTimeStep + 1);
      m_mvState.reader().update(m_mvState);
      Vrui::scheduleUpdate(Vrui::getApplicationTime() + 1.0/125.0);
      }
    else if(this->Loop)
      {
      m_mvState.reader().setTimeStep(m_mvState.reader().timeStepRange()[0]);
      m_mvState.reader().update(m_mvState);
      Vrui::scheduleUpdate(Vrui::getApplicationTime() + 1.0/125.0);
      }
    else
      {
      this->AnimationControl->stopAnimation();
      this->IsPlaying = !this->IsPlaying;
      }
    }
  this->AnimationControl->updateTimeInformation();
}

//----------------------------------------------------------------------------
void MooseViewer::initContext(GLContextData& contextData) const
{
  this->Superclass::initContext(contextData);

  // Initialize the display:
  this->centerDisplay();
}

//----------------------------------------------------------------------------
void MooseViewer::display(GLContextData& contextData) const
{
  /* Update color map. */
  // TODO this should be moved to frame().
  auto metaData = m_mvState.reader().variableMetaData(m_mvState.colorByArray());
  if (metaData.valid())
    {
    m_mvState.colorMap().SetTableRange(metaData.range);
    }

  this->Superclass::display(contextData);
}

//----------------------------------------------------------------------------
void MooseViewer::toggleFPSCallback(Misc::CallbackData *cbData)
{
  m_mvState.framerate().setVisible(!m_mvState.framerate().visible());
}

//----------------------------------------------------------------------------
void MooseViewer::centerDisplayCallback(Misc::CallbackData*)
{
  this->centerDisplay();
}

//----------------------------------------------------------------------------
void MooseViewer::opacitySliderCallback(
  GLMotif::Slider::ValueChangedCallbackData* callBackData)
{
  m_mvState.geometry().setOpacity(static_cast<double>(callBackData->value));
  opacityValue->setValue(callBackData->value);
}

//----------------------------------------------------------------------------
void MooseViewer::sampleSliderCallback(
  GLMotif::Slider::ValueChangedCallbackData* callBackData)
{
  m_mvState.volume().setDimension(static_cast<double>(callBackData->value));
  std::stringstream ss;
  ss << m_mvState.volume().dimension() << " x "
     << m_mvState.volume().dimension() << " x "
     << m_mvState.volume().dimension();
  sampleValue->setString(ss.str().c_str());
}

//----------------------------------------------------------------------------
void MooseViewer::radiusSliderCallback(
  GLMotif::Slider::ValueChangedCallbackData* callBackData)
{
  m_mvState.volume().setRadius(static_cast<double>(callBackData->value));
  if (callBackData->value == 0.)
    {
    radiusValue->setString("Auto");
    }
  else
    {
    radiusValue->setValue(callBackData->value);
    }
}

//----------------------------------------------------------------------------
void MooseViewer::sharpnessSliderCallback(
  GLMotif::Slider::ValueChangedCallbackData* callBackData)
{
  m_mvState.volume().setSharpness(static_cast<double>(callBackData->value));
  sharpnessValue->setValue(callBackData->value);
}

//----------------------------------------------------------------------------
void MooseViewer::showVariableDialogCallback(
    GLMotif::ToggleButton::ValueChangedCallbackData *callBackData)
{
  GLMotif::WidgetManager *mgr = Vrui::getWidgetManager();

  if (callBackData->set)
    {
    GLMotif::WidgetManager::Transformation xform =
        mgr->calcWidgetTransformation(mainMenu);
    mgr->popupPrimaryWidget(this->variablesDialog, xform);
    }
  else
    {
    mgr->popdownWidget(this->variablesDialog);
    }
}

//----------------------------------------------------------------------------
void MooseViewer::changeRepresentationCallback(
  GLMotif::ToggleButton::ValueChangedCallbackData* callBackData)
{
  /* Adjust representation state based on which toggle button changed state: */
  if (strcmp(callBackData->toggle->getName(), "ShowSurface") == 0)
    {
    m_mvState.geometry().setRepresentation(mvGeometry::Surface);
    m_mvState.geometry().setVisible(true);
    m_mvState.volume().setVisible(false);
    }
  else if (strcmp(callBackData->toggle->getName(), "ShowSurfaceWithEdges") == 0)
    {
    m_mvState.geometry().setRepresentation(mvGeometry::SurfaceWithEdges);
    m_mvState.geometry().setVisible(true);
    m_mvState.volume().setVisible(false);
    }
  else if (strcmp(callBackData->toggle->getName(), "ShowWireframe") == 0)
    {
    m_mvState.geometry().setRepresentation(mvGeometry::Wireframe);
    m_mvState.geometry().setVisible(true);
    m_mvState.volume().setVisible(false);
    }
  else if (strcmp(callBackData->toggle->getName(), "ShowPoints") == 0)
    {
    m_mvState.geometry().setRepresentation(mvGeometry::Points);
    m_mvState.geometry().setVisible(true);
    m_mvState.volume().setVisible(false);
    }
  else if (strcmp(callBackData->toggle->getName(), "ShowNone") == 0)
    {
    m_mvState.geometry().setRepresentation(mvGeometry::NoGeometry);
    m_mvState.geometry().setVisible(false);
    m_mvState.volume().setVisible(false);
    }
  else if (strcmp(callBackData->toggle->getName(), "ShowVolume") == 0)
    {
    m_mvState.geometry().setRepresentation(mvGeometry::NoGeometry);
    m_mvState.geometry().setVisible(false);
    m_mvState.volume().setVisible(true);
    }
  else if (strcmp(callBackData->toggle->getName(), "ShowOutline") == 0)
    {
    m_mvState.outline().setVisible(callBackData->set);
    }
}
//----------------------------------------------------------------------------
void MooseViewer::changeAnalysisToolsCallback(
  GLMotif::ToggleButton::ValueChangedCallbackData* callBackData)
{
  /* Set the new analysis tool: */
  if (strcmp(callBackData->toggle->getName(), "Slice") == 0)
  {
    m_mvState.slice().setVisible(callBackData->set);
  }
}

//----------------------------------------------------------------------------
void MooseViewer::showRenderingDialogCallback(
  GLMotif::ToggleButton::ValueChangedCallbackData* callBackData)
{
  /* open/close rendering dialog based on which toggle button changed state: */
  if (strcmp(callBackData->toggle->getName(), "ShowRenderingDialog") == 0)
    {
    if (callBackData->set)
      {
      /* Open the rendering dialog at the same position as the main menu: */
      Vrui::getWidgetManager()->popupPrimaryWidget(
        renderingDialog,
        Vrui::getWidgetManager()->calcWidgetTransformation(mainMenu));
      }
    else
      {
      /* Close the rendering dialog: */
      Vrui::popdownPrimaryWidget(renderingDialog);
      }
    }
}

//----------------------------------------------------------------------------
void MooseViewer::showColorEditorDialogCallback(
  GLMotif::ToggleButton::ValueChangedCallbackData* callBackData)
{
  /* open/close transfer function dialog based on which toggle button changed state: */
  if (strcmp(callBackData->toggle->getName(), "ShowColorEditorDialog") == 0)
    {
    if (callBackData->set)
      {
      /* Open the transfer function dialog at the same position as the main menu: */
      Vrui::getWidgetManager()->popupPrimaryWidget(this->ColorEditor,
        Vrui::getWidgetManager()->calcWidgetTransformation(mainMenu));
      }
    else
      {
      /* Close the transfer function dialog: */
      Vrui::popdownPrimaryWidget(this->ColorEditor);
    }
  }
}

//----------------------------------------------------------------------------
void MooseViewer::showAnimationDialogCallback(
  GLMotif::ToggleButton::ValueChangedCallbackData* callBackData)
{
  /* open/close animation dialog based on which toggle button changed state: */
  if (strcmp(callBackData->toggle->getName(), "ShowAnimationDialog") == 0)
    {
    if (callBackData->set)
      {
      /* Open the animation dialog at the same position as the main menu: */
      Vrui::getWidgetManager()->popupPrimaryWidget(this->AnimationControl,
        Vrui::getWidgetManager()->calcWidgetTransformation(mainMenu));
      }
    else
      {
      /* Close the transfer function dialog: */
      Vrui::popdownPrimaryWidget(this->AnimationControl);
    }
  }
}

//----------------------------------------------------------------------------
void MooseViewer::changeVariablesCallback(
    GLMotif::ListBox::SelectionChangedCallbackData *callBackData)
{
  bool enable;
  switch (callBackData->reason)
    {
    case GLMotif::ListBox::SelectionChangedCallbackData::SELECTION_CLEARED:
      // Nothing should call selectionCleared, handle this if that changes:
      std::cerr << "Unhandled SELECTION_CLEARED event." << std::endl;
      return;
    default:
      std::cerr << "Unrecognized variable change reason: "
                << callBackData->reason << std::endl;
      return;
    case GLMotif::ListBox::SelectionChangedCallbackData::NUMITEMS_CHANGED:
      return; // don't care
    case GLMotif::ListBox::SelectionChangedCallbackData::ITEM_SELECTED:
      enable = true;
      break;
    case GLMotif::ListBox::SelectionChangedCallbackData::ITEM_DESELECTED:
      enable = false;
      break;
    }

  std::string array(callBackData->listBox->getItem(callBackData->item));
  if (enable)
    {
    m_mvState.reader().requestVariable(array);
    }
  else
    {
    m_mvState.reader().unrequestVariable(array);
    }

  this->updateColorByVariablesMenu();
}

//----------------------------------------------------------------------------
void MooseViewer::changeColorByVariablesCallback(
  GLMotif::ToggleButton::ValueChangedCallbackData* callBackData)
{
  // If there's only one variable, ignore the request to disable it.
  if (m_mvState.reader().requestedVariables().size() == 1)
    {
    if (!callBackData->set)
      {
      callBackData->toggle->setToggle(true);
      }
    return;
    }

  if (m_mvState.colorByArray() == callBackData->toggle->getName())
    {
    return;
    }

  m_mvState.setColorByArray(callBackData->toggle->getName());

  this->updateScalarRange();
  Vrui::requestUpdate();
}

//----------------------------------------------------------------------------
void MooseViewer::changeColorMapCallback(
  GLMotif::RadioBox::ValueChangedCallbackData* callBackData)
{
  int value = callBackData->radioBox->getToggleIndex(
    callBackData->newSelectedToggle);
  this->ColorEditor->changeColorMap(value);
  this->updateColorMap();
}

//----------------------------------------------------------------------------
void MooseViewer::colorMapChangedCallback(
  Misc::CallbackData* callBackData)
{
  this->updateColorMap();
}

//----------------------------------------------------------------------------
void MooseViewer::alphaChangedCallback(Misc::CallbackData* callBackData)
{
  this->updateColorMap();
}

//----------------------------------------------------------------------------
void MooseViewer::updateColorMap(void)
{
  // Complexity is to accurately record mtimes. Many of the calls to this
  // function could be refactored out to just initialize & handle callbacks.
  double tmp[256 * 4];
  this->ColorEditor->exportColorMap(tmp);
  this->ColorEditor->exportAlpha(tmp);

  // Do nothing if the colormap hasn't actually changed.
  if (std::equal(tmp, tmp + 256 * 4, m_colorMapCache))
    {
    return;
    }

  // Sync the cache to the new data:
  std::copy(tmp, tmp + 256 * 4, m_colorMapCache);

  // Update the actual colormap
  m_mvState.colorMap().SetNumberOfTableValues(256);
  for (int i = 0; i < 256; ++i)
    {
    m_mvState.colorMap().SetTableValue(i, m_colorMapCache + 4*i);
    }

  // Redraw
  Vrui::requestUpdate();
}

//----------------------------------------------------------------------------
float * MooseViewer::getHistogram(void)
{
  return this->Histogram;
}

//----------------------------------------------------------------------------
void MooseViewer::updateHistogram(void)
{
  if (this->HistogramMTime > m_mvState.reader().dataObject()->GetMTime() &&
      this->HistogramMTime > m_mvState.colorByMTime())
    {
    // Up to date.
    return;
    }

  std::fill(this->Histogram, this->Histogram + 256, 0.f);

  auto metaData = m_mvState.reader().variableMetaData(m_mvState.colorByArray());
  if (metaData.valid())
    {
    vtkCompositeDataIterator *it =
        m_mvState.reader().typedDataObject()->NewIterator();
    for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextItem())
      {
      vtkDataSet *ds = vtkDataSet::SafeDownCast(it->GetCurrentDataObject());
      if (!ds)
        {
        continue;
        }

      vtkDataArray *array = nullptr;
      switch (metaData.location)
        {
        case mvReader::VariableMetaData::Location::PointData:
          array = ds->GetPointData()->GetArray(m_mvState.colorByArray().c_str());
          break;

        case mvReader::VariableMetaData::Location::CellData:
          array = ds->GetCellData()->GetArray(m_mvState.colorByArray().c_str());
          break;

        case mvReader::VariableMetaData::Location::FieldData:
          array = ds->GetFieldData()->GetArray(m_mvState.colorByArray().c_str());
          break;

        default:
          break;
        }

      if (!array)
        {
        continue;
        }

      vtkIdType numTuples = array->GetNumberOfTuples();
      double min = metaData.range[0];
      double spread = metaData.range[1] - min;
      if (spread < 1e-6) // Constant data...
        {
        continue;
        }
      else
        {
        for (vtkIdType tuple = 0; tuple < numTuples; ++tuple)
          {
          size_t bin = static_cast<size_t>(
                (array->GetComponent(tuple, 0) - min) * 255. / spread);
          assert(bin < 256);
          ++this->Histogram[bin];
          }
        }
      }
    it->Delete();
    }

  this->HistogramMTime.Modified();

  this->ColorEditor->setHistogram(this->Histogram);
  this->ContoursDialog->setHistogram(this->Histogram);
  Vrui::requestUpdate();
}

//----------------------------------------------------------------------------
void MooseViewer::updateScalarRange(void)
{
  auto metaData = m_mvState.reader().variableMetaData(m_mvState.colorByArray());
  if (metaData.valid())
    {
    this->ScalarRange[0] = metaData.range[0];
    this->ScalarRange[1] = metaData.range[1];
    }
  this->ColorEditor->setScalarRange(this->ScalarRange);
}

//----------------------------------------------------------------------------
void MooseViewer::showContoursDialogCallback(
  GLMotif::ToggleButton::ValueChangedCallbackData* callBackData)
{
  /* open/close slices dialog based on which toggle button changed state: */
  if (strcmp(callBackData->toggle->getName(), "ShowContoursDialog") == 0)
    {
    if (callBackData->set)
      {
      /* Open the slices dialog at the same position as the main menu: */
      Vrui::getWidgetManager()->popupPrimaryWidget(ContoursDialog,
        Vrui::getWidgetManager()->calcWidgetTransformation(mainMenu));
      this->setContourVisible(true);
      }
    else
      {
      /* Close the slices dialog: */
      Vrui::popdownPrimaryWidget(ContoursDialog);
      }
    }
}

//----------------------------------------------------------------------------
void MooseViewer::contourValueChangedCallback(Misc::CallbackData*)
{
  m_mvState.contours().setContourValues(this->ContoursDialog->getContourValues());
  Vrui::requestUpdate();
}

//----------------------------------------------------------------------------
void MooseViewer::setContourVisible(bool visible)
{
  m_mvState.contours().setVisible(visible);
}

//----------------------------------------------------------------------------
void MooseViewer::setRequestedRenderMode(int mode)
{
  m_mvState.volume().setRenderMode(mode);
}

//----------------------------------------------------------------------------
int MooseViewer::getRequestedRenderMode(void) const
{
  return m_mvState.volume().renderMode();
}

//----------------------------------------------------------------------------
void MooseViewer::setScalarMinimum(double min)
{
  if (min < this->ScalarRange[1])
    {
    this->ScalarRange[0] = min;
    }
  Vrui::requestUpdate();
}

//----------------------------------------------------------------------------
void MooseViewer::setScalarMaximum(double max)
{
  if (max > this->ScalarRange[0])
    {
    this->ScalarRange[1] = max;
    }
  Vrui::requestUpdate();
}

//----------------------------------------------------------------------------
void MooseViewer::centerDisplay() const
{
  auto bbox = m_mvState.reader().bounds();
  double center[3];
  bbox.GetCenter(center);
  Vrui::setNavigationTransformation(Vrui::Point(center),
                                    0.75 * bbox.GetDiagonalLength());
}
