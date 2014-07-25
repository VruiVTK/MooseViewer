#include "ClippingPlane.h"

/*
 * ClippingPlane - Constructor for ClippingPlane class.
 */
ClippingPlane::ClippingPlane(void) {
} // end ClippingPlane()

/*
 * ~ClippingPlane - Destructor for ClippingPlane class.
 */
ClippingPlane::~ClippingPlane(void) {
} // end ~ClippingPlane()

/*
 * isActive
 *
 * return - bool
 */
bool ClippingPlane::isActive(void) {
	return active;
} // end isActive()

/*
 * setActive
 *
 * parameter _active - bool
 */
void ClippingPlane::setActive(bool _active) {
	active = _active;
} // end setActive()

/*
 * isAllocated
 *
 * return - bool
 */
bool ClippingPlane::isAllocated(void) {
	return allocated;
} // end isAllocated()

/*
 * setAllocated
 *
 * parameter _allocated - bool
 */
void ClippingPlane::setAllocated(bool _allocated) {
	allocated = _allocated;
} // end setAllocated()

/*
 * getPlane
 *
 * return - Vrui::Plane
 */
Vrui::Plane ClippingPlane::getPlane(void) {
	return plane;
} // end getPlane()

/*
 * setPlane
 *
 * parameter _plane - Vrui::Plane
 */
void ClippingPlane::setPlane(Vrui::Plane _plane) {
	plane = _plane;
} // end setPlane()
