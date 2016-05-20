# - Try to find vtkVRUI
# Once done this will define
#  vtkVRUI_FOUND - System has vtkVRUI
#  vtkVRUI_INCLUDE_DIRS - The vtkVRUI include directories
#  vtkVRUI_LIBRARIES - The libraries needed to use vtkVRUI
#  vtkVRUI_DEFINITIONS - Compiler switches required for using vtkVRUI

if(NOT vtkVRUI_FOUND)
  find_path(vtkVRUI_INCLUDE_DIRS vvApplication.h
            PATHS ENV PATH
            PATH_SUFFIXES vtkVRUI)
  find_library(vtkVRUI_LIBRARIES NAMES vtkVRUI
               PATHS "${vtkVRUI_INCLUDE_DIRS}/../lib" ENV LD_LIBRARY_PATH)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(vtkVRUI DEFAULT_MSG
                                    vtkVRUI_LIBRARIES vtkVRUI_INCLUDE_DIRS)
endif()
