#ifndef SCALARWIDGETCHANGEDCALLBACKDATA_H_
#define SCALARWIDGETCHANGEDCALLBACKDATA_H_

#include "ScalarWidgetCallbackData.h"

// begin Forward Declarations
class ScalarWidget;
// end Forward Declarations

class ScalarWidgetChangedCallbackData : public ScalarWidgetCallbackData {
public:
	ScalarWidgetChangedCallbackData(void);
	ScalarWidgetChangedCallbackData(ScalarWidget* _scalarWidget);
	~ScalarWidgetChangedCallbackData(void);
};

#endif /*SCALARWIDGETCHANGEDCALLBACKDATA_H_*/
