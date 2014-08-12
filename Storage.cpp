#include "ControlPoint.h"
#include "RGBAColor.h"
#include "Storage.h"

/*
 * Storage - Constructor for Storage.
 */
Storage::Storage(void) {
	numberOfControlPoints=0;
	values=0;
	rgbaColors=0;
} // end Storage()

/*
 * Storage - Constructor for Storage.
 *
 * parameter first - ControlPoint*
 */
Storage::Storage(ControlPoint* first) {
	numberOfControlPoints=0;
	values=0;
	rgbaColors=0;
	for(ControlPoint* controlPointPointer=first; controlPointPointer!=0; controlPointPointer=controlPointPointer->right)
		++numberOfControlPoints;
	values=new double[numberOfControlPoints];
	rgbaColors=new RGBAColor[numberOfControlPoints];
	int i=0;
	for (const ControlPoint* controlPointPointer=first; controlPointPointer!=0; controlPointPointer=controlPointPointer->right, ++i) {
		values[i]=controlPointPointer->value;
		for(int j=0;j<4;++j) rgbaColors[i].setValues(j,controlPointPointer->rgbaColor->getValues(j));
	}
} // end Storage()

/*
 * ~Storage - Destructor for Storage.
 */
Storage::~Storage(void) {
	delete[] values;
	delete[] rgbaColors;
} // end ~Storage()

/*
 * getNumberOfControlPoints
 *
 * return - int
 */
int Storage::getNumberOfControlPoints() {
	return numberOfControlPoints;
} // end getNumberOfControlPoints()

/*
 * getRGBAColors
 *
 * return - RGBAColor*
 */
RGBAColor* Storage::getRGBAColors() {
	return rgbaColors;
} // end getRGBAColors()

/*
 * getRGBAColors
 *
 * parameter i - int
 * return - RGBAColor*
 */
RGBAColor* Storage::getRGBAColors(int i) {
	return &rgbaColors[i];
} // end getRGBAColors()

/*
 * getValues
 *
 * return - double*
 */
double* Storage::getValues() {
	return values;
} // end getValues()

/*
 * getValues
 *
 * parameter i - int
 * return - double
 */
double Storage::getValues(int i) {
	return values[i];
} // end getValues()
