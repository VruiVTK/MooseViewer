#ifndef STORAGE_H_
#define STORAGE_H_

// begin Forward Declarations
class ControlPoint;
// end Forward Declarations

class Storage {
public:
	Storage(void);
	Storage(ControlPoint* first);
	~Storage(void);
	int getNumberOfControlPoints();
	RGBAColor* getRGBAColors();
	RGBAColor* getRGBAColors(int i);
	double* getValues();
	double getValues(int i);
private:
	int numberOfControlPoints;
	double* values;
	RGBAColor* rgbaColors;
	Storage(const Storage& source);
	Storage& operator=(const Storage& source);
};

#endif /*STORAGE_H_*/
