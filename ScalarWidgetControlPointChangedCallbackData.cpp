#include "ScalarWidget.h"
#include "ScalarWidgetControlPoint.h"
#include "ScalarWidgetControlPointChangedCallbackData.h"

/*
 * ScalarWidgetControlPointChangedCallbackData - Constructor for ScalarWidgetControlPointChangedCallbackData.
 */
ScalarWidgetControlPointChangedCallbackData::ScalarWidgetControlPointChangedCallbackData(void) {
} // end ScalarWidgetControlPointChangedCallbackData()

/*
 * ScalarWidgetControlPointChangedCallbackData - Constructor for ScalarWidgetControlPointChangedCallbackData.
 *
 * parameter _scalarWidget - ScalarWidget*
 * parameter _previousControlPoint - ScalarWidgetControlPoint*
 * parameter _currentControlPoint - ScalarWidgetControlPoint*
 */
ScalarWidgetControlPointChangedCallbackData::ScalarWidgetControlPointChangedCallbackData(ScalarWidget* _scalarWidget,
		ScalarWidgetControlPoint* _previousControlPoint, ScalarWidgetControlPoint* _currentControlPoint) :
	ScalarWidgetCallbackData(_scalarWidget) {
	previousControlPoint=_previousControlPoint;
	currentControlPoint=_currentControlPoint;
} // end ScalarWidgetControlPointChangedCallbackData()

/*
 * ~ScalarWidgetControlPointChangedCallbackData - Destructor for ScalarWidgetControlPointChangedCallbackData.
 */
ScalarWidgetControlPointChangedCallbackData::~ScalarWidgetControlPointChangedCallbackData(void) {
	previousControlPoint=0;
	currentControlPoint=0;
} // end ~ScalarWidgetControlPointChangedCallbackData()

/*
 * getCurrentControlPoint
 *
 * return - ScalarWidgetControlPoint*
 */
ScalarWidgetControlPoint* ScalarWidgetControlPointChangedCallbackData::getCurrentControlPoint(void) {
	return currentControlPoint;
} // end getCurrentControlPoint()
