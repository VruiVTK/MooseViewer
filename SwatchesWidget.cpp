#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>

#include "SwatchesWidget.h"

/*
 * SwatchesWidget - Constructor for the SwatchesWidget class.
 *
 * parameter _name - const char*
 * parameter _parent - GLMotif::Container*
 * parameter volume - Volume*
 * parameter _manageChild - bool
 */
SwatchesWidget::SwatchesWidget(const char* _name, GLMotif::Container* _parent, bool _manageChild) :
	GLMotif::Widget(_name, _parent, false) {
	isSelected = false;
	numberOfColumns = 31;
	numberOfRows = 9;
	defaultSwatchSize[0] = 5;
	defaultSwatchSize[1] = 5;
	preferredSwatchSize[0] = defaultSwatchSize[0];
	preferredSwatchSize[1] = defaultSwatchSize[1];
	swatchSize[0] = defaultSwatchSize[0];
	swatchSize[1] = defaultSwatchSize[1];
	gap[0] = 1;
	gap[1] = 1;
	currentColor = new float[3];
	marginWidth=0.0f;
	preferredSize[0]=0.0f;
	preferredSize[1]=0.0f;
	preferredSize[2]=0.0f;
	if (_manageChild)
		manageChild();
} // end SwatchesWidget()

/*
 * ~SwatchesWidget - Destructor for the SwatchesWidget class.
 */
SwatchesWidget::~SwatchesWidget(void) {
} // end ~SwatchesWidget()

/*
 * calcNaturalSize - Determine the natural size of the swatches. A virtual function of GLMotif::Widget base.
 *
 * return - GLMotif::Vector
 */
GLMotif::Vector SwatchesWidget::calcNaturalSize(void) const {
	GLMotif::Vector result=preferredSize;
	result[0]+=2.0f*marginWidth;
	result[1]+=2.0f*marginWidth;
	return calcExteriorSize(result);
} // end calcNaturalSize()

/*
 * draw - Draw the selectable swatches.
 *
 * parameter glContextData - GLContextData&
 */
void SwatchesWidget::draw(GLContextData& glContextData) const {
	Widget::draw(glContextData);
	GLfloat x1=swatchesAreaBox.getCorner(0)[0];
	GLfloat x2=swatchesAreaBox.getCorner(1)[0];
	GLfloat y1=swatchesAreaBox.getCorner(0)[1];
	GLfloat y2=swatchesAreaBox.getCorner(2)[1];
	GLfloat z=swatchesAreaBox.getCorner(0)[2];
	GLfloat width = x2-x1;
	GLfloat height = y2-y1;
	drawMargin();
	GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
	if (lightingEnabled)
		glDisable(GL_LIGHTING);
	//drawSwatchesWidgetArea();
	drawSwatchesWidget(x1, y1, z, width, height);
	if (lightingEnabled)
		glEnable(GL_LIGHTING);
} // end draw()

/*
 * drawSwatchesWidget - Draw histogram.
 *
 * parameter x - GLfloat
 * parameter y - GLfloat
 * parameter z - GLfloat
 * parameter width - GLfloat
 * parameter height - GLfloat
 */
void SwatchesWidget::drawSwatchesWidget(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height) const {
	GLfloat _x = 0.0f;
	GLfloat _y = 0.0f;
	for (int row=0; row<numberOfRows; row++) {
		for (int column=0; column<numberOfColumns; column++) {
			glColor3ubv(getColorForCell(column, row));
			glBegin(GL_QUADS);
			_x = GLfloat((float(column))/(float(numberOfColumns))*(width)+x);
			_y = GLfloat((float(row))/(float(numberOfRows))*(height)+y);
			glVertex3f(_x, _y, z);
			_x = GLfloat((float(column+1))/(float(numberOfColumns))*(width)+x);
			glVertex3f(_x, _y, z);
			_y = GLfloat((float(row+1))/(float(numberOfRows))*(height)+y);
			glVertex3f(_x, _y, z);
			_x = GLfloat((float(column))/(float(numberOfColumns))*(width)+x);
			glVertex3f(_x, _y, z);
			glEnd();
		}
	}
} // end drawSwatchesWidget()

/*
 * drawSwatchesWidgetArea - Draw histogram area in a background color.
 */
void SwatchesWidget::drawSwatchesWidgetArea(void) const {
	glColor3f(0.7f, 0.7f, 0.7f);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex(swatchesAreaBox.getCorner(0));
	glVertex(swatchesAreaBox.getCorner(1));
	glVertex(swatchesAreaBox.getCorner(3));
	glVertex(swatchesAreaBox.getCorner(2));
	glEnd();
} // end drawSwatchesWidgetArea()

/*
 * drawMargin - Draw margin area in background color.
 */
void SwatchesWidget::drawMargin(void) const {
	glColor(backgroundColor);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex(getInterior().getCorner(0));
	glVertex(swatchesAreaBox.getCorner(0));
	glVertex(swatchesAreaBox.getCorner(2));
	glVertex(getInterior().getCorner(2));
	glVertex(getInterior().getCorner(1));
	glVertex(getInterior().getCorner(3));
	glVertex(swatchesAreaBox.getCorner(3));
	glVertex(swatchesAreaBox.getCorner(1));
	glVertex(getInterior().getCorner(0));
	glVertex(getInterior().getCorner(1));
	glVertex(swatchesAreaBox.getCorner(1));
	glVertex(swatchesAreaBox.getCorner(0));
	glVertex(getInterior().getCorner(2));
	glVertex(swatchesAreaBox.getCorner(2));
	glVertex(swatchesAreaBox.getCorner(3));
	glVertex(getInterior().getCorner(3));
	glEnd();
} // end drawMargin()

/*
 * findRecipient - Determine which is the applicable widget of the event. A virtual function of GLMotif::Widget base.
 *
 * parameter event - GLMotif::Event&
 * return - bool
 */
bool SwatchesWidget::findRecipient(GLMotif::Event& event) {
	if (isSelected) {
		return event.setTargetWidget(this, event.calcWidgetPoint(this));
	} else
		return GLMotif::Widget::findRecipient(event);
} // end findRecipient()

/*
 * setBounds
 *
 * parameter width - GLfloat
 * parameter height - GLfloat
 */
void SwatchesWidget::setBounds(GLfloat width, GLfloat height) {
	setPreferredSwatchSize();
	if (width > preferredSwatchSize[0]) {
		swatchSize[0] = (width - (numberOfColumns * gap[0])) / numberOfColumns;
	} else {
		swatchSize[0] = defaultSwatchSize[0];
	}
	if (height > preferredSwatchSize[1]) {
		swatchSize[1] = (height - (numberOfRows * gap[1])) / numberOfRows;
	} else {
		swatchSize[1] = defaultSwatchSize[1];
	}
} // end setBounds()

/*
 * getColorChangedCallbacks
 *
 * return - Misc::CallbackList&
 */
Misc::CallbackList& SwatchesWidget::getColorChangedCallbacks(void) {
	return colorChangedCallbacks;
} // end getColorChangedCallbacks()

/**
 * getColorForCell
 *
 * parameter column - int
 * parameter row - int
 * return - GLubyte *
 */
GLubyte* SwatchesWidget::getColorForCell(int column, int row) const {
	if (column >= 0 && column < numberOfColumns && row >= 0 && row < numberOfRows) {
		return swatchColors[(row * numberOfColumns) + column];
	} else
		return swatchColors[0];
} // end getColorForCell()

/*
 * getColorForLocation
 *
 * parameter x - double
 * parameter y - double
 * return - GLubyte*
 */
GLubyte* SwatchesWidget::getColorForLocation(double x, double y) {
	GLfloat x1=swatchesAreaBox.getCorner(0)[0];
	GLfloat x2=swatchesAreaBox.getCorner(1)[0];
	GLfloat y1=swatchesAreaBox.getCorner(0)[1];
	GLfloat y2=swatchesAreaBox.getCorner(2)[1];
	GLfloat width = x2-x1;
	GLfloat height = y2-y1;
	int column = int(((x-x1) / width)*double(numberOfColumns));
	int row = int(((y-y1) / height)*double(numberOfRows));
	return getColorForCell(column, row);
} // end getColorForLocation()

/*
 * getCurrentColor
 *
 * return - float*
 */
float* SwatchesWidget::getCurrentColor(void) {
	return currentColor;
} // end getCurrentColor()

/*
 * setMarginWidth - Set the margin width.
 *
 * parameter _margineWidth - GLfloat
 */
void SwatchesWidget::setMarginWidth(GLfloat _marginWidth) {
	marginWidth=_marginWidth;
	if (isManaged) {
		parent->requestResize(this, calcNaturalSize());
	} else
		resize(GLMotif::Box(GLMotif::Vector(0.0f, 0.0f, 0.0f), calcNaturalSize()));
} // end setMarginWidth()

/*
 * setPreferredSize - Set color map preferred size.
 *
 * parameter _preferredSize - const GLMotif::Vector&
 */
void SwatchesWidget::setPreferredSize(const GLMotif::Vector& _preferredSize) {
	preferredSize=_preferredSize;
	if (isManaged) {
		parent->requestResize(this, calcNaturalSize());
	} else
		resize(GLMotif::Box(GLMotif::Vector(0.0f, 0.0f, 0.0f), calcNaturalSize()));
	GLfloat x1=swatchesAreaBox.getCorner(0)[0];
	GLfloat x2=swatchesAreaBox.getCorner(1)[0];
	GLfloat y1=swatchesAreaBox.getCorner(0)[1];
	GLfloat y2=swatchesAreaBox.getCorner(2)[1];
	GLfloat z=swatchesAreaBox.getCorner(0)[2];
	GLfloat width = x2-x1;
	GLfloat height = y2-y1;
	setBounds(width, height);
	setSwatchesSize();
} // end setPreferredSize()

/*
 * setPreferredSwatchSize
 */
void SwatchesWidget::setPreferredSwatchSize(void) {
	preferredSwatchSize[0] = numberOfColumns * (defaultSwatchSize[0] + gap[0]);
	preferredSwatchSize[1] = numberOfRows * (defaultSwatchSize[1] + gap[1]);
} // end setPreferredSwatchSize()

/*
 * setSwatchesSize
 */
void SwatchesWidget::setSwatchesSize(void) {
	preferredSwatchSize[0] = numberOfColumns * (swatchSize[0] + gap[0]);
	preferredSwatchSize[1] = numberOfRows * (swatchSize[1] + gap[1]);
} // end setSwatchesSize()

/*
 * pointerButtonDown - Pointer button down event handler. A virtual function of GLMotif::Widget base.
 *
 * parameter event - GLMotif::Event&
 */
void SwatchesWidget::pointerButtonDown(GLMotif::Event& event) {
	isSelected = true;
	double _x=event.getWidgetPoint().getPoint()[0];
	double _y=event.getWidgetPoint().getPoint()[1];
	GLfloat x1=swatchesAreaBox.getCorner(0)[0];
	GLfloat y1=swatchesAreaBox.getCorner(0)[1];
	GLubyte* _color = getColorForLocation(_x, _y);
	currentColor[0] = float(_color[0])/255.0f;
	currentColor[1] = float(_color[1])/255.0f;
	currentColor[2] = float(_color[2])/255.0f;
	Misc::CallbackData callbackData;
	colorChangedCallbacks.call(&callbackData);
} // end pointerButtonDown()

/*
 * pointerButtonUp - Pointer button up event handler. A virtual function of GLMotif::Widget base.
 *
 * parameter event - GLMotif::Event&
 */
void SwatchesWidget::pointerButtonUp(GLMotif::Event& event) {
	isSelected = false;
} // end pointerButtonUp()

/*
 * pointerMotion - Pointer motion event handler. A virtual function of GLMotif::Widget base.
 *
 * parameter event - GLMotif::Event&
 */
void SwatchesWidget::pointerMotion(GLMotif::Event& event) {
} // pointerMotion()

/*
 * resize - Resize the swatches display. A virtual function of GLMotif::Widget base.
 *
 * parameter _exterior - const GLMotif::Box&
 */
void SwatchesWidget::resize(const GLMotif::Box& _exterior) {
	GLMotif::Widget::resize(_exterior);
	swatchesAreaBox=getInterior();
	swatchesAreaBox.doInset(GLMotif::Vector(marginWidth, marginWidth, 0.0f));
} // end resize()
