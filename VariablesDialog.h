#ifndef VARIABLESDIALOG_INCLUDED
#define VARIABLESDIALOG_INCLUDED

#include <GLMotif/PopupWindow.h>

#include <string>
#include <vector>

namespace GLMotif {
class ScrolledListBox;
class ToggleButton;
} // end namespace GLMotif

/* Dialog for active variable selection. */
class VariablesDialog : public GLMotif::PopupWindow
{
public:
  VariablesDialog();
  ~VariablesDialog();

  void clearAllVariables();

  void addVariable(const std::string &var);

  GLMotif::ScrolledListBox *getScrolledListBox() { return List; }

private:
  GLMotif::ScrolledListBox *List;
};

#endif // VARIABLESDIALOG_INCLUDED
