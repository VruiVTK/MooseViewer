#include "ColorMap.h"
#include "ControlPoint.h"
#include "ControlPointChangedCallbackData.h"

/*
 * ControlPointChangedCallbackData - Constructor for ControlPointChangedCallbackData.
 */
ControlPointChangedCallbackData::ControlPointChangedCallbackData() {
} // end ControlPointChangedCallbackData()

/*
 * ControlPointChangedCallbackData - Constructor for ControlPointChangedCallbackData.
 */
ControlPointChangedCallbackData::ControlPointChangedCallbackData(ColorMap* _colorMap, ControlPoint* _previousControlPoint,
		ControlPoint* _currentControlPoint) :
	ColorMapCallbackData(_colorMap) {
	previousControlPoint=_previousControlPoint;
	currentControlPoint=_currentControlPoint;
} // end ControlPointChangedCallbackData()

/*
 * ~ControlPointChangedCallbackData - Destructor for ControlPointChangedCallbackData.
 */
ControlPointChangedCallbackData::~ControlPointChangedCallbackData() {
	previousControlPoint=0;
	currentControlPoint=0;
} // end ~ControlPointChangedCallbackData()

/*
 * getCurrentControlPoint
 *
 * return - ControlPoint*
 */
ControlPoint* ControlPointChangedCallbackData::getCurrentControlPoint(void) {
	return currentControlPoint;
} // end getCurrentControlPoint()
