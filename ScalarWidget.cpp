#include <cfloat>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

/* Vrui includes */
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <Math/Math.h>
#include <Misc/File.h>

#include "ScalarWidgetChangedCallbackData.h"
#include "ScalarWidgetControlPoint.h"
#include "ScalarWidgetControlPointChangedCallbackData.h"
#include "ScalarWidget.h"
#include "ScalarWidgetStorage.h"

/*
 * ScalarWidget - Constructor for the ScalarWidget class.
 *
 * parameter _name - const char*
 * parameter _parent - GLMotif::Container*
 * parameter _component - int
 * parameter _manageChild - bool
 */
ScalarWidget::ScalarWidget(const char* _name, GLMotif::Container* _parent, int _component, bool _manageChild) :
    GLMotif::Widget(_name, _parent, false), currentGaussian(-1), gaussian(false), dragging(false), numberOfGaussians(0),
            unselected(false) {
    is1D = false;
    numberOfRedGaussians = 0;
    numberOfGreenGaussians = 0;
    numberOfBlueGaussians = 0;
    numberOfAlphaGaussians = 0;
    redGaussian = false;
    greenGaussian = false;
    blueGaussian = false;
    alphaGaussian = false;
    component = _component;
    numberOfEntries = 256;
    redHistogram = new float[numberOfEntries];
    greenHistogram = new float[numberOfEntries];
    blueHistogram = new float[numberOfEntries];
    alphaHistogram = new float[numberOfEntries];
    for (int i = 0; i < numberOfEntries; i++) {
        redHistogram[i] = 0.0f;
        greenHistogram[i] = 0.0f;
        blueHistogram[i] = 0.0f;
        alphaHistogram[i] = 0.0f;
    }
    calculateHistogram();
    marginWidth = 0.0f;
    redOpacities = new float[256];
    greenOpacities = new float[256];
    blueOpacities = new float[256];
    alphaOpacities = new float[256];
    for (int i = 0; i < 256; i++) {
        redOpacities[i] = float(0);
        greenOpacities[i] = float(0);
        blueOpacities[i] = float(0);
        alphaOpacities[i] = float(0);
    }
    preferredSize[0] = 0.0f;
    preferredSize[1] = 0.0f;
    preferredSize[2] = 0.0f;
    controlPointSize = marginWidth * 0.5f;
    controlPointScalar = 1.0f;
    valueRange = std::pair<double, double>(0.0, 1.0);
    redFirst = new ScalarWidgetControlPoint(0.0, 0.0f);
    redLast = new ScalarWidgetControlPoint(1.0, 1.0f);
    redFirst->right = redLast;
    redLast->left = redFirst;
    greenFirst = new ScalarWidgetControlPoint(0.0, 0.0f);
    greenLast = new ScalarWidgetControlPoint(1.0, 1.0f);
    greenFirst->right = greenLast;
    greenLast->left = greenFirst;
    blueFirst = new ScalarWidgetControlPoint(0.0, 0.0f);
    blueLast = new ScalarWidgetControlPoint(1.0, 1.0f);
    blueFirst->right = blueLast;
    blueLast->left = blueFirst;
    alphaFirst = new ScalarWidgetControlPoint(0.0, 0.0f);
    alphaLast = new ScalarWidgetControlPoint(1.0, 1.0f);
    alphaFirst->right = alphaLast;
    alphaLast->left = alphaFirst;
    updatePointers(component);
    controlPoint = NULL;
    dragging = false;
    for (int i = 0; i < 3; ++i)
         dragOffset[i] = float(0);
    updateControlPoints();
    if (_manageChild)
        manageChild();
} // end ScalarWidget()

/*
 * ~ScalarWidget - Destructor for the ScalarWidget class.
 */
ScalarWidget::~ScalarWidget(void) {
    delete[] histogram;
    delete[] opacities;
    ScalarWidgetControlPoint* controlPointPtr = first->right;
    while (controlPointPtr != last) {
        ScalarWidgetControlPoint* next = controlPointPtr->right;
        delete controlPointPtr;
        controlPointPtr = next;
    }
} // end ~ScalarWidget()

/*
 * addGaussian
 *
 * parameter x - float
 * parameter h - float
 * parameter w - float
 * parameter bx - float
 * parameter by - float
 */
void ScalarWidget::addGaussian(float x, float h, float w, float bx, float by) {
    gaussians[numberOfGaussians++] = Gaussian(x, h, w, bx, by);
} // end addGaussian()

/*
 * calculateHistogram
 */
void ScalarWidget::calculateHistogram(void) {
} // end calculateHistogram()

/*
 * calcNaturalSize - Determine the natural size of the component. A virtual function of GLMotif::Widget base.
 *
 * return - GLMotif::Vector
 */
GLMotif::Vector ScalarWidget::calcNaturalSize(void) const {
    GLMotif::Vector result = preferredSize;
    result[0] += 2.0f * marginWidth;
    result[1] += 2.0f * marginWidth;
    return calcExteriorSize(result);
} // end calcNaturalSize()

/*
 * create - Create scalar ramp.
 *
 * parameter ramp - int
 */
void ScalarWidget::create(int ramp) {
    double _minimum = valueRange.first;
    double _maximum = valueRange.second;
    deleteControlPoints();
    if (ramp == UP_RAMP) {
        first = new ScalarWidgetControlPoint(_minimum, 0.0f);
        last = new ScalarWidgetControlPoint(_maximum, 1.0f);
        ScalarWidgetControlPoint* _controlPoint[2];
        _controlPoint[0] = first;
        _controlPoint[1] = last;
        for (int i = 0; i < 1; ++i) {
            _controlPoint[i + 1]->left = _controlPoint[i];
            _controlPoint[i]->right = _controlPoint[i + 1];
        }
    } else if (ramp == DOWN_RAMP) {
        first = new ScalarWidgetControlPoint(_minimum, 1.0f);
        last = new ScalarWidgetControlPoint(_maximum, 0.0f);
        ScalarWidgetControlPoint* _controlPoint[2];
        _controlPoint[0] = first;
        _controlPoint[1] = last;
        for (int i = 0; i < 1; ++i) {
            _controlPoint[i + 1]->left = _controlPoint[i];
            _controlPoint[i]->right = _controlPoint[i + 1];
        }
    } else if (ramp == CONSTANT_RAMP) {
        first = new ScalarWidgetControlPoint(_minimum, 0.5f);
        last = new ScalarWidgetControlPoint(_maximum, 0.5f);
        ScalarWidgetControlPoint* _controlPoint[2];
        _controlPoint[0] = first;
        _controlPoint[1] = last;
        for (int i = 0; i < 1; ++i) {
            _controlPoint[i + 1]->left = _controlPoint[i];
            _controlPoint[i]->right = _controlPoint[i + 1];
        }
    } else if (ramp == SEISMIC_RAMP) {
        first = new ScalarWidgetControlPoint(_minimum, 1.0f);
        last = new ScalarWidgetControlPoint(_maximum, 1.0f);
        ScalarWidgetControlPoint* _controlPoint[4];
        _controlPoint[0] = first;
        _controlPoint[1] = new ScalarWidgetControlPoint((_maximum - _minimum) / 2.0 - (_maximum - _minimum) * 0.2, 0.0f);
        _controlPoint[2] = new ScalarWidgetControlPoint((_maximum - _minimum) / 2.0 + (_maximum - _minimum) * 0.2, 0.0f);
        _controlPoint[3] = last;
        for (int i = 0; i < 3; ++i) {
            _controlPoint[i + 1]->left = _controlPoint[i];
            _controlPoint[i]->right = _controlPoint[i + 1];
        }
    }
    updateControlPoints();
    ScalarWidgetChangedCallbackData changedCallbackData(this);
    changedCallbacks.call(&changedCallbackData);
} // end create()

/*
 * create - Create scalar ramp.
 *
 * parameter ramp - int
 */
void ScalarWidget::create(int ramp, int component) {
    if (component == 0) {
        first = redFirst;
        last = redLast;
        gaussians = redGaussians;
        histogram = redHistogram;
        opacities = redOpacities;
    } else if (component == 1) {
        first = greenFirst;
        last = greenLast;
        gaussians = greenGaussians;
        histogram = greenHistogram;
        opacities = greenOpacities;
    } else if (component == 2) {
        first = blueFirst;
        last = blueLast;
        gaussians = blueGaussians;
        histogram = blueHistogram;
        opacities = blueOpacities;
    } else if (component == 3) {
        first = alphaFirst;
        last = alphaLast;
        gaussians = alphaGaussians;
        histogram = alphaHistogram;
        opacities = alphaOpacities;
    }
    create(ramp);
    if (this->component == 0) {
        first = redFirst;
        last = redLast;
        gaussians = redGaussians;
        histogram = redHistogram;
        opacities = redOpacities;
    } else if (this->component == 1) {
        first = greenFirst;
        last = greenLast;
        gaussians = greenGaussians;
        histogram = greenHistogram;
        opacities = greenOpacities;
    } else if (this->component == 2) {
        first = blueFirst;
        last = blueLast;
        gaussians = blueGaussians;
        histogram = blueHistogram;
        opacities = blueOpacities;
    } else if (this->component == 3) {
        first = alphaFirst;
        last = alphaLast;
        gaussians = alphaGaussians;
        histogram = alphaHistogram;
        opacities = alphaOpacities;
    }
} // end create;

/*
 * create - Create scalar ramp.
 *
 * parameter rampCreationType - int
 * parameter _minimum - double
 * parameter _maximum - double
 */
void ScalarWidget::create(int rampCreationType, double _minimum, double _maximum) {
    deleteControlPoints();
    if (rampCreationType == UP_RAMP) {
        first = new ScalarWidgetControlPoint(_minimum, 0.0f);
        last = new ScalarWidgetControlPoint(_maximum, 1.0f);
        ScalarWidgetControlPoint* _controlPoint[2];
        _controlPoint[0] = first;
        _controlPoint[1] = last;
        for (int i = 0; i < 1; ++i) {
            _controlPoint[i + 1]->left = _controlPoint[i];
            _controlPoint[i]->right = _controlPoint[i + 1];
        }
    } else if (rampCreationType == DOWN_RAMP) {
        first = new ScalarWidgetControlPoint(_minimum, 1.0f);
        last = new ScalarWidgetControlPoint(_maximum, 0.0f);
        ScalarWidgetControlPoint* _controlPoint[2];
        _controlPoint[0] = first;
        _controlPoint[1] = last;
        for (int i = 0; i < 1; ++i) {
            _controlPoint[i + 1]->left = _controlPoint[i];
            _controlPoint[i]->right = _controlPoint[i + 1];
        }
    } else if (rampCreationType == CONSTANT_RAMP) {
        first = new ScalarWidgetControlPoint(_minimum, 1.0f);
        last = new ScalarWidgetControlPoint(_maximum, 1.0f);
        ScalarWidgetControlPoint* _controlPoint[2];
        _controlPoint[0] = first;
        _controlPoint[1] = last;
        for (int i = 0; i < 1; ++i) {
            _controlPoint[i + 1]->left = _controlPoint[i];
            _controlPoint[i]->right = _controlPoint[i + 1];
        }
    } else if (rampCreationType == SEISMIC_RAMP) {
        first = new ScalarWidgetControlPoint(_minimum, 1.0f);
        last = new ScalarWidgetControlPoint(_maximum, 1.0f);
        ScalarWidgetControlPoint* _controlPoint[4];
        _controlPoint[0] = first;
        _controlPoint[1] = new ScalarWidgetControlPoint((_maximum - _minimum) / 2.0 - (_maximum - _minimum) * 0.2, 0.0f);
        _controlPoint[2] = new ScalarWidgetControlPoint((_maximum - _minimum) / 2.0 + (_maximum - _minimum) * 0.2, 0.0f);
        _controlPoint[3] = last;
        for (int i = 0; i < 3; ++i) {
            _controlPoint[i + 1]->left = _controlPoint[i];
            _controlPoint[i]->right = _controlPoint[i + 1];
        }
    }
    valueRange.first = _minimum;
    valueRange.second = _maximum;
    updateControlPoints();
    ScalarWidgetChangedCallbackData changedCallbackData(this);
    changedCallbacks.call(&changedCallbackData);
} // end create()

/*
 * create - Create scalar ramp.
 *
 * parameter ramp - int
 * parameter _minimum - double
 * parameter _maximum - double
 * parameter component - int
 */
void ScalarWidget::create(int ramp, double _minimum, double _maximum, int component) {
    if (component == 0) {
        first = redFirst;
        last = redLast;
        gaussians = redGaussians;
        histogram = redHistogram;
        opacities = redOpacities;
    } else if (component == 1) {
        first = greenFirst;
        last = greenLast;
        gaussians = greenGaussians;
        histogram = greenHistogram;
        opacities = greenOpacities;
    } else if (component == 2) {
        first = blueFirst;
        last = blueLast;
        gaussians = blueGaussians;
        histogram = blueHistogram;
        opacities = blueOpacities;
    } else if (component == 3) {
        first = alphaFirst;
        last = alphaLast;
        gaussians = alphaGaussians;
        histogram = alphaHistogram;
        opacities = alphaOpacities;
    }
    create(ramp, _minimum, _maximum);
    if (this->component == 0) {
        first = redFirst;
        last = redLast;
        gaussians = redGaussians;
        histogram = redHistogram;
        opacities = redOpacities;
    } else if (this->component == 1) {
        first = greenFirst;
        last = greenLast;
        gaussians = greenGaussians;
        histogram = greenHistogram;
        opacities = greenOpacities;
    } else if (this->component == 2) {
        first = blueFirst;
        last = blueLast;
        gaussians = blueGaussians;
        histogram = blueHistogram;
        opacities = blueOpacities;
    } else if (this->component == 3) {
        first = alphaFirst;
        last = alphaLast;
        gaussians = alphaGaussians;
        histogram = alphaHistogram;
        opacities = alphaOpacities;
    }
} // end create;

/*
 * deleteControlPoint - Delete selected control point.
 */
void ScalarWidget::deleteControlPoint(void) {
    if (!gaussian) {
        if (controlPoint != 0 && controlPoint != first && controlPoint != last) {
            ScalarWidgetControlPoint* _controlPoint = controlPoint;
            ScalarWidgetControlPointChangedCallbackData controlPointChangedCallbackData(this, controlPoint, 0);
            controlPoint = 0;
            controlPointChangedCallbacks.call(&controlPointChangedCallbackData);
            _controlPoint->left->right = _controlPoint->right;
            _controlPoint->right->left = _controlPoint->left;
            delete _controlPoint;
            updateControlPoints();
            ScalarWidgetChangedCallbackData changedCallbackData(this);
            changedCallbacks.call(&changedCallbackData);
        }
    } else {
        removeGaussian(currentGaussian);
        currentGaussian = numberOfGaussians;
        currentMode = modeNone;
        getOpacities();
        ScalarWidgetChangedCallbackData changedCallbackData(this);
        changedCallbacks.call(&changedCallbackData);
    }
} // end deleteControlPoint()

/*
 * deleteControlPoints - Delete scalar control points.
 */
void ScalarWidget::deleteControlPoints(void) {
    if (controlPoint != 0) {
        ScalarWidgetControlPointChangedCallbackData controlPointChangedCallbackData(this, controlPoint, 0);
        controlPoint = 0;
        controlPointChangedCallbacks.call(&controlPointChangedCallbackData);
    }
    ScalarWidgetControlPoint* controlPointPtr = first->right;
    while (controlPointPtr != last) {
        ScalarWidgetControlPoint* next = controlPointPtr->right;
        delete controlPointPtr;
        controlPointPtr = next;
    }
    first->right = last;
    last->left = first;
} // end deleteControlPoints()

/*
 * determineControlPoint - Determine if and which control point was selected.
 *
 * parameter event - GLMotif::Event&
 */
ScalarWidgetControlPoint* ScalarWidget::determineControlPoint(GLMotif::Event& event) {
    GLfloat minimumDistanceSquared = Math::sqr(controlPointSize * 1.5f);
    ScalarWidgetControlPoint* _controlPoint = 0;
    for (ScalarWidgetControlPoint* controlPointPtr = first; controlPointPtr != 0; controlPointPtr = controlPointPtr->right) {
        GLMotif::Point currentPoint;
        if (this->is1D) {
            currentPoint = GLMotif::Point(controlPointPtr->getX(), areaBox.getCorner(0)[1], areaBox.getCorner(0)[2]);
        }
        else {
            currentPoint = GLMotif::Point(controlPointPtr->getX(), controlPointPtr->getY(), areaBox.getCorner(0)[2]);
        }
        GLfloat distanceSquared = Geometry::sqrDist(currentPoint, event.getWidgetPoint().getPoint());
        if (minimumDistanceSquared > distanceSquared) {
            minimumDistanceSquared = distanceSquared;
            _controlPoint = controlPointPtr;
            for (int i = 0; i < 2; ++i)
                dragOffset[i] = float(event.getWidgetPoint().getPoint()[i] - currentPoint[i]);
            dragOffset[2] = float(0);
        }
    }
    return _controlPoint;
} // end determineControlPoint()

/*
 * draw - Draw the editable scalar component.
 *
 * parameter contextData - GLContextData&
 */
void ScalarWidget::draw(GLContextData& contextData) const {
    Widget::draw(contextData);
    drawMargin();
    GLboolean lightingEnabled = glIsEnabled(GL_LIGHTING);
    if (lightingEnabled)
        glDisable(GL_LIGHTING);
//    drawArea();
    drawHistogram();
    if (lightingEnabled)
        glEnable(GL_LIGHTING);
    GLfloat lineWidth;
    glGetFloatv(GL_LINE_WIDTH, &lineWidth);
    drawLine();
    glLineWidth(lineWidth);
    drawControlPoints();
} // end draw()

/*
 * drawArea - Draw histogram area in a background color.
 */
void ScalarWidget::drawArea(void) const {
    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex(areaBox.getCorner(0));
    glVertex(areaBox.getCorner(1));
    glVertex(areaBox.getCorner(3));
    glVertex(areaBox.getCorner(2));
    glEnd();
} // end drawArea()

/*
 * drawControlPoints - Draw control points.
 */
void ScalarWidget::drawControlPoints(void) const {
    GLfloat _normal = 1.0f / Math::sqrt(3.0f);
    if (!gaussian) {
        glBegin(GL_TRIANGLES);
        for (ScalarWidgetControlPoint* controlPointPtr = first; controlPointPtr != 0; controlPointPtr = controlPointPtr->right) {
            if (controlPointPtr == controlPoint)
                glColor3f(1.0f, 1.0f, 1.0f);
            else {
                if (component == RED_COMPONENT) {
                    glColor3f(1.0f, 0.0f, 0.0f);
                } else if (component == GREEN_COMPONENT) {
                    glColor3f(0.0f, 1.0f, 0.0f);
                } else if (component == BLUE_COMPONENT) {
                    glColor3f(0.0f, 0.0f, 1.0f);
                } else if (component == ALPHA_COMPONENT) {
                    glColor3f(1.0f, 1.0f, 0.0f);
                }
            }
            glNormal3f(-_normal, _normal, _normal);
            glVertex3f(controlPointPtr->getX() - controlPointSize, controlPointPtr->getY(), areaBox.getCorner(0)[2]);
            glVertex3f(controlPointPtr->getX(), controlPointPtr->getY(), areaBox.getCorner(0)[2] + controlPointSize);
            glVertex3f(controlPointPtr->getX(), controlPointPtr->getY() + controlPointSize, areaBox.getCorner(0)[2]);
            glNormal3f(_normal, _normal, _normal);
            glVertex3f(controlPointPtr->getX(), controlPointPtr->getY() + controlPointSize, areaBox.getCorner(0)[2]);
            glVertex3f(controlPointPtr->getX(), controlPointPtr->getY(), areaBox.getCorner(0)[2] + controlPointSize);
            glVertex3f(controlPointPtr->getX() + controlPointSize, controlPointPtr->getY(), areaBox.getCorner(0)[2]);
            glNormal3f(_normal, -_normal, _normal);
            glVertex3f(controlPointPtr->getX() + controlPointSize, controlPointPtr->getY(), areaBox.getCorner(0)[2]);
            glVertex3f(controlPointPtr->getX(), controlPointPtr->getY(), areaBox.getCorner(0)[2] + controlPointSize);
            glVertex3f(controlPointPtr->getX(), controlPointPtr->getY() - controlPointSize, areaBox.getCorner(0)[2]);
            glNormal3f(-_normal, -_normal, _normal);
            glVertex3f(controlPointPtr->getX(), controlPointPtr->getY() - controlPointSize, areaBox.getCorner(0)[2]);
            glVertex3f(controlPointPtr->getX(), controlPointPtr->getY(), areaBox.getCorner(0)[2] + controlPointSize);
            glVertex3f(controlPointPtr->getX() - controlPointSize, controlPointPtr->getY(), areaBox.getCorner(0)[2]);
        }
        glEnd();
    } else {
        GLfloat x1 = areaBox.getCorner(0)[0];
        GLfloat x2 = areaBox.getCorner(1)[0];
        GLfloat y1 = areaBox.getCorner(0)[1];
        GLfloat y2 = areaBox.getCorner(2)[1];
        for (int p = 0; p < numberOfGaussians; p++) {
            float position = gaussians[p].getX();
            float height = gaussians[p].getH();
            float width = gaussians[p].getW();
            float xBias = gaussians[p].getBx();
            float yBias = gaussians[p].getBy();
            GLfloat x = GLfloat((position + xBias) * (x2 - x1) + x1);
            GLfloat xr = GLfloat((position + width) * (x2 - x1) + x1);
            GLfloat xl = GLfloat((position - width) * (x2 - x1) + x1);
            GLfloat y = GLfloat((height) * (y2 - y1) / (1.0f - 0.0f) + y1) - controlPointSize;
            GLfloat y0 = GLfloat(0.0f * (y2 - y1) / (1.0f - 0.0f) + y1);
            GLfloat yb = GLfloat(((height / 4.) + (yBias * height / 4.)) * (y2 - y1) / (1.0f - 0.0f) + y1);

            // square: position
            if (currentGaussian == p && currentMode == modeX) {
                if (dragging)
                    glColor3f(0.0f, 1.0f, 0.0f);
                else
                    glColor3f(0.0f, 1.0f, 1.0f);
            } else
                glColor3f(0.0f, 0.0f, 1.0f);
            glBegin(GL_QUADS);
            {
                glNormal3f(0.0f, 0.0f, 1.0f);
                glVertex3f(x - controlPointSize, y0, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                glVertex3f(x + controlPointSize, y0, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                glVertex3f(x + controlPointSize, y0 + controlPointSize, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                glVertex3f(x - controlPointSize, y0 + controlPointSize, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
            }
            glEnd();

            // diamond: bias (horizontal and vertical)
            if (currentGaussian == p && currentMode == modeB) {
                if (dragging)
                    glColor3f(0.0f, 1.0f, 0.0f);
                else
                    glColor3f(0.0f, 1.0f, 1.0f);
            } else
                glColor3f(0.0f, 0.0f, 1.0f);
            glLineWidth(1.0f);
            glBegin(GL_LINE_STRIP);
            {
                glVertex3f(x, yb, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                glVertex3f(x, yb + controlPointSize, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
            }
            glEnd();
            if (xBias > 0) {
                glBegin(GL_LINE_STRIP);
                {
                    glVertex3f(x, yb - controlPointSize, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                    glVertex3f(x + controlPointSize, yb, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                    glVertex3f(x, yb + controlPointSize, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                }
                glEnd();
            } else {
                glBegin(GL_LINE_STRIP);
                {
                    glVertex3f(x, yb, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                    glVertex3f(x + controlPointSize, yb, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                }
                glEnd();
            }
            if (xBias < 0) {
                glBegin(GL_LINE_STRIP);
                {
                    glVertex3f(x, yb - controlPointSize, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                    glVertex3f(x - controlPointSize, yb, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                    glVertex3f(x, yb + controlPointSize, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                }
                glEnd();
            } else {
                glBegin(GL_LINE_STRIP);
                {
                    glVertex3f(x - controlPointSize, yb, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                    glVertex3f(x, yb, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                }
                glEnd();
            }
            if (yBias > 0) {
                glBegin(GL_LINE_STRIP);
                {
                    glVertex3f(x - controlPointSize, yb, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                    glVertex3f(x, yb - controlPointSize, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                    glVertex3f(x + controlPointSize, yb, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                }
                glEnd();
            } else {
                glBegin(GL_LINE_STRIP);
                {
                    glVertex3f(x, yb - controlPointSize, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                    glVertex3f(x, yb, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                }
                glEnd();
            }

            // up triangle: height
            if (currentGaussian == p && currentMode == modeH) {
                if (dragging)
                    glColor3f(0.0f, 1.0f, 0.0f);
                else
                    glColor3f(0.0f, 1.0f, 1.0f);
            } else
                glColor3f(0.0f, 0.0f, 1.0f);
            glBegin(GL_TRIANGLES);
            {
                glNormal3f(0.0f, 0.0f, 1.0f);
                glVertex3f(x + controlPointSize, y + controlPointSize, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                glVertex3f(x, y + 2.0f * controlPointSize, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                glVertex3f(x - controlPointSize, y + controlPointSize, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
            }
            glEnd();

            // triangle: width (R)
            if (currentGaussian == p && (currentMode == modeWR || currentMode == modeW)) {
                if (dragging)
                    glColor3f(0.0f, 1.0f, 0.0f);
                else
                    glColor3f(0.0f, 1.0f, 1.0f);
            } else
                glColor3f(0.0f, 0.0f, 1.0f);
            glBegin(GL_TRIANGLES);
            {
                glNormal3f(0.0f, 0.0f, 1.0f);
                glVertex3f(xr, y0 + controlPointSize, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                glVertex3f(xr, y0, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                glVertex3f(xr + controlPointSize, y0 + controlPointSize, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
            }
            glEnd();

            // triangle: width (L)
            if (currentGaussian == p && (currentMode == modeWL || currentMode == modeW)) {
                if (dragging)
                    glColor3f(0.0f, 1.0f, 0.0f);
                else
                    glColor3f(0.0f, 1.0f, 1.0f);
            } else
                glColor3f(0.0f, 0.0f, 1.0f);
            glBegin(GL_TRIANGLES);
            {
                glNormal3f(0.0f, 0.0f, 1.0f);
                glVertex3f(xl, y0 + controlPointSize, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                glVertex3f(xl - controlPointSize, y0 + controlPointSize, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
                glVertex3f(xl, y0, areaBox.getCorner(0)[2] + marginWidth * 0.25f);
            }
            glEnd();
        }
    }
} // end drawControlPoints()

/*
 * drawHistogram - Draw histogram.
 */
void ScalarWidget::drawHistogram(void) const {
    GLfloat x1 = areaBox.getCorner(0)[0];
    GLfloat x2 = areaBox.getCorner(1)[0];
    GLfloat y1 = areaBox.getCorner(0)[1];
    GLfloat y2 = areaBox.getCorner(2)[1];
    GLfloat z = areaBox.getCorner(0)[2];
    if (component == RED_COMPONENT) {
        glColor3f(0.3f, 0.0f, 0.0f);
    } else if (component == GREEN_COMPONENT) {
        glColor3f(0.0f, 0.3f, 0.0f);
    } else if (component == BLUE_COMPONENT) {
        glColor3f(0.0f, 0.0f, 0.3f);
    } else if (component == ALPHA_COMPONENT) {
        glColor3f(0.3f, 0.3f, 0.0f);
    }
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i < numberOfEntries; i++) {
        GLfloat x = GLfloat((float(i)) / (float(numberOfEntries)) * (x2 - x1) + x1);
        GLfloat y = GLfloat((histogram[i] - 0.0f) * (y2 - y1) / (1.0f - 0.0f) + y1);
        glVertex3f(x, y, z);
        glVertex3f(x, y1, z);
    }
    glEnd();
} // end drawHistogram()

/*
 * drawLine - Draw component line for transform.
 */
void ScalarWidget::drawLine(void) const {
    if (!gaussian) {
        glLineWidth(3.0f);
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_LINE_STRIP);
        for (ScalarWidgetControlPoint* controlPointPtr = first; controlPointPtr != 0; controlPointPtr = controlPointPtr->right)
            glVertex3f(controlPointPtr->getX(), controlPointPtr->getY(), areaBox.getCorner(0)[2] + marginWidth * 0.25f);
        glEnd();
        glLineWidth(1.0f);
        if (component == RED_COMPONENT) {
            glColor3f(1.0f, 0.0f, 0.0f);
        } else if (component == GREEN_COMPONENT) {
            glColor3f(0.0f, 1.0f, 0.0f);
        } else if (component == BLUE_COMPONENT) {
            glColor3f(0.0f, 0.0f, 1.0f);
        } else if (component == ALPHA_COMPONENT) {
            glColor3f(1.0f, 1.0f, 1.0f);
        }
        glBegin(GL_LINE_STRIP);
        for (ScalarWidgetControlPoint* controlPointPtr = first; controlPointPtr != 0; controlPointPtr = controlPointPtr->right)
            glVertex3f(controlPointPtr->getX(), controlPointPtr->getY(), areaBox.getCorner(0)[2] + marginWidth * 0.25f);
        glEnd();
    } else {
        GLfloat x1 = areaBox.getCorner(0)[0];
        GLfloat x2 = areaBox.getCorner(1)[0];
        GLfloat y1 = areaBox.getCorner(0)[1];
        GLfloat y2 = areaBox.getCorner(2)[1];
        GLfloat z = areaBox.getCorner(0)[2];
        glLineWidth(3.0f);
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i < 256; i++) {
            GLfloat x = GLfloat((float(i)) / (float(256)) * (x2 - x1) + x1);
            GLfloat y = GLfloat((opacities[i] - 0.0f) * (y2 - y1) / (1.0f - 0.0f) + y1);
            glVertex3f(x, y, z);
        }
        glEnd();
        glLineWidth(1.0f);
        if (component == RED_COMPONENT) {
            glColor3f(1.0f, 0.0f, 0.0f);
        } else if (component == GREEN_COMPONENT) {
            glColor3f(0.0f, 1.0f, 0.0f);
        } else if (component == BLUE_COMPONENT) {
            glColor3f(0.0f, 0.0f, 1.0f);
        } else if (component == ALPHA_COMPONENT) {
            glColor3f(1.0f, 1.0f, 1.0f);
        }
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i < 256; i++) {
            GLfloat x = GLfloat((float(i)) / (float(256)) * (x2 - x1) + x1);
            GLfloat y = GLfloat((opacities[i] - 0.0f) * (y2 - y1) / (1.0f - 0.0f) + y1);
            glVertex3f(x, y, z);
        }
        glEnd();
    }
} // end drawLine()

/*
 * drawMargin - Draw margin area in background color.
 */
void ScalarWidget::drawMargin(void) const {
    glColor(backgroundColor);
    glBegin(GL_QUADS);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex(getInterior().getCorner(0));
    glVertex(areaBox.getCorner(0));
    glVertex(areaBox.getCorner(2));
    glVertex(getInterior().getCorner(2));
    glVertex(getInterior().getCorner(1));
    glVertex(getInterior().getCorner(3));
    glVertex(areaBox.getCorner(3));
    glVertex(areaBox.getCorner(1));
    glVertex(getInterior().getCorner(0));
    glVertex(getInterior().getCorner(1));
    glVertex(areaBox.getCorner(1));
    glVertex(areaBox.getCorner(0));
    glVertex(getInterior().getCorner(2));
    glVertex(areaBox.getCorner(2));
    glVertex(areaBox.getCorner(3));
    glVertex(getInterior().getCorner(3));
    glEnd();
} // end drawMargin()

/*
 * exportScalar - Export the current scalar component.
 *
 * parameter colormap - double*
 */
void ScalarWidget::exportScalar(double* colormap) const {
    if (!gaussian) {
        for (int i = 0; i < numberOfEntries; ++i) {
            double value = double(i) * (valueRange.second - valueRange.first) / double(numberOfEntries - 1) + valueRange.first;
            ScalarWidgetControlPoint* previousControlPoint = first;
            ScalarWidgetControlPoint* nextControlPoint;
            for (nextControlPoint = previousControlPoint->right; nextControlPoint != last && nextControlPoint->getValue() < value; previousControlPoint
                    = nextControlPoint, nextControlPoint = nextControlPoint->right)
                ;
            GLfloat w2 = GLfloat((value - previousControlPoint->getValue()) / (nextControlPoint->getValue()
                    - previousControlPoint->getValue()));
            GLfloat w1 = GLfloat((nextControlPoint->getValue() - value) / (nextControlPoint->getValue()
                    - previousControlPoint->getValue()));
            colormap[4*i + component] = (double) (previousControlPoint->getScalar() * w1
                    + nextControlPoint->getScalar() * w2);
        }
    } else {
        for (int i = 0; i < 256; i++)
            colormap[4*i + component] = (double) (opacities[i]);
    }
} // end exportScalar()

std::vector<double> ScalarWidget::exportControlPointValues( void )
{
  std::vector<double> controlPointValues;
  ScalarWidgetControlPoint* previousControlPoint = first;
  ScalarWidgetControlPoint* nextControlPoint;
  if (!gaussian)
    {
    for (nextControlPoint = previousControlPoint->right;
      nextControlPoint != last; nextControlPoint = nextControlPoint->right)
      {
      if( nextControlPoint == last)
        {
        continue;
        }
      controlPointValues.push_back(
        static_cast<double>(nextControlPoint->getValue() * 255.0));
      }
    }
  return controlPointValues;
}


/*
 * exportScalar - Export the current scalar component.
 *
 * parameter colormap - double*
 * parameter component - int
 */
void ScalarWidget::exportScalar(double* colormap, int component) {
    saveState();
    updatePointers(component);
    if (!gaussian) {
        for (int i = 0; i < numberOfEntries; ++i) {
            double value = double(i) * (valueRange.second - valueRange.first) / double(numberOfEntries - 1) + valueRange.first;
            ScalarWidgetControlPoint* previousControlPoint = first;
            ScalarWidgetControlPoint* nextControlPoint;
            for (nextControlPoint = previousControlPoint->right; nextControlPoint != last && nextControlPoint->getValue() < value; previousControlPoint
                    = nextControlPoint, nextControlPoint = nextControlPoint->right)
                ;
            GLfloat w2 = GLfloat((value - previousControlPoint->getValue()) / (nextControlPoint->getValue()
                    - previousControlPoint->getValue()));
            GLfloat w1 = GLfloat((nextControlPoint->getValue() - value) / (nextControlPoint->getValue()
                    - previousControlPoint->getValue()));
            colormap[4*i + component] = (double) (previousControlPoint->getScalar() * w1
                    + nextControlPoint->getScalar() * w2);
        }
    } else {
        getOpacities();
        for (int i = 0; i < 256; i++)
            colormap[4*i + component] = (double) (opacities[i]);
    }
    updatePointers(this->component);
} // end exportScalar()

/*
 * findGaussianControlPoint
 *
 * parameter x - float
 * parameter y - float
 * parameter z - float
 */
bool ScalarWidget::findGaussianControlPoint(float x, float y, float z) {
    currentGaussian = -1;
    currentMode = modeNone;
    bool found = false;
    float mindist = FLT_MAX;
    float x1 = areaBox.getCorner(0)[0];
    float x2 = areaBox.getCorner(1)[0];
    float y1 = areaBox.getCorner(0)[1];
    float y2 = areaBox.getCorner(2)[1];
    float ax = x * (x2 - x1) + x1;
    float ay = y * (y2 - y1) / (1.0f - 0.0f) + y1;
    float az = z;
    for (int p = 0; p < numberOfGaussians; p++) {
        float xc = float(gaussians[p].getX() + gaussians[p].getBx()) * (x2 - x1) + x1;
        float xr = float(gaussians[p].getX() + gaussians[p].getW()) * (x2 - x1) + x1;
        float xl = float(gaussians[p].getX() - gaussians[p].getW()) * (x2 - x1) + x1;
        float yc = float(gaussians[p].getH()) * (y2 - y1) / (1.0f - 0.0f) + y1;
        float y0 = float(0) * (y2 - y1) / (1.0f - 0.0f) + y1;
        float yb = float(gaussians[p].getH() / 4. + gaussians[p].getBy() * gaussians[p].getH() / 4.) * (y2 - y1) / (1.0f - 0.0f)
                + y1;
        float z0 = float(areaBox.getCorner(0)[2]);

        float d1 = dist3(ax, ay, az, xc, y0, z0);
        float d2 = dist3(ax, ay, az, xc, yc, z0);
        float d3 = dist3(ax, ay, az, xr, y0, z0);
        float d4 = dist3(ax, ay, az, xl, y0, z0);
        float d5 = dist3(ax, ay, az, xc, yb, z0);

        float rad = Math::sqr(1.5f * controlPointSize);

        if (d1 < rad && mindist > d1) {
            currentGaussian = p;
            currentMode = modeX;
            mindist = d1;
            found = true;
        }
        if (d2 < rad && mindist > d2) {
            currentGaussian = p;
            currentMode = modeH;
            mindist = d2;
            found = true;
        }
        if (d3 < rad && mindist > d3) {
            currentGaussian = p;
            currentMode = modeWR;
            mindist = d3;
            found = true;
        }
        if (d4 < rad && mindist > d4) {
            currentGaussian = p;
            currentMode = modeWL;
            mindist = d4;
            found = true;
        }
        if (d5 < rad && mindist > d5) {
            currentGaussian = p;
            currentMode = modeB;
            mindist = d5;
            found = true;
        }
    }
    return found;
} // end findGaussianControlPoint()

/*
 * findRecipient - Determine which is the applicable widget of the event. A virtual function of GLMotif::Widget base.
 *
 * parameter event - GLMotif::Event&
 * return - bool
 */
bool ScalarWidget::findRecipient(GLMotif::Event& event) {
    if (dragging || unselected)
        return event.setTargetWidget(this, event.calcWidgetPoint(this));
    else
        return GLMotif::Widget::findRecipient(event);
} // end findRecipient()

/*
 * getChangedCallbacks
 *
 * return - Misc::CallbackList&
 */
Misc::CallbackList& ScalarWidget::getChangedCallbacks(void) {
    return changedCallbacks;
} // end getChangedCallbacks()

/*
 * setComponent
 *
 * parameter component - int
 */
void ScalarWidget::setComponent(int component) {
    saveState();
    this->component = component;
    updatePointers(component);
    updateControlPoints();
} // end setComponent()

/*
 * getControlPointChangedCallbacks
 *
 * return - Misc::CallbackList&
 */
Misc::CallbackList& ScalarWidget::getControlPointChangedCallbacks(void) {
    return controlPointChangedCallbacks;
} // end getControlPointChangedCallbacks()

/*
 * getControlPointScalar
 *
 * return - float
 */
float ScalarWidget::getControlPointScalar(void) {
    return controlPoint->getScalar();
} // end getControlPointScalar()

/*
 * setControlPointScalar - Set current control point scalar.
 *
 * parameter _scalar - float
 */
void ScalarWidget::setControlPointScalar(float _scalar) {
    if (controlPoint != 0) {
        controlPoint->setScalar(_scalar);
        controlPointScalar = _scalar;
        updateControlPoints();
        ScalarWidgetChangedCallbackData changedCallbackData(this);
        changedCallbacks.call(&changedCallbackData);
    }
} // end setControlPointScalar()

/*
 * setControlPointSize - Set control point size.
 *
 * parameter _controlPointSize - GLfloat
 */
void ScalarWidget::setControlPointSize(GLfloat _controlPointSize) {
    controlPointSize = _controlPointSize;
} // end setControlPointSize()

/*
 * setControlPointValue - Set current control point value.
 *
 * parameter _value - double
 */
void ScalarWidget::setControlPointValue(double _value) {
    if (controlPoint != 0 && controlPoint->left != 0 && controlPoint->right != 0) {
        if (_value < first->getValue())
            controlPoint->setValue(first->getValue());
        else if (_value > last->getValue())
            controlPoint->setValue(last->getValue());
        else
            controlPoint->setValue(_value);
        updateControlPoints();
        ScalarWidgetChangedCallbackData callbackData(this);
        changedCallbacks.call(&callbackData);
    }
} // end setControlPointValue()

/*
 * isDragging
 *
 * return - bool
 */
bool ScalarWidget::isDragging(void) const {
    if (unselected) {
        return false;
    } else
        return dragging;
} // end isDragging()

/*
 * getGaussian
 *
 * return - bool
 */
bool ScalarWidget::getGaussian(void) {
    return gaussian;
} // end getGaussian()

/*
 * setGaussian
 *
 * parameter gaussian - bool
 */
void ScalarWidget::setGaussian(bool gaussian) {
    this->gaussian = gaussian;
    ScalarWidgetChangedCallbackData callbackData(this);
    changedCallbacks.call(&callbackData);
} // end setGaussian()

/*
 * getOpacities
 */
void ScalarWidget::getOpacities(void) {
    for (int i = 0; i < 256; i++)
        opacities[i] = float(0);
    for (int p = 0; p < numberOfGaussians; p++) {
        float position = gaussians[p].getX();
        float height = gaussians[p].getH();
        float width = gaussians[p].getW();
        float xbias = gaussians[p].getBx();
        float ybias = gaussians[p].getBy();
        for (int i = 0; i < 256; i++) {
            float x = float(i) / float(255);
            // clamp non-zero values to pos +/- width
            if (x > position + width || x < position - width) {
                opacities[i] = opacities[i] > 0.0 ? opacities[i] : 0.0f;
                continue;
            }

            // non-zero width
            if (width == 0)
                width = .00001;

            // translate the original x to a new x based on the xbias
            float x0;
            if (xbias == 0 || x == position + xbias) {
                x0 = x;
            } else if (x > position + xbias) {
                if (width == xbias)
                    x0 = position;
                else
                    x0 = position + (x - position - xbias) * (width / (width - xbias));
            } else // (x < pos+xbias)
            {
                if (-width == xbias)
                    x0 = position;
                else
                    x0 = position - (x - position - xbias) * (width / (width + xbias));
            }

            // center around 0 and normalize to -1,1
            float x1 = (x0 - position) / width;

            // do a linear interpolation between:
            //    a gaussian and a parabola        if 0<ybias<1
            //    a parabola and a step function   if 1<ybias<2
            float h0a = exp(-(4 * x1 * x1));
            float h0b = 1. - x1 * x1;
            float h0c = 1.;
            float h1;
            if (ybias < 1)
                h1 = ybias * h0b + (1 - ybias) * h0a;
            else
                h1 = (2 - ybias) * h0b + (ybias - 1) * h0c;
            float h2 = height * h1;

            // perform the MAX over different gaussians, not the sum
            opacities[i] = opacities[i] > h2 ? opacities[i] : h2;
        }
    }
} // end getOpacities()

/*
 * getOpacities
 *
 * parameter component - int
 */
void ScalarWidget::getOpacities(int component) {
    saveState();
    updatePointers(component);
    getOpacities();
    updatePointers(this->component);
} // end getOpacities

/*
 * setMarginWidth - Set the margin width.
 *
 * parameter _margineWidth - GLfloat
 */
void ScalarWidget::setMarginWidth(GLfloat _marginWidth) {
    marginWidth = _marginWidth;
    if (isManaged) {
        parent->requestResize(this, calcNaturalSize());
    } else
        resize(GLMotif::Box(GLMotif::Vector(0.0f, 0.0f, 0.0f), calcNaturalSize()));
} // end setMarginWidth()

/*
 * getNumberOfControlPoints - Get number of control points.
 *
 * return - int
 */
int ScalarWidget::getNumberOfControlPoints(void) const {
    int numberOfControlPoints = 0;
    for (ScalarWidgetControlPoint* controlPointPtr = first; controlPointPtr != 0; controlPointPtr = controlPointPtr->right)
        ++numberOfControlPoints;
    return numberOfControlPoints;
} // end getNumberOfControlPoints()

/*
 * setPreferredSize - Set preferred size.
 *
 * parameter _preferredSize - const GLMotif::Vector&
 */
void ScalarWidget::setPreferredSize(const GLMotif::Vector& _preferredSize) {
    preferredSize = _preferredSize;
    if (isManaged) {
        parent->requestResize(this, calcNaturalSize());
    } else
        resize(GLMotif::Box(GLMotif::Vector(0.0f, 0.0f, 0.0f), calcNaturalSize()));
} // end setPreferredSize()

/*
 * getScalar - Get scalar.
 *
 * parameter _scalar - float*
 */
void ScalarWidget::getScalar(float* _scalar) {
    for (int i = 0; i < numberOfEntries; ++i) {
        double value = double(i) * (valueRange.second - valueRange.first) / double(numberOfEntries - 1) + valueRange.first;
        ScalarWidgetControlPoint* previousControlPoint = first;
        ScalarWidgetControlPoint* nextControlPoint;
        for (nextControlPoint = previousControlPoint->right; nextControlPoint != last && nextControlPoint->getValue() < value; previousControlPoint
                = nextControlPoint, nextControlPoint = nextControlPoint->right)
            ;
        double w2 = (value - previousControlPoint->getValue()) / (nextControlPoint->getValue() - previousControlPoint->getValue());
        double w1 = (nextControlPoint->getValue() - value) / (nextControlPoint->getValue() - previousControlPoint->getValue());
        _scalar[i] = (float) (previousControlPoint->getScalar() * w1 + nextControlPoint->getScalar() * w2);
    }
} // end getScalar()

/*
 * getStorage - Get the scalar.
 *
 * return - ScalarWidgetStorage*
 */
ScalarWidgetStorage* ScalarWidget::getStorage(void) const {
    return new ScalarWidgetStorage(first);
} // end getStorage()

/*
 * setStorage - Set control points for the scalar.
 *
 * parameter _storage - ScalarWidgetStorage*
 */
void ScalarWidget::setStorage(ScalarWidgetStorage* _storage) {
    deleteControlPoints();
    first->setValue(_storage->getValue(0));
    first->setScalar(_storage->getScalar(0));
    ScalarWidgetControlPoint* leftControlPoint = first;
    for (int i = 1; i < _storage->getNumberOfControlPoints() - 1; ++i) {
        ScalarWidgetControlPoint* _controlPoint = new ScalarWidgetControlPoint(_storage->getValue(i), _storage->getScalar(i));
        _controlPoint->left = leftControlPoint;
        leftControlPoint->right = _controlPoint;
        leftControlPoint = _controlPoint;
    }
    last->setValue(_storage->getValue(_storage->getNumberOfControlPoints() - 1));
    last->setScalar(_storage->getScalar(_storage->getNumberOfControlPoints() - 1));
    last->left = leftControlPoint;
    leftControlPoint->right = last;
    valueRange.first = first->getValue();
    valueRange.second = last->getValue();
    updateControlPoints();
    ScalarWidgetChangedCallbackData changedCallbackData(this);
    changedCallbacks.call(&changedCallbackData);
} // end setStorage()

/*
 * getValueRange
 *
 * return - std::pair<double,double>&
 */
const std::pair<double, double>& ScalarWidget::getValueRange(void) const {
    return valueRange;
} // end getValueRange()

/*
 * insertControlPoint - Insert control point.
 *
 * parameter _value - double
 */
void ScalarWidget::insertControlPoint(double _value) {
    ScalarWidgetControlPoint* previousControlPoint = first;
    ScalarWidgetControlPoint* nextControlPoint;
    for (nextControlPoint = previousControlPoint->right; nextControlPoint != last && nextControlPoint->getValue() < _value; previousControlPoint
            = nextControlPoint, nextControlPoint = nextControlPoint->right)
        ;
    GLfloat w2 = GLfloat((_value - previousControlPoint->getValue()) / (nextControlPoint->getValue()
            - previousControlPoint->getValue()));
    GLfloat w1 = GLfloat((nextControlPoint->getValue() - _value)
            / (nextControlPoint->getValue() - previousControlPoint->getValue()));
    ScalarWidgetControlPoint* _controlPoint = new ScalarWidgetControlPoint(_value, previousControlPoint->getScalar() * w1
            + nextControlPoint->getScalar() * w2);
    _controlPoint->left = previousControlPoint;
    previousControlPoint->right = _controlPoint;
    _controlPoint->right = nextControlPoint;
    nextControlPoint->left = _controlPoint;
    updateControlPoints();
    ScalarWidgetChangedCallbackData alphaChangedCallbackData(this);
    changedCallbacks.call(&alphaChangedCallbackData);
    ScalarWidgetControlPointChangedCallbackData controlPointChangedCallbackData(this, controlPoint, _controlPoint);
    controlPoint = _controlPoint;
    controlPointChangedCallbacks.call(&controlPointChangedCallbackData);
} // end insertControlPoint()

/*
 * pointerButtonDown - Pointer button down event handler. A virtual function of GLMotif::Widget base.
 *
 * parameter event - GLMotif::Event&
 */
void ScalarWidget::pointerButtonDown(GLMotif::Event& event) {
    if (!gaussian) {
        ScalarWidgetControlPoint* _controlPoint = determineControlPoint(event);
        if (_controlPoint != controlPoint) {
            ScalarWidgetControlPointChangedCallbackData controlPointChangedCallbackData(this, controlPoint, _controlPoint);
            controlPoint = _controlPoint;
            controlPointChangedCallbacks.call(&controlPointChangedCallbackData);
            unselected = true;
        } else if (_controlPoint == 0) {
            double _value = (event.getWidgetPoint().getPoint()[0] - double(areaBox.getCorner(0)[0])) * (valueRange.second
                    - valueRange.first) / double(areaBox.getCorner(1)[0] - areaBox.getCorner(0)[0]) + valueRange.first;
            if (_value <= valueRange.first || _value >= valueRange.second) {
                _controlPoint = controlPoint;
            } else {
                insertControlPoint(_value);
            }
            dragging = true;
            unselected = false;
        } else if (_controlPoint == controlPoint) {
            dragging = true;
            unselected = false;
        }
    } else {
        GLMotif::Point _point = event.getWidgetPoint().getPoint() - dragOffset;
        float x = (_point[0] - float(areaBox.getCorner(0)[0])) * (valueRange.second - valueRange.first)
                / float(areaBox.getCorner(1)[0] - areaBox.getCorner(0)[0]) + valueRange.first;
        float y = (_point[1] - areaBox.getCorner(0)[1]) * (1.0f - 0.0f) / (areaBox.getCorner(2)[1] - areaBox.getCorner(0)[1])
                + 0.0f;
        float z = _point[2];
        if (!findGaussianControlPoint(x, y, z)) {
            currentGaussian = -1;
            currentMode = modeNone;
            addGaussian(x, y, 0.001, 0, 0);
        }
        dragging = true;
    }
} // end pointerButtonDown()

/*
 * pointerButtonUp - Pointer button up event handler. A virtual function of GLMotif::Widget base.
 *
 * parameter event - GLMotif::Event&
 */
void ScalarWidget::pointerButtonUp(GLMotif::Event& event) {
    if (dragging) {
        dragging = false;
        ScalarWidgetChangedCallbackData changedCallbackData(this);
        changedCallbacks.call(&changedCallbackData);
    }
} // end pointerButtonUp()

/*
 * pointerMotion - Pointer motion event handler. A virtual function of GLMotif::Widget base.
 *
 * parameter event - GLMotif::Event&
 */
void ScalarWidget::pointerMotion(GLMotif::Event& event) {
    if (!gaussian) {
        if (dragging) {
            GLMotif::Point _point = event.getWidgetPoint().getPoint() - dragOffset;
            double _value = (_point[0] - double(areaBox.getCorner(0)[0])) * (valueRange.second - valueRange.first)
                    / double(areaBox.getCorner(1)[0] - areaBox.getCorner(0)[0]) + valueRange.first;
            if (controlPoint == first)
                _value = valueRange.first;
            else if (controlPoint == last)
                _value = valueRange.second;
            else if (_value < controlPoint->left->getValue())
                _value = controlPoint->left->getValue();
            else if (_value > controlPoint->right->getValue())
                _value = controlPoint->right->getValue();
            GLfloat _scalar;
            if(!is1D) {
                _scalar = (_point[1] - areaBox.getCorner(0)[1]) * (1.0f - 0.0f) / (areaBox.getCorner(2)[1] - areaBox.getCorner(
                    0)[1]) + 0.0f;
            }
            else {
                _scalar = 0.0f;
            }
            if (_scalar < 0.0f)
                _scalar = 0.0f;
            else if (_scalar > 1.0f)
                _scalar = 1.0f;
            controlPoint->setValue(_value);
            controlPoint->setScalar(_scalar);
            updateControlPoints();
            ScalarWidgetChangedCallbackData changedCallbackData(this);
            changedCallbacks.call(&changedCallbackData);
        }
    } else {
        getOpacities();
        GLMotif::Point _point = event.getWidgetPoint().getPoint() - dragOffset;
        float x = (_point[0] - float(areaBox.getCorner(0)[0])) * (valueRange.second - valueRange.first)
                / float(areaBox.getCorner(1)[0] - areaBox.getCorner(0)[0]) + valueRange.first;
        if (x < valueRange.first)
            x = valueRange.first;
        if (x > valueRange.second)
            x = valueRange.second;
        float y = (_point[1] - areaBox.getCorner(0)[1]) * (1.0f - 0.0f) / (areaBox.getCorner(2)[1] - areaBox.getCorner(0)[1])
                + 0.0f;
        if (y < 0.0f)
            y = 0.0f;
        else if (y > 1.0f)
            y = 1.0f;
        float z = _point[2];
        if (!dragging) {
            int oldGaussian = currentGaussian;
            Mode oldMode = currentMode;
            findGaussianControlPoint(x, y, z);
            if (oldGaussian != currentGaussian || oldMode != currentMode) {
                ScalarWidgetChangedCallbackData changedCallbackData(this);
                changedCallbacks.call(&changedCallbackData);
            }
        } else {
            switch (currentMode) {
                case modeX:
                    gaussians[currentGaussian].setX(x - gaussians[currentGaussian].getBx());
                    break;
                case modeH:
                    gaussians[currentGaussian].setH(y);
                    break;
                case modeW:
                    gaussians[currentGaussian].setW(fabs(x - gaussians[currentGaussian].getX()) > 0.01 ? fabs(x
                            - gaussians[currentGaussian].getX()) : 0.01);
                    break;
                case modeWR:
                    gaussians[currentGaussian].setW((x - gaussians[currentGaussian].getX()) > 0.01 ? (x
                            - gaussians[currentGaussian].getX()) : 0.01);
                    if (gaussians[currentGaussian].getW() < fabs(gaussians[currentGaussian].getBx()))
                        gaussians[currentGaussian].setW(fabs(gaussians[currentGaussian].getBx()));
                    break;
                case modeWL:
                    gaussians[currentGaussian].setW(
                            (gaussians[currentGaussian].getX() - x) > 0.01 ? (gaussians[currentGaussian].getX() - x) : 0.01);
                    if (gaussians[currentGaussian].getW() < fabs(gaussians[currentGaussian].getBx()))
                        gaussians[currentGaussian].setW(fabs(gaussians[currentGaussian].getBx()));
                    break;
                case modeB:
                    gaussians[currentGaussian].setBx(x - gaussians[currentGaussian].getX());
                    if (gaussians[currentGaussian].getBx() > gaussians[currentGaussian].getW())
                        gaussians[currentGaussian].setBx(gaussians[currentGaussian].getW());
                    if (gaussians[currentGaussian].getBx() < -gaussians[currentGaussian].getW())
                        gaussians[currentGaussian].setBx(-gaussians[currentGaussian].getW());
                    if (fabs(gaussians[currentGaussian].getBx()) < .001)
                        gaussians[currentGaussian].setBx(0);

                    gaussians[currentGaussian].setBy(4 * (y - gaussians[currentGaussian].getH() / 4.)
                            / gaussians[currentGaussian].getH());
                    if (gaussians[currentGaussian].getBy() > 2)
                        gaussians[currentGaussian].setBy(2);
                    if (gaussians[currentGaussian].getBy() < 0)
                        gaussians[currentGaussian].setBy(0);
                    break;
            }
        }
        ScalarWidgetChangedCallbackData changedCallbackData(this);
        changedCallbacks.call(&changedCallbackData);
    }
} // pointerMotion()

/*
 * removeGaussian
 *
 * parameter which - int
 */
void ScalarWidget::removeGaussian(int which) {
    for (int i = which; i < numberOfGaussians - 1; i++)
        gaussians[i] = gaussians[i + 1];
    numberOfGaussians--;
} // end removeGaussian()

/*
 * resize - Resize the component display. A virtual function of GLMotif::Widget base.
 *
 * parameter _exterior - const GLMotif::Box&
 */
void ScalarWidget::resize(const GLMotif::Box& _exterior) {
    GLMotif::Widget::resize(_exterior);
    areaBox = getInterior();
    areaBox.doInset(GLMotif::Vector(marginWidth, marginWidth, 0.0f));
    updateControlPoints();
} // end resize()

/*
 * saveState
 */
void ScalarWidget::saveState(void) {
    if (component == 0) {
        numberOfRedGaussians = numberOfGaussians;
        redGaussian = gaussian;
    } else if (component == 1) {
        numberOfGreenGaussians = numberOfGaussians;
        greenGaussian = gaussian;
    } else if (component == 2) {
        numberOfBlueGaussians = numberOfGaussians;
        blueGaussian = gaussian;
    } else if (component == 3) {
        numberOfAlphaGaussians = numberOfGaussians;
        alphaGaussian = gaussian;
    }
} // saveState()

/*
 * selectControlPoint - Select control point.
 *
 * parameter i - int
 */
void ScalarWidget::selectControlPoint(int i) {
    ScalarWidgetControlPoint* controlPointPtr = 0;
    if (i >= 0)
        for (controlPointPtr = first; i > 0 && controlPointPtr != 0; controlPointPtr = controlPointPtr->right, --i)
            ;
    ScalarWidgetControlPointChangedCallbackData controlPointChangedCallbackData(this, controlPoint, controlPointPtr);
    controlPoint = controlPointPtr;
    controlPointChangedCallbacks.call(&controlPointChangedCallbackData);
} // end selectControlPoint()

/*
 * updateControlPoints - Update the control points.
 */
void ScalarWidget::updateControlPoints(void) {
    GLfloat x1 = areaBox.getCorner(0)[0];
    GLfloat x2 = areaBox.getCorner(1)[0];
    GLfloat y1 = areaBox.getCorner(0)[1];
    GLfloat y2 = areaBox.getCorner(2)[1];
    for (ScalarWidgetControlPoint* controlPointPtr = first; controlPointPtr != 0; controlPointPtr = controlPointPtr->right) {
        controlPointPtr->setX(GLfloat((controlPointPtr->getValue() - valueRange.first) / (valueRange.second - valueRange.first))
                * (x2 - x1) + x1);
        if (this->is1D) {
          controlPointPtr->setY(y1);
        }
        else {
          controlPointPtr->setY(GLfloat((controlPointPtr->getScalar() - 0.0f) * (y2 - y1) / (1.0f - 0.0f) + y1));
        }
    }
} // end updateControlPoints()

/*
 * updatePointers
 *
 * parameter component - int
 */
void ScalarWidget::updatePointers(int component) {
    if (component == 0) {
        first = redFirst;
        last = redLast;
        gaussians = redGaussians;
        histogram = redHistogram;
        opacities = redOpacities;
        numberOfGaussians = numberOfRedGaussians;
        gaussian = redGaussian;
    } else if (component == 1) {
        first = greenFirst;
        last = greenLast;
        gaussians = greenGaussians;
        histogram = greenHistogram;
        opacities = greenOpacities;
        numberOfGaussians = numberOfGreenGaussians;
        gaussian = greenGaussian;
    } else if (component == 2) {
        first = blueFirst;
        last = blueLast;
        gaussians = blueGaussians;
        histogram = blueHistogram;
        opacities = blueOpacities;
        numberOfGaussians = numberOfBlueGaussians;
        gaussian = blueGaussian;
    } else if (component == 3) {
        first = alphaFirst;
        last = alphaLast;
        gaussians = alphaGaussians;
        histogram = alphaHistogram;
        opacities = alphaOpacities;
        numberOfGaussians = numberOfAlphaGaussians;
        gaussian = alphaGaussian;
    }
} // end updatePointers()

/*
 * setHistogram
 *
 * parameter hist - float *
 */
void ScalarWidget::setHistogram(float* hist)
{
  float max_val = 1.0f;
  float min_val = 0.0f;
  for(int i = 1; i < 256; ++i)
    {
    if(hist[i] < min_val)
      {
      min_val = hist[i];
      }
    else if(hist[i] > max_val)
      {
      max_val = hist[i];
      }
    }
  for(int i = 0; i < 256; ++i)
    {
    this->histogram[i] = 3*(hist[i] - min_val)/(max_val - min_val);
    if(this->histogram[i] > 1.0f)
      {
      this->histogram[i] = 1.0f;
      }
    }
}

/*
 * is1D
 *
 * parameter enable - bool
 */
void ScalarWidget::useAs1DWidget(bool enable)
{
  this->is1D = enable;
}
