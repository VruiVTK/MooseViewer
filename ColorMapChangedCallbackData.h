#ifndef COLORMAPCHANGEDCALLBACKDATA_H_
#define COLORMAPCHANGEDCALLBACKDATA_H_

#include "ColorMapCallbackData.h"

// begin Forward Declarations
class ColorMap;
// end Forward Declarations

class ColorMapChangedCallbackData : public ColorMapCallbackData {
public:
	ColorMapChangedCallbackData();
	ColorMapChangedCallbackData(ColorMap* colorMap);
	~ColorMapChangedCallbackData();
};

#endif /*COLORMAPCHANGEDCALLBACKDATA_H_*/
