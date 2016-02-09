CMAKE_MINIMUM_REQUIRED(VERSION 3.1)

IF(POLICY CMP0020)
  CMAKE_POLICY(SET CMP0020 NEW)
ENDIF()

PROJECT(MooseViewer)

SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

INCLUDE(FindPkgConfig)

IF(NOT VRUI_PKGCONFIG_DIR)
  FIND_PATH(VRUI_PKGCONFIG_DIR Vrui.pc)
ENDIF()
SET(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH}:${VRUI_PKGCONFIG_DIR}")

PKG_SEARCH_MODULE(VRUI REQUIRED Vrui>=3000000)
STRING(REPLACE "-framework;" "-framework " VRUI_LDFLAGS "${VRUI_LDFLAGS}")
INCLUDE_DIRECTORIES(${VRUI_INCLUDE_DIRS})
ADD_DEFINITIONS(${VRUI_CFLAGS_OTHER})

# Find VTK
FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

IF (VTK_VERSION VERSION_LESS "6.2")
  MESSAGE(FATAL_ERROR "Require VTK version 6.2 or higher")
ENDIF ()

IF (${VTK_RENDERING_BACKEND} STREQUAL "OpenGL")
  FIND_PACKAGE (GLEW REQUIRED)
  IF (NOT GLEW_FOUND)
    MESSAGE (FATAL_ERROR "Glew required. Please set GLEW_DIR")
  ENDIF ()
  INCLUDE_DIRECTORIES (${GLEW_INCLUDE_DIR})
ENDIF ()

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
INCLUDE(InstallRequiredSystemLibraries)

SET(${PROJECT_NAME}_SRCS
  AnimationDialog.cpp
  AnimationDialog.h
  ArrayLocator.h
  BaseLocator.cpp
  BaseLocator.h
  ClippingPlane.cpp
  ClippingPlane.h
  ClippingPlaneLocator.cpp
  ClippingPlaneLocator.h
  ColorMapCallbackData.cpp
  ColorMapCallbackData.h
  ColorMapChangedCallbackData.cpp
  ColorMapChangedCallbackData.h
  ColorMap.cpp
  ColorMap.h
  Contours.cpp
  Contours.h
  ControlPointChangedCallbackData.cpp
  ControlPointChangedCallbackData.h
  ControlPoint.cpp
  ControlPoint.h
  Gaussian.cpp
  Gaussian.h
  main.cpp
  MooseViewer.cpp
  MooseViewer.h
  mvApplicationState.cpp
  mvApplicationState.h
  mvAsyncGLObject.cpp
  mvAsyncGLObject.h
  mvContextState.cpp
  mvContextState.h
  mvContours.cpp
  mvContours.h
  mvFramerate.cpp
  mvFramerate.h
  mvGeometry.cpp
  mvGeometry.h
  mvGLObject.cpp
  mvGLObject.h
  mvOutline.cpp
  mvOutline.h
  mvVolume.cpp
  mvVolume.h
  RGBAColor.cpp
  RGBAColor.h
  ScalarWidgetCallbackData.cpp
  ScalarWidgetCallbackData.h
  ScalarWidgetChangedCallbackData.cpp
  ScalarWidgetChangedCallbackData.h
  ScalarWidgetControlPointChangedCallbackData.cpp
  ScalarWidgetControlPointChangedCallbackData.h
  ScalarWidgetControlPoint.cpp
  ScalarWidgetControlPoint.h
  ScalarWidget.cpp
  ScalarWidget.h
  ScalarWidgetStorage.cpp
  ScalarWidgetStorage.h
  Storage.cpp
  Storage.h
  SwatchesWidget.cpp
  SwatchesWidget.h
  TransferFunction1D.cpp
  TransferFunction1D.h
  VariablesDialog.cpp
  VariablesDialog.h
  WidgetHints.cpp
  WidgetHints.h
  )

ADD_EXECUTABLE(${PROJECT_NAME} ${${PROJECT_NAME}_SRCS})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${VTK_LIBRARIES} "${VRUI_LDFLAGS}")

TARGET_COMPILE_FEATURES(${PROJECT_NAME}
  PUBLIC
    cxx_final
    cxx_override
)

IF (${VTK_RENDERING_BACKEND} STREQUAL "OpenGL")
  TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${GLEW_LIBRARY})
ENDIF ()

INSTALL(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )
