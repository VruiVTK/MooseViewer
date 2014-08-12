#include "ScalarWidget.h"
#include "ScalarWidgetChangedCallbackData.h"


/*
 * ScalarWidgetChangedCallbackData - Constructor for ScalarWidgetChangedCallbackData.
 */
ScalarWidgetChangedCallbackData::ScalarWidgetChangedCallbackData(void) {
} // end ScalarWidgetChangedCallbackData()

/*
 * ScalarWidgetChangedCallbackData - Constructor for ScalarWidgetChangedCallbackData.
 *
 * parameter _scalarWidget - ScalarWidget*
 */
ScalarWidgetChangedCallbackData::ScalarWidgetChangedCallbackData(ScalarWidget* _scalarWidget) :
	ScalarWidgetCallbackData(_scalarWidget) {
} // end ScalarWidgetChangedCallbackData()

/*
 * ~ScalarWidgetChangedCallbackData - Destructor for ScalarWidgetChangedCallbackData.
 */
ScalarWidgetChangedCallbackData::~ScalarWidgetChangedCallbackData(void) {
} // end ~ScalarWidgetChangedCallbackData()
