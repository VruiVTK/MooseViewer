#include <iostream>

/* Vrui includes */
#include <Vrui/LocatorTool.h>
#include <Geometry/Point.h>
#include <Geometry/Vector.h>
#include <Geometry/OrthogonalTransformation.h>

#include "BaseLocator.h"
#include "ClippingPlane.h"
#include "ClippingPlaneLocator.h"
#include "MooseViewer.h"

/*
 * ClippingPlaneLocator - Constructor for ClippingPlaneLocator class.
 *
 * parameter locatorTool - Vrui::LocatorTool *
 * parameter MooseViewer - MooseViewer *
 */
ClippingPlaneLocator::ClippingPlaneLocator(Vrui::LocatorTool * locatorTool,
		MooseViewer* MooseViewer) :
	BaseLocator(locatorTool, MooseViewer), clippingPlane(0) {
	/* Find a clipping plane index for this locator: */
	ClippingPlane * clippingPlanes = MooseViewer->getClippingPlanes();
	for (int i=0; i<MooseViewer->getNumberOfClippingPlanes(); ++i)
		if (!clippingPlanes[i].isAllocated()) {
			clippingPlane=&clippingPlanes[i];
			break;
		}

	/* Allocate the clipping plane: */
	if (clippingPlane!=0) {
		clippingPlane->setActive(false);
		clippingPlane->setAllocated(true);
	}
} // end ClippingPlaneLocator()

/*
 * ~ClippingPlaneLocator - Destructor for ClippingPlaneLocator class.
 */
ClippingPlaneLocator::~ClippingPlaneLocator(void) {
	if (clippingPlane!=0) {
		clippingPlane->setActive(false);
		clippingPlane->setAllocated(false);
	}
} // end ~ClippingPlaneLocator()

/*
 * motionCallback
 *
 * parameter callbackData - Vrui::LocatorTool::MotionCallbackData *
 */
void ClippingPlaneLocator::motionCallback(
		Vrui::LocatorTool::MotionCallbackData* callbackData) {
	if (clippingPlane!=0&&clippingPlane->isActive()) {
		Vrui::Vector planeNormal=
				callbackData->currentTransformation.transform(Vrui::Vector(0,
						1, 0));
		Vrui::Point planePoint=callbackData->currentTransformation.getOrigin();
		clippingPlane->setPlane(Vrui::Plane(planeNormal, planePoint));
	}
} // end motionCallback()

/*
 * buttonPressCallback
 *
 * parameter callbackData - Vrui::LocatorTool::ButtonPressCallbackData *
 */
void ClippingPlaneLocator::buttonPressCallback(
		Vrui::LocatorTool::ButtonPressCallbackData* callbackData) {
	if (clippingPlane!=0)
		clippingPlane->setActive(true);
} // end buttonPressCallback()

/*
 * buttonReleaseCallback
 *
 * parameter callbackData - Vrui::LocatorTool::ButtonReleaseCallbackData *
 */
void ClippingPlaneLocator::buttonReleaseCallback(
		Vrui::LocatorTool::ButtonReleaseCallbackData* callbackData) {
	if (clippingPlane!=0)
		clippingPlane->setActive(false);
} // end buttonReleaseCallback()
