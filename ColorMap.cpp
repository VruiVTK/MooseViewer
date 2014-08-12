#include <iostream>
#include <fstream>
#include <GL/GLColorTemplates.h>
#include <GL/GLVertexTemplates.h>
#include <Math/Math.h>
#include <Misc/File.h>

#include "ColorMap.h"
#include "ControlPoint.h"
#include "ColorMapChangedCallbackData.h"
#include "ControlPointChangedCallbackData.h"
#include "RGBAColor.h"
#include "Storage.h"

/*
 * ColorMap - Constructor for the ColorMap class.
 *
 * parameter _name - const char*
 * parameter _parent - GLMotif::Container*
 * parameter _manageChild - bool
 */
ColorMap::ColorMap(const char* _name, GLMotif::Container* _parent, bool _manageChild) :
	GLMotif::Widget(_name, _parent, false) {
	marginWidth=0.0f;
	preferredSize[0]=0.0f;
	preferredSize[1]=0.0f;
	preferredSize[2]=0.0f;
	controlPointSize=marginWidth*0.5f;
	controlPointColor=new RGBAColor(1.0f, 0.0f, 0.0f, 0.0f);
	valueRange=std::pair<double,double>(0.0, 1.0);
	first=new ControlPoint(0.0,new RGBAColor(0.0f, 0.0f, 0.0f, 0.0f));
	last=new ControlPoint(1.0,new RGBAColor(1.0f, 1.0f, 1.0f, 0.0f));
	first->right=last;
	last->left=first;
	controlPoint=NULL;
	isDragging=false;
	updateControlPoints();
	if (_manageChild) manageChild();
}

/*
 * ~ColorMap - Destructor for the ColorMap class.
 */
ColorMap::~ColorMap(void) {
	delete controlPointColor;
	ControlPoint* controlPointPtr=first->right;
	while (controlPointPtr!=last) {
		ControlPoint* next=controlPointPtr->right;
		delete controlPointPtr;
		controlPointPtr=next;
	}
}

/*
 * calcNaturalSize - Determine the natural size of the colormap. A virtual function of GLMotif::Widget base.
 *
 * return - GLMotif::Vector
 */
GLMotif::Vector ColorMap::calcNaturalSize(void) const {
	GLMotif::Vector result=preferredSize;
	result[0]+=2.0f*marginWidth;
	result[1]+=2.0f*marginWidth;
	return calcExteriorSize(result);
}

/*
 * createColorMap - Create color map.
 *
 * parameter colormap - int
 */
void ColorMap::createColorMap(int colormap) {
	deleteColorMap();
	double _minimum=valueRange.first;
	double _maximum=valueRange.second;
	if (colormap==CFULL_RAINBOW) {
		first=new ControlPoint(FULL_RAINBOW[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(FULL_RAINBOW[0][1],FULL_RAINBOW[0][2],FULL_RAINBOW[0][3],0.0f));
		ControlPoint* _controlPoint[7];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(FULL_RAINBOW[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(FULL_RAINBOW[1][1],FULL_RAINBOW[1][2],FULL_RAINBOW[1][3],0.0f));
		_controlPoint[2] =new ControlPoint(FULL_RAINBOW[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(FULL_RAINBOW[2][1],FULL_RAINBOW[2][2],FULL_RAINBOW[2][3],0.0f));
		_controlPoint[3] =new ControlPoint(FULL_RAINBOW[3][0]*(_maximum-_minimum)+_minimum,new RGBAColor(FULL_RAINBOW[3][1],FULL_RAINBOW[3][2],FULL_RAINBOW[3][3],0.0f));
		_controlPoint[4] =new ControlPoint(FULL_RAINBOW[4][0]*(_maximum-_minimum)+_minimum,new RGBAColor(FULL_RAINBOW[4][1],FULL_RAINBOW[4][2],FULL_RAINBOW[4][3],0.0f));
		_controlPoint[5] =new ControlPoint(FULL_RAINBOW[5][0]*(_maximum-_minimum)+_minimum,new RGBAColor(FULL_RAINBOW[5][1],FULL_RAINBOW[5][2],FULL_RAINBOW[5][3],0.0f));
		last=new ControlPoint(FULL_RAINBOW[6][0]*(_maximum-_minimum)+_minimum,new RGBAColor(FULL_RAINBOW[6][1],FULL_RAINBOW[6][2],FULL_RAINBOW[6][3],0.0f));
		_controlPoint[6]=last;
		for (int i=0; i<6; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colormap==CINVERSE_FULL_RAINBOW) {
		first=new ControlPoint(INVERSE_FULL_RAINBOW[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_FULL_RAINBOW[0][1],INVERSE_FULL_RAINBOW[0][2],INVERSE_FULL_RAINBOW[0][3],0.0f));
		ControlPoint* _controlPoint[7];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(INVERSE_FULL_RAINBOW[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_FULL_RAINBOW[1][1],INVERSE_FULL_RAINBOW[1][2],INVERSE_FULL_RAINBOW[1][3],0.0f));
		_controlPoint[2] =new ControlPoint(INVERSE_FULL_RAINBOW[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_FULL_RAINBOW[2][1],INVERSE_FULL_RAINBOW[2][2],INVERSE_FULL_RAINBOW[2][3],0.0f));
		_controlPoint[3] =new ControlPoint(INVERSE_FULL_RAINBOW[3][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_FULL_RAINBOW[3][1],INVERSE_FULL_RAINBOW[3][2],INVERSE_FULL_RAINBOW[3][3],0.0f));
		_controlPoint[4] =new ControlPoint(INVERSE_FULL_RAINBOW[4][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_FULL_RAINBOW[4][1],INVERSE_FULL_RAINBOW[4][2],INVERSE_FULL_RAINBOW[4][3],0.0f));
		_controlPoint[5] =new ControlPoint(INVERSE_FULL_RAINBOW[5][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_FULL_RAINBOW[5][1],INVERSE_FULL_RAINBOW[5][2],INVERSE_FULL_RAINBOW[5][3],0.0f));
		last=new ControlPoint(INVERSE_FULL_RAINBOW[6][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_FULL_RAINBOW[6][1],INVERSE_FULL_RAINBOW[6][2],INVERSE_FULL_RAINBOW[6][3],0.0f));
		_controlPoint[6]=last;
		for (int i=0; i<6; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colormap==CRAINBOW) {
		first=new ControlPoint(RAINBOW[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(RAINBOW[0][1],RAINBOW[0][2],RAINBOW[0][3],0.0f));
		ControlPoint* _controlPoint[5];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(RAINBOW[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(RAINBOW[1][1],RAINBOW[1][2],RAINBOW[1][3],0.0f));
		_controlPoint[2] =new ControlPoint(RAINBOW[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(RAINBOW[2][1],RAINBOW[2][2],RAINBOW[2][3],0.0f));
		_controlPoint[3] =new ControlPoint(RAINBOW[3][0]*(_maximum-_minimum)+_minimum,new RGBAColor(RAINBOW[3][1],RAINBOW[3][2],RAINBOW[3][3],0.0f));
		last=new ControlPoint(RAINBOW[4][0]*(_maximum-_minimum)+_minimum,new RGBAColor(RAINBOW[4][1],RAINBOW[4][2],RAINBOW[4][3],0.0f));
		_controlPoint[4]=last;
		for (int i=0; i<4; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colormap==CINVERSE_RAINBOW) {
		first=new ControlPoint(INVERSE_RAINBOW[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_RAINBOW[0][1],INVERSE_RAINBOW[0][2],INVERSE_RAINBOW[0][3],0.0f));
		ControlPoint* _controlPoint[5];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(INVERSE_RAINBOW[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_RAINBOW[1][1],INVERSE_RAINBOW[1][2],INVERSE_RAINBOW[1][3],0.0f));
		_controlPoint[2] =new ControlPoint(INVERSE_RAINBOW[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_RAINBOW[2][1],INVERSE_RAINBOW[2][2],INVERSE_RAINBOW[2][3],0.0f));
		_controlPoint[3] =new ControlPoint(INVERSE_RAINBOW[3][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_RAINBOW[3][1],INVERSE_RAINBOW[3][2],INVERSE_RAINBOW[3][3],0.0f));
		last=new ControlPoint(INVERSE_RAINBOW[4][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_RAINBOW[4][1],INVERSE_RAINBOW[4][2],INVERSE_RAINBOW[4][3],0.0f));
		_controlPoint[4]=last;
		for (int i=0; i<4; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colormap==CCOLD_TO_HOT) {
		first=new ControlPoint(COLD_TO_HOT[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(COLD_TO_HOT[0][1],COLD_TO_HOT[0][2],COLD_TO_HOT[0][3],0.0f));
		ControlPoint* _controlPoint[3];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(COLD_TO_HOT[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(COLD_TO_HOT[1][1],COLD_TO_HOT[1][2],COLD_TO_HOT[1][3],0.0f));
		last=new ControlPoint(COLD_TO_HOT[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(COLD_TO_HOT[2][1],COLD_TO_HOT[2][2],COLD_TO_HOT[2][3],0.0f));
		_controlPoint[2]=last;
		for (int i=0; i<2; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colormap==CHOT_TO_COLD) {
		first=new ControlPoint(HOT_TO_COLD[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HOT_TO_COLD[0][1],HOT_TO_COLD[0][2],HOT_TO_COLD[0][3],0.0f));
		ControlPoint* _controlPoint[3];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(HOT_TO_COLD[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HOT_TO_COLD[1][1],HOT_TO_COLD[1][2],HOT_TO_COLD[1][3],0.0f));
		last=new ControlPoint(HOT_TO_COLD[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HOT_TO_COLD[2][1],HOT_TO_COLD[2][2],HOT_TO_COLD[2][3],0.0f));
		_controlPoint[2]=last;
		for (int i=0; i<2; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colormap==CBLACK_TO_WHITE) {
		ControlPoint* _controlPoint[2];
		first=new ControlPoint(BLACK_TO_WHITE[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(BLACK_TO_WHITE[0][1], BLACK_TO_WHITE[0][2], BLACK_TO_WHITE[0][3], 0.0f));
		last=new ControlPoint(BLACK_TO_WHITE[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(BLACK_TO_WHITE[1][1], BLACK_TO_WHITE[1][2], BLACK_TO_WHITE[1][3], 0.0f));
		_controlPoint[0]=first;
		_controlPoint[1]=last;
		for (int i=0; i<1; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colormap==CWHITE_TO_BLACK) {
		ControlPoint* _controlPoint[2];
		first=new ControlPoint(WHITE_TO_BLACK[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(WHITE_TO_BLACK[0][1], WHITE_TO_BLACK[0][2], WHITE_TO_BLACK[0][3], 0.0f));
		last=new ControlPoint(WHITE_TO_BLACK[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(WHITE_TO_BLACK[1][1], WHITE_TO_BLACK[1][2], WHITE_TO_BLACK[1][3], 0.0f));
		_controlPoint[0]=first;
		_controlPoint[1]=last;
		for (int i=0; i<1; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colormap==CHSB_HUES) {
		first=new ControlPoint(HSB_HUES[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HSB_HUES[0][1],HSB_HUES[0][2],HSB_HUES[0][3],0.0f));
		ControlPoint* _controlPoint[7];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(HSB_HUES[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HSB_HUES[1][1],HSB_HUES[1][2],HSB_HUES[1][3],0.0f));
		_controlPoint[2] =new ControlPoint(HSB_HUES[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HSB_HUES[2][1],HSB_HUES[2][2],HSB_HUES[2][3],0.0f));
		_controlPoint[3] =new ControlPoint(HSB_HUES[3][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HSB_HUES[3][1],HSB_HUES[3][2],HSB_HUES[3][3],0.0f));
		_controlPoint[4] =new ControlPoint(HSB_HUES[4][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HSB_HUES[4][1],HSB_HUES[4][2],HSB_HUES[4][3],0.0f));
		_controlPoint[5] =new ControlPoint(HSB_HUES[5][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HSB_HUES[5][1],HSB_HUES[5][2],HSB_HUES[5][3],0.0f));
		last=new ControlPoint(HSB_HUES[6][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HSB_HUES[6][1],HSB_HUES[6][2],HSB_HUES[6][3],0.0f));
		_controlPoint[6]=last;
		for (int i=0; i<6; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colormap==CINVERSE_HSB_HUES) {
		first=new ControlPoint(INVERSE_HSB_HUES[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_HSB_HUES[0][1],INVERSE_HSB_HUES[0][2],INVERSE_HSB_HUES[0][3],0.0f));
		ControlPoint* _controlPoint[7];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(INVERSE_HSB_HUES[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_HSB_HUES[1][1],INVERSE_HSB_HUES[1][2],INVERSE_HSB_HUES[1][3],0.0f));
		_controlPoint[2] =new ControlPoint(INVERSE_HSB_HUES[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_HSB_HUES[2][1],INVERSE_HSB_HUES[2][2],INVERSE_HSB_HUES[2][3],0.0f));
		_controlPoint[3] =new ControlPoint(INVERSE_HSB_HUES[3][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_HSB_HUES[3][1],INVERSE_HSB_HUES[3][2],INVERSE_HSB_HUES[3][3],0.0f));
		_controlPoint[4] =new ControlPoint(INVERSE_HSB_HUES[4][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_HSB_HUES[4][1],INVERSE_HSB_HUES[4][2],INVERSE_HSB_HUES[4][3],0.0f));
		_controlPoint[5] =new ControlPoint(INVERSE_HSB_HUES[5][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_HSB_HUES[5][1],INVERSE_HSB_HUES[5][2],INVERSE_HSB_HUES[5][3],0.0f));
		last=new ControlPoint(INVERSE_HSB_HUES[6][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_HSB_HUES[6][1],INVERSE_HSB_HUES[6][2],INVERSE_HSB_HUES[6][3],0.0f));
		_controlPoint[6]=last;
		for (int i=0; i<6; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colormap==CDAVINCI) {
		first=new ControlPoint(DAVINCI[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[0][1],DAVINCI[0][2],DAVINCI[0][3],0.0f));
		ControlPoint* _controlPoint[11];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(DAVINCI[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[1][1],DAVINCI[1][2],DAVINCI[1][3],0.0f));
		_controlPoint[2] =new ControlPoint(DAVINCI[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[2][1],DAVINCI[2][2],DAVINCI[2][3],0.0f));
		_controlPoint[3] =new ControlPoint(DAVINCI[3][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[3][1],DAVINCI[3][2],DAVINCI[3][3],0.0f));
		_controlPoint[4] =new ControlPoint(DAVINCI[4][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[4][1],DAVINCI[4][2],DAVINCI[4][3],0.0f));
		_controlPoint[5] =new ControlPoint(DAVINCI[5][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[5][1],DAVINCI[5][2],DAVINCI[5][3],0.0f));
		_controlPoint[6] =new ControlPoint(DAVINCI[6][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[6][1],DAVINCI[6][2],DAVINCI[6][3],0.0f));
		_controlPoint[7] =new ControlPoint(DAVINCI[7][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[7][1],DAVINCI[7][2],DAVINCI[7][3],0.0f));
		_controlPoint[8] =new ControlPoint(DAVINCI[8][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[8][1],DAVINCI[8][2],DAVINCI[8][3],0.0f));
		_controlPoint[9] =new ControlPoint(DAVINCI[9][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[9][1],DAVINCI[9][2],DAVINCI[9][3],0.0f));
		last=new ControlPoint(DAVINCI[10][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[10][1],DAVINCI[10][2],DAVINCI[10][3],0.0f));
		_controlPoint[10]=last;
		for (int i=0; i<10; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colormap==CINVERSE_DAVINCI) {
		first=new ControlPoint(INVERSE_DAVINCI[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[0][1],INVERSE_DAVINCI[0][2],INVERSE_DAVINCI[0][3],0.0f));
		ControlPoint* _controlPoint[11];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(INVERSE_DAVINCI[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[1][1],INVERSE_DAVINCI[1][2],INVERSE_DAVINCI[1][3],0.0f));
		_controlPoint[2] =new ControlPoint(INVERSE_DAVINCI[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[2][1],INVERSE_DAVINCI[2][2],INVERSE_DAVINCI[2][3],0.0f));
		_controlPoint[3] =new ControlPoint(INVERSE_DAVINCI[3][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[3][1],INVERSE_DAVINCI[3][2],INVERSE_DAVINCI[3][3],0.0f));
		_controlPoint[4] =new ControlPoint(INVERSE_DAVINCI[4][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[4][1],INVERSE_DAVINCI[4][2],INVERSE_DAVINCI[4][3],0.0f));
		_controlPoint[5] =new ControlPoint(INVERSE_DAVINCI[5][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[5][1],INVERSE_DAVINCI[5][2],INVERSE_DAVINCI[5][3],0.0f));
		_controlPoint[6] =new ControlPoint(INVERSE_DAVINCI[6][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[6][1],INVERSE_DAVINCI[6][2],INVERSE_DAVINCI[6][3],0.0f));
		_controlPoint[7] =new ControlPoint(INVERSE_DAVINCI[7][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[7][1],INVERSE_DAVINCI[7][2],INVERSE_DAVINCI[7][3],0.0f));
		_controlPoint[8] =new ControlPoint(INVERSE_DAVINCI[8][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[8][1],INVERSE_DAVINCI[8][2],INVERSE_DAVINCI[8][3],0.0f));
		_controlPoint[9] =new ControlPoint(INVERSE_DAVINCI[9][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[9][1],INVERSE_DAVINCI[9][2],INVERSE_DAVINCI[9][3],0.0f));
		last=new ControlPoint(INVERSE_DAVINCI[10][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[10][1],INVERSE_DAVINCI[10][2],INVERSE_DAVINCI[10][3],0.0f));
		_controlPoint[10]=last;
		for (int i=0; i<10; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colormap==CSEISMIC) {
		first=new ControlPoint(SEISMIC[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(SEISMIC[0][1],SEISMIC[0][2],SEISMIC[0][3],0.0f));
		ControlPoint* _controlPoint[3];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(SEISMIC[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(SEISMIC[1][1],SEISMIC[1][2],SEISMIC[1][3],0.0f));
		last=new ControlPoint(SEISMIC[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(SEISMIC[2][1],SEISMIC[2][2],SEISMIC[2][3],0.0f));
		_controlPoint[2]=last;
		for (int i=0; i<2; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colormap==CINVERSE_SEISMIC) {
		first=new ControlPoint(INVERSE_SEISMIC[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_SEISMIC[0][1],INVERSE_SEISMIC[0][2],INVERSE_SEISMIC[0][3],0.0f));
		ControlPoint* _controlPoint[3];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(INVERSE_SEISMIC[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_SEISMIC[1][1],INVERSE_SEISMIC[1][2],INVERSE_SEISMIC[1][3],0.0f));
		last=new ControlPoint(INVERSE_SEISMIC[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_SEISMIC[2][1],INVERSE_SEISMIC[2][2],INVERSE_SEISMIC[2][3],0.0f));
		_controlPoint[2]=last;
		for (int i=0; i<2; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	}
	updateControlPoints();
	ColorMapChangedCallbackData colorMapChangedCallbackData(this);
	colorMapChangedCallbacks.call(&colorMapChangedCallbackData);
}

/*
 * createColorMap - Create color map.
 *
 * parameter colorMapCreationType - int
 * parameter _minimum - double
 * parameter _maximum - double
 */
void ColorMap::createColorMap(int colorMapCreationType, double _minimum, double _maximum) {
	deleteColorMap();
	if (colorMapCreationType==CWHITE_TO_BLACK) {
		ControlPoint* _controlPoint[2];
		first=new ControlPoint(WHITE_TO_BLACK[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(WHITE_TO_BLACK[0][1], WHITE_TO_BLACK[0][2], WHITE_TO_BLACK[0][3], 0.0f));
		last=new ControlPoint(WHITE_TO_BLACK[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(WHITE_TO_BLACK[1][1], WHITE_TO_BLACK[1][2], WHITE_TO_BLACK[1][3], 0.0f));
		_controlPoint[0]=first;
		_controlPoint[1]=last;
		for (int i=0; i<1; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colorMapCreationType==CBLACK_TO_WHITE) {
		ControlPoint* _controlPoint[2];
		first=new ControlPoint(BLACK_TO_WHITE[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(BLACK_TO_WHITE[0][1], BLACK_TO_WHITE[0][2], BLACK_TO_WHITE[0][3], 0.0f));
		last=new ControlPoint(BLACK_TO_WHITE[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(BLACK_TO_WHITE[1][1], BLACK_TO_WHITE[1][2], BLACK_TO_WHITE[1][3], 0.0f));
		_controlPoint[0]=first;
		_controlPoint[1]=last;
		for (int i=0; i<1; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colorMapCreationType==CFULL_RAINBOW) {
		first=new ControlPoint(FULL_RAINBOW[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(FULL_RAINBOW[0][1],FULL_RAINBOW[0][2],FULL_RAINBOW[0][3],0.0f));
		ControlPoint* _controlPoint[7];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(FULL_RAINBOW[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(FULL_RAINBOW[1][1],FULL_RAINBOW[1][2],FULL_RAINBOW[1][3],0.0f));
		_controlPoint[2] =new ControlPoint(FULL_RAINBOW[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(FULL_RAINBOW[2][1],FULL_RAINBOW[2][2],FULL_RAINBOW[2][3],0.0f));
		_controlPoint[3] =new ControlPoint(FULL_RAINBOW[3][0]*(_maximum-_minimum)+_minimum,new RGBAColor(FULL_RAINBOW[3][1],FULL_RAINBOW[3][2],FULL_RAINBOW[3][3],0.0f));
		_controlPoint[4] =new ControlPoint(FULL_RAINBOW[4][0]*(_maximum-_minimum)+_minimum,new RGBAColor(FULL_RAINBOW[4][1],FULL_RAINBOW[4][2],FULL_RAINBOW[4][3],0.0f));
		_controlPoint[5] =new ControlPoint(FULL_RAINBOW[5][0]*(_maximum-_minimum)+_minimum,new RGBAColor(FULL_RAINBOW[5][1],FULL_RAINBOW[5][2],FULL_RAINBOW[5][3],0.0f));
		last=new ControlPoint(FULL_RAINBOW[6][0]*(_maximum-_minimum)+_minimum,new RGBAColor(FULL_RAINBOW[6][1],FULL_RAINBOW[6][2],FULL_RAINBOW[6][3],0.0f));
		_controlPoint[6]=last;
		for (int i=0; i<6; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colorMapCreationType==CINVERSE_FULL_RAINBOW) {
		first=new ControlPoint(INVERSE_FULL_RAINBOW[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_FULL_RAINBOW[0][1],INVERSE_FULL_RAINBOW[0][2],INVERSE_FULL_RAINBOW[0][3],0.0f));
		ControlPoint* _controlPoint[7];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(INVERSE_FULL_RAINBOW[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_FULL_RAINBOW[1][1],INVERSE_FULL_RAINBOW[1][2],INVERSE_FULL_RAINBOW[1][3],0.0f));
		_controlPoint[2] =new ControlPoint(INVERSE_FULL_RAINBOW[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_FULL_RAINBOW[2][1],INVERSE_FULL_RAINBOW[2][2],INVERSE_FULL_RAINBOW[2][3],0.0f));
		_controlPoint[3] =new ControlPoint(INVERSE_FULL_RAINBOW[3][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_FULL_RAINBOW[3][1],INVERSE_FULL_RAINBOW[3][2],INVERSE_FULL_RAINBOW[3][3],0.0f));
		_controlPoint[4] =new ControlPoint(INVERSE_FULL_RAINBOW[4][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_FULL_RAINBOW[4][1],INVERSE_FULL_RAINBOW[4][2],INVERSE_FULL_RAINBOW[4][3],0.0f));
		_controlPoint[5] =new ControlPoint(INVERSE_FULL_RAINBOW[5][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_FULL_RAINBOW[5][1],INVERSE_FULL_RAINBOW[5][2],INVERSE_FULL_RAINBOW[5][3],0.0f));
		last=new ControlPoint(INVERSE_FULL_RAINBOW[6][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_FULL_RAINBOW[6][1],INVERSE_FULL_RAINBOW[6][2],INVERSE_FULL_RAINBOW[6][3],0.0f));
		_controlPoint[6]=last;
		for (int i=0; i<6; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colorMapCreationType==CHSB_HUES) {
		first=new ControlPoint(HSB_HUES[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HSB_HUES[0][1],HSB_HUES[0][2],HSB_HUES[0][3],0.0f));
		ControlPoint* _controlPoint[7];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(HSB_HUES[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HSB_HUES[1][1],HSB_HUES[1][2],HSB_HUES[1][3],0.0f));
		_controlPoint[2] =new ControlPoint(HSB_HUES[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HSB_HUES[2][1],HSB_HUES[2][2],HSB_HUES[2][3],0.0f));
		_controlPoint[3] =new ControlPoint(HSB_HUES[3][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HSB_HUES[3][1],HSB_HUES[3][2],HSB_HUES[3][3],0.0f));
		_controlPoint[4] =new ControlPoint(HSB_HUES[4][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HSB_HUES[4][1],HSB_HUES[4][2],HSB_HUES[4][3],0.0f));
		_controlPoint[5] =new ControlPoint(HSB_HUES[5][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HSB_HUES[5][1],HSB_HUES[5][2],HSB_HUES[5][3],0.0f));
		last=new ControlPoint(HSB_HUES[6][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HSB_HUES[6][1],HSB_HUES[6][2],HSB_HUES[6][3],0.0f));
		_controlPoint[6]=last;
		for (int i=0; i<6; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colorMapCreationType==CINVERSE_HSB_HUES) {
		first=new ControlPoint(INVERSE_HSB_HUES[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_HSB_HUES[0][1],INVERSE_HSB_HUES[0][2],INVERSE_HSB_HUES[0][3],0.0f));
		ControlPoint* _controlPoint[7];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(INVERSE_HSB_HUES[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_HSB_HUES[1][1],INVERSE_HSB_HUES[1][2],INVERSE_HSB_HUES[1][3],0.0f));
		_controlPoint[2] =new ControlPoint(INVERSE_HSB_HUES[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_HSB_HUES[2][1],INVERSE_HSB_HUES[2][2],INVERSE_HSB_HUES[2][3],0.0f));
		_controlPoint[3] =new ControlPoint(INVERSE_HSB_HUES[3][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_HSB_HUES[3][1],INVERSE_HSB_HUES[3][2],INVERSE_HSB_HUES[3][3],0.0f));
		_controlPoint[4] =new ControlPoint(INVERSE_HSB_HUES[4][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_HSB_HUES[4][1],INVERSE_HSB_HUES[4][2],INVERSE_HSB_HUES[4][3],0.0f));
		_controlPoint[5] =new ControlPoint(INVERSE_HSB_HUES[5][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_HSB_HUES[5][1],INVERSE_HSB_HUES[5][2],INVERSE_HSB_HUES[5][3],0.0f));
		last=new ControlPoint(INVERSE_HSB_HUES[6][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_HSB_HUES[6][1],INVERSE_HSB_HUES[6][2],INVERSE_HSB_HUES[6][3],0.0f));
		_controlPoint[6]=last;
		for (int i=0; i<6; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colorMapCreationType==CDAVINCI) {
		first=new ControlPoint(DAVINCI[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[0][1],DAVINCI[0][2],DAVINCI[0][3],0.0f));
		ControlPoint* _controlPoint[11];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(DAVINCI[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[1][1],DAVINCI[1][2],DAVINCI[1][3],0.0f));
		_controlPoint[2] =new ControlPoint(DAVINCI[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[2][1],DAVINCI[2][2],DAVINCI[2][3],0.0f));
		_controlPoint[3] =new ControlPoint(DAVINCI[3][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[3][1],DAVINCI[3][2],DAVINCI[3][3],0.0f));
		_controlPoint[4] =new ControlPoint(DAVINCI[4][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[4][1],DAVINCI[4][2],DAVINCI[4][3],0.0f));
		_controlPoint[5] =new ControlPoint(DAVINCI[5][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[5][1],DAVINCI[5][2],DAVINCI[5][3],0.0f));
		_controlPoint[6] =new ControlPoint(DAVINCI[6][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[6][1],DAVINCI[6][2],DAVINCI[6][3],0.0f));
		_controlPoint[7] =new ControlPoint(DAVINCI[7][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[7][1],DAVINCI[7][2],DAVINCI[7][3],0.0f));
		_controlPoint[8] =new ControlPoint(DAVINCI[8][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[8][1],DAVINCI[8][2],DAVINCI[8][3],0.0f));
		_controlPoint[9] =new ControlPoint(DAVINCI[9][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[9][1],DAVINCI[9][2],DAVINCI[9][3],0.0f));
		last=new ControlPoint(DAVINCI[10][0]*(_maximum-_minimum)+_minimum,new RGBAColor(DAVINCI[10][1],DAVINCI[10][2],DAVINCI[10][3],0.0f));
		_controlPoint[10]=last;
		for (int i=0; i<10; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colorMapCreationType==CINVERSE_DAVINCI) {
		first=new ControlPoint(INVERSE_DAVINCI[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[0][1],INVERSE_DAVINCI[0][2],INVERSE_DAVINCI[0][3],0.0f));
		ControlPoint* _controlPoint[11];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(INVERSE_DAVINCI[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[1][1],INVERSE_DAVINCI[1][2],INVERSE_DAVINCI[1][3],0.0f));
		_controlPoint[2] =new ControlPoint(INVERSE_DAVINCI[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[2][1],INVERSE_DAVINCI[2][2],INVERSE_DAVINCI[2][3],0.0f));
		_controlPoint[3] =new ControlPoint(INVERSE_DAVINCI[3][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[3][1],INVERSE_DAVINCI[3][2],INVERSE_DAVINCI[3][3],0.0f));
		_controlPoint[4] =new ControlPoint(INVERSE_DAVINCI[4][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[4][1],INVERSE_DAVINCI[4][2],INVERSE_DAVINCI[4][3],0.0f));
		_controlPoint[5] =new ControlPoint(INVERSE_DAVINCI[5][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[5][1],INVERSE_DAVINCI[5][2],INVERSE_DAVINCI[5][3],0.0f));
		_controlPoint[6] =new ControlPoint(INVERSE_DAVINCI[6][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[6][1],INVERSE_DAVINCI[6][2],INVERSE_DAVINCI[6][3],0.0f));
		_controlPoint[7] =new ControlPoint(INVERSE_DAVINCI[7][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[7][1],INVERSE_DAVINCI[7][2],INVERSE_DAVINCI[7][3],0.0f));
		_controlPoint[8] =new ControlPoint(INVERSE_DAVINCI[8][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[8][1],INVERSE_DAVINCI[8][2],INVERSE_DAVINCI[8][3],0.0f));
		_controlPoint[9] =new ControlPoint(INVERSE_DAVINCI[9][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[9][1],INVERSE_DAVINCI[9][2],INVERSE_DAVINCI[9][3],0.0f));
		last=new ControlPoint(INVERSE_DAVINCI[10][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_DAVINCI[10][1],INVERSE_DAVINCI[10][2],INVERSE_DAVINCI[10][3],0.0f));
		_controlPoint[10]=last;
		for (int i=0; i<10; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colorMapCreationType==CRAINBOW) {
		first=new ControlPoint(RAINBOW[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(RAINBOW[0][1],RAINBOW[0][2],RAINBOW[0][3],0.0f));
		ControlPoint* _controlPoint[5];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(RAINBOW[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(RAINBOW[1][1],RAINBOW[1][2],RAINBOW[1][3],0.0f));
		_controlPoint[2] =new ControlPoint(RAINBOW[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(RAINBOW[2][1],RAINBOW[2][2],RAINBOW[2][3],0.0f));
		_controlPoint[3] =new ControlPoint(RAINBOW[3][0]*(_maximum-_minimum)+_minimum,new RGBAColor(RAINBOW[3][1],RAINBOW[3][2],RAINBOW[3][3],0.0f));
		last=new ControlPoint(RAINBOW[4][0]*(_maximum-_minimum)+_minimum,new RGBAColor(RAINBOW[4][1],RAINBOW[4][2],RAINBOW[4][3],0.0f));
		_controlPoint[4]=last;
		for (int i=0; i<4; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colorMapCreationType==CINVERSE_RAINBOW) {
		first=new ControlPoint(INVERSE_RAINBOW[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_RAINBOW[0][1],INVERSE_RAINBOW[0][2],INVERSE_RAINBOW[0][3],0.0f));
		ControlPoint* _controlPoint[5];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(INVERSE_RAINBOW[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_RAINBOW[1][1],INVERSE_RAINBOW[1][2],INVERSE_RAINBOW[1][3],0.0f));
		_controlPoint[2] =new ControlPoint(INVERSE_RAINBOW[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_RAINBOW[2][1],INVERSE_RAINBOW[2][2],INVERSE_RAINBOW[2][3],0.0f));
		_controlPoint[3] =new ControlPoint(INVERSE_RAINBOW[3][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_RAINBOW[3][1],INVERSE_RAINBOW[3][2],INVERSE_RAINBOW[3][3],0.0f));
		last=new ControlPoint(INVERSE_RAINBOW[4][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_RAINBOW[4][1],INVERSE_RAINBOW[4][2],INVERSE_RAINBOW[4][3],0.0f));
		_controlPoint[4]=last;
		for (int i=0; i<4; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colorMapCreationType==CCOLD_TO_HOT) {
		first=new ControlPoint(COLD_TO_HOT[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(COLD_TO_HOT[0][1],COLD_TO_HOT[0][2],COLD_TO_HOT[0][3],0.0f));
		ControlPoint* _controlPoint[3];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(COLD_TO_HOT[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(COLD_TO_HOT[1][1],COLD_TO_HOT[1][2],COLD_TO_HOT[1][3],0.0f));
		last=new ControlPoint(COLD_TO_HOT[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(COLD_TO_HOT[2][1],COLD_TO_HOT[2][2],COLD_TO_HOT[2][3],0.0f));
		_controlPoint[2]=last;
		for (int i=0; i<2; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colorMapCreationType==CHOT_TO_COLD) {
		first=new ControlPoint(HOT_TO_COLD[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HOT_TO_COLD[0][1],HOT_TO_COLD[0][2],HOT_TO_COLD[0][3],0.0f));
		ControlPoint* _controlPoint[3];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(HOT_TO_COLD[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HOT_TO_COLD[1][1],HOT_TO_COLD[1][2],HOT_TO_COLD[1][3],0.0f));
		last=new ControlPoint(HOT_TO_COLD[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(HOT_TO_COLD[2][1],HOT_TO_COLD[2][2],HOT_TO_COLD[2][3],0.0f));
		_controlPoint[2]=last;
		for (int i=0; i<2; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colorMapCreationType==CSEISMIC) {
		first=new ControlPoint(SEISMIC[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(SEISMIC[0][1],SEISMIC[0][2],SEISMIC[0][3],0.0f));
		ControlPoint* _controlPoint[3];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(SEISMIC[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(SEISMIC[1][1],SEISMIC[1][2],SEISMIC[1][3],0.0f));
		last=new ControlPoint(SEISMIC[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(SEISMIC[2][1],SEISMIC[2][2],SEISMIC[2][3],0.0f));
		_controlPoint[2]=last;
		for (int i=0; i<2; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	} else if (colorMapCreationType==CINVERSE_SEISMIC) {
		first=new ControlPoint(INVERSE_SEISMIC[0][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_SEISMIC[0][1],INVERSE_SEISMIC[0][2],INVERSE_SEISMIC[0][3],0.0f));
		ControlPoint* _controlPoint[3];
		_controlPoint[0]=first;
		_controlPoint[1] =new ControlPoint(INVERSE_SEISMIC[1][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_SEISMIC[1][1],INVERSE_SEISMIC[1][2],INVERSE_SEISMIC[1][3],0.0f));
		last=new ControlPoint(INVERSE_SEISMIC[2][0]*(_maximum-_minimum)+_minimum,new RGBAColor(INVERSE_SEISMIC[2][1],INVERSE_SEISMIC[2][2],INVERSE_SEISMIC[2][3],0.0f));
		_controlPoint[2]=last;
		for (int i=0; i<2; ++i) {
			_controlPoint[i+1]->left=_controlPoint[i];
			_controlPoint[i]->right=_controlPoint[i+1];
		}
	}
	valueRange.first=_minimum;
	valueRange.second=_maximum;
	updateControlPoints();
	ColorMapChangedCallbackData colorMapChangedCallbackData(this);
	colorMapChangedCallbacks.call(&colorMapChangedCallbackData);
}

/*
 * deleteColorMap - Delete the color map.
 */
void ColorMap::deleteColorMap(void) {
	if (controlPoint!=0) {
		ControlPointChangedCallbackData callbackData(this, controlPoint, 0);
		controlPoint=0;
		controlPointChangedCallbacks.call(&callbackData);
	}
	ControlPoint* controlPointPtr=first->right;
	while (controlPointPtr!=last) {
		ControlPoint* next=controlPointPtr->right;
		delete controlPointPtr;
		controlPointPtr=next;
	}
	first->right=last;
	last->left=first;
}

/*
 * deleteControlPoint - Delete selected control point.
 */
void ColorMap::deleteControlPoint(void) {
	if (controlPoint!=0&&controlPoint!=first&&controlPoint!=last) {
		ControlPoint* _controlPoint=controlPoint;
		ControlPointChangedCallbackData controlPointChangedCallbackData(this, controlPoint, 0);
		controlPoint=0;
		controlPointChangedCallbacks.call(&controlPointChangedCallbackData);
		_controlPoint->left->right=_controlPoint->right;
		_controlPoint->right->left=_controlPoint->left;
		delete _controlPoint;
		updateControlPoints();
		ColorMapChangedCallbackData colorMapChangedCallbackData(this);
		colorMapChangedCallbacks.call(&colorMapChangedCallbackData);
	}
}

/*
 * determineControlPoint - Determine if and which control point was selected.
 *
 * parameter event - GLMotif::Event&
 */
ControlPoint* ColorMap::determineControlPoint(GLMotif::Event& event) {
	GLfloat minimumDistanceSquared=Math::sqr(controlPointSize*1.5f);
	ControlPoint* _controlPoint=0;
	for (ControlPoint* controlPointPtr=first; controlPointPtr!=0; controlPointPtr=controlPointPtr->right) {
		GLMotif::Point currentPoint(controlPointPtr->x, controlPointPtr->y, colorMapAreaBox.getCorner(0)[2]);
		GLfloat distanceSquared=Geometry::sqrDist(currentPoint, event.getWidgetPoint().getPoint());
		if (minimumDistanceSquared>distanceSquared) {
			minimumDistanceSquared=distanceSquared;
			_controlPoint=controlPointPtr;
			for (int i=0; i<2; ++i)
				dragOffset[i]=Scalar(event.getWidgetPoint().getPoint()[i]-currentPoint[i]);
			dragOffset[2]=Scalar(0);
		}
	}
	return _controlPoint;
}
/*
 * draw - Draw the editable color map.
 *
 * parameter contextData - GLContextData&
 */
void ColorMap::draw(GLContextData& contextData) const {
	Widget::draw(contextData);
	drawMargin();
	GLboolean lightingEnabled=glIsEnabled(GL_LIGHTING);
	if (lightingEnabled)
		glDisable(GL_LIGHTING);
	drawColorMap();
	if (lightingEnabled)
		glEnable(GL_LIGHTING);
	GLfloat lineWidth;
	glGetFloatv(GL_LINE_WIDTH, &lineWidth);
	drawControlPoints();
	glLineWidth(lineWidth);
}

/*
 * drawColorMap - Draw color map.
 */
void ColorMap::drawColorMap(void) const {
	glBegin(GL_QUAD_STRIP);
	for (ControlPoint* controlPointPtr=first; controlPointPtr!=0; controlPointPtr=controlPointPtr->right) {
		glColor3f(controlPointPtr->rgbaColor->getValues(0), controlPointPtr->rgbaColor->getValues(1),
				controlPointPtr->rgbaColor->getValues(2));
		glVertex3f(controlPointPtr->x, colorMapAreaBox.getCorner(2)[1], colorMapAreaBox.getCorner(0)[2]);
		glVertex3f(controlPointPtr->x, colorMapAreaBox.getCorner(0)[1], colorMapAreaBox.getCorner(0)[2]);
	}
	glEnd();
}

/*
 * drawControlPoints - Draw control points.
 */
void ColorMap::drawControlPoints(void) const {
	GLfloat _normal=1.0f/Math::sqrt(3.0f);
	GLfloat y1=colorMapAreaBox.getCorner(0)[1];
	GLfloat y2=colorMapAreaBox.getCorner(2)[1];
	GLfloat z=colorMapAreaBox.getCorner(0)[2];
	glLineWidth(1.0f);
	glColor3f(0.0f, 0.0f, 0.0f);
	for (ControlPoint* controlPointPtr=first; controlPointPtr!=0; controlPointPtr=controlPointPtr->right) {
		glBegin(GL_LINE_STRIP);
		glVertex3f(controlPointPtr->x-controlPointSize*0.25f, y2, z);
		glVertex3f(controlPointPtr->x-controlPointSize*0.25f, y1, z);
		glEnd();
		glBegin(GL_LINE_STRIP);
		glVertex3f(controlPointPtr->x+controlPointSize*0.25f, y2, z);
		glVertex3f(controlPointPtr->x+controlPointSize*0.25f, y1, z);
		glEnd();
	}
	glBegin(GL_TRIANGLES);
	for (ControlPoint* controlPointPtr=first; controlPointPtr!=0; controlPointPtr=controlPointPtr->right) {
		if (controlPointPtr==controlPoint)
			glColor3f(controlPointColor->getValues(0), controlPointColor->getValues(1), controlPointColor->getValues(2));
		else
			glColor(foregroundColor);
		glNormal3f(-_normal, _normal, _normal);
		glVertex3f(controlPointPtr->x-controlPointSize, y1, z);
		glVertex3f(controlPointPtr->x, y1, z+controlPointSize);
		glVertex3f(controlPointPtr->x, y1+controlPointSize, z);
		glNormal3f(_normal, _normal, _normal);
		glVertex3f(controlPointPtr->x, y1+controlPointSize, z);
		glVertex3f(controlPointPtr->x, y1, z+controlPointSize);
		glVertex3f(controlPointPtr->x+controlPointSize, y1, z);
		glNormal3f(_normal, -_normal, _normal);
		glVertex3f(controlPointPtr->x+controlPointSize, y1, z);
		glVertex3f(controlPointPtr->x, y1, z+controlPointSize);
		glVertex3f(controlPointPtr->x, y1-controlPointSize, z);
		glNormal3f(-_normal, -_normal, _normal);
		glVertex3f(controlPointPtr->x, y1-controlPointSize, z);
		glVertex3f(controlPointPtr->x, y1, z+controlPointSize);
		glVertex3f(controlPointPtr->x-controlPointSize, y1, z);
	}
	glEnd();
}

/*
 * drawMargin - Draw margin area in background color.
 */
void ColorMap::drawMargin(void) const {
	glColor(backgroundColor);
	glBegin(GL_QUADS);
	glNormal3f(0.0f, 0.0f, 1.0f);
	glVertex(getInterior().getCorner(0));
	glVertex(colorMapAreaBox.getCorner(0));
	glVertex(colorMapAreaBox.getCorner(2));
	glVertex(getInterior().getCorner(2));
	glVertex(getInterior().getCorner(1));
	glVertex(getInterior().getCorner(3));
	glVertex(colorMapAreaBox.getCorner(3));
	glVertex(colorMapAreaBox.getCorner(1));
	glVertex(getInterior().getCorner(0));
	glVertex(getInterior().getCorner(1));
	glVertex(colorMapAreaBox.getCorner(1));
	glVertex(colorMapAreaBox.getCorner(0));
	glVertex(getInterior().getCorner(2));
	glVertex(colorMapAreaBox.getCorner(2));
	glVertex(colorMapAreaBox.getCorner(3));
	glVertex(getInterior().getCorner(3));
	glEnd();
}

/*
 * exportColorMap - Export the current color map.
 *
 * parameter colormap - double*
 */
void ColorMap::exportColorMap(double* colormap) const {
	int numberOfEntries=256;
	for (int i=0; i<numberOfEntries; ++i) {
		double value=double(i)*(valueRange.second-valueRange.first)/double(numberOfEntries-1)+valueRange.first;
		ControlPoint* previousControlPoint=first;
		ControlPoint* nextControlPoint;
		for (nextControlPoint=previousControlPoint->right; nextControlPoint!=last&&nextControlPoint->value<value;
				previousControlPoint =nextControlPoint, nextControlPoint=nextControlPoint->right)
			;
		GLfloat w2=GLfloat((value-previousControlPoint->value)/(nextControlPoint->value-previousControlPoint->value));
		GLfloat w1=GLfloat((nextControlPoint->value-value)/(nextControlPoint->value-previousControlPoint->value));
		for (int j=0; j<3; ++j)
			colormap[4*i+j]=(double)((previousControlPoint->rgbaColor->getValues(j)*w1
					+nextControlPoint->rgbaColor->getValues(j)*w2));
		colormap[4*i+3]=(double)(1.0);
	}
}

/*
 * findRecipient - Determine which the applicable widget of the event. A virtual function of GLMotif::Widget base.
 *
 * parameter event - GLMotif::Event&
 * return - bool
 */
bool ColorMap::findRecipient(GLMotif::Event& event) {
	if (isDragging) {
		return event.setTargetWidget(this, event.calcWidgetPoint(this));
	} else
		return GLMotif::Widget::findRecipient(event);
}

/*
 * getColorMap - Get the color map.
 *
 * return - Storage*
 */
Storage* ColorMap::getColorMap(void) const {
	return new Storage(first);
}

/*
 * setColorMap - Set control points for the color map.
 *
 * parameter _storage - Storage*
 */
void ColorMap::setColorMap(Storage* _storage) {
	deleteColorMap();
	first->value=_storage->getValues(0);
	first->rgbaColor=_storage->getRGBAColors(0);
	ControlPoint* leftControlPoint=first;
	for (int i=1; i<_storage->getNumberOfControlPoints()-1; ++i) {
		ControlPoint* _controlPoint=new ControlPoint(_storage->getValues(i),_storage->getRGBAColors(i));
		_controlPoint->left=leftControlPoint;
		leftControlPoint->right=_controlPoint;
		leftControlPoint=_controlPoint;
	}
	last->value=_storage->getValues(_storage->getNumberOfControlPoints()-1);
	last->rgbaColor=_storage->getRGBAColors(_storage->getNumberOfControlPoints()-1);
	last->left=leftControlPoint;
	leftControlPoint->right=last;
	valueRange.first=first->value;
	valueRange.second=last->value;
	updateControlPoints();
	ColorMapChangedCallbackData colorMapChangedCallbackData(this);
	colorMapChangedCallbacks.call(&colorMapChangedCallbackData);
}

Misc::CallbackList& ColorMap::getColorMapChangedCallbacks(void) {
	return colorMapChangedCallbacks;
}

Misc::CallbackList& ColorMap::getControlPointChangedCallbacks(void) {
	return controlPointChangedCallbacks;
}

RGBAColor* ColorMap::getControlPointColor(void) {
	return controlPoint->rgbaColor;
}

/*
 * setControlPointColor - Set current control point color.
 *
 * parameter _rgbaColor - RGBAColor
 */
void ColorMap::setControlPointColor(RGBAColor rgbaColor) {
	if (controlPoint!=0) {
		for (int i=0; i<3; ++i)
			controlPoint->rgbaColor->setValues(i, rgbaColor.getValues(i));
		updateControlPoints();
		ColorMapChangedCallbackData colorMapChangedCallbackData(this);
		colorMapChangedCallbacks.call(&colorMapChangedCallbackData);
	}
}

/*
 * setControlPointColor - Set current control point color.
 *
 * parameter _rgbaColor - RGBAColor*
 */
void ColorMap::setControlPointColor(RGBAColor* _rgbaColor) {
	controlPointColor=_rgbaColor;
}

/*
 * setControlPointSize - Set control point size.
 *
 * parameter _controlPointSize - GLfloat
 */
void ColorMap::setControlPointSize(GLfloat _controlPointSize) {
	controlPointSize=_controlPointSize;
}

/*
 * setControlPointValue - Set current control point value.
 *
 * parameter _value - double
 */
void ColorMap::setControlPointValue(double _value) {
	if (controlPoint!=0&&controlPoint->left!=0&&controlPoint->right!=0) {
		if (_value<first->value)
			controlPoint->value=first->value;
		else if (_value>last->value)
			controlPoint->value=last->value;
		else
			controlPoint->value=_value;
		updateControlPoints();
		ColorMapChangedCallbackData colorMapChangedCallbackData(this);
		colorMapChangedCallbacks.call(&colorMapChangedCallbackData);
	}
}

/*
 * setMarginWidth - Set the margin width.
 *
 * parameter _margineWidth - GLfloat
 */
void ColorMap::setMarginWidth(GLfloat _marginWidth) {
	marginWidth=_marginWidth;
	if (isManaged) {
		parent->requestResize(this, calcNaturalSize());
	} else
		resize(GLMotif::Box(GLMotif::Vector(0.0f, 0.0f, 0.0f), calcNaturalSize()));
}

/*
 * getNumberOfControlPoints - Get number of control points.
 *
 * return - int
 */
int ColorMap::getNumberOfControlPoints(void) const {
	int numberOfControlPoints=0;
	for (ControlPoint* controlPointPtr=first; controlPointPtr!=0; controlPointPtr=controlPointPtr->right)
		++numberOfControlPoints;
	return numberOfControlPoints;
}

/*
 * setPreferredSize - Set color map preferred size.
 *
 * parameter _preferredSize - const GLMotif::Vector&
 */
void ColorMap::setPreferredSize(const GLMotif::Vector& _preferredSize) {
	preferredSize=_preferredSize;
	if (isManaged) {
		parent->requestResize(this, calcNaturalSize());
	} else
		resize(GLMotif::Box(GLMotif::Vector(0.0f, 0.0f, 0.0f), calcNaturalSize()));
}

const std::pair<double,double>& ColorMap::getValueRange(void) const {
	return valueRange;
}

/*
 * insertControlPoint - Insert control point.
 *
 * parameter _value - double
 */
void ColorMap::insertControlPoint(double _value) {
	ControlPoint* previousControlPoint=first;
	ControlPoint* nextControlPoint;
	for (nextControlPoint=previousControlPoint->right; nextControlPoint!=last&&nextControlPoint->value <_value;
			previousControlPoint=nextControlPoint, nextControlPoint=nextControlPoint->right)
		;
	GLfloat w2=GLfloat((_value-previousControlPoint->value)/(nextControlPoint->value -previousControlPoint->value));
	GLfloat w1=GLfloat((nextControlPoint->value-_value)/(nextControlPoint->value -previousControlPoint->value));
	RGBAColor* rgbaColor = new RGBAColor(0.0f,0.0f,0.0f,0.0f);
	for (int i=0; i<4; ++i)
		rgbaColor->setValues(i, previousControlPoint->rgbaColor->getValues(i)*w1+nextControlPoint->rgbaColor->getValues(i)*w2);
	ControlPoint* _controlPoint=new ControlPoint(_value,rgbaColor);
	_controlPoint->left=previousControlPoint;
	previousControlPoint->right=_controlPoint;
	_controlPoint->right=nextControlPoint;
	nextControlPoint->left=_controlPoint;
	updateControlPoints();
	ColorMapChangedCallbackData colorMapChangedCallbackData(this);
	colorMapChangedCallbacks.call(&colorMapChangedCallbackData);
	ControlPointChangedCallbackData controlPointChangedCallbackData(this, controlPoint, _controlPoint);
	controlPoint=_controlPoint;
	controlPointChangedCallbacks.call(&controlPointChangedCallbackData);
}

/*
 * pointerButtonDown - Pointer button down event handler. A virtual function of GLMotif::Widget base.
 *
 * parameter event - GLMotif::Event&
 */
void ColorMap::pointerButtonDown(GLMotif::Event& event) {
	ControlPoint* _controlPoint=determineControlPoint(event);
	if (_controlPoint!=controlPoint) {
		ControlPointChangedCallbackData callbackData(this, controlPoint, _controlPoint);
		controlPoint=_controlPoint;
		controlPointChangedCallbacks.call(&callbackData);
	} else if (_controlPoint==0) {
		double _value=(event.getWidgetPoint().getPoint()[0]-double(colorMapAreaBox.getCorner(0)[0]))*(valueRange.second-valueRange.first)/double(colorMapAreaBox.getCorner(1)[0]-colorMapAreaBox.getCorner(0)[0])+valueRange.first;
		if (_value<=valueRange.first || _value>=valueRange.second) {
			_controlPoint=controlPoint;
		} else {
			insertControlPoint(_value);
		}
	} else if (_controlPoint==controlPoint) {
		isDragging=true;
	}
}

/*
 * pointerButtonUp - Pointer button up event handler. A virtual function of GLMotif::Widget base.
 *
 * parameter event - GLMotif::Event&
 */
void ColorMap::pointerButtonUp(GLMotif::Event& event) {
	if (isDragging) {
		isDragging=false;
	}
}

/*
 * pointerMotion - Pointer motion event handler. A virtual function of GLMotif::Widget base.
 *
 * parameter event - GLMotif::Event&
 */
void ColorMap::pointerMotion(GLMotif::Event& event) {
	if (isDragging) {
		GLMotif::Point _point=event.getWidgetPoint().getPoint()-dragOffset;
		double _value=(_point[0]-double(colorMapAreaBox.getCorner(0)[0]))*(valueRange.second-valueRange.first)/double(colorMapAreaBox.getCorner(1)[0]-colorMapAreaBox.getCorner(0)[0])+valueRange.first;
		if (controlPoint==first)
			_value=valueRange.first;
		else if (controlPoint==last)
			_value=valueRange.second;
		else if (_value<controlPoint->left->value)
			_value=controlPoint->left->value;
		else if (_value>controlPoint->right->value)
			_value=controlPoint->right->value;
		GLfloat _opacity=0.0f;
		controlPoint->value=_value;
		controlPoint->rgbaColor->setValues(3, _opacity);
		updateControlPoints();
		ColorMapChangedCallbackData colorMapChangedCallbackData(this);
		colorMapChangedCallbacks.call(&colorMapChangedCallbackData);
	}
}

/*
 * resize - Resize the color map display. A virtual function of GLMotif::Widget base.
 *
 * parameter _exterior - const GLMotif::Box&
 */
void ColorMap::resize(const GLMotif::Box& _exterior) {
	GLMotif::Widget::resize(_exterior);
	colorMapAreaBox=getInterior();
	colorMapAreaBox.doInset(GLMotif::Vector(marginWidth, marginWidth, 0.0f));
	updateControlPoints();
}

/*
 * selectControlPoint - Select control point.
 *
 * parameter i - int
 */
void ColorMap::selectControlPoint(int i) {
	ControlPoint* controlPointPtr=0;
	if (i>=0)
		for (controlPointPtr=first; i>0&&controlPointPtr!=0; controlPointPtr=controlPointPtr->right, --i)
			;
	ControlPointChangedCallbackData callbackData(this, controlPoint, controlPointPtr);
	controlPoint=controlPointPtr;
	controlPointChangedCallbacks.call(&callbackData);
}

/*
 * updateControlPoints - Update the control points.
 */
void ColorMap::updateControlPoints(void) {
	GLfloat x1=colorMapAreaBox.getCorner(0)[0];
	GLfloat x2=colorMapAreaBox.getCorner(1)[0];
	GLfloat y1=colorMapAreaBox.getCorner(0)[1];
	GLfloat y2=colorMapAreaBox.getCorner(2)[1];
	for (ControlPoint* controlPointPtr=first; controlPointPtr!=0; controlPointPtr=controlPointPtr->right) {
		controlPointPtr->x=GLfloat((controlPointPtr->value-valueRange.first)/(valueRange.second -valueRange.first))*(x2-x1)+x1;
		controlPointPtr->y=(controlPointPtr->rgbaColor->getValues(3)-0.0f)*(y2-y1)/(1.0f-0.0f)+y1;
	}
}
