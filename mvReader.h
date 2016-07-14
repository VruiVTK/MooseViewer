#ifndef MVREADER_H
#define MVREADER_H

#include <vtkBoundingBox.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <vvReader.h>

#include <map>
#include <set>
#include <limits>
#include <vector>

class vtkExodusIIReader;
class vtkImageData;
class vtkMultiBlockDataSet;
class vtkResampleToImage;

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
class mvReader : public vvReader
{
public:
  struct VariableMetaData;
  using Variables = std::set<std::string>;
  using VariableMetaDataMap = std::map<std::string, VariableMetaData>;

  mvReader();
  ~mvReader();

  /**
   * Convenience method to retrieve the data object with proper type.
   */
  vtkMultiBlockDataSet* typedDataObject() const;
  vtkImageData* typedReducedDataObject() const;

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
  void syncReaderState() override;
  bool dataNeedsUpdate() override;
  void executeReaderInformation() override;
  void executeReaderData() override;
  void updateInformationCache() override;
  void updateDataCache() override;

  void syncReducerState() override;
  bool reducerNeedsUpdate() override;
  void executeReducer() override;
  void updateReducedData() override;

private:
  vtkNew<vtkExodusIIReader> m_reader;
  VariableMetaDataMap m_variableMap;

  vtkNew<vtkResampleToImage> m_reducer;

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
