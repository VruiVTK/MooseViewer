#include "ScalarWidget.h"
#include "ScalarWidgetCallbackData.h"

/*
 * ScalarWidgetCallbackData - Constructor for ScalarWidgetCallbackData.
 */
ScalarWidgetCallbackData::ScalarWidgetCallbackData(void) {
} // end ScalarWidgetCallbackData()

/*
 * ScalarWidgetCallbackData - Constructor for ScalarWidgetCallbackData.
 *
 * parameter _scalarWidget - ScalarWidget*
 */
ScalarWidgetCallbackData::ScalarWidgetCallbackData(ScalarWidget* _scalarWidget) {
	scalarWidget=_scalarWidget;
} //end ScalarWidgetCallbackData()

/*
 * ~ScalarWidgetCallbackData - Destructor for ScalarWidgetCallbackData.
 */
ScalarWidgetCallbackData::~ScalarWidgetCallbackData(void) {
	scalarWidget=0;
} // end ~ScalarWidgetCallbackData()
