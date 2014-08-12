#include "ScalarWidgetControlPoint.h"

/*
 * ScalarWidgetControlPoint - Constructor for ScalarWidgetControlPoint.
 */
ScalarWidgetControlPoint::ScalarWidgetControlPoint(void) {
} // end ScalarWidgetControlPoint()

/*
 * ScalarWidgetControlPoint - Constructor for ScalarWidgetControlPoint.
 *
 * parameter _value - double
 * parameter _scalar - float
 */
ScalarWidgetControlPoint::ScalarWidgetControlPoint(double _value, float _scalar) {
	value=_value;
	scalar=_scalar;
	left=0;
	right=0;
} // end ScalarWidgetControlPoint()

/*
 * ~ScalarWidgetControlPoint - Destructor for ScalarWidgetControlPoint.
 */
ScalarWidgetControlPoint::~ScalarWidgetControlPoint(void) {
} // end ~ScalarWidgetControlPoint()

/*
 * getScalar
 *
 * return - float
 */
float ScalarWidgetControlPoint::getScalar(void) {
	return scalar;
} // end getScalar()

/*
 * setScalar
 *
 * parameter _scalar - float
 */
void ScalarWidgetControlPoint::setScalar(float _scalar) {
	scalar = _scalar;
} // end setScalar()

/*
 * getValue
 *
 * return - double
 */
double ScalarWidgetControlPoint::getValue(void) {
	return value;
} // end getValue()

/*
 * setValue
 *
 * parameter _value - double
 */
void ScalarWidgetControlPoint::setValue(double _value) {
	value = _value;
} // end setValue()

/*
 * getX
 *
 * return - GLfloat
 */
GLfloat ScalarWidgetControlPoint::getX(void) {
	return x;
} // end getX()

/*
 * setX
 *
 * parameter _x - GLfloat
 */
void ScalarWidgetControlPoint::setX(GLfloat _x) {
	x = _x;
} // end setX()

/*
 * getY
 *
 * return - GLfloat
 */
GLfloat ScalarWidgetControlPoint::getY(void) {
	return y;
} // end getY()

/*
 * setY
 *
 * parameter _y - GLfloat
 */
void ScalarWidgetControlPoint::setY(GLfloat _y) {
	y = _y;
} // end setY()
