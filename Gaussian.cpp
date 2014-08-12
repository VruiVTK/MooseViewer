#include "Gaussian.h"

/*
 * Guassian - Constructor for Gaussian class.
 */
Gaussian::Gaussian(void) :
    x(0), h(0), w(0), bx(0), by(0) {
} // end Gaussian()

/*
 * Gaussian - Constructor for Gaussian class.
 *
 * parameter _x - float
 * parameter _h - float
 * parameter _w - float
 * parameter _bx - float
 * parameter _by - float
 */
Gaussian::Gaussian(float _x, float _h, float _w, float _bx, float _by) :
    x(_x), h(_h), w(_w), bx(_bx), by(_by) {
} // end Gaussian()

/*
 * ~Gaussian - Destructor for Gaussian class.
 */
Gaussian::~Gaussian(void) {
} // end ~Gaussian()

/*
 * getBx
 *
 * return - float
 */
float Gaussian::getBx(void) const {
    return bx;
} // end getBx()

/*
 * setBx
 *
 * parameter bx - float
 */
void Gaussian::setBx(float bx) {
    this->bx = bx;
} // end setBx()

/*
 * getBy
 *
 * return - float
 */
float Gaussian::getBy(void) const {
    return by;
} // end getBx()

/*
 * setBy
 *
 * parameter by - float
 */
void Gaussian::setBy(float by) {
    this->by = by;
} // end setBx()

/*
 * getX
 *
 * return - float
 */
float Gaussian::getX(void) const {
    return x;
} // end getX()

/*
 * setX
 *
 * parameter x - float
 */
void Gaussian::setX(float x) {
    this->x = x;
} // end setX()

/*
 * getH
 *
 * return - float
 */
float Gaussian::getH(void) const {
    return h;
} // end getH()

/*
 * setH
 *
 * parameter h - float
 */
void Gaussian::setH(float h) {
    this->h = h;
} // end setH()

/*
 * getW
 *
 * return - float
 */
float Gaussian::getW(void) const {
    return w;
} // end getW()

/*
 * setW
 *
 * parameter w - float
 */
void Gaussian::setW(float w) {
    this->w = w;
} // end setW()
