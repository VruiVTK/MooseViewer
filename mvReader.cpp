#include "mvReader.h"

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

#include <cassert>
#include <iostream>

//------------------------------------------------------------------------------
mvReader::mvReader()
  : m_numberOfTimeSteps(0),
    m_timeStep(0),
    m_timeStepRange{0, 0},
    m_timeRange{0., 0.}
{
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
}

//------------------------------------------------------------------------------
vtkMultiBlockDataSet *mvReader::typedDataObject() const
{
  return static_cast<vtkMultiBlockDataSet*>(m_dataObject.Get());
}

//------------------------------------------------------------------------------
vtkImageData *mvReader::typedReducedDataObject() const
{
  return static_cast<vtkImageData*>(m_reducedData.Get());
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
  return !m_dataObject || m_dataObject->GetMTime() < m_reader->GetMTime();
}

//------------------------------------------------------------------------------
void mvReader::executeReaderInformation()
{
  m_reader->UpdateInformation();
}

//------------------------------------------------------------------------------
void mvReader::executeReaderData()
{
  m_reader->Update();
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
  m_dataObject.TakeReference(mbds->NewInstance());
  m_dataObject->ShallowCopy(mbds);

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
  vtkCompositeDataIterator *i = this->typedDataObject()->NewIterator();
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
void mvReader::syncReducerState()
{
  if (!m_dataObject)
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
  m_reducer->SetSourceData(m_dataObject.Get());
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
void mvReader::executeReducer()
{
  m_reducer->Update();
}

//------------------------------------------------------------------------------
void mvReader::updateReducedData()
{
  vtkDataObject *output = m_reducer->GetOutputDataObject(0);
  m_reducedData.TakeReference(output->NewInstance());
  m_reducedData->ShallowCopy(output);
}
