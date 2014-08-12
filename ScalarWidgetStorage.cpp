#include "ScalarWidgetControlPoint.h"
#include "ScalarWidgetStorage.h"

/*
 * ScalarWidgetStorage - Constructor for ScalarWidgetStorage.
 */
ScalarWidgetStorage::ScalarWidgetStorage(void) {
	numberOfControlPoints=0;
	values=0;
	scalars=0;
} // end ScalarWidgetStorage()

/*
 * ScalarWidgetStorage - Constructor for ScalarWidgetStorage.
 *
 * parameter first - ScalarWidgetControlPoint*
 */
ScalarWidgetStorage::ScalarWidgetStorage(ScalarWidgetControlPoint* first) {
	numberOfControlPoints=0;
	values=0;
	scalars=0;
	for (ScalarWidgetControlPoint* controlPointPtr=first; controlPointPtr!=0; controlPointPtr=controlPointPtr->right)
		++numberOfControlPoints;
	values=new double[numberOfControlPoints];
	scalars=new float[numberOfControlPoints];
	int i=0;
	for (ScalarWidgetControlPoint* controlPointPtr=first; controlPointPtr!=0; controlPointPtr
			=controlPointPtr->right, ++i) {
		values[i]=controlPointPtr->getValue();
		scalars[i]=controlPointPtr->getScalar();
	}
} // end ScalarWidgetStorage()

/*
 * ~ScalarWidgetStorage - Destructor for ScalarWidgetStorage.
 */
ScalarWidgetStorage::~ScalarWidgetStorage(void) {
	delete[] values;
	delete[] scalars;
} // end ~ScalarWidgetStorage()

/*
 * getNumberOfControlPoints
 *
 * return - int
 */
int ScalarWidgetStorage::getNumberOfControlPoints() {
	return numberOfControlPoints;
} // end getNumberOfControlPoints()

/*
 * getScalars
 *
 * return - float*
 */
float* ScalarWidgetStorage::getScalars() {
	return scalars;
} // end getScalars()

/*
 * getScalar
 *
 * parameter i - int
 * return - float
 */
float ScalarWidgetStorage::getScalar(int i) {
	return scalars[i];
} // end getScalar()

/*
 * getValues
 *
 * return - double*
 */
double* ScalarWidgetStorage::getValues() {
	return values;
} // end getValues()

/*
 * getValue
 *
 * parameter i - int
 * return - double
 */
double ScalarWidgetStorage::getValue(int i) {
	return values[i];
} // end getValue()
