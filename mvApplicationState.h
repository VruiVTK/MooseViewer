#ifndef MVAPPLICATIONSTATE_H
#define MVAPPLICATIONSTATE_H

#include <vvApplicationState.h>

#include <vtkTimeStamp.h>

#include <string>
#include <vector>

class mvContours;
class mvGeometry;
class mvInteractor;
class mvOutline;
class mvReader;
class mvSlice;
class mvVolume;
class vtkExodusIIReader;
class vtkLookupTable;
class WidgetHints;

/**
 * @brief The mvApplicationState class holds MooseViewer application state.
 *
 * mvApplicationState holds application state needed by vvGLObjects.
 * An instance of this class is used to update vvGLObjects via
 * vvGLObject::syncApplicationState in MooseViewer::frame().
 */
class mvApplicationState : public vvApplicationState
{
public:
  using Superclass = vvApplicationState;
  mvApplicationState();
  ~mvApplicationState();

  void init() override;

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

  /** Main polydata geometry rendering. */
  mvGeometry& geometry() { return *m_geometry; }
  const mvGeometry& geometry() const { return *m_geometry; }

  /** Analysis tool interaction. */
  mvInteractor& interactor() { return *m_interactor; }
  const mvInteractor& interactor() const { return *m_interactor; }

  /** Render dataset outline. */
  mvOutline& outline() { return *m_outline; }
  const mvOutline& outline() const { return *m_outline; }

  /** File reader.
   * Access is not const-correct because VTK is not const-correct. */
  mvReader& reader() const { return *m_reader; }

  /** Slicer. */
  mvSlice& slice() { return *m_slice; }
  const mvSlice& slice() const { return *m_slice; }

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

  vtkLookupTable *m_colorMap;
  std::string m_colorByArray;
  vtkTimeStamp m_colorByMTime;
  mvContours *m_contours;
  mvGeometry *m_geometry;
  mvInteractor *m_interactor;
  mvOutline *m_outline;
  mvReader *m_reader;
  mvSlice *m_slice;
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
