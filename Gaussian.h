#ifndef GAUSSIAN_H_
#define GAUSSIAN_H_

class Gaussian {
public:
    Gaussian(void);
    Gaussian(float _x, float _h, float _w, float _bx, float _by);
    ~Gaussian(void);
    float getBx(void) const;
    void setBx(float bx);
    float getBy(void) const;
    void setBy(float by);
    float getX(void) const;
    void setX(float x);
    float getH(void) const;
    void setH(float h);
    float getW(void) const;
    void setW(float w);
private:
    float x;
    float h;
    float w;
    float bx;
    float by;
};

#endif /* GAUSSIAN_H_ */
