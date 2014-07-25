#ifndef _FLASHLIGHTLOCATOR_H_
#define _FLASHLIGHTLOCATOR_H_

#include "BaseLocator.h"
#include "MooseViewer.h"
#include <vtkSmartPointer.h>

#include <Vrui/LocatorTool.h>

// Begin forward declarations
class vtkLight;
// End forward declarations

class FlashlightLocator : public BaseLocator
{
public:
  FlashlightLocator(Vrui::LocatorTool* locatorTool,
    MooseViewer * MooseViewer);
  ~FlashlightLocator(void);

  virtual void buttonPressCallback(
    Vrui::LocatorTool::ButtonPressCallbackData* callbackData);
  virtual void buttonReleaseCallback(
    Vrui::LocatorTool::ButtonReleaseCallbackData* callbackData);
  virtual void motionCallback(
    Vrui::LocatorTool::MotionCallbackData* callbackData);
private:
  int * FlashlightSwitch;
  double * FlashlightPosition;
  double * FlashlightDirection;
};
#endif //_FLASHLIGHTLOCATOR_H_
