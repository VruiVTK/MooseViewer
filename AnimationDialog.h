#ifndef _ANIMATIONDIALOG_H
#define _ANIMATIONDIALOG_H

// Vrui includes
#include <GLMotif/Button.h>
#include <GLMotif/PopupWindow.h>
#include <GLMotif/RowColumn.h>
#include <GLMotif/StyleSheet.h>
#include <GLMotif/WidgetManager.h>

// Forward declarations
class MooseViewer;

class AnimationDialog: public GLMotif::PopupWindow
{
public:
    AnimationDialog(MooseViewer * mooseViewer);
    virtual ~AnimationDialog(void);

    void stopAnimation(void);

private:
    MooseViewer * mooseViewer;

    void initialize(void);
    void createAnimationDialog(const GLMotif::StyleSheet& styleSheet);
    GLMotif::RowColumn* createButtonBox(GLMotif::RowColumn*& colorMapDialog);

    void renderFrameCallback(GLMotif::Button::CallbackData* _callbackData);
    void playPauseCallback(GLMotif::Button::CallbackData* _callbackData);

    GLMotif::Button* playButton;
};

#endif //_ANIMATIONDIALOG_H
