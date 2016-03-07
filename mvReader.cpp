#include "mvReader.h"

#include <Vrui/Vrui.h>

#include <vtkCellData.h>
#include <vtkCompositeDataIterator.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkImageData.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkExodusIIReader.h>
#include <vtkFieldData.h>
#include <vtkInformation.h>
#include <vtkPointData.h>
#include <vtkPointInterpolator.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkTimerLog.h>
#include <vtkVoronoiKernel.h>

#include "mvApplicationState.h"
#include "mvProgress.h"
#include "mvProgressCookie.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <sstream>

//------------------------------------------------------------------------------
mvReader::mvReader()
  : m_cookie(nullptr),
    m_reducerCookie(nullptr),
    m_numberOfTimeSteps(0),
    m_timeStep(0),
    m_timeStepRange{0, 0},
    m_timeRange{0., 0.},
    m_benchmark(false)
{
  m_reader->GenerateObjectIdCellArrayOn();
  m_reader->GenerateGlobalElementIdArrayOn();
  m_reader->GenerateGlobalNodeIdArrayOn();

  m_reducerSeed->SetDimensions(64, 64, 64);
  m_reducer->SetKernel(m_reducerKernel.Get());
  m_reducer->SetInputDataObject(m_reducerSeed.Get());
  m_reducer->SetNullPointsStrategyToNullValue();
  m_reducer->SetNullValue(0.0);
  m_reducer->PassPointArraysOff();
  m_reducer->PassCellArraysOff();
  m_reducer->PassFieldArraysOff();
}

//------------------------------------------------------------------------------
mvReader::~mvReader()
{
  // Wait for background processes to finish:
  if (m_future.valid())
    {
    std::cout << "Waiting for file read to complete..." << std::endl;
    m_future.wait();
    }

  if (m_reducerFuture.valid())
    {
    std::cout << "Waiting for async data reduction to complete..." << std::endl;
    m_reducerFuture.wait();
    }
}

//------------------------------------------------------------------------------
void mvReader::update(const mvApplicationState &appState)
{
  // Are we currently reading the file?
  if (m_future.valid())
    {
    // Get the current state of the update thread:
    std::future_status state = m_future.wait_for(std::chrono::milliseconds(0));

    // We always use the async launch policy:
    assert("Always async." && state != std::future_status::deferred);

    if (state == std::future_status::ready)
      {
      // Clear the future.
      m_future.get();

      // Sync the cached data.
      this->updateInformationCache();
      this->updateDataCache();

      // Invalidate the reduced dataset as it is now out of date. This prevents
      // LOD actors from displaying incorrect lowres data.
      this->invalidateReducedData();

      // Clean up the progress monitor:
      assert("Cookie exists." && m_cookie != nullptr);
      appState.progress().removeEntry(m_cookie);
      m_cookie = nullptr;
      }
    else
      {
      // Still running, do nothing.
      return;
      }
    }

  // At this point, we know the background thread is not running and the
  // cache object is up-to-date with the last execution. Check to see if the
  // reader needs to re-run:
  this->syncReaderState();
  if (this->dataNeedsUpdate())
    {
    assert("Cookie cleaned up." && m_cookie == nullptr);
    m_cookie = appState.progress().addEntry("Reading Data File");

    m_future = std::async(std::launch::async,
                          &mvReader::executeReaderData, this);

    // Don't bother updating reduced data until the main data is up-to-date:
    return;
    }

  // Update the reduced data as well. Logic is the same as above.
  if (m_reducerFuture.valid())
    {
    std::chrono::seconds now(0);
    std::future_status state = m_reducerFuture.wait_for(now);
    assert("Always async." && state != std::future_status::deferred);
    if (state == std::future_status::ready)
      {
      m_reducerFuture.get(); // Clear the thread state.
      this->updateReducedData();
      assert("Cookie exists." && m_reducerCookie != nullptr);
      appState.progress().removeEntry(m_reducerCookie);
      m_reducerCookie = nullptr;
      }
    else
      {
      return;
      }
    }

  this->syncReducerState(appState);
  if (this->reducerNeedsUpdate())
    {
    this->invalidateReducedData();
    assert("Cookie cleaned up." && m_reducerCookie == nullptr);
    m_reducerCookie = appState.progress().addEntry("Generating Reduced Data");
    m_reducerFuture = std::async(std::launch::async,
                                 &mvReader::executeReducer, this);
    }
}

//------------------------------------------------------------------------------
void mvReader::updateInformation()
{
  // Only update when the background thread is not running:
  if (m_future.valid())
    {
    return;
    }

  this->syncReaderState();
  this->executeReaderInformation();
  this->updateInformationCache();
}

//------------------------------------------------------------------------------
void mvReader::clearRequestedVariables()
{
  m_requestedVariables.clear();
}

//------------------------------------------------------------------------------
void mvReader::requestVariable(const std::string &variable)
{
  if (!this->isVariableAvailable(variable))
    {
    std::cerr << "Ignoring request for variable '" << variable << "', as it "
                 "is not available to the reader at this time." << std::endl;
    return;
    }

  m_requestedVariables.insert(variable);
}

//------------------------------------------------------------------------------
void mvReader::unrequestVariable(const std::string &variable)
{
  m_requestedVariables.erase(variable);
}

//------------------------------------------------------------------------------
vtkMultiBlockDataSet* mvReader::dataObject() const
{
  return m_data.Get();
}

//------------------------------------------------------------------------------
vtkDataObject *mvReader::reducedDataObject() const
{
  return m_reducedData.Get();
}

//------------------------------------------------------------------------------
void mvReader::syncReaderState()
{
  m_reader->SetFileName(m_fileName.c_str());
  m_reader->SetTimeStep(m_timeStep);

  // Sync variables:
  const int numPointArrays = m_reader->GetNumberOfPointResultArrays();
  for (int i = 0; i < numPointArrays; ++i)
    {
    std::string array = m_reader->GetPointResultArrayName(i);
    m_reader->SetPointResultArrayStatus(
          array.c_str(), this->isVariableRequested(array) ? 1 : 0);
    }
  const int numElementArrays = m_reader->GetNumberOfElementResultArrays();
  for (int i = 0; i < numElementArrays; ++i)
    {
    std::string array = m_reader->GetElementResultArrayName(i);
    m_reader->SetElementResultArrayStatus(
          array.c_str(), this->isVariableRequested(array) ? 1 : 0);
    }
}

//------------------------------------------------------------------------------
bool mvReader::dataNeedsUpdate()
{
  return !m_data || m_data->GetMTime() < m_reader->GetMTime();
}

//------------------------------------------------------------------------------
void mvReader::executeReaderInformation()
{
  m_reader->UpdateInformation();
}

//------------------------------------------------------------------------------
void mvReader::executeReaderData()
{
  vtkTimerLog *log = nullptr;
  if (m_benchmark)
    {
    log = vtkTimerLog::New();
    std::cerr << "Updating full dataset.\n";
    log->StartTimer();
    }

  m_reader->Update();

  if (log != nullptr)
    {
    log->StopTimer();
    std::ostringstream out;
    out << "Full dataset ready (" << log->GetElapsedTime() << "s)\n";
    std::cerr << out.str();
    log->Delete();
    }

  Vrui::requestUpdate();
}

//------------------------------------------------------------------------------
void mvReader::updateInformationCache()
{
  m_numberOfTimeSteps = m_reader->GetNumberOfTimeSteps();
  m_reader->GetTimeStepRange(m_timeStepRange);
  vtkInformation *info = m_reader->GetOutputInformation(0);
  info->Get(vtkStreamingDemandDrivenPipeline::TIME_RANGE(), m_timeRange);

  // Set available arrays:
  m_availableVariables.clear();
  m_availableVariables.insert(m_reader->GetPedigreeNodeIdArrayName());
  m_availableVariables.insert(m_reader->GetObjectIdArrayName());
  m_availableVariables.insert(m_reader->GetPedigreeElementIdArrayName());
  const int numPointArrays = m_reader->GetNumberOfPointResultArrays();
  for (int i = 0; i < numPointArrays; ++i)
    {
    m_availableVariables.insert(m_reader->GetPointResultArrayName(i));
    }
  const int numElementArrays = m_reader->GetNumberOfElementResultArrays();
  for (int i = 0; i < numElementArrays; ++i)
    {
    m_availableVariables.insert(m_reader->GetElementResultArrayName(i));
    }
}

//------------------------------------------------------------------------------
void mvReader::updateDataCache()
{
  // Copy data object:
  vtkMultiBlockDataSet *mbds = m_reader->GetOutput();
  m_data.TakeReference(mbds->NewInstance());
  m_data->ShallowCopy(mbds);

  // Collect metadata next:

  // Reset state:
  m_bounds.Reset();
  m_variableMap.clear();

  // Helper lambda to update m_variableMap with the arrays in fd.
  auto mergeMetaData = [&](VariableMetaData::Location loc, vtkFieldData *fd)
  {
    const int size = fd->GetNumberOfArrays();
    for (int i = 0; i < size; ++i)
      {
      vtkDataArray *array = fd->GetArray(i);
      if (!array) // Might be abstract, GetArray has an implicit SafeDownCast.
        {
        continue;
        }
      std::string name = array->GetName() ? array->GetName() : "";
      auto iter = m_variableMap.find(name);
      if (iter == m_variableMap.end())
        { // New metadata
        auto r = m_variableMap.insert(std::make_pair(name,
                                                     VariableMetaData(loc)));
        iter = r.first;
        }
      VariableMetaData &metaData = iter->second;

      // Sanity check:
      if (metaData.location != loc)
        { // Shouldn't happen, but just warn and continue on.
        std::cerr << "Field data location mismatch for array: " << name << "\n";
        }

      // Merge the ranges:
      double range[2];
      array->GetRange(range);
      metaData.range[0] = std::min(range[0], metaData.range[0]);
      metaData.range[1] = std::max(range[1], metaData.range[1]);
      }
  };

  // Process datasets:
  vtkCompositeDataIterator *i = m_data->NewIterator();
  for (i->InitTraversal(); !i->IsDoneWithTraversal(); i->GoToNextItem())
    {
    if (vtkDataSet *ds = vtkDataSet::SafeDownCast(i->GetCurrentDataObject()))
      {
      mergeMetaData(VariableMetaData::Location::FieldData, ds->GetFieldData());
      mergeMetaData(VariableMetaData::Location::PointData, ds->GetPointData());
      mergeMetaData(VariableMetaData::Location::CellData,  ds->GetCellData());
      double b[6];
      ds->GetBounds(b);
      m_bounds.AddBounds(b);
      }
    }
  i->Delete();
}

//------------------------------------------------------------------------------
void mvReader::syncReducerState(const mvApplicationState &appState)
{
  if (!m_data)
    {
    m_reducer->SetSourceData(nullptr);
    return;
    }

  std::array<double, 3> dataDims;
  std::array<int, 3> seedDims;
  std::array<double, 3> spacing;
  std::array<double, 3> origin;

  m_reducerSeed->GetDimensions(seedDims.data());
  m_bounds.GetLengths(dataDims.data());
  m_bounds.GetMinPoint(origin[0], origin[1], origin[2]);

  spacing[0] = dataDims[0] / static_cast<double>(seedDims[0] - 1);
  spacing[1] = dataDims[1] / static_cast<double>(seedDims[1] - 1);
  spacing[2] = dataDims[2] / static_cast<double>(seedDims[2] - 1);

  m_reducerSeed->SetOrigin(origin.data());
  m_reducerSeed->SetSpacing(spacing.data());
  m_reducer->SetSourceData(m_data.Get());
}

//------------------------------------------------------------------------------
bool mvReader::reducerNeedsUpdate()
{
  return
      m_reducer->GetInputDataObject(1, 0) && // Check source (port 1).
      (!m_reducedData.Get() ||
       m_reducer->GetMTime() > m_reducedData->GetMTime());
}

//------------------------------------------------------------------------------
bool mvReader::invalidateReducedData()
{
  if (m_benchmark)
    {
    std::cerr << "Reduced data invalidated.\n";
    }
  m_reducedData = nullptr;
}

//------------------------------------------------------------------------------
void mvReader::executeReducer()
{
  vtkTimerLog *log = nullptr;
  if (m_benchmark)
    {
    log = vtkTimerLog::New();
    std::cerr << "Generating reduced data.\n";
    log->StartTimer();
    }

  m_reducer->Update();

  if (log != nullptr)
    {
    log->StopTimer();
    std::ostringstream out;
    out << "Reduced data ready (" << log->GetElapsedTime() << "s)\n";
    std::cerr << out.str();
    log->Delete();
    }

  Vrui::requestUpdate();
}

//------------------------------------------------------------------------------
void mvReader::updateReducedData()
{
  vtkDataObject *output = m_reducer->GetOutputDataObject(0);
  m_reducedData.TakeReference(output->NewInstance());
  m_reducedData->ShallowCopy(output);
}
