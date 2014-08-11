// STL includes
#include <algorithm>
#include <iostream>
#include <math.h>
#include <string>

// OpenGL/Motif includes
#include <GL/GLContextData.h>
#include <GL/gl.h>
#include <GLMotif/CascadeButton.h>
#include <GLMotif/Menu.h>
#include <GLMotif/Popup.h>
#include <GLMotif/PopupMenu.h>
#include <GLMotif/RadioBox.h>
#include <GLMotif/ToggleButton.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/SubMenu.h>
#include <GLMotif/WidgetManager.h>

// VRUI includes
#include <Vrui/Application.h>
#include <Vrui/Tool.h>
#include <Vrui/ToolManager.h>
#include <Vrui/Vrui.h>

// VTK includes
#include <ExternalVTKWidget.h>
#include <vtkActor.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkCubeSource.h>
#include <vtkExodusIIReader.h>
#include <vtkLight.h>
#include <vtkNew.h>
#include <vtkOutlineFilter.h>
#include <vtkPointData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

#include <vtkCellData.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkUnstructuredGrid.h>

// MooseViewer includes
#include "BaseLocator.h"
#include "ClippingPlane.h"
#include "ClippingPlaneLocator.h"
#include "FlashlightLocator.h"
#include "MooseViewer.h"

//----------------------------------------------------------------------------
MooseViewer::DataItem::DataItem(void)
{
  /* Initialize VTK renderwindow and renderer */
  this->externalVTKWidget = vtkSmartPointer<ExternalVTKWidget>::New();
  this->actor = vtkSmartPointer<vtkActor>::New();
  this->actor->GetProperty()->SetEdgeColor(1.0, 1.0, 1.0);
  this->externalVTKWidget->GetRenderer()->AddActor(this->actor);

  this->compositeFilter = vtkSmartPointer<vtkCompositeDataGeometryFilter>::New();

  this->flashlight = vtkSmartPointer<vtkLight>::New();
  this->flashlight->SwitchOff();
  this->flashlight->SetLightTypeToHeadlight();
  this->flashlight->SetColor(0.0, 1.0, 1.0);
  this->flashlight->SetConeAngle(10);
  this->flashlight->SetPositional(true);
  this->externalVTKWidget->GetRenderer()->AddLight(this->flashlight);

  this->actorOutline = vtkSmartPointer<vtkActor>::New();
  this->actorOutline->GetProperty()->SetColor(1.0, 1.0, 1.0);
  this->externalVTKWidget->GetRenderer()->AddActor(this->actorOutline);
}

//----------------------------------------------------------------------------
MooseViewer::DataItem::~DataItem(void)
{
}

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
  colorByVariablesMenu(0)
{
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
  if(this->DataBounds)
    {
    delete[] this->DataBounds;
    }
  if(this->FlashlightSwitch)
    {
    delete[] this->FlashlightSwitch;
    }
  if(this->FlashlightPosition)
    {
    delete[] this->FlashlightPosition;
    }
  if(this->FlashlightDirection)
    {
    delete[] this->FlashlightDirection;
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
//  reader->GenerateObjectIdCellArrayOn();
//  reader->GenerateGlobalElementIdArrayOn();
//  reader->GenerateGlobalNodeIdArrayOn();
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

  for (i = 0; i < this->reader->GetNumberOfPointResultArrays(); ++i)
    {
    GLMotif::ToggleButton* button = new GLMotif::ToggleButton(
      this->reader->GetPointResultArrayName(i),
      variablesMenu, this->reader->GetPointResultArrayName(i));
    button->getValueChangedCallbacks().add(
      this, &MooseViewer::changeVariablesCallback);
    button->setToggle(false);
    }
}

//----------------------------------------------------------------------------
void MooseViewer::updateColorByVariablesMenu(void)
{
  /* Preserve the selection */
  std::string selectedToggle;
  GLMotif::RadioBox* old_colorby_RadioBox =
    static_cast<GLMotif::RadioBox*> (colorByVariablesMenu->getChild(0));
  if (old_colorby_RadioBox)
    {
    GLMotif::ToggleButton* selectedToggleButton =
      old_colorby_RadioBox->getSelectedToggle();
    selectedToggle.assign(selectedToggleButton->getString());
    }

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
}

//----------------------------------------------------------------------------
void MooseViewer::initContext(GLContextData& contextData) const
{
  /* Create a new context data item */
  DataItem* dataItem = new DataItem();
  contextData.addDataItem(this, dataItem);

  vtkNew<vtkPolyDataMapper> mapper;
  dataItem->actor->SetMapper(mapper.GetPointer());

  vtkNew<vtkOutlineFilter> dataOutline;

  dataItem->compositeFilter->SetInputConnection(this->reader->GetOutputPort());
  dataItem->compositeFilter->Update();
  dataItem->compositeFilter->GetOutput()->GetBounds(this->DataBounds);
  mapper->SetInputConnection(dataItem->compositeFilter->GetOutputPort());

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

  this->reader->Update();

  /* Get context data item */
  DataItem* dataItem = contextData.retrieveDataItem<DataItem>(this);

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
  this->reader->SetPointResultArrayStatus(nameStr.c_str(), setter);
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
