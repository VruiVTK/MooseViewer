#ifndef MVAPPLICATIONSTATE_H
#define MVAPPLICATIONSTATE_H

class ArrayLocator;
class mvContours;
class mvVolume;
class mvFramerate;
class mvGeometry;
class mvGLObject;
class mvOutline;
class vtkExodusIIReader;
class vtkLookupTable;
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

  /** Color map.
   * Access is not const-correct because VTK is not const-correct. */
  vtkLookupTable& colorMap() const { return *m_colorMap; }

  /** Contouring rendering object. */
  mvContours& contours() { return *m_contours; }
  const mvContours& contours() const { return *m_contours; }

  /** Framerate overlay. */
  mvFramerate& framerate() { return *m_framerate; }
  const mvFramerate& framerate() const { return *m_framerate; }

  /** Main polydata geometry rendering. */
  mvGeometry& geometry() { return *m_geometry; }
  const mvGeometry& geometry() const { return *m_geometry; }

  /** Array locator. */
  ArrayLocator& locator() { return *m_locator; }
  const ArrayLocator& locator() const { return *m_locator; }

  /** Render dataset outline. */
  mvOutline& outline() { return *m_outline; }
  const mvOutline& outline() const { return *m_outline; }

  /** File reader.
   * Access is not const-correct because VTK is not const-correct. */
  vtkExodusIIReader& reader() const { return *m_reader; }

  /** Volume rendering object. */
  mvVolume& volume() { return *m_volume; }
  const mvVolume& volume() const { return *m_volume; }

  /** UI hints from JSON file. */
  WidgetHints &widgetHints() { return *m_widgetHints; }
  const WidgetHints &widgetHints() const { return *m_widgetHints; }

private:
  // Not implemented:
  mvApplicationState(const mvApplicationState&);
  mvApplicationState& operator=(const mvApplicationState&);

  Objects m_objects;

  vtkLookupTable *m_colorMap;
  mvContours *m_contours;
  mvFramerate *m_framerate;
  mvGeometry *m_geometry;
  ArrayLocator *m_locator;
  mvOutline *m_outline;
  vtkExodusIIReader *m_reader;
  mvVolume *m_volume;
  WidgetHints *m_widgetHints;
};

#endif // MVAPPLICATIONSTATE_H
