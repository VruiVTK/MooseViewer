#ifndef MVREADER_H
#define MVREADER_H

#include <vtkBoundingBox.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <chrono>
#include <future>
#include <map>
#include <set>
#include <limits>
#include <string>
#include <vector>

class vtkExodusIIReader;
class vtkMultiBlockDataSet;

/**
 * @brief The mvReader class manages loading the current dataset from a
 * file.
 *
 * This class manages the current dataset. It provides metadata and performs
 * asynchronous updates, rereading the data from an Exodus II file as the
 * reading parameters change.
 *
 * It is important to keep in mind that the data is read asynchronously. For
 * example, if a new variable is requested and then update() is called, the new
 * variable will not be available yet. It will become available on the first
 * call to update() after the asynchronous read completes.
 *
 * The updateInformation() method does execute synchronously, as long as no
 * background update is in progress.
 */
class mvReader
{
public:
  struct VariableMetaData;
  typedef std::set<std::string> Variables;
  typedef std::map<std::string, VariableMetaData> VariableMetaDataMap;

  mvReader();
  ~mvReader();

  /**
   * Trigger an update cycle. This is the synchronization point between the
   * background reading thread and the main GUI thread. It will start a new
   * asynchronous read if the reading parameters have changed, and it will
   * update heavier data (e.g. dataObject(), variableMetaData(), ...) the
   * first time it is called after a background read completes.
   */
  void update();

  /**
   * This immediately and synchronously reads the file's metadata and updates
   * lightweight data, such as the list of available data.
   *
   * This method does nothing if the file reader is already doing an
   * asynchronous read from a call to update().
   */
  void updateInformation();

  /**
   * Returns true if the background thread is running, false, otherwise.
   *
   * If a non-zero @a duration is set, the background process will be given
   * the specified amount of time to complete and this function will block. If
   * @a duration is zero, the call will not block.
   *
   * @note This method should be use very sparingly. Rather than synchronizing
   * data changes using this method, write code that is capable of waiting for
   * the result to be available in a later renderframe.
   */
  template <typename Rep, typename Period>
  bool running(const std::chrono::duration<Rep, Period> &duration =
               std::chrono::seconds(0)) const;

  /**
   * The name of the Exodus II file to read.
   * @{
   */
  void setFileName(const std::string &name) { m_fileName = name; }
  const std::string& fileName() const { return m_fileName; }
  /** @} */

  /**
   * The variables (i.e. attribute arrays) that can be read from the file.
   * This data is populated by updateInformation().
   */
  const Variables& availableVariables() const { return m_availableVariables; }

  /** Return true if @a variable is an availableVariable(). */
  bool isVariableAvailable(const std::string &variable);

  /**
   * The variables that are requested to be loaded. These will be read in the
   * next call to update(), but may or may not be loaded now. This data is
   * user-populated.
   */
  const Variables& requestedVariables() const { return m_requestedVariables; }

  /** Clear the list of requested variables. */
  void clearRequestedVariables();

  /**
   * Request that @a variable will be loaded on the next update(). @a variable
   * must exist in availableVariables().
   */
  void requestVariable(const std::string &variable);

  /** Request that @a variable *not* be loaded on the next update(). */
  void unrequestVariable(const std::string &variable);

  /** Returns true if @a variable is currently requested. */
  bool isVariableRequested(const std::string &variable);

  /**
   * The variables that are currently loaded into dataObject.
   * @sa variableMetaData()
   */
  Variables loadedVariables() const;

  /**
   * Returns a map<arrayName, metaData> for the loadedVariables().
   */
  const VariableMetaDataMap& variableMetaData() const { return m_variableMap; }

  /**
   * Returns an instance of VariableMetaData for @a var. If @a var is not
   * loaded, this returns an invalid metadata object
   * (See VariableMetaData::valid()).
   */
  VariableMetaData variableMetaData(const std::string &var) const;

  /**
   * Returns true if @a variable is loaded by the current dataObject().
   */
  bool isVariableLoaded(const std::string &variable);

  /**
   * The current dataObject. This is updated when update() is called after a
   * background file read completes.
   */
  vtkMultiBlockDataSet *dataObject() const;

  /** The bounds of the current dataObject. May be invalid. */
  const vtkBoundingBox& bounds() const { return m_bounds; }

  /** The number of timesteps present in dataObject(). */
  int numberOfTimeSteps() const { return m_numberOfTimeSteps; }

  /** The index of the current timestep. @{ */
  int timeStep() const { return m_timeStep; }
  void setTimeStep(int t) { m_timeStep = t; }
  /** @} */

  /** The inclusive range of valid timestep indices. @{ */
  const int* timeStepRange() const { return m_timeStepRange; }
  void timeStepRange(int r[2]);
  /** @} */

  /** The inclusive range of timestep times, represented as a double. @{ */
  const double* timeRange() const { return m_timeRange; }
  void timeRange(double r[2]);
  /** @} */

private:
  void syncReaderState();
  bool dataNeedsUpdate();
  void executeReaderInformation();
  void executeReaderData();
  void updateInformationCache();
  void updateDataCache();

private:
  std::future<void> m_future;
  vtkNew<vtkExodusIIReader> m_reader;

  vtkSmartPointer<vtkMultiBlockDataSet> m_data;
  VariableMetaDataMap m_variableMap;

  std::string m_fileName;

  vtkBoundingBox m_bounds;

  int m_numberOfTimeSteps;
  int m_timeStep;
  int m_timeStepRange[2];
  double m_timeRange[2];

  Variables m_availableVariables;
  Variables m_requestedVariables;
};

/**
 * Holds metadata concerning a variable in the loaded dataset.
 */
struct mvReader::VariableMetaData
{
  enum class Location
    {
    Invalid,
    PointData,
    CellData,
    FieldData
    };

  explicit VariableMetaData(Location loc = Location::Invalid,
                            double min = std::numeric_limits<double>::max(),
                            double max = std::numeric_limits<double>::min())
    : location(loc), range{min, max} {}

  bool valid() const { return location != Location::Invalid; }

  Location location;
  double range[2];
};

//------------------------------------------------------------------------------
template <typename Rep, typename Period>
bool mvReader::
running(const std::chrono::duration<Rep, Period> &duration) const
{
  if (m_future.valid())
    {
    return m_future.wait_for(duration) != std::future_status::ready;
    }
  return false;
}

//------------------------------------------------------------------------------
inline bool mvReader::isVariableRequested(const std::string &variable)
{
  return m_requestedVariables.find(variable) != m_requestedVariables.end();
}

//------------------------------------------------------------------------------
inline mvReader::VariableMetaData
mvReader::variableMetaData(const std::string &var) const
{
  auto iter = m_variableMap.find(var);
  return iter != m_variableMap.end() ? iter->second : VariableMetaData();
}

//------------------------------------------------------------------------------
inline mvReader::Variables mvReader::loadedVariables() const
{
  Variables result;
  for (const auto &t : m_variableMap)
    {
    result.insert(result.end(), t.first);
    }
  return result;
}

//------------------------------------------------------------------------------
inline bool mvReader::isVariableLoaded(const std::string &var)
{
  return m_variableMap.find(var) != m_variableMap.end();
}

//------------------------------------------------------------------------------
inline bool mvReader::isVariableAvailable(const std::string &variable)
{
  return m_availableVariables.find(variable) != m_availableVariables.end();
}

//------------------------------------------------------------------------------
inline void mvReader::timeStepRange(int r[2])
{
  r[0] = m_timeStepRange[0];
  r[1] = m_timeStepRange[1];
}

#endif // MVREADER_H
