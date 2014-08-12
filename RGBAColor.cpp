#include "RGBAColor.h"

/*
 * RGBAColor - Constructor for RGBAColor.
 */
RGBAColor::RGBAColor() {
	values = new float[4];
	for (int i=0; i<4; ++i)
		values[i]=float(0.0);
} // end RGBAColor()

/*
 * RGBAColor - Constructor for RGBAColor.
 *
 * parameter _values[4] - float[4]
 */
RGBAColor::RGBAColor(float _values[4]) {
	values = new float[4];
	for (int i=0; i<4; ++i)
		values[i]=_values[i];
} // end RGBAColor()

/*
 * RGBAColor - Constructor for RGBAColor.
 *
 * parameter r - float
 * parameter g - float
 * parameter b - float
 * parameter a - float
 */
RGBAColor::RGBAColor(float r, float g, float b, float a) {
	values = new float[4];
	values[0]=r;
	values[1]=g;
	values[2]=b;
	values[3]=a;
} // end RGBAColor()

/*
 * ~RGBAColor - Destructor for RGBAColor.
 */
RGBAColor::~RGBAColor() {
	delete[] values;
} // end ~RGBAColor()

/*
 * getColor - Get color.
 *
 * return - float *
 */
float * RGBAColor::getColor(void) const {
    return values;
} // end getColor()

/*
 * getValues - Get color values.
 *
 * return - float*
 */
float* RGBAColor::getValues(void) {
	return values;
} // end getValues()

/*
 * setValues - Set color values.
 *
 * parameter _values[4] - float[4]
 */
void RGBAColor::setValues(float _values[4]) {
	for (int i=0; i<4; ++i)
		values[i]=_values[i];
} // end setValues()

/*
 * getValues - Get color value i.
 *
 * parameter i - int
 * return - float
 */
float RGBAColor::getValues(int i) {
	return values[i];
} // end getValues()

/*
 * setValues - Set color value i.
 *
 * parameter i - int
 * parameter value - float
 */
void RGBAColor::setValues(int i, float value) {
	values[i]=value;
} // end setValues()
