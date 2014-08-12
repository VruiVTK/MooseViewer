#ifndef CONTROLPOINTCHANGEDCALLBACKDATA_H_
#define CONTROLPOINTCHANGEDCALLBACKDATA_H_

#include "ColorMapCallbackData.h"

// begin Forward Declarations
class ColorMap;
class ControlPoint;
// end Forward Declarations

class ControlPointChangedCallbackData : public ColorMapCallbackData {
public:
	ControlPointChangedCallbackData();
	ControlPointChangedCallbackData(ColorMap* _colorMap, ControlPoint* _previousControlPoint, ControlPoint* _currentControlPoint);
	~ControlPointChangedCallbackData();
	ControlPoint* getCurrentControlPoint(void);
private:
	ControlPoint* previousControlPoint;
	ControlPoint* currentControlPoint;
};

#endif /*CONTROLPOINTCHANGEDCALLBACKDATA_H_*/
