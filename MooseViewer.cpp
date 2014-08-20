// STL includes
#include <algorithm>
#include <iostream>
#include <math.h>
#include <sstream>
#include <string>

// OpenGL/Motif includes
#include <GL/GLContextData.h>
#include <GL/gl.h>
#include <GLMotif/CascadeButton.h>
#include <GLMotif/Menu.h>
#include <GLMotif/Pager.h>
#include <GLMotif/Popup.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/RadioBox.h>
#include <GLMotif/Separator.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/SubMenu.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/WidgetManager.h>

// VRUI includes
#include <Vrui/Application.h>
#include <Vrui/Tool.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Vrui.h>
#include <Vrui/WindowProperties.h>

// VTK includes
#include <ExternalVTKWidget.h>
#include <vtkActor.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkCubeSource.h>
#include <vtkExodusIIReader.h>
#include <vtkLight.h>
#include <vtkLookupTable.h>
#include <vtkNew.h>
#include <vtkOutlineFilter.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

#include <vtkCellData.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkUnstructuredGrid.h>

// MooseViewer includes
#include "AnimationDialog.h"
#include "BaseLocator.h"
#include "ClippingPlane.h"
#include "ClippingPlaneLocator.h"
#include "ColorMap.h"
#include "DataItem.h"
#include "FlashlightLocator.h"
#include "MooseViewer.h"
#include "ScalarWidget.h"
#include "TransferFunction1D.h"

//----------------------------------------------------------------------------
MooseViewer::MooseViewer(int& argc,char**& argv)
  :Vrui::Application(argc,argv),
  FileName(0),
  mainMenu(NULL),
  renderingDialog(NULL),
  Opacity(1.0),
  opacityValue(NULL),
  Outline(true),
  RepresentationType(2),
  FirstFrame(true),
  analysisTool(0),
  ClippingPlanes(NULL),
  NumberOfClippingPlanes(6),
  FlashlightSwitch(0),
  FlashlightPosition(0),
  FlashlightDirection(0),
  variablesMenu(0),
  colorByVariablesMenu(0),
  IsPlaying(false),
  Loop(false)
{
  /* Set Window properties:
   * Since the application requires translucency, GLX_ALPHA_SIZE is set to 1 at
   * context (VRWindow) creation time. To do this, we set the 4th component of
   * ColorBufferSize in WindowProperties to 1. This should be done in the
   * constructor to make sure it is set before the main loop is called.
   */
  Vrui::WindowProperties properties;
  properties.setColorBufferSize(0,1);
  Vrui::requestWindowProperties(properties);

  /* Create the user interface: */
  renderingDialog = createRenderingDialog();
  mainMenu=createMainMenu();
  Vrui::setMainMenu(mainMenu);

  this->reader = vtkSmartPointer<vtkExodusIIReader>::New();
  this->variables.clear();

  this->DataBounds = new double[6];
  this->FlashlightSwitch = new int[1];
  this->FlashlightSwitch[0] = 0;
  this->FlashlightPosition = new double[3];
  this->FlashlightDirection = new double[3];

  /* Color Map */
  this->ColorMap = new double[4*256];

  /* Initialize the clipping planes */
  ClippingPlanes = new ClippingPlane[NumberOfClippingPlanes];
  for(int i = 0; i < NumberOfClippingPlanes; ++i)
    {
    ClippingPlanes[i].setAllocated(false);
    ClippingPlanes[i].setActive(false);
    }
}

//----------------------------------------------------------------------------
MooseViewer::~MooseViewer(void)
{
  if (this->DataBounds)
    {
    delete[] this->DataBounds;
    this->DataBounds = NULL;
    }
  if (this->FlashlightSwitch)
    {
    delete[] this->FlashlightSwitch;
    this->FlashlightSwitch = NULL;
    }
  if (this->FlashlightPosition)
    {
    delete[] this->FlashlightPosition;
    this->FlashlightPosition = NULL;
    }
  if (this->FlashlightDirection)
    {
    delete[] this->FlashlightDirection;
    this->FlashlightDirection = NULL;
    }
  if (this->ColorMap)
    {
    delete[] this->ColorMap;
    this->ColorMap = NULL;
    }
}

//----------------------------------------------------------------------------
void MooseViewer::setFileName(const char* name)
{
  if(this->FileName && name && (!strcmp(this->FileName, name)))
    {
    return;
    }
  if(this->FileName && name)
    {
    delete [] this->FileName;
    }
  this->FileName = new char[strlen(name) + 1];
  strcpy(this->FileName, name);

  this->reader->SetFileName(this->FileName);
  this->reader->UpdateInformation();
  this->updateVariablesMenu();
  reader->GenerateObjectIdCellArrayOn();
  reader->GenerateGlobalElementIdArrayOn();
  reader->GenerateGlobalNodeIdArrayOn();
}

//----------------------------------------------------------------------------
const char* MooseViewer::getFileName(void)
{
  return this->FileName;
}

//----------------------------------------------------------------------------
GLMotif::PopupMenu* MooseViewer::createMainMenu(void)
{
  GLMotif::PopupMenu* mainMenuPopup =
    new GLMotif::PopupMenu("MainMenuPopup",Vrui::getWidgetManager());
  mainMenuPopup->setTitle("Main Menu");
  GLMotif::Menu* mainMenu = new GLMotif::Menu("MainMenu",mainMenuPopup,false);

  GLMotif::CascadeButton* variablesCascade =
    new GLMotif::CascadeButton("VariablesCascade", mainMenu, "Variables");
  variablesCascade->setPopup(createVariablesMenu());

  GLMotif::CascadeButton* colorByVariablesCascade =
    new GLMotif::CascadeButton("colorByVariablesCascade", mainMenu, "Color By");
  colorByVariablesCascade->setPopup(createColorByVariablesMenu());

  GLMotif::CascadeButton * colorMapSubCascade =
    new GLMotif::CascadeButton("ColorMapSubCascade", mainMenu, "Color Map");
  colorMapSubCascade->setPopup(createColorMapSubMenu());

   GLMotif::ToggleButton * showColorEditorDialog =
     new GLMotif::ToggleButton("ShowColorEditorDialog", mainMenu,
    "Color Editor");
  showColorEditorDialog->setToggle(false);
  showColorEditorDialog->getValueChangedCallbacks().add(
    this, &MooseViewer::showColorEditorDialogCallback);

   GLMotif::ToggleButton * showAnimationDialog =
     new GLMotif::ToggleButton("ShowAnimationDialog", mainMenu,
    "Animation");
  showAnimationDialog->setToggle(false);
  showAnimationDialog->getValueChangedCallbacks().add(
    this, &MooseViewer::showAnimationDialogCallback);

  GLMotif::CascadeButton* representationCascade =
    new GLMotif::CascadeButton("RepresentationCascade", mainMenu,
    "Representation");
  representationCascade->setPopup(createRepresentationMenu());

  GLMotif::CascadeButton* analysisToolsCascade =
    new GLMotif::CascadeButton("AnalysisToolsCascade", mainMenu,
    "Analysis Tools");
  analysisToolsCascade->setPopup(createAnalysisToolsMenu());

  GLMotif::Button* centerDisplayButton =
    new GLMotif::Button("CenterDisplayButton",mainMenu,"Center Display");
  centerDisplayButton->getSelectCallbacks().add(
    this,&MooseViewer::centerDisplayCallback);

  GLMotif::ToggleButton * showRenderingDialog =
    new GLMotif::ToggleButton("ShowRenderingDialog", mainMenu,
    "Rendering");
  showRenderingDialog->setToggle(false);
  showRenderingDialog->getValueChangedCallbacks().add(
    this, &MooseViewer::showRenderingDialogCallback);

  mainMenu->manageChild();
  return mainMenuPopup;
}

//----------------------------------------------------------------------------
GLMotif::Popup* MooseViewer::createRepresentationMenu(void)
{
  const GLMotif::StyleSheet* ss = Vrui::getWidgetManager()->getStyleSheet();

  GLMotif::Popup* representationMenuPopup =
    new GLMotif::Popup("representationMenuPopup", Vrui::getWidgetManager());
  GLMotif::SubMenu* representationMenu = new GLMotif::SubMenu(
    "representationMenu", representationMenuPopup, false);

  GLMotif::ToggleButton* showOutline=new GLMotif::ToggleButton(
    "ShowOutline",representationMenu,"Outline");
  showOutline->getValueChangedCallbacks().add(
    this,&MooseViewer::changeRepresentationCallback);
  showOutline->setToggle(true);

  GLMotif::Label* representation_Label = new GLMotif::Label(
    "Representations", representationMenu,"Representations:");

  GLMotif::RadioBox* representation_RadioBox =
    new GLMotif::RadioBox("Representation RadioBox",representationMenu,true);

  GLMotif::ToggleButton* showNone=new GLMotif::ToggleButton(
    "ShowNone",representation_RadioBox,"None");
  showNone->getValueChangedCallbacks().add(
    this,&MooseViewer::changeRepresentationCallback);
  GLMotif::ToggleButton* showPoints=new GLMotif::ToggleButton(
    "ShowPoints",representation_RadioBox,"Points");
  showPoints->getValueChangedCallbacks().add(
    this,&MooseViewer::changeRepresentationCallback);
  GLMotif::ToggleButton* showWireframe=new GLMotif::ToggleButton(
    "ShowWireframe",representation_RadioBox,"Wireframe");
  showWireframe->getValueChangedCallbacks().add(
    this,&MooseViewer::changeRepresentationCallback);
  GLMotif::ToggleButton* showSurface=new GLMotif::ToggleButton(
    "ShowSurface",representation_RadioBox,"Surface");
  showSurface->getValueChangedCallbacks().add(
    this,&MooseViewer::changeRepresentationCallback);
  GLMotif::ToggleButton* showSurfaceWithEdges=new GLMotif::ToggleButton(
    "ShowSurfaceWithEdges",representation_RadioBox,"Surface with Edges");
  showSurfaceWithEdges->getValueChangedCallbacks().add(
    this,&MooseViewer::changeRepresentationCallback);

  representation_RadioBox->setSelectionMode(GLMotif::RadioBox::ATMOST_ONE);
  representation_RadioBox->setSelectedToggle(showSurface);

  representationMenu->manageChild();
  return representationMenuPopup;
}

//----------------------------------------------------------------------------
GLMotif::Popup * MooseViewer::createAnalysisToolsMenu(void)
{
  const GLMotif::StyleSheet* ss = Vrui::getWidgetManager()->getStyleSheet();

  GLMotif::Popup * analysisToolsMenuPopup = new GLMotif::Popup(
    "analysisToolsMenuPopup", Vrui::getWidgetManager());
  GLMotif::SubMenu* analysisToolsMenu = new GLMotif::SubMenu(
    "representationMenu", analysisToolsMenuPopup, false);

  GLMotif::RadioBox * analysisTools_RadioBox = new GLMotif::RadioBox(
    "analysisTools", analysisToolsMenu, true);

  GLMotif::ToggleButton* showClippingPlane=new GLMotif::ToggleButton(
    "ClippingPlane",analysisTools_RadioBox,"Clipping Plane");
  showClippingPlane->getValueChangedCallbacks().add(
    this,&MooseViewer::changeAnalysisToolsCallback);
  GLMotif::ToggleButton* showFlashlight=new GLMotif::ToggleButton(
    "Flashlight",analysisTools_RadioBox,"Flashlight");
  showFlashlight->getValueChangedCallbacks().add(
    this,&MooseViewer::changeAnalysisToolsCallback);

  analysisTools_RadioBox->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
  analysisTools_RadioBox->setSelectedToggle(showClippingPlane);

  analysisToolsMenu->manageChild();
  return analysisToolsMenuPopup;
}

//----------------------------------------------------------------------------
GLMotif::Popup* MooseViewer::createVariablesMenu(void)
{
  const GLMotif::StyleSheet* ss = Vrui::getWidgetManager()->getStyleSheet();

  GLMotif::Popup* variablesMenuPopup =
    new GLMotif::Popup("variablesMenuPopup", Vrui::getWidgetManager());
  this->variablesMenu = new GLMotif::SubMenu(
    "variablesMenu", variablesMenuPopup, false);

  this->variablesMenu->manageChild();
  return variablesMenuPopup;
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
void MooseViewer::updateVariablesMenu(void)
{
  /* Clear the menu first */
  int i;
  for (i = this->variablesMenu->getNumRows(); i >= 0; --i)
    {
    variablesMenu->removeWidgets(i);
    }

  const GLMotif::StyleSheet* ss = Vrui::getWidgetManager()->getStyleSheet();

  GLMotif::Label* pointArraysLabel = new GLMotif::Label(
    "Point_Arrays", variablesMenu, "Point Arrays");
  GLMotif::ToggleButton* nodeIdbutton = new GLMotif::ToggleButton(
    this->reader->GetPedigreeNodeIdArrayName(),
    variablesMenu, "Pedigree Node Ids");
  nodeIdbutton->getValueChangedCallbacks().add(
    this, &MooseViewer::changeVariablesCallback);
  nodeIdbutton->setToggle(false);

  GLMotif::Pager* pointsPager = NULL;
  int currentPage = -1;
  for (i = 0; i < this->reader->GetNumberOfPointResultArrays(); ++i)
    {
    GLMotif::Container* pointArraysContainer;
    if (this->reader->GetNumberOfPointResultArrays() > 8)
      {
      if (!pointsPager)
        {
        pointsPager = new GLMotif::Pager("PointsPager", variablesMenu, true);
        }
      int newPage = static_cast<int> (i / 8);
      if (currentPage != newPage)
        {
        currentPage = newPage;
        std::stringstream ss;
        ss << "PointsMenu" << currentPage;
        GLMotif::Menu* menu = new GLMotif::Menu(
          ss.str().c_str(), pointsPager, true);
        pointArraysContainer = menu;
        }
      }
    else
      {
      pointArraysContainer = variablesMenu;
      }
    GLMotif::ToggleButton* button = new GLMotif::ToggleButton(
      this->reader->GetPointResultArrayName(i),
      pointArraysContainer, this->reader->GetPointResultArrayName(i));
    button->getValueChangedCallbacks().add(
      this, &MooseViewer::changeVariablesCallback);
    button->setToggle(false);
    }

  GLMotif::Separator * pointsep = new GLMotif::Separator(
    "Points_Separator", variablesMenu, GLMotif::Separator::HORIZONTAL,
    ss->menuButtonBorderWidth, GLMotif::Separator::RAISED);

  GLMotif::Label* elementArraysLabel = new GLMotif::Label(
    "Element_Arrays", variablesMenu, "Element Arrays");

  GLMotif::ToggleButton* objectIdbutton = new GLMotif::ToggleButton(
    this->reader->GetObjectIdArrayName(),
    variablesMenu, "Object Ids");
  objectIdbutton->getValueChangedCallbacks().add(
    this, &MooseViewer::changeVariablesCallback);
  objectIdbutton->setToggle(false);

  GLMotif::ToggleButton* elementIdbutton = new GLMotif::ToggleButton(
    this->reader->GetPedigreeElementIdArrayName(),
    variablesMenu, "Pedigree Element Ids");
  elementIdbutton->getValueChangedCallbacks().add(
    this, &MooseViewer::changeVariablesCallback);
  elementIdbutton->setToggle(false);

  GLMotif::Pager* elementsPager = NULL;
  currentPage = -1;
  for (i = 0; i < this->reader->GetNumberOfElementResultArrays(); ++i)
    {
    GLMotif::Container* elementArraysContainer;
    if (this->reader->GetNumberOfElementResultArrays() > 8)
      {
      if (!elementsPager)
        {
        elementsPager =
          new GLMotif::Pager("ElementsPager", variablesMenu, true);
        }
      int newPage = static_cast<int>(i / 8);
      if( currentPage != newPage)
        {
        currentPage = newPage;
        std::stringstream ss;
        ss << "ElementsMenu" << currentPage;
        GLMotif::Menu* menu = new GLMotif::Menu(
          ss.str().c_str(), elementsPager, true);
        elementArraysContainer = menu;
        }
      }
    else
      {
      elementArraysContainer = variablesMenu;
      }
    GLMotif::ToggleButton* button = new GLMotif::ToggleButton(
      this->reader->GetElementResultArrayName(i),
      elementArraysContainer, this->reader->GetElementResultArrayName(i));
    button->getValueChangedCallbacks().add(
      this, &MooseViewer::changeVariablesCallback);
    button->setToggle(false);

    if (pointsPager)
      {
      pointsPager->setCurrentChildIndex(0);
      }
    if (elementsPager)
      {
      elementsPager->setCurrentChildIndex(0);
      }
    }
}

//----------------------------------------------------------------------------
std::string MooseViewer::getSelectedColorByArrayName(void) const
{
  std::string selectedToggle;
  GLMotif::RadioBox* radioBox =
    static_cast<GLMotif::RadioBox*> (colorByVariablesMenu->getChild(0));
  if (radioBox && (radioBox->getNumRows() > 0))
    {
    GLMotif::ToggleButton* selectedToggleButton =
      radioBox->getSelectedToggle();
    if (selectedToggleButton)
      {
      selectedToggle.assign(selectedToggleButton->getString());
      }
    }
  return selectedToggle;
}

//----------------------------------------------------------------------------
void MooseViewer::updateColorByVariablesMenu(void)
{
  /* Preserve the selection */
  std::string selectedToggle = this->getSelectedColorByArrayName();

  /* Clear the menu first */
  int i;
  for (i = this->colorByVariablesMenu->getNumRows(); i >= 0; --i)
    {
    colorByVariablesMenu->removeWidgets(i);
    }

  if (this->variables.size() > 0)
    {
    std::sort(this->variables.begin(), this->variables.end());
    GLMotif::RadioBox* colorby_RadioBox =
      new GLMotif::RadioBox("Color RadioBox",colorByVariablesMenu,true);

    int selectedIndex = -1;

    for (i = 0; i < this->variables.size(); ++i)
      {
      GLMotif::ToggleButton* button = new GLMotif::ToggleButton(
        this->variables[i].c_str(),
        colorby_RadioBox, this->variables[i].c_str());
      button->getValueChangedCallbacks().add(
        this, &MooseViewer::changeColorByVariablesCallback);
      button->setToggle(false);
      if ( !selectedToggle.empty() && (selectedIndex < 0) &&
        (selectedToggle.compare(this->variables[i]) == 0))
        {
        selectedIndex = i;
        }
      }

    selectedIndex = selectedIndex > 0 ? selectedIndex : 0;
    colorby_RadioBox->setSelectedToggle(selectedIndex);
    colorby_RadioBox->setSelectionMode(GLMotif::RadioBox::ATMOST_ONE);
    }
}

//----------------------------------------------------------------------------
GLMotif::Popup* MooseViewer::createColorMapSubMenu(void)
{
  GLMotif::Popup * colorMapSubMenuPopup = new GLMotif::Popup(
    "ColorMapSubMenuPopup", Vrui::getWidgetManager());
  GLMotif::RadioBox* colorMaps = new GLMotif::RadioBox(
    "ColorMaps", colorMapSubMenuPopup, false);
  colorMaps->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
  colorMaps->addToggle("Full Rainbow");
  colorMaps->addToggle("Inverse Full Rainbow");
  colorMaps->addToggle("Rainbow");
  colorMaps->addToggle("Inverse Rainbow");
  colorMaps->addToggle("Cold to Hot");
  colorMaps->addToggle("Hot to Cold");
  colorMaps->addToggle("Black to White");
  colorMaps->addToggle("White to Black");
  colorMaps->addToggle("HSB Hues");
  colorMaps->addToggle("Inverse HSB Hues");
  colorMaps->addToggle("Davinci");
  colorMaps->addToggle("Inverse Davinci");
  colorMaps->addToggle("Seismic");
  colorMaps->addToggle("Inverse Seismic");
  colorMaps->setSelectedToggle(3);
  colorMaps->getValueChangedCallbacks().add(this,
    &MooseViewer::changeColorMapCallback);
  colorMaps->manageChild();
  return colorMapSubMenuPopup;
} // end createColorMapSubMenu()

//----------------------------------------------------------------------------
GLMotif::PopupWindow* MooseViewer::createRenderingDialog(void) {
  const GLMotif::StyleSheet& ss = *Vrui::getWidgetManager()->getStyleSheet();
  GLMotif::PopupWindow * dialogPopup = new GLMotif::PopupWindow(
    "RenderingDialogPopup", Vrui::getWidgetManager(), "Rendering Dialog");
  GLMotif::RowColumn * dialog = new GLMotif::RowColumn(
    "RenderingDialog", dialogPopup, false);
  dialog->setOrientation(GLMotif::RowColumn::HORIZONTAL);

  /* Create opacity slider */
  GLMotif::Slider* opacitySlider = new GLMotif::Slider(
    "OpacitySlider", dialog, GLMotif::Slider::HORIZONTAL, ss.fontHeight*10.0f);
  opacitySlider->setValue(Opacity);
  opacitySlider->setValueRange(0.0, 1.0, 0.1);
  opacitySlider->getValueChangedCallbacks().add(
    this, &MooseViewer::opacitySliderCallback);
  opacityValue = new GLMotif::TextField("OpacityValue", dialog, 6);
  opacityValue->setFieldWidth(6);
  opacityValue->setPrecision(3);
  opacityValue->setValue(Opacity);

  dialog->manageChild();
  return dialogPopup;
}

//----------------------------------------------------------------------------
void MooseViewer::frame(void)
{
  if(this->FirstFrame)
    {
    /* Initialize the color editor */
    this->ColorEditor = new TransferFunction1D(this);
    this->ColorEditor->createTransferFunction1D(CINVERSE_RAINBOW,
      CONSTANT_RAMP, 0.0, 1.0);
    this->ColorEditor->getColorMapChangedCallbacks().add(
      this, &MooseViewer::colorMapChangedCallback);
    this->ColorEditor->getAlphaChangedCallbacks().add(this,
      &MooseViewer::alphaChangedCallback);
    updateColorMap();
    updateAlpha();

    /* Initialize the Animation control */
    this->AnimationControl = new AnimationDialog(this);

    /* Compute the data center and Radius once */
    this->Center[0] = (this->DataBounds[0] + this->DataBounds[1])/2.0;
    this->Center[1] = (this->DataBounds[2] + this->DataBounds[3])/2.0;
    this->Center[2] = (this->DataBounds[4] + this->DataBounds[5])/2.0;

    this->Radius = sqrt((this->DataBounds[1] - this->DataBounds[0])*
                        (this->DataBounds[1] - this->DataBounds[0]) +
                        (this->DataBounds[3] - this->DataBounds[2])*
                        (this->DataBounds[3] - this->DataBounds[2]) +
                        (this->DataBounds[5] - this->DataBounds[4])*
                        (this->DataBounds[5] - this->DataBounds[4]));
    /* Scale the Radius */
    this->Radius *= 0.75;
    /* Initialize Vrui navigation transformation: */
    centerDisplayCallback(0);
    this->FirstFrame = false;
    }
  this->updateColorMap();
  this->updateAlpha();
  if (this->IsPlaying)
    {
    int currentTimeStep = this->reader->GetTimeStep();
    if (currentTimeStep < this->reader->GetTimeStepRange()[1])
      {
      this->reader->SetTimeStep(currentTimeStep + 1);
      Vrui::scheduleUpdate(Vrui::getApplicationTime() + 1.0/125.0);
      }
    else if(this->Loop)
      {
      this->reader->SetTimeStep(this->reader->GetTimeStepRange()[0]);
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
  /* Create a new context data item */
  DataItem* dataItem = new DataItem();
  contextData.addDataItem(this, dataItem);

  vtkNew<vtkOutlineFilter> dataOutline;

  dataItem->compositeFilter->SetInputConnection(this->reader->GetOutputPort());
  dataItem->compositeFilter->Update();
  dataItem->compositeFilter->GetOutput()->GetBounds(this->DataBounds);

  dataOutline->SetInputConnection(dataItem->compositeFilter->GetOutputPort());

  vtkNew<vtkPolyDataMapper> mapperOutline;
  mapperOutline->SetInputConnection(dataOutline->GetOutputPort());
  dataItem->actorOutline->SetMapper(mapperOutline.GetPointer());
  dataItem->actorOutline->GetProperty()->SetColor(1,1,1);
}

//----------------------------------------------------------------------------
void MooseViewer::display(GLContextData& contextData) const
{
  int numberOfSupportedClippingPlanes;
  glGetIntegerv(GL_MAX_CLIP_PLANES, &numberOfSupportedClippingPlanes);
  int clippingPlaneIndex = 0;
  for (int i = 0; i < NumberOfClippingPlanes &&
    clippingPlaneIndex < numberOfSupportedClippingPlanes; ++i)
    {
    if (ClippingPlanes[i].isActive())
      {
        /* Enable the clipping plane: */
        glEnable(GL_CLIP_PLANE0 + clippingPlaneIndex);
        GLdouble clippingPlane[4];
        for (int j = 0; j < 3; ++j)
            clippingPlane[j] = ClippingPlanes[i].getPlane().getNormal()[j];
        clippingPlane[3] = -ClippingPlanes[i].getPlane().getOffset();
        glClipPlane(GL_CLIP_PLANE0 + clippingPlaneIndex, clippingPlane);
        /* Go to the next clipping plane: */
        ++clippingPlaneIndex;
      }
    }

  /* Make sure the reader M-time changes for each context */
  this->reader->Update();

  /* Get context data item */
  DataItem* dataItem = contextData.retrieveDataItem<DataItem>(this);

  /* Color by selected array */
  std::string selectedArray = this->getSelectedColorByArrayName();
  if (!selectedArray.empty())
    {
    dataItem->mapper->SelectColorArray(selectedArray.c_str());

    bool dataArrayFound = true;

    dataItem->compositeFilter->Update();
    vtkSmartPointer<vtkDataArray> dataArray = vtkDataArray::SafeDownCast(
      dataItem->compositeFilter->GetOutput()->GetPointData(
        )->GetArray(selectedArray.c_str()));
    if (!dataArray)
      {
      dataArray = vtkDataArray::SafeDownCast(
        dataItem->compositeFilter->GetOutput()->GetCellData(
          )->GetArray(selectedArray.c_str()));
      dataItem->mapper->SetScalarModeToUseCellFieldData();
      if (!dataArray)
        {
        std::cerr << "The selected array is neither PointDataArray"\
          " nor CellDataArray" << std::endl;
        dataArrayFound = false;
        }
      }
    else
      {
      dataItem->mapper->SetScalarModeToUsePointFieldData();
      }

    if (dataArrayFound)
      {
      dataItem->mapper->SetScalarRange(dataArray->GetRange());
      dataItem->lut->SetTableRange(dataArray->GetRange());
      }

    for (int i = 0; i < 256; ++i)
      {
      dataItem->lut->SetTableValue(i,
        this->ColorMap[4*i + 0],
        this->ColorMap[4*i + 1],
        this->ColorMap[4*i + 2],
        this->ColorMap[4*i + 3]);
      }
    }

  if(this->FlashlightSwitch[0])
    {
    dataItem->flashlight->SetPosition(this->FlashlightPosition);
    dataItem->flashlight->SetFocalPoint(this->FlashlightDirection);
    dataItem->flashlight->SwitchOn();
    }
  else
    {
    dataItem->flashlight->SwitchOff();
    }

  /* Enable/disable the outline */
  if (this->Outline)
    {
    dataItem->actorOutline->VisibilityOn();
    }
  else
    {
    dataItem->actorOutline->VisibilityOff();
    }

  /* Set actor opacity */
  dataItem->actor->GetProperty()->SetOpacity(this->Opacity);
  /* Set the appropriate representation */
  if (this->RepresentationType != -1)
    {
    dataItem->actor->VisibilityOn();
    if (this->RepresentationType == 3)
      {
      dataItem->actor->GetProperty()->SetRepresentationToSurface();
      dataItem->actor->GetProperty()->EdgeVisibilityOn();
      }
    else
      {
      dataItem->actor->GetProperty()->SetRepresentation(
        this->RepresentationType);
      dataItem->actor->GetProperty()->EdgeVisibilityOff();
      }
    }
  else
    {
    dataItem->actor->VisibilityOff();
    }

  /* Render the scene */
  dataItem->externalVTKWidget->GetRenderWindow()->Render();

  clippingPlaneIndex = 0;
  for (int i = 0; i < NumberOfClippingPlanes &&
    clippingPlaneIndex < numberOfSupportedClippingPlanes; ++i)
    {
    if (ClippingPlanes[i].isActive())
      {
        /* Disable the clipping plane: */
        glDisable(GL_CLIP_PLANE0 + clippingPlaneIndex);
        /* Go to the next clipping plane: */
        ++clippingPlaneIndex;
      }
    }
}

//----------------------------------------------------------------------------
void MooseViewer::centerDisplayCallback(Misc::CallbackData* callBackData)
{
  if(!this->DataBounds)
    {
    std::cerr << "ERROR: Data bounds not set!!" << std::endl;
    return;
    }
  Vrui::setNavigationTransformation(this->Center, this->Radius);
}

//----------------------------------------------------------------------------
void MooseViewer::opacitySliderCallback(
  GLMotif::Slider::ValueChangedCallbackData* callBackData)
{
  this->Opacity = static_cast<double>(callBackData->value);
  opacityValue->setValue(callBackData->value);
}

//----------------------------------------------------------------------------
void MooseViewer::changeRepresentationCallback(
  GLMotif::ToggleButton::ValueChangedCallbackData* callBackData)
{
  /* Adjust representation state based on which toggle button changed state: */
  if (strcmp(callBackData->toggle->getName(), "ShowSurface") == 0)
    {
    this->RepresentationType = 2;
    }
  else if (strcmp(callBackData->toggle->getName(), "ShowSurfaceWithEdges") == 0)
    {
    this->RepresentationType = 3;
    }
  else if (strcmp(callBackData->toggle->getName(), "ShowWireframe") == 0)
    {
    this->RepresentationType = 1;
    }
  else if (strcmp(callBackData->toggle->getName(), "ShowPoints") == 0)
    {
    this->RepresentationType = 0;
    }
  else if (strcmp(callBackData->toggle->getName(), "ShowNone") == 0)
    {
    this->RepresentationType = -1;
    }
  else if (strcmp(callBackData->toggle->getName(), "ShowOutline") == 0)
    {
    this->Outline = callBackData->set;
    }
}
//----------------------------------------------------------------------------
void MooseViewer::changeAnalysisToolsCallback(
  GLMotif::ToggleButton::ValueChangedCallbackData* callBackData)
{
  /* Set the new analysis tool: */
  if (strcmp(callBackData->toggle->getName(), "ClippingPlane") == 0)
  {
    this->analysisTool = 0;
  }
  else if (strcmp(callBackData->toggle->getName(), "Flashlight") == 0)
  {
    this->analysisTool = 1;
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
ClippingPlane * MooseViewer::getClippingPlanes(void)
{
  return this->ClippingPlanes;
}

//----------------------------------------------------------------------------
int MooseViewer::getNumberOfClippingPlanes(void)
{
  return this->NumberOfClippingPlanes;
}

//----------------------------------------------------------------------------
void MooseViewer::toolCreationCallback(
  Vrui::ToolManager::ToolCreationCallbackData * callbackData)
{
  /* Check if the new tool is a locator tool: */
  Vrui::LocatorTool* locatorTool =
    dynamic_cast<Vrui::LocatorTool*> (callbackData->tool);
  if (locatorTool != 0)
    {
    BaseLocator* newLocator;
    if (analysisTool == 0)
      {
      /* Create a clipping plane locator object and
       * associate it with the new tool:
       */
      newLocator = new ClippingPlaneLocator(locatorTool, this);
      }
    else if (analysisTool == 1)
      {
      /* Create a flashlight locator object and
       * associate it with the new tool:
       */
      newLocator = new FlashlightLocator(locatorTool, this);
      }

    /* Add new locator to list: */
    baseLocators.push_back(newLocator);
    }
}

//----------------------------------------------------------------------------
void MooseViewer::toolDestructionCallback(
  Vrui::ToolManager::ToolDestructionCallbackData * callbackData)
{
  /* Check if the to-be-destroyed tool is a locator tool: */
  Vrui::LocatorTool* locatorTool =
    dynamic_cast<Vrui::LocatorTool*> (callbackData->tool);
  if (locatorTool != 0)
    {
    /* Find the data locator associated with the tool in the list: */
    for (BaseLocatorList::iterator blIt = baseLocators.begin();
      blIt != baseLocators.end(); ++blIt)
      {
      if ((*blIt)->getTool() == locatorTool)
        {
        /* Remove the locator: */
        delete *blIt;
        baseLocators.erase(blIt);
        break;
        }
      }
    }
}

//----------------------------------------------------------------------------
void MooseViewer::changeVariablesCallback(
  GLMotif::ToggleButton::ValueChangedCallbackData* callBackData)
{
  std::string nameStr = std::string(callBackData->toggle->getName());
  int setter = callBackData->set ? 1 : 0;

  int numPointResultArrays = this->reader->GetNumberOfPointResultArrays();
  int numElementResultArrays = this->reader->GetNumberOfElementResultArrays();

  int i;
  for (i = 0; i < numPointResultArrays; ++i)
    {
    if (nameStr.compare(std::string(
          this->reader->GetPointResultArrayName(i))) == 0)
      {
      this->reader->SetPointResultArrayStatus(nameStr.c_str(), setter);
      break;
      }
    }
  if (i == numPointResultArrays)
    {
    for (i = 0; i < numElementResultArrays; ++i)
      {
      if (nameStr.compare(std::string(
            this->reader->GetElementResultArrayName(i))) == 0)
        {
        this->reader->SetElementResultArrayStatus(nameStr.c_str(), setter);
        break;
        }
      }
    }

  std::vector< std::string >::iterator iter;
  for (iter = this->variables.begin(); iter != this->variables.end(); ++iter)
    {
    if ((*iter).compare(nameStr) == 0)
      {
      break;
      }
    }
  if (iter == this->variables.end())
    {
    variables.push_back(nameStr);
    }
  else
    {
    this->variables.erase(iter);
    }
  this->updateColorByVariablesMenu();
}

//----------------------------------------------------------------------------
void MooseViewer::changeColorByVariablesCallback(
  GLMotif::ToggleButton::ValueChangedCallbackData* callBackData)
{
  if (this->variables.size() == 1)
    {
    callBackData->toggle->setToggle(true);
    }
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
  this->updateAlpha();
  Vrui::requestUpdate();
}

//----------------------------------------------------------------------------
void MooseViewer::colorMapChangedCallback(
  Misc::CallbackData* callBackData)
{
  this->ColorEditor->exportColorMap(this->ColorMap);
  this->updateAlpha();
  Vrui::requestUpdate();
}

//----------------------------------------------------------------------------
void MooseViewer::alphaChangedCallback(Misc::CallbackData* callBackData)
{
  this->ColorEditor->exportAlpha(this->ColorMap);
  Vrui::requestUpdate();
}

//----------------------------------------------------------------------------
void MooseViewer::updateAlpha(void)
{
  this->ColorEditor->exportAlpha(this->ColorMap);
  Vrui::requestUpdate();
}

//----------------------------------------------------------------------------
void MooseViewer::updateColorMap(void)
{
  this->ColorEditor->exportColorMap(this->ColorMap);
  Vrui::requestUpdate();
}

//----------------------------------------------------------------------------
int * MooseViewer::getFlashlightSwitch(void)
{
  return this->FlashlightSwitch;
}

//----------------------------------------------------------------------------
double * MooseViewer::getFlashlightPosition(void)
{
  return this->FlashlightPosition;
}

//----------------------------------------------------------------------------
double * MooseViewer::getFlashlightDirection(void)
{
  return this->FlashlightDirection;
}
