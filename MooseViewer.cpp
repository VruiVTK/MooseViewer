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
#include <vtkAppendPolyData.h>
#include <vtkCellData.h>
#include <vtkCellDataToPointData.h>
#include <vtkColorTransferFunction.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkContourFilter.h>
#include <vtkCubeSource.h>
#include <vtkExodusIIReader.h>
#include <vtkGaussianSplatter.h>
#include <vtkImageData.h>
#include <vtkLight.h>
#include <vtkLookupTable.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkNew.h>
#include <vtkOutlineFilter.h>
#include <vtkPiecewiseFunction.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkSmartVolumeMapper.h>
#include <vtkUnstructuredGrid.h>

// MooseViewer includes
#include "AnimationDialog.h"
#include "BaseLocator.h"
#include "ClippingPlane.h"
#include "ClippingPlaneLocator.h"
#include "ColorMap.h"
#include "Contours.h"
#include "DataItem.h"
#include "FlashlightLocator.h"
#include "Isosurfaces.h"
#include "MooseViewer.h"
#include "ScalarWidget.h"
#include "TransferFunction1D.h"

//----------------------------------------------------------------------------
MooseViewer::MooseViewer(int& argc,char**& argv)
  :Vrui::Application(argc,argv),
  aIsosurface(0),
  AIsosurface(false),
  analysisTool(0),
  bIsosurface(0),
  BIsosurface(false),
  cIsosurface(0),
  CIsosurface(false),
  ClippingPlanes(NULL),
  colorByVariablesMenu(0),
  ContoursDialog(NULL),
  ContourVisible(true),
  FileName(0),
  FirstFrame(true),
  FlashlightDirection(0),
  FlashlightPosition(0),
  FlashlightSwitch(0),
  GaussianSplatterDims(30),
  GaussianSplatterExp(-1.0),
  GaussianSplatterRadius(0.01),
  isosurfacesDialog(NULL),
  IsPlaying(false),
  Loop(false),
  mainMenu(NULL),
  NumberOfClippingPlanes(6),
  Opacity(1.0),
  opacityValue(NULL),
  Outline(true),
  renderingDialog(NULL),
  RepresentationType(2),
  RequestedRenderMode(3),
  sampleValue(NULL),
  variablesMenu(0),
  Volume(false)
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

  /* Isosurfaces */
  this->IsosurfaceColormap = new double[4*256];

  this->isosurfaceLUT = vtkSmartPointer<vtkLookupTable>::New();
  this->isosurfaceLUT->SetNumberOfColors(256);
  this->isosurfaceLUT->Build();

  /* Color Map */
  this->ColorMap = new double[4*256];

  /* Histogram */
  this->Histogram = new float[256];
  for(int j = 0; j < 256; ++j)
    {
    this->Histogram[j] = 0.0;
    }

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
  if(this->Histogram)
    {
    delete[] this->Histogram;
    }
  if(this->IsosurfaceColormap)
    {
    delete[] this->IsosurfaceColormap;
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

  GLMotif::ToggleButton * showContoursDialog = new GLMotif::ToggleButton(
    "ShowContoursDialog", mainMenu, "Contours");
  showContoursDialog->setToggle(false);
  showContoursDialog->getValueChangedCallbacks().add(this,
    &MooseViewer::showContoursDialogCallback);

  GLMotif::ToggleButton * showIsosurfacesDialog =
    new GLMotif::ToggleButton("ShowIsosurfacesDialog", mainMenu,
    "Isosurfaces");
  showIsosurfacesDialog->setToggle(false);
  showIsosurfacesDialog->getValueChangedCallbacks().add(
    this, &MooseViewer::showIsosurfacesDialogCallback);

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
  GLMotif::ToggleButton* showVolume=new GLMotif::ToggleButton(
    "ShowVolume",representation_RadioBox,"Volume");
  showVolume->getValueChangedCallbacks().add(
    this,&MooseViewer::changeRepresentationCallback);

  representation_RadioBox->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
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
vtkSmartPointer<vtkDataArray> MooseViewer::getSelectedArray(int & type) const
{
  vtkSmartPointer<vtkDataArray> dataArray;
  std::string selectedArray = this->getSelectedColorByArrayName();
  if (selectedArray.empty())
    {
    return dataArray;
    }

  vtkSmartPointer<vtkCompositeDataGeometryFilter> compositeFilter =
    vtkSmartPointer<vtkCompositeDataGeometryFilter>::New();
  compositeFilter->SetInputConnection(this->reader->GetOutputPort());
  compositeFilter->Update();

  dataArray = vtkDataArray::SafeDownCast(
    compositeFilter->GetOutput()->GetPointData(
      )->GetArray(selectedArray.c_str()));
  if (!dataArray)
    {
    dataArray = vtkDataArray::SafeDownCast(
      compositeFilter->GetOutput()->GetCellData(
        )->GetArray(selectedArray.c_str()));
    if (!dataArray)
      {
      std::cerr << "The selected array is neither PointDataArray"\
        " nor CellDataArray" << std::endl;
      return dataArray;
      }
    else
      {
      type = 1; // CellData
      return dataArray;
      }
    }
  else
    {
    type = 0;
    return dataArray; // PointData
    }
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
    colorby_RadioBox->setSelectionMode(GLMotif::RadioBox::ALWAYS_ONE);
    this->updateHistogram();
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
  opacitySlider->setValue(Opacity);
  opacitySlider->setValueRange(0.0, 1.0, 0.1);
  opacitySlider->getValueChangedCallbacks().add(
    this, &MooseViewer::opacitySliderCallback);
  opacityValue = new GLMotif::TextField("OpacityValue", opacityRow, 6);
  opacityValue->setFieldWidth(6);
  opacityValue->setPrecision(3);
  opacityValue->setValue(Opacity);
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
  sampleSlider->setValue(GaussianSplatterExp);
  sampleSlider->setValueRange(20, 200.0, 10.0);
  sampleSlider->getValueChangedCallbacks().add(
    this, &MooseViewer::sampleSliderCallback);
  sampleValue = new GLMotif::TextField("SampleValue", sampleRow, 6);
  sampleValue->setFieldWidth(6);
  sampleValue->setPrecision(3);
  std::stringstream stringstr;
  stringstr << GaussianSplatterDims << " x " << GaussianSplatterDims <<
    " x " << GaussianSplatterDims;
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
  radiusSlider->setValue(GaussianSplatterRadius);
  radiusSlider->setValueRange(0.0, 0.1, 0.01);
  radiusSlider->getValueChangedCallbacks().add(
    this, &MooseViewer::radiusSliderCallback);
  radiusValue = new GLMotif::TextField("RadiusValue", radiusRow, 6);
  radiusValue->setFieldWidth(6);
  radiusValue->setPrecision(3);
  radiusValue->setValue(GaussianSplatterRadius);
  radiusRow->manageChild();

  GLMotif::RowColumn * exponentRow = new GLMotif::RowColumn(
    "ExponentRow", dialog, false);
  exponentRow->setOrientation(GLMotif::RowColumn::HORIZONTAL);
  exponentRow->setPacking(GLMotif::RowColumn::PACK_GRID);
  GLMotif::Label* expLabel = new GLMotif::Label(
    "ExpLabel", exponentRow, "Volume Sampling Exponent");
  GLMotif::Slider* exponentSlider = new GLMotif::Slider(
    "ExponentSlider", exponentRow, GLMotif::Slider::HORIZONTAL, ss.fontHeight*10.0f);
  exponentSlider->setValue(GaussianSplatterExp);
  exponentSlider->setValueRange(-5.0, 5.0, 1.0);
  exponentSlider->getValueChangedCallbacks().add(
    this, &MooseViewer::exponentSliderCallback);
  exponentValue = new GLMotif::TextField("ExponentValue", exponentRow, 6);
  exponentValue->setFieldWidth(6);
  exponentValue->setPrecision(3);
  exponentValue->setValue(GaussianSplatterExp);
  exponentRow->manageChild();

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
      UP_RAMP, 0.0, 1.0);
    this->ColorEditor->getColorMapChangedCallbacks().add(
      this, &MooseViewer::colorMapChangedCallback);
    this->ColorEditor->getAlphaChangedCallbacks().add(this,
      &MooseViewer::alphaChangedCallback);
    updateColorMap();
    updateAlpha();

    /* Isosurfaces */
    this->isosurfacesDialog = new Isosurfaces(
      this->IsosurfaceColormap, this);
    this->isosurfacesDialog->setIsosurfacesColorMap(
      CINVERSE_RAINBOW, 0.0, 1.0);
    this->isosurfacesDialog->exportIsosurfacesColorMap(
      this->IsosurfaceColormap);
    this->updateIsosurfaceColorMap(this->IsosurfaceColormap);
    this->aIsosurface = 0.0;
    this->bIsosurface = 0.0;
    this->cIsosurface = 0.0;

    /* Contours */
    this->ContoursDialog = new Contours(this);
    this->ContoursDialog->getAlphaChangedCallbacks().add(this,
      &MooseViewer::contourValueChangedCallback);

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
  this->updateHistogram();
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

  dataItem->mapperVolume->SetRequestedRenderMode(this->RequestedRenderMode);

  vtkNew<vtkPolyDataMapper> mapperOutline;
  mapperOutline->SetInputConnection(dataOutline->GetOutputPort());
  dataItem->actorOutline->SetMapper(mapperOutline.GetPointer());
  dataItem->actorOutline->GetProperty()->SetColor(1,1,1);

  dataItem->aContour->SetValue(0, this->aIsosurface);
  dataItem->aContourMapper->SetLookupTable(this->isosurfaceLUT);

  dataItem->bContour->SetValue(0, this->bIsosurface);
  dataItem->bContourMapper->SetLookupTable(this->isosurfaceLUT);

  dataItem->cContour->SetValue(0, this->cIsosurface);
  dataItem->cContourMapper->SetLookupTable(this->isosurfaceLUT);
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
  int selectedArrayType = -1;
  double* dataRange = NULL;
  if (!selectedArray.empty())
    {
    dataItem->mapper->SelectColorArray(selectedArray.c_str());

    bool dataArrayFound = true;

    vtkSmartPointer<vtkUnstructuredGrid> usg =
      vtkSmartPointer<vtkUnstructuredGrid>::New();
    usg->DeepCopy(
      vtkUnstructuredGrid::SafeDownCast(vtkMultiBlockDataSet::SafeDownCast(
          this->reader->GetOutput()->GetBlock(0))->GetBlock(0)));

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
      vtkSmartPointer<vtkCellDataToPointData> cellToPoint =
        vtkSmartPointer<vtkCellDataToPointData>::New();
      cellToPoint->SetInputData(usg);
      cellToPoint->Update();
      usg->DeepCopy(vtkUnstructuredGrid::SafeDownCast(cellToPoint->GetOutput()));
      if (!dataArray)
        {
        std::cerr << "The selected array is neither PointDataArray"\
          " nor CellDataArray" << std::endl;
        dataArrayFound = false;
        }
      else
        {
        selectedArrayType = 1;
        }
      }
    else
      {
      dataItem->mapper->SetScalarModeToUsePointFieldData();
      selectedArrayType = 0;
      }

    if (dataArrayFound)
      {
      dataRange = dataArray->GetRange();
      dataItem->mapper->SetScalarRange(dataRange);
      dataItem->lut->SetTableRange(dataRange);
      dataItem->aContourMapper->SetScalarRange(dataRange);
      dataItem->bContourMapper->SetScalarRange(dataRange);
      dataItem->cContourMapper->SetScalarRange(dataRange);
      this->isosurfaceLUT->SetTableRange(dataRange);
      }

    usg->GetPointData()->SetActiveScalars(selectedArray.c_str());
    int imageExtent[6] = {0, this->GaussianSplatterDims-1,
      0, this->GaussianSplatterDims-1, 0, this->GaussianSplatterDims-1};
    dataItem->gaussian->GetOutput()->SetExtent(imageExtent);
    dataItem->gaussian->SetInputData(usg);
    dataItem->gaussian->SetModelBounds(usg->GetBounds());
    dataItem->gaussian->SetSampleDimensions(this->GaussianSplatterDims,
      this->GaussianSplatterDims, this->GaussianSplatterDims);
    dataItem->gaussian->SetRadius(this->GaussianSplatterRadius);
    dataItem->gaussian->SetExponentFactor(this->GaussianSplatterExp);

    dataItem->colorFunction->RemoveAllPoints();
    dataItem->opacityFunction->RemoveAllPoints();
    double dataRangeMax = dataRange ? dataRange[1] : 1.0;
    double dataRangeMin = dataRange ? dataRange[0] : 0.0;
    double step = (dataRangeMax - dataRangeMin)/255.0;
    for (int i = 0; i < 256; ++i)
      {
      dataItem->lut->SetTableValue(i,
        this->ColorMap[4*i + 0],
        this->ColorMap[4*i + 1],
        this->ColorMap[4*i + 2],
        this->ColorMap[4*i + 3]);
      dataItem->colorFunction->AddRGBPoint(
        dataRangeMin + (double)(i*step),
        this->ColorMap[4*i + 0],
        this->ColorMap[4*i + 1],
        this->ColorMap[4*i + 2]);
      dataItem->opacityFunction->AddPoint(
        dataRangeMin + (double)(i*step),
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
  if (this->Volume)
    {
    if (!selectedArray.empty())
      {
      dataItem->actorVolume->VisibilityOn();
      }
    }
  else
    {
    dataItem->actorVolume->VisibilityOff();
    }

  /* Isosurfaces */
  if (this->AIsosurface)
    {
    if (!selectedArray.empty() && (selectedArrayType >= 0) && dataRange)
      {
      dataItem->aContour->SetInputData(vtkMultiBlockDataSet::SafeDownCast(
          this->reader->GetOutput()->GetBlock(0))->GetBlock(0));
      double aIsosurfaceValue = (this->aIsosurface / 255.0)*
        (dataRange[1] - dataRange[0]) + dataRange[0];
      dataItem->aContour->SetInputArrayToProcess(0,0,0,
          vtkDataObject::FIELD_ASSOCIATION_POINTS, selectedArray.c_str());
      dataItem->aContour->SetValue(0, aIsosurfaceValue);
      dataItem->aContour->GetOutput()->GetPointData()->SetActiveScalars(
        selectedArray.c_str());
      dataItem->actorAContour->VisibilityOn();
      }
    }
  else
    {
    dataItem->actorAContour->VisibilityOff();
    }

  if (this->BIsosurface)
    {
    if (!selectedArray.empty() && (selectedArrayType >= 0) && dataRange)
      {
      dataItem->bContour->SetInputData(vtkMultiBlockDataSet::SafeDownCast(
          this->reader->GetOutput()->GetBlock(0))->GetBlock(0));
      double bIsosurfaceValue = (this->bIsosurface / 255.0)*
        (dataRange[1] - dataRange[0]) + dataRange[0];
      dataItem->bContour->SetInputArrayToProcess(0,0,0,
          vtkDataObject::FIELD_ASSOCIATION_POINTS, selectedArray.c_str());
      dataItem->bContour->SetValue(0, bIsosurfaceValue);
      dataItem->bContour->GetOutput()->GetPointData()->SetActiveScalars(
        selectedArray.c_str());
      dataItem->actorBContour->VisibilityOn();
      }
    }
  else
    {
    dataItem->actorBContour->VisibilityOff();
    }

  if (this->CIsosurface)
    {
    if (!selectedArray.empty() && (selectedArrayType >= 0) && dataRange)
      {
      dataItem->cContour->SetInputData(vtkMultiBlockDataSet::SafeDownCast(
          this->reader->GetOutput()->GetBlock(0))->GetBlock(0));
      double cIsosurfaceValue = (this->cIsosurface / 255.0)*
        (dataRange[1] - dataRange[0]) + dataRange[0];
      dataItem->cContour->SetInputArrayToProcess(0,0,0,
          vtkDataObject::FIELD_ASSOCIATION_POINTS, selectedArray.c_str());
      dataItem->cContour->SetValue(0, cIsosurfaceValue);
      dataItem->cContour->GetOutput()->GetPointData()->SetActiveScalars(
        selectedArray.c_str());
      dataItem->actorCContour->VisibilityOn();
      }
    }
  else
    {
    dataItem->actorCContour->VisibilityOff();
    }

  /* Contours */
  if (this->ContourVisible)
    {
    if (!selectedArray.empty() && (selectedArrayType >= 0) && dataRange)
      {
      vtkSmartPointer<vtkMultiBlockDataSet> mb =
        vtkMultiBlockDataSet::SafeDownCast(
          this->reader->GetOutput()->GetBlock(0));
      dataItem->contours->RemoveAllInputConnections(0);
      for (int i = 0; i < mb->GetNumberOfBlocks(); ++i)
        {
        vtkNew<vtkCellDataToPointData> cellToPointData;
        cellToPointData->SetInputData(mb->GetBlock(i));

        vtkNew<vtkContourFilter> contour;
        contour->ComputeScalarsOn();
        contour->SetInputConnection(cellToPointData->GetOutputPort());
        contour->SetInputArrayToProcess(0,0,0,
          vtkDataObject::FIELD_ASSOCIATION_POINTS, selectedArray.c_str());
        contour->SetNumberOfContours(this->ContourValues.size());
        for (int c = 0; c < this->ContourValues.size(); ++c)
          {
          double val = (this->ContourValues.at(c) / 255.0)*
            (dataRange[1] - dataRange[0]) + dataRange[0];
          contour->SetValue(c, val);
          }
        dataItem->contours->AddInputConnection(0, contour->GetOutputPort());
        }
      dataItem->contourMapper->SetInputConnection(
        dataItem->contours->GetOutputPort());
      dataItem->contourMapper->SetScalarRange(dataRange);
      dataItem->contourMapper->SelectColorArray(selectedArray.c_str());
      dataItem->contourActor->VisibilityOn();
      }
    }
  else
    {
    dataItem->contourActor->VisibilityOff();
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
void MooseViewer::sampleSliderCallback(
  GLMotif::Slider::ValueChangedCallbackData* callBackData)
{
  this->GaussianSplatterDims = static_cast<double>(callBackData->value);
  std::stringstream ss;
  ss << GaussianSplatterDims << " x " << GaussianSplatterDims <<
    " x " << GaussianSplatterDims;
  sampleValue->setString(ss.str().c_str());
}

//----------------------------------------------------------------------------
void MooseViewer::radiusSliderCallback(
  GLMotif::Slider::ValueChangedCallbackData* callBackData)
{
  this->GaussianSplatterRadius = static_cast<double>(callBackData->value);
  radiusValue->setValue(callBackData->value);
}

//----------------------------------------------------------------------------
void MooseViewer::exponentSliderCallback(
  GLMotif::Slider::ValueChangedCallbackData* callBackData)
{
  this->GaussianSplatterExp = static_cast<double>(callBackData->value);
  exponentValue->setValue(callBackData->value);
}

//----------------------------------------------------------------------------
void MooseViewer::changeRepresentationCallback(
  GLMotif::ToggleButton::ValueChangedCallbackData* callBackData)
{
  /* Adjust representation state based on which toggle button changed state: */
  if (strcmp(callBackData->toggle->getName(), "ShowSurface") == 0)
    {
    this->RepresentationType = 2;
    this->Volume = false;
    }
  else if (strcmp(callBackData->toggle->getName(), "ShowSurfaceWithEdges") == 0)
    {
    this->RepresentationType = 3;
    this->Volume = false;
    }
  else if (strcmp(callBackData->toggle->getName(), "ShowWireframe") == 0)
    {
    this->RepresentationType = 1;
    this->Volume = false;
    }
  else if (strcmp(callBackData->toggle->getName(), "ShowPoints") == 0)
    {
    this->RepresentationType = 0;
    this->Volume = false;
    }
  else if (strcmp(callBackData->toggle->getName(), "ShowNone") == 0)
    {
    this->RepresentationType = -1;
    this->Volume = false;
    }
  else if (strcmp(callBackData->toggle->getName(), "ShowVolume") == 0)
    {
    this->Volume = callBackData->set;
    if (this->Volume)
      {
      this->RepresentationType = -1;
      }
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
void MooseViewer::showIsosurfacesDialogCallback(
  GLMotif::ToggleButton::ValueChangedCallbackData* callBackData)
{
  /* open/close isosurfaces dialog based on which toggle button changed state: */
  if (strcmp(callBackData->toggle->getName(), "ShowIsosurfacesDialog") == 0) {
    if (callBackData->set) {
      /* Open the isosurfaces dialog at the same position as the main menu: */
      Vrui::getWidgetManager()->popupPrimaryWidget(
        isosurfacesDialog, Vrui::getWidgetManager(
          )->calcWidgetTransformation(mainMenu));
    } else {
      /* Close the isosurfaces dialog: */
      Vrui::popdownPrimaryWidget(isosurfacesDialog);
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
  this->updateHistogram();
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

//----------------------------------------------------------------------------
float * MooseViewer::getHistogram(void)
{
  return this->Histogram;
}

//----------------------------------------------------------------------------
void MooseViewer::updateHistogram(void)
{
  /* Clear the histogram */
  for(int j = 0; j < 256; ++j)
    {
    this->Histogram[j] = 0.0;
    }

  int type = -1;
  vtkSmartPointer<vtkDataArray> dataArray = this->getSelectedArray(type);
  if (!dataArray || (type < 0))
    {
    return;
    }

  double * scalarRange = dataArray->GetRange();
  if (fabs(scalarRange[1] - scalarRange[0]) < 1e-6)
    {
    return;
    }

  // Divide the range into 256 bins
  for (int i = 0; i < dataArray->GetNumberOfTuples(); ++i)
    {
    double * tuple = dataArray->GetTuple(i);
    int bin = static_cast<int>((tuple[0] - scalarRange[0])*255.0 /
      (scalarRange[1] - scalarRange[0]));
    this->Histogram[bin] += 1;
    }

  this->ColorEditor->setHistogram(this->Histogram);
  this->ContoursDialog->setHistogram(this->Histogram);
  Vrui::requestUpdate();
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
      }
    else
      {
      /* Close the slices dialog: */
      Vrui::popdownPrimaryWidget(ContoursDialog);
      }
    }
}

//----------------------------------------------------------------------------
void MooseViewer::contourValueChangedCallback(Misc::CallbackData* callBackData)
{
  this->ContourValues = ContoursDialog->getContourValues();
  Vrui::requestUpdate();
}

//----------------------------------------------------------------------------
void MooseViewer::setContourVisible(bool visible)
{
  this->ContourVisible = visible;
}

//----------------------------------------------------------------------------
void MooseViewer::setRequestedRenderMode(int mode)
{
  this->RequestedRenderMode = mode;
}

//----------------------------------------------------------------------------
int MooseViewer::getRequestedRenderMode(void) const
{
  return this->RequestedRenderMode;
}

//----------------------------------------------------------------------------
void MooseViewer::setAIsosurface(float aIsosurface)
{
  this->aIsosurface = aIsosurface;
}

//----------------------------------------------------------------------------
void MooseViewer::setBIsosurface(float bIsosurface)
{
  this->bIsosurface = bIsosurface;
}

//----------------------------------------------------------------------------
void MooseViewer::setCIsosurface(float cIsosurface)
{
  this->cIsosurface = cIsosurface;
}

//----------------------------------------------------------------------------
void MooseViewer::showAIsosurface(bool AIsosurface)
{
  this->AIsosurface = AIsosurface;
}

//----------------------------------------------------------------------------
void MooseViewer::showBIsosurface(bool BIsosurface)
{
  this->BIsosurface = BIsosurface;
}

//----------------------------------------------------------------------------
void MooseViewer::showCIsosurface(bool CIsosurface)
{
  this->CIsosurface = CIsosurface;
}

//----------------------------------------------------------------------------
void MooseViewer::updateIsosurfaceColorMap(double* IsosurfaceColormap)
{
  this->IsosurfaceColormap = IsosurfaceColormap;
  for (int i=0;i<256;i++)
    {
    this->isosurfaceLUT->SetTableValue(i,
      this->IsosurfaceColormap[4*i + 0],this->IsosurfaceColormap[4*i + 1],
      this->IsosurfaceColormap[4*i + 2], 1.0);
    }
}

