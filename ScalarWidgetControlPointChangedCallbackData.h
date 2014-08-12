#ifndef SCALARWIDGETCONTROLPOINTCHANGEDCALLBACKDATA_H_
#define SCALARWIDGETCONTROLPOINTCHANGEDCALLBACKDATA_H_

#include "ScalarWidgetCallbackData.h"

// begin Forward Declarations
class ScalarWidget;
class ScalarWidgetControlPoint;
// end Forward Declarations

class ScalarWidgetControlPointChangedCallbackData : public ScalarWidgetCallbackData {
public:
	ScalarWidgetControlPointChangedCallbackData(void);
	ScalarWidgetControlPointChangedCallbackData(ScalarWidget * _scalarWidget, ScalarWidgetControlPoint * _previousControlPoint, ScalarWidgetControlPoint * _currentControlPoint);
	~ScalarWidgetControlPointChangedCallbackData(void);
	ScalarWidgetControlPoint* getCurrentControlPoint(void);
private:
    ScalarWidgetControlPoint* previousControlPoint;
    ScalarWidgetControlPoint* currentControlPoint;
};

#endif /*SCALARWIDGETCONTROLPOINTCHANGEDCALLBACKDATA_H_*/
