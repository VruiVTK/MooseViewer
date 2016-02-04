#ifndef MVAPPLICATIONSTATE_H
#define MVAPPLICATIONSTATE_H

class ArrayLocator;
class mvContours;
class mvGLObject;
class vtkExodusIIReader;
class WidgetHints;

#include <string>
#include <vector>

/**
 * @brief The mvApplicationState class holds MooseViewer application state.
 *
 * mvApplicationState holds application state needed by mvGLObjects.
 * An instance of this class is used to update mvGLObjects via
 * mvGLObject::syncApplicationState in MooseViewer::frame().
 */
class mvApplicationState
{
public:
  typedef std::vector<mvGLObject*> Objects;

  mvApplicationState();
  ~mvApplicationState();

  /** List of all mvGLObjects. */
  Objects& objects() { return m_objects; }
  const Objects& objects() const { return m_objects; }

  /** Contouring rendering object. */
  mvContours& contours() { return *m_contours; }
  const mvContours& contours() const { return *m_contours; }

  /** Array locator. */
  ArrayLocator& locator() { return *m_locator; }
  const ArrayLocator& locator() const { return *m_locator; }

  /** File reader.
   * Access is not const-correct because VTK is not const-correct. */
  vtkExodusIIReader& reader() const { return *m_reader; }

  /** UI hints from JSON file. */
  WidgetHints &widgetHints() { return *m_widgetHints; }
  const WidgetHints &widgetHints() const { return *m_widgetHints; }

private:
  // Not implemented:
  mvApplicationState(const mvApplicationState&);
  mvApplicationState& operator=(const mvApplicationState&);

  Objects m_objects;

  mvContours *m_contours;
  ArrayLocator *m_locator;
  vtkExodusIIReader *m_reader;
  WidgetHints *m_widgetHints;
};

#endif // MVAPPLICATIONSTATE_H
