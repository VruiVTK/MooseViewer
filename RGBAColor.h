#ifndef RGBACOLOR_H_
#define RGBACOLOR_H_

class RGBAColor {
public:
	RGBAColor();
	RGBAColor(float _values[4]);
	RGBAColor(float r, float g, float b, float a);
	virtual ~RGBAColor();
	float * getColor(void) const;
	float * getValues(void);
	void setValues(float _values[4]);
	float getValues(int i);
	void setValues(int i, float value);
private:
	float * values;
};

#endif /*RGBACOLOR_H_*/
