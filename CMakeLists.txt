CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

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
ADD_DEFINITIONS(${VRUI_CFLAGS})

# Find VTK
FIND_PACKAGE(VTK REQUIRED)
INCLUDE(${VTK_USE_FILE})

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
INCLUDE(InstallRequiredSystemLibraries)

SET(${PROJECT_NAME}_SRCS
  AnimationDialog.cpp
  AnimationDialog.h
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
  DataItem.cpp
  DataItem.h
  FlashlightLocator.cpp
  FlashlightLocator.h
  Gaussian.cpp
  Gaussian.h
  main.cpp
  MooseViewer.cpp
  MooseViewer.h
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
  )

ADD_EXECUTABLE(${PROJECT_NAME} ${${PROJECT_NAME}_SRCS})

TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${VTK_LIBRARIES} "${VRUI_LDFLAGS}")

INSTALL(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )
