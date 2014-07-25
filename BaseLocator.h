#ifndef BASELOCATOR_H_
#define BASELOCATOR_H_

/* Vrui includes */
#include <Vrui/LocatorToolAdapter.h>

// begin Forward Declarations
namespace Vrui {
class LocatorTool;
}
class MooseViewer;
// end Forward Declarations
class BaseLocator : public Vrui::LocatorToolAdapter {
public:
	BaseLocator(Vrui::LocatorTool* _locatorTool, MooseViewer* _application);
	~BaseLocator();
	virtual void highlightLocator(GLContextData& contextData) const;
	virtual void glRenderAction(GLContextData& contextData) const;
	virtual void glRenderActionTransparent(GLContextData& contextData) const;
private:
	MooseViewer* application;
};

#endif /*BASELOCATOR_H_*/
