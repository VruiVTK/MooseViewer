#ifndef ARRAYLOCATOR_H
#define ARRAYLOCATOR_H

#include <vtkCellData.h>
#include <vtkCompositeDataIterator.h>
#include <vtkCompositeDataSet.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkTypeTraits.h>

#include <algorithm>

// Utility to locate a field data array when the field association and dataset
// type are unknown.
//
// Pass the vtkDataObject and array name to the constructor.
// If dataObj is a vtkDataSet, search the point, cell, and field data.
// If dataObj is a vtkCompositeDataSet, use the first leaf that is a
// vtkDataSet.
//
// ArrayLocator::Association will be set to the location of the array.
// ArrayLocator::Range will be set to the range of the array.
// ArrayLocator::IsComposite will be true if the dataset is composite.
//
struct ArrayLocator
{
  enum Location
    {
    Invalid, // Input is invalid.
    NotFound, // Array was not found in point data, cell data, or field data.
    PointData, // Array is in the PointData.
    CellData, // Array is in the CellData.
    FieldData // Array is in the FieldData.
    };

  Location Association;
  double Range[2];
  bool IsComposite;

  ArrayLocator(vtkDataObject *dataObj, const std::string &arrayName)
    : Association(Invalid),
      IsComposite(false)
  {
    this->Range[0] = vtkTypeTraits<double>::Max();
    this->Range[1] = vtkTypeTraits<double>::Min();

    if (!dataObj || arrayName.empty())
      {
      return;
      }

    // Find a dataset. If we have composite data, search the leaves.
    vtkDataSet *ds = vtkDataSet::SafeDownCast(dataObj);
    vtkCompositeDataSet *cds = ds ? NULL
                                  : vtkCompositeDataSet::SafeDownCast(dataObj);
    if (!ds && !cds)
      {
      // Print an error message as this is unlikely to be intentional:
      std::cerr << "Input dataObj is neither a vtkDataSet or a "
                   "vtkCompositeDataSet! ClassName: " << dataObj->GetClassName()
                << std::endl;
      return;
      }

    if (cds)
      {
      this->IsComposite = true;
      vtkCompositeDataIterator *it = cds->NewIterator();
      it->InitTraversal();
      while (!it->IsDoneWithTraversal())
        {
        if (ds = vtkDataSet::SafeDownCast(it->GetCurrentDataObject()))
          {
          break;
          }
        it->GoToNextItem();
        }
      it->Delete();
      }

    if (!ds)
      {
      // No datasets in the composite data
      return;
      }

    vtkDataArray *array = NULL;
    if (array = ds->GetPointData()->GetArray(arrayName.c_str()))
      {
      this->Association = PointData;
      }
    else if (array = ds->GetCellData()->GetArray(arrayName.c_str()))
      {
      this->Association = CellData;
      }
    else if (array = ds->GetFieldData()->GetArray(arrayName.c_str()))
      {
      this->Association = FieldData;
      }
    else
      {
      this->Association = NotFound;
      return;
      }

    // Compute the range:
    if (cds)
      {
      double blockRange[2];
      vtkCompositeDataIterator *it = cds->NewIterator();
      for (it->InitTraversal(); !it->IsDoneWithTraversal(); it->GoToNextItem())
        {
        if (ds = vtkDataSet::SafeDownCast(it->GetCurrentDataObject()))
          {
          switch (this->Association)
            {
            case PointData:
              array = ds->GetPointData()->GetArray(arrayName.c_str());
              break;

            case CellData:
              array = ds->GetCellData()->GetArray(arrayName.c_str());
              break;

            case FieldData:
              array = ds->GetFieldData()->GetArray(arrayName.c_str());
              break;

            default:
              // abort on debug builds:
              assert(!"Internal error: Invalid enum value.");
            }
          if (array)
            {
            array->GetRange(blockRange);
            this->Range[0] = std::min(this->Range[0], blockRange[0]);
            this->Range[1] = std::max(this->Range[1], blockRange[1]);
            }
          }
        }
      }
    else // Compute range for non-composite datasets.
      {
      array->GetRange(this->Range);
      }
  }
};

#endif // ARRAYLOCATOR_H
