#include "VariablesDialog.h"

#include <Vrui/Vrui.h>

#include <GLMotif/ScrolledListBox.h>
#include <GLMotif/ToggleButton.h>

using GLMotif::ListBox;
using GLMotif::PopupWindow;
using GLMotif::ScrolledListBox;

//------------------------------------------------------------------------------
VariablesDialog::VariablesDialog()
  : PopupWindow("Variables", Vrui::getWidgetManager(), "Active Variables"),
    List(new ScrolledListBox("VariableList", this, ListBox::MULTIPLE, 20, 8))
{
}

//------------------------------------------------------------------------------
VariablesDialog::~VariablesDialog()
{
}

//------------------------------------------------------------------------------
void VariablesDialog::clearAllVariables()
{
  this->List->getListBox()->clear();
}

//------------------------------------------------------------------------------
void VariablesDialog::addVariable(const std::string &var)
{
  this->List->getListBox()->addItem(var.c_str());
}
