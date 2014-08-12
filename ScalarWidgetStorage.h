#ifndef SCALARWIDGETSTORAGE_H_
#define SCALARWIDGETSTORAGE_H_

// begin Forward Declarations
class ScalarWidgetControlPoint;
// end Forward Declarations

class ScalarWidgetStorage {
public:
	ScalarWidgetStorage(void);
	ScalarWidgetStorage(ScalarWidgetControlPoint* first);
	~ScalarWidgetStorage(void);
	int getNumberOfControlPoints();
	float* getScalars();
	float getScalar(int i);
	double* getValues();
	double getValue(int i);
private:
	int numberOfControlPoints;
	double* values;
	float* scalars;
};

#endif /*SCALARWIDGETSTORAGE_H_*/
