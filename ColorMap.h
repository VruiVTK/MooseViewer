#ifndef COLORMAP_INCLUDED
#define COLORMAP_INCLUDED

#include <GL/gl.h>

/* Vrui includes */
#include <GL/GLColorMap.h>
#include <GLMotif/Container.h>
#include <GLMotif/Event.h>
#include <GLMotif/Types.h>
#include <GLMotif/Widget.h>
#include <Misc/CallbackList.h>

/* FULL_RAINBOW */

static float FULL_RAINBOW[7][4] = {
	{0.00f, 0.93f, 0.51f, 0.93f},
	{0.16f, 0.40f, 0.00f, 0.40f},
	{0.33f, 0.00f, 0.00f, 1.00f},
	{0.50f, 0.00f, 1.00f, 0.00f},
	{0.66f, 1.00f, 1.00f, 0.00f},
	{0.83f, 1.00f, 0.50f, 0.00f},
	{1.00f, 1.00f, 0.00f, 0.00f}
};

static float INVERSE_FULL_RAINBOW[7][4] = {
	{0.00f, 1.00f, 0.00f, 0.00f},
	{0.16f, 1.00f, 0.50f, 0.00f},
	{0.33f, 1.00f, 1.00f, 0.00f},
	{0.50f, 0.00f, 1.00f, 0.00f},
	{0.66f, 0.00f, 0.00f, 1.00f},
	{0.83f, 0.40f, 0.00f, 0.40f},
	{1.00f, 0.93f, 0.51f, 0.93f}
};

/* RAINBOW */
static float RAINBOW[5][4] = {
	{0.00f, 0.00f, 0.00f, 1.00f},
	{0.25f, 0.00f, 1.00f, 1.00f},
	{0.50f, 0.00f, 1.00f, 0.00f},
	{0.75f, 1.00f, 1.00f, 0.00f},
	{1.00f, 1.00f, 0.00f, 0.00f}
};

static float INVERSE_RAINBOW[5][4] = {
	{0.00f, 1.00f, 0.00f, 0.00f},
	{0.25f, 1.00f, 1.00f, 0.00f},
	{0.50f, 0.00f, 1.00f, 0.00f},
	{0.75f, 0.00f, 1.00f, 1.00f},
	{1.00f, 0.00f, 0.00f, 1.00f}
};

/* COLD_TO_HOT */
static float COLD_TO_HOT[3][4] = {
	{0.00f, 0.00f, 0.00f, 1.00f},
	{0.50f, 0.75f, 0.00f, 0.75f},
	{1.00f, 1.00f, 0.00f, 0.00f}
};

static float HOT_TO_COLD[3][4] = {
	{0.00f, 1.00f, 0.00f, 0.00f},
	{0.50f, 0.75f, 0.00f, 0.75f},
	{1.00f, 0.00f, 0.00f, 1.00f}
};

/* BLACK_TO_WHITE */
static float BLACK_TO_WHITE[2][4] = {
	{0.00f, 0.00f, 0.00f, 0.00f},
	{1.00f, 1.00f, 1.00f, 1.00f}
};

static float WHITE_TO_BLACK[2][4] = {
	{0.00f, 1.00f, 1.00f, 1.00f},
	{1.00f, 0.00f, 0.00f, 0.00f}
};

/* HSB_HUES */
static float HSB_HUES[7][4] = {
	{0.00f, 1.00f, 0.00f, 0.00f},
	{0.16f, 1.00f, 1.00f, 0.00f},
	{0.33f, 0.00f, 1.00f, 0.00f},
	{0.50f, 0.00f, 1.00f, 1.00f},
	{0.66f, 0.00f, 0.00f, 1.00f},
	{0.83f, 1.00f, 0.00f, 1.00f},
	{1.00f, 1.00f, 0.00f, 0.00f}
};

static float INVERSE_HSB_HUES[7][4] = {
	{0.00f, 1.00f, 0.00f, 0.00f},
	{0.16f, 1.00f, 0.00f, 1.00f},
	{0.33f, 0.00f, 0.00f, 1.00f},
	{0.50f, 0.00f, 1.00f, 1.00f},
	{0.66f, 0.00f, 1.00f, 0.00f},
	{0.83f, 1.00f, 1.00f, 0.00f},
	{1.00f, 1.00f, 0.00f, 0.00f}
};

/* DAVINCI */
static float DAVINCI[11][4] = {
	{0.00f, 0.00f, 0.00f, 0.00f},
	{0.10f, 0.18f, 0.08f, 0.00f},
	{0.20f, 0.27f, 0.18f, 0.08f},
	{0.30f, 0.37f, 0.27f, 0.18f},
	{0.40f, 0.47f, 0.37f, 0.27f},
	{0.50f, 0.57f, 0.47f, 0.37f},
	{0.60f, 0.67f, 0.57f, 0.47f},
	{0.70f, 0.76f, 0.67f, 0.57f},
	{0.80f, 0.86f, 0.76f, 0.67f},
	{0.90f, 1.00f, 0.86f, 0.76f},
	{1.00f, 1.00f, 1.00f, 1.00f}
};

static float INVERSE_DAVINCI[11][4] = {
	{0.00f, 1.00f, 1.00f, 1.00f},
	{0.10f, 1.00f, 0.86f, 0.76f},
	{0.20f, 0.86f, 0.76f, 0.67f},
	{0.30f, 0.76f, 0.67f, 0.57f},
	{0.40f, 0.67f, 0.57f, 0.47f},
	{0.50f, 0.57f, 0.47f, 0.37f},
	{0.60f, 0.47f, 0.37f, 0.27f},
	{0.70f, 0.37f, 0.27f, 0.18f},
	{0.80f, 0.27f, 0.18f, 0.08f},
	{0.90f, 0.18f, 0.08f, 0.00f},
	{1.00f, 0.00f, 0.00f, 0.00f}
};

/* SEISMIC */
static float SEISMIC[3][4] = {
	{0.00f, 1.00f, 0.00f, 0.00f},
	{0.50f, 1.00f, 1.00f, 1.00f},
	{1.00f, 0.00f, 0.00f, 1.00f}
};

static float INVERSE_SEISMIC[3][4] = {
	{0.00f, 0.00f, 0.00f, 1.00f},
	{0.50f, 1.00f, 1.00f, 1.00f},
	{1.00f, 1.00f, 0.00f, 0.00f}
};

#define CFULL_RAINBOW 0
#define CINVERSE_FULL_RAINBOW 1
#define CRAINBOW 2
#define CINVERSE_RAINBOW 3
#define CCOLD_TO_HOT 4
#define CHOT_TO_COLD 5
#define CBLACK_TO_WHITE 6
#define CWHITE_TO_BLACK 7
#define CHSB_HUES 8
#define CINVERSE_HSB_HUES 9
#define CDAVINCI 10
#define CINVERSE_DAVINCI 11
#define CSEISMIC 12
#define CINVERSE_SEISMIC 13

// begin Forward Declarations
class ControlPoint;
class RGBAColor;
class Storage;
// end Forward Declarations

class ColorMap : public GLMotif::Widget {
public:
	typedef float Scalar;
	ColorMap(const char* _name, GLMotif::Container* _parent, bool _manageChild=true);
	virtual ~ColorMap(void);
	virtual GLMotif::Vector calcNaturalSize(void) const;
	void createColorMap(int colormap);
	void createColorMap(int colorMapCreationType, double _minimum, double _maximum);
	void deleteControlPoint(void);
	ControlPoint* determineControlPoint(GLMotif::Event& event);
	virtual void draw(GLContextData& contextData) const;
	void drawColorMap(void) const;
	void drawControlPoints(void) const;
	void drawMargin(void) const;
	void exportColorMap(double* colormap) const;
	virtual bool findRecipient(GLMotif::Event& event);
	Storage* getColorMap(void) const;
	void setColorMap(Storage* _colorMap);
	Misc::CallbackList& getColorMapChangedCallbacks(void);
	Misc::CallbackList& getControlPointChangedCallbacks(void);
	RGBAColor* getControlPointColor(void);
	void setControlPointColor(RGBAColor _rgbaColor);
	void setControlPointColor(RGBAColor* _rgbaColor);
	void setControlPointSize(GLfloat _controlPointSize);
	void setControlPointValue(double _value);
	void setMarginWidth(GLfloat _marginWidth);
	int getNumberOfControlPoints(void) const;
	void setPreferredSize(const GLMotif::Vector& _preferredSize);
	const std::pair<double,double>& getValueRange(void) const;
	void insertControlPoint(double _value);
	virtual void pointerButtonDown(GLMotif::Event& event);
	virtual void pointerButtonUp(GLMotif::Event& event);
	virtual void pointerMotion(GLMotif::Event& event);
	virtual void resize(const GLMotif::Box& _exterior);
	void selectControlPoint(int i);
private:
	GLMotif::Box colorMapAreaBox;
	Misc::CallbackList colorMapChangedCallbacks;
	ControlPoint* controlPoint;
	Misc::CallbackList controlPointChangedCallbacks;
	RGBAColor* controlPointColor;
	GLfloat controlPointSize;
	GLMotif::Point::Vector dragOffset;
	ControlPoint* first;
	bool isDragging;
	ControlPoint* last;
	GLfloat marginWidth;
	GLMotif::Vector preferredSize;
	std::pair<double,double> valueRange;
	void deleteColorMap(void);
	void updateControlPoints(void);
};

#endif
