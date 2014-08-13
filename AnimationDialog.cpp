// MooseViewer includes
#include "AnimationDialog.h"
#include "MooseViewer.h"

// Vrui includes
#include <Vrui/Vrui.h>

// VTK includes
#include <vtkExodusIIReader.h>

/*
 * AnimationDialog - Constructor for AnimationDialog class.
 * 		extends GLMotif::PopupWindow
 */
AnimationDialog::AnimationDialog(MooseViewer * mooseViewer) :
    GLMotif::PopupWindow("AnimationDialog", Vrui::getWidgetManager(),
      "Animation")
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

  int currentTimeStep = this->mooseViewer->reader->GetTimeStep();
  int firstTimeStep = this->mooseViewer->reader->GetTimeStepRange()[0];
  int lastTimeStep = this->mooseViewer->reader->GetTimeStepRange()[1];

  if (strcmp(_callbackData->button->getName(), "First") == 0)
    {
    this->mooseViewer->reader->SetTimeStep(
      this->mooseViewer->reader->GetTimeStepRange()[0]);
    }
  else if (strcmp(_callbackData->button->getName(), "Previous") == 0)
    {
    if (currentTimeStep > firstTimeStep)
      {
      this->mooseViewer->reader->SetTimeStep(
        this->mooseViewer->reader->GetTimeStep() - 1);
      }
    }
  else if (strcmp(_callbackData->button->getName(), "Next") == 0)
    {
    if (currentTimeStep < lastTimeStep)
      {
      this->mooseViewer->reader->SetTimeStep(
        this->mooseViewer->reader->GetTimeStep() + 1);
      }
    }
  else if (strcmp(_callbackData->button->getName(), "Last") == 0)
    {
    this->mooseViewer->reader->SetTimeStep(
      this->mooseViewer->reader->GetTimeStepRange()[1]);
    }
  Vrui::requestUpdate();
}

/*
 * playPauseCallback - Render the appropriate frame.
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
 * stopAnimation - Stop the animation.
 */
void AnimationDialog::stopAnimation(void)
{
  this->playButton->setString("Play");
}
