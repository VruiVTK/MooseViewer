#include <iostream>

/* Vrui includes */
#include <Vrui/LocatorTool.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>

/* VTK includes */
#include <vtkLight.h>

/* MooseViewer includes */
#include "BaseLocator.h"
#include "FlashlightLocator.h"
#include "MooseViewer.h"

/*
 * FlashlightLocator - Constructor for FlashlightLocator class.
 *
 * parameter locatorTool - Vrui::LocatorTool *
 * parameter MooseViewer - MooseViewer *
 */
FlashlightLocator::FlashlightLocator(Vrui::LocatorTool * locatorTool,
  MooseViewer* MooseViewer) :
  BaseLocator(locatorTool, MooseViewer),
  FlashlightSwitch(0),
  FlashlightPosition(0),
  FlashlightDirection(0)
{
  this->FlashlightSwitch = MooseViewer->getFlashlightSwitch();
  this->FlashlightPosition = MooseViewer->getFlashlightPosition();
  this->FlashlightDirection = MooseViewer->getFlashlightDirection();
} // end FlashlightLocator()

/*
 * ~FlashlightLocator - Destructor for FlashlightLocator class.
 */
FlashlightLocator::~FlashlightLocator(void) {
} // end ~FlashlightLocator()

/*
 * motionCallback
 *
 * parameter callbackData - Vrui::LocatorTool::MotionCallbackData *
 */
void FlashlightLocator::motionCallback(
		Vrui::LocatorTool::MotionCallbackData* callbackData) {
          Vrui::Point position = callbackData->currentTransformation.getOrigin();
          Vrui::Vector planeNormal =
            callbackData->currentTransformation.transform(Vrui::Vector(0,1,0));
          this->FlashlightPosition[0] = position[0];
          this->FlashlightPosition[1] = position[1];
          this->FlashlightPosition[2] = position[2];
          this->FlashlightDirection[0] = planeNormal[0];
          this->FlashlightDirection[1] = planeNormal[1];
          this->FlashlightDirection[2] = planeNormal[2];
} // end motionCallback()

/*
 * buttonPressCallback
 *
 * parameter callbackData - Vrui::LocatorTool::ButtonPressCallbackData *
 */
void FlashlightLocator::buttonPressCallback(
		Vrui::LocatorTool::ButtonPressCallbackData* callbackData)
{
  this->FlashlightSwitch[0] = 1;
} // end buttonPressCallback()

/*
 * buttonReleaseCallback
 *
 * parameter callbackData - Vrui::LocatorTool::ButtonReleaseCallbackData *
 */
void FlashlightLocator::buttonReleaseCallback(
		Vrui::LocatorTool::ButtonReleaseCallbackData* callbackData)
{
  this->FlashlightSwitch[0] = 0;
} // end buttonReleaseCallback()
