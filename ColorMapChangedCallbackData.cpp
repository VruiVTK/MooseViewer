#include "ColorMap.h"
#include "ColorMapChangedCallbackData.h"

/*
 * ColorMapChangedCallbackData - Constructor for ColorMapChangedCallbackData.
 */
ColorMapChangedCallbackData::ColorMapChangedCallbackData() {
} // end ColorMapChangedCallbackData()

/*
 * ColorMapChangedCallbackData - Constructor for ColorMapChangedCallbackData.
 *
 * parameter _colorMap - ColorMap*
 */
ColorMapChangedCallbackData::ColorMapChangedCallbackData(ColorMap* colorMap) :
	ColorMapCallbackData(colorMap) {
} // end ColorMapChangedCallbackData()

/*
 * ~ColorMapChangedCallbackData - Destructor for ColorMapChangedCallbackData.
 */
ColorMapChangedCallbackData::~ColorMapChangedCallbackData() {
} // end ~ColorMapChangedCallbackData()
