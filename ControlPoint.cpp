#include "ControlPoint.h"
#include "RGBAColor.h"

/*
 * ControlPoint - Constructor for ControlPoint.
 */
ControlPoint::ControlPoint() {
} // end ControlPoint()

/*
 * ControlPoint - Constructor for ControlPoint.
 *
 * parameter _value - double
 * parameter _rgbaColor - RGBAColor
 */
ControlPoint::ControlPoint(double _value, RGBAColor* _rgbaColor) {
	value=_value;
	rgbaColor=_rgbaColor;
	left=0;
	right=0;
} // end ControlPoint()

/*
 * ~ControlPoint - Destructor for ControlPoint.
 */
ControlPoint::~ControlPoint() {
	rgbaColor=0;
} // end ~ControlPoint()
