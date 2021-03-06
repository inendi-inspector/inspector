#
# @file
#

include_directories(SYSTEM AFTER ${ICU_INCLUDE})

################################################
# Ressource declaration
################################################

set(INSPECTOR_ICON "resources////inspector.ico")

SET(MOC_HEADERS
include/PVFilesTypesSelWidget.h
include/PVMainWindow.h
include/PVStringListChooserWidget.h

widgets/format-builder/include/PVFormatBuilderWidget.h
widgets/format-builder/include/PVNrawListingWidget.h
widgets/format-builder/include/PVXmlTreeView.h 
widgets/format-builder/include/PVXmlParamColorDialog.h
widgets/format-builder/include/PVXmlParamComboBox.h
widgets/format-builder/include/PVXmlParamList.h
widgets/format-builder/include/PVXmlParamTextEdit.h
widgets/format-builder/include/PVXmlParamWidget.h
widgets/format-builder/include/PVXmlParamWidgetBoardAxis.h
widgets/format-builder/include/PVXmlParamWidgetBoardFilter.h
widgets/format-builder/include/PVXmlParamWidgetBoardSplitterRegEx.h
widgets/format-builder/include/PVXmlRegValidatorHighLight.h
widgets/format-builder/include/PVXmlTimeValidatorHighLight.h
widgets/format-builder/include/PVOptionsWidget.h
)

SET(SRC_FILES
main.cpp
PVFilesTypesSelWidget.cpp
PVMainWindow.cpp
PVMainWindowMenusActions.cpp
PVMainWindowSlots.cpp
PVSaveDataTreeDialog.cpp
PVStringListChooserWidget.cpp

widgets/format-builder/PVFormatBuilderWidget.cpp
widgets/format-builder/PVNrawListingModel.cpp
widgets/format-builder/PVNrawListingWidget.cpp
widgets/format-builder/PVXmlDomModel.cpp
widgets/format-builder/PVXmlParamColorDialog.cpp
widgets/format-builder/PVXmlParamComboBox.cpp
widgets/format-builder/PVXmlParamList.cpp
widgets/format-builder/PVXmlParamTextEdit.cpp
widgets/format-builder/PVXmlParamWidget.cpp
widgets/format-builder/PVXmlParamWidgetBoardAxis.cpp
widgets/format-builder/PVXmlParamWidgetBoardFilter.cpp
widgets/format-builder/PVXmlParamWidgetBoardSplitterRegEx.cpp
widgets/format-builder/PVXmlParamWidgetEditorBox.cpp
widgets/format-builder/PVXmlRegValidatorHighLight.cpp
widgets/format-builder/PVXmlTreeItemDelegate.cpp
widgets/format-builder/PVXmlTreeView.cpp
widgets/format-builder/PVXmlTimeValidatorHighLight.cpp
widgets/format-builder/PVOptionsWidget.cpp
)

SET(QRC_FILES
inendi-inspector_resource.qrc
)

################################################
# Generate extra QT files (moc, uic, rcc)
################################################

QT5_WRAP_CPP(MOC_SRC ${MOC_HEADERS} OPTIONS ${QT5_WRAP_OPTIONS})
QT5_ADD_RESOURCES(QRC_FILES_SRCS ${QRC_FILES})

################################################
# Handle licences for inendi-inspector
################################################

add_executable(inendi-inspector ${SRC_FILES} ${MOC_SRC} ${MOC_HEADERS} ${QRC_FILES_SRCS})

set_property(TARGET inendi-inspector APPEND PROPERTY LINK_FLAGS)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

set(PV_QT_LIBS Qt5Core Qt5Widgets Qt5Network Qt5Xml Qt5Widgets)

target_link_libraries(inendi-inspector ssl ${QT_LIBRARIES} ${PVPARALLELVIEW_LIBRARIES}
    pvguiqt pvparallelview inendi pvkernel ${TBB_LIBRARIES} ${ICU_LIBRARY}
    ${OpenMP_LIBRARIES} ${PV_QT_LIBS} ${PVLICENSE_LIBRARIES})

INSTALL(TARGETS inendi-inspector DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
