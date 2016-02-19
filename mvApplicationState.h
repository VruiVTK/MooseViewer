#ifndef MVAPPLICATIONSTATE_H
#define MVAPPLICATIONSTATE_H

class mvContours;
class mvFramerate;
class mvGeometry;
class mvGLObject;
class mvOutline;
class mvProgress;
class mvReader;
class mvVolume;
class vtkExodusIIReader;
class vtkLookupTable;
class WidgetHints;

#include <vtkTimeStamp.h>

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
  using Objects = std::vector<mvGLObject*>;

  mvApplicationState();
  ~mvApplicationState();

  /** List of all mvGLObjects. */
  Objects& objects() { return m_objects; }
  const Objects& objects() const { return m_objects; }

  /** Currently selected array for scalar color mapping, etc. */
  const std::string& colorByArray() const { return m_colorByArray; }
  void setColorByArray(const std::string &a);
  unsigned long colorByMTime() const { return m_colorByMTime.GetMTime(); }

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

  /** Render dataset outline. */
  mvOutline& outline() { return *m_outline; }
  const mvOutline& outline() const { return *m_outline; }

  /** Async progress monitor.
   * Unfortunately not const-correct, since this needs to be modified in
   * mvAsyncGLObject::syncApplicationState(const mvAppState&). */
  mvProgress& progress() const { return *m_progress; }

  /** File reader.
   * Access is not const-correct because VTK is not const-correct. */
  mvReader& reader() const { return *m_reader; }

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
  std::string m_colorByArray;
  vtkTimeStamp m_colorByMTime;
  mvContours *m_contours;
  mvFramerate *m_framerate;
  mvGeometry *m_geometry;
  mvOutline *m_outline;
  mvProgress *m_progress;
  mvReader *m_reader;
  mvVolume *m_volume;
  WidgetHints *m_widgetHints;
};

inline void mvApplicationState::setColorByArray(const std::string &a)
{
  if (a != m_colorByArray)
    {
    m_colorByArray = a;
    m_colorByMTime.Modified();
    }
}

#endif // MVAPPLICATIONSTATE_H
