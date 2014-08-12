#ifndef SCALARWIDGETCALLBACKDATA_H_
#define SCALARWIDGETCALLBACKDATA_H_

/* Vrui includes */
#include <Misc/CallbackData.h>

// begin Forward Declarations
class ScalarWidget;
// end Forward Declarations

class ScalarWidgetCallbackData : public Misc::CallbackData {
public:
	ScalarWidgetCallbackData(void);
	ScalarWidgetCallbackData(ScalarWidget* _scalarWidget);
	~ScalarWidgetCallbackData(void);
	ScalarWidget* scalarWidget;
};

#endif /*SCALARWIDGETCALLBACKDATA_H_*/
