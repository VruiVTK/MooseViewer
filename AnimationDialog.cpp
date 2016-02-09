// MooseViewer includes
#include "AnimationDialog.h"
#include "MooseViewer.h"

// GL includes
#include <GL/GLFont.h>

// Vrui includes
#include <Vrui/Vrui.h>

// VTK includes
#include <vtkExodusIIReader.h>
#include <vtkInformation.h>
#include <vtkStreamingDemandDrivenPipeline.h>

// STL includes
#include <iomanip>
#include <sstream>

/*
 * AnimationDialog - Constructor for AnimationDialog class.
 * 		extends GLMotif::PopupWindow
 */
AnimationDialog::AnimationDialog(MooseViewer * mooseViewer) :
    GLMotif::PopupWindow("AnimationDialog", Vrui::getWidgetManager(),
      "Animation"),
    minTime(-1),
    maxTime(-1),
    numberOfTimeSteps(0)
{
  this->mooseViewer = mooseViewer;
  initialize();
}

/*
 * ~AnimationDialog - Destructor for AnimationDialog class.
 */
AnimationDialog::~AnimationDialog(void)
{
}

/*
 * initialize - Initialize the GUI for the AnimationDialog class.
 */
void AnimationDialog::initialize(void)
{
  vtkInformation* info = this->mooseViewer->reader().GetOutputInformation(0);
  this->minTime = info->Get(vtkStreamingDemandDrivenPipeline::TIME_RANGE())[0];
  this->maxTime = info->Get(vtkStreamingDemandDrivenPipeline::TIME_RANGE())[1];
  this->numberOfTimeSteps = this->mooseViewer->reader().GetNumberOfTimeSteps();

  const GLMotif::StyleSheet& styleSheet =
    *Vrui::getWidgetManager()->getStyleSheet();
  createAnimationDialog(styleSheet);
}

/*
 * createAnimationDialog - Create animation dialog.
 *
 * parameter styleSheet - const GLMotif::StyleSheet&
 */
void AnimationDialog::createAnimationDialog(const GLMotif::StyleSheet& styleSheet)
{
  GLMotif::RowColumn* animationDialog =
    new GLMotif::RowColumn("AnimationDialog", this, false);
  GLMotif::RowColumn* buttonBox = createButtonBox(animationDialog);
  buttonBox->manageChild();
  GLMotif::RowColumn* timeStepBox = createTimeStepBox(animationDialog);
  timeStepBox->manageChild();
  animationDialog->manageChild();
}

/*
 * createButtonBox - Create a box to hold buttons.
 *
 * parameter animationDialog - GLMotif::RowColumn*&
 * return - GLMotif::RowColumn*
 */
GLMotif::RowColumn* AnimationDialog::createButtonBox(
  GLMotif::RowColumn*& animationDialog)
{
  GLMotif::RowColumn* buttonBox =
    new GLMotif::RowColumn("ButtonBox", animationDialog, false);
  buttonBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
  buttonBox->setPacking(GLMotif::RowColumn::PACK_GRID);
  GLMotif::Button* firstButton = new GLMotif::Button(
    "First", buttonBox, "First Frame");
  firstButton->getSelectCallbacks().add(this,
    &AnimationDialog::renderFrameCallback);
  GLMotif::Button* previousButton = new GLMotif::Button(
    "Previous", buttonBox, "Previous Frame");
  previousButton->getSelectCallbacks().add(this,
    &AnimationDialog::renderFrameCallback);
  playButton = new GLMotif::Button(
    "PlayPause", buttonBox, "Play");
  playButton->getSelectCallbacks().add(this,
    &AnimationDialog::playPauseCallback);
  GLMotif::Button* nextButton = new GLMotif::Button(
    "Next", buttonBox, "Next Frame");
  nextButton->getSelectCallbacks().add(this,
    &AnimationDialog::renderFrameCallback);
  GLMotif::Button* lastButton = new GLMotif::Button(
    "Last", buttonBox, "Last Frame");
  lastButton->getSelectCallbacks().add(this,
    &AnimationDialog::renderFrameCallback);
  return buttonBox;
}

/*
 * createTimeStepBox - Create a box to hold timestep buttons
 *
 * parameter animationDialog - GLMotif::RowColumn*&
 * return - GLMotif::RowColumn*
 */
GLMotif::RowColumn* AnimationDialog::createTimeStepBox(
  GLMotif::RowColumn*& animationDialog)
{
  GLMotif::RowColumn* buttonBox =
    new GLMotif::RowColumn("ButtonBox", animationDialog, false);
  buttonBox->setOrientation(GLMotif::RowColumn::HORIZONTAL);
  buttonBox->setPacking(GLMotif::RowColumn::PACK_GRID);
  GLMotif::ToggleButton* loopButton = new GLMotif::ToggleButton(
    "Loop", buttonBox, "Loop");
  loopButton->setToggle(false);
  loopButton->getValueChangedCallbacks().add(this,
    &AnimationDialog::loopPlayCallback);
  std::stringstream stimeStepRange;
  stimeStepRange << "Step (" << std::setprecision(4) <<
    0 << " - " << this->numberOfTimeSteps - 1 << "):";
  GLMotif::Label* timeStepLabel = new GLMotif::Label(
    "TimeStep", buttonBox, stimeStepRange.str().c_str());
  timeStepLabel->setHAlignment(GLFont::Center);
  this->stepField = new GLMotif::TextField(
    "TimeStepField", buttonBox, 6);
  this->stepField->setHAlignment(GLFont::Center);
  this->stepField->setEditable(false);
  this->stepField->setValue(0);
  std::stringstream stimeValueRange;
  stimeValueRange << "Time (" << std::setprecision(4) <<
    this->minTime << " - " << this->maxTime << "):";
  GLMotif::Label* timeValueLabel = new GLMotif::Label(
    "TimeValue", buttonBox, stimeValueRange.str().c_str());
  timeValueLabel->setHAlignment(GLFont::Center);
  this->timeField = new GLMotif::TextField(
    "TimeValueField", buttonBox, 6);
  this->timeField->setHAlignment(GLFont::Center);
  this->timeField->setEditable(false);
  this->timeField->setPrecision(4);
  this->timeField->setValue(0.0f);
  return buttonBox;
}

/*
 * renderFrameCallback - Render the appropriate frame.
 *
 * parameter _callbackData - Misc::CallbackData*
 */
void AnimationDialog::renderFrameCallback(
  GLMotif::Button::CallbackData* _callbackData)
{
  if (this->mooseViewer->IsPlaying)
    {
    return;
    }

  int currentTimeStep = this->mooseViewer->reader().GetTimeStep();
  int firstTimeStep = this->mooseViewer->reader().GetTimeStepRange()[0];
  int lastTimeStep = this->mooseViewer->reader().GetTimeStepRange()[1];

  if (strcmp(_callbackData->button->getName(), "First") == 0)
    {
    this->mooseViewer->reader().SetTimeStep(
      this->mooseViewer->reader().GetTimeStepRange()[0]);
    }
  else if (strcmp(_callbackData->button->getName(), "Previous") == 0)
    {
    if (currentTimeStep > firstTimeStep)
      {
      this->mooseViewer->reader().SetTimeStep(
        this->mooseViewer->reader().GetTimeStep() - 1);
      }
    else if (this->mooseViewer->Loop)
      {
      this->mooseViewer->reader().SetTimeStep(
        this->mooseViewer->reader().GetTimeStepRange()[1]);
      }
    }
  else if (strcmp(_callbackData->button->getName(), "Next") == 0)
    {
    if (currentTimeStep < lastTimeStep)
      {
        this->mooseViewer->reader().SetTimeStep(
          this->mooseViewer->reader().GetTimeStep() + 1);
      }
    else if (this->mooseViewer->Loop)
      {
      this->mooseViewer->reader().SetTimeStep(
        this->mooseViewer->reader().GetTimeStepRange()[0]);
      }
    }
  else if (strcmp(_callbackData->button->getName(), "Last") == 0)
    {
    this->mooseViewer->reader().SetTimeStep(
      this->mooseViewer->reader().GetTimeStepRange()[1]);
    }
  Vrui::requestUpdate();
}

/*
 * playPauseCallback - Play/Pause animation.
 *
 * parameter _callbackData - Misc::CallbackData*
 */
void AnimationDialog::playPauseCallback(
  GLMotif::Button::CallbackData* _callbackData)
{
  this->mooseViewer->IsPlaying ?
    _callbackData->button->setString("Play") :
    _callbackData->button->setString("Pause");
  this->mooseViewer->IsPlaying = !this->mooseViewer->IsPlaying;
  Vrui::requestUpdate();
}

/*
 * loopPlayCallback - Loop around.
 *
 * parameter _callbackData - Misc::CallbackData*
 */
void AnimationDialog::loopPlayCallback(
  GLMotif::ToggleButton::ValueChangedCallbackData* _callbackData)
{
  this->mooseViewer->Loop = _callbackData->set;
}

/*
 * stopAnimation - Stop the animation.
 */
void AnimationDialog::stopAnimation(void)
{
  this->playButton->setString("Play");
}

/*
 * updateTimeInformation - Update the step and value information.
 */
void AnimationDialog::updateTimeInformation(void)
{
  int currentTimeStep = this->mooseViewer->reader().GetTimeStep();
  this->stepField->setValue(currentTimeStep);

  double timeValue = 0.0;
  if (this->numberOfTimeSteps > 1)
    {
    timeValue = ((this->maxTime - this->minTime) *
      (double(currentTimeStep)/(this->numberOfTimeSteps - 1))) + this->minTime;
    timeValue = (timeValue < 1e-4) ? 0.0 : timeValue;
    }
  else
    {
    timeValue = this->minTime;
    }
  this->timeField->setValue(timeValue);
}
