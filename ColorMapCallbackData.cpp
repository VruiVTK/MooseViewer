#include "ColorMapCallbackData.h"
#include "ColorMap.h"

/*
 * ColorMapCallbackData - Constructor for ColorMapCallbackData.
 */
ColorMapCallbackData::ColorMapCallbackData() {
} // end ColorMapCallbackData()

/*
 * ColorMapCallbackData - Constructor for ColorMapCallbackData.
 *
 * parameter _colorMap - ColorMap
 */
ColorMapCallbackData::ColorMapCallbackData(ColorMap* _colorMap) {
	colorMap = _colorMap;
} //end ColorMapCallbackData()

/*
 * ~ColorMapCallbackData - Destructor for ColorMapCallbackData.
 */
ColorMapCallbackData::~ColorMapCallbackData() {
	colorMap=0;
} // end ~ColorMapCallbackData()
