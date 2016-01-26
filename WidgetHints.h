#ifndef WIDGETHINTS_H
#define WIDGETHINTS_H

#include <string>

/**
 * @brief The WidgetHint class manages information about initial widget states.
 *
 * Certain UI elements can be modified by providing hints in a JSON file. The
 * file is read once at application startup, and hints are applied during the
 * initial creation of the widgets.
 *
 * The format of the file is a tree of JSON objects with the following form:
 *
 * {
 *   "TopLevelWidget": {
 *     "enabled": true,
 *     "MenuWidget": {
 *       "enabled": false,
 *     },
 *     "AnotherMenuWidget": {
 *       "SomeButton": {
 *         "enabled": false
 *       }
 *     }
 *   }
 * }
 *
 * In this example, MenuWidget (and its children) and SomeButton will not be
 * created.
 *
 * Note the following conventions:
 * - Widget names start with a capital letter.
 * - Hints start with a lowercase level.
 *
 * Currently, the only hint supported is 'enabled', which always defaults to
 * 'true' if not specified. Marking a widget as disabled will prevent the
 * widget from being instantiated during UI creation, as GLMotif does not
 * support hiding widgets.
 *
 * Widgets are looked up using a path-like specification of names. For example,
 * SomeButton in the example above is located at
 * "/TopLevelWidget/AnotherMenuWidget/SomeButton".
 *
 * The pushGroup and popGroup methods are used to simplify lookups. Instead of
 * testing SomeButton's enabled status the obvious way:
 *
 * widgetHints->isEnabled("/TopLevelWidget/AnotherMenuWidget/SomeButton");
 *
 * the group system can be used to help traverse the tree:
 *
 * widgetHints->pushGroup("TopLevelWidget");
 * widgetHints->pushGroup("AnotherMenuWidget");
 * widgetHints->isEnabled("SomeButton");
 * widgetHints->popGroup();
 * widgetHints->popGroup();
 *
 */
class WidgetHints
{
public:
  WidgetHints();
  ~WidgetHints();

  /** Reset all hints. */
  void reset();

  /** Load hints from the JSON hint file at @a fileName. */
  bool loadFile(const std::string &fileName);

  /** Push a path prefix onto the group stack. */
  void pushGroup(const std::string &group);

  /** Pop the last group off of the group stack. */
  void popGroup();

  /** Return true if the widget at @a name should be used. */
  bool isEnabled(const std::string &name) const;

private:
  struct Internal;
  Internal *Internals;
};

#endif // WIDGETVISIBILITY_H
