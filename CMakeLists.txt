cmake_minimum_required (VERSION 3.5 FATAL_ERROR)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "20")
set(RELEASE_SERVICE_VERSION_MINOR "11")
set(RELEASE_SERVICE_VERSION_MICRO "80")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(kimagemapeditor VERSION ${RELEASE_SERVICE_VERSION})

set (QT_MIN_VERSION "5.7.0")
set (KF5_MIN_VERSION "5.15.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

add_definitions(-DQURL_NO_CAST_FROM_STRING)

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS
    Gui
    Widgets
    WebEngineWidgets
)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    ConfigWidgets
    CoreAddons
    Crash
    DBusAddons
    DocTools
    GuiAddons
    IconThemes
    Parts
    I18n
    WidgetsAddons
    XmlGui
)

include(FeatureSummary)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMOptionalAddSubdirectory)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)

ecm_setup_version(${RELEASE_SERVICE_VERSION} VARIABLE_PREFIX KIMAGEMAPEDITOR VERSION_HEADER kimagemapeditor_version.h)

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

add_subdirectory( pics )
ecm_optional_add_subdirectory(doc)

set(libkimagemapeditorcommon_SRC
    areacreator.cpp
    kimearea.cpp
    kimecommands.cpp
    kimedialogs.cpp
    kimagemapeditor.cpp
    drawzone.cpp
    arealistview.cpp
    imageslistview.cpp
    mapslistview.cpp
    imagemapchoosedialog.cpp
)
qt5_add_resources(libkimagemapeditorcommon_SRC kimagemapeditor.qrc)

ecm_qt_declare_logging_category(libkimagemapeditorcommon_SRC
    HEADER kimagemapeditor_debug.h
    IDENTIFIER KIMAGEMAPEDITOR_LOG
    CATEGORY_NAME org.kde.kimagemapeditor
    DEFAULT_SEVERITY Warning
)

########### next target ###############

set(kimagemapeditor_bin_SRCS main.cpp kimeshell.cpp ${libkimagemapeditorcommon_SRC})


add_executable(kimagemapeditor_bin ${kimagemapeditor_bin_SRCS})

set_target_properties(kimagemapeditor_bin PROPERTIES OUTPUT_NAME kimagemapeditor)

target_link_libraries(kimagemapeditor_bin KF5::CoreAddons KF5::I18n KF5::ConfigCore KF5::IconThemes KF5::Parts Qt5::WebEngineWidgets)

install(TARGETS kimagemapeditor_bin  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

set(kimagemapeditor_PART_SRCS ${libkimagemapeditorcommon_SRC})


add_library(kimagemapeditor MODULE ${kimagemapeditor_PART_SRCS})


target_link_libraries(kimagemapeditor KF5::CoreAddons KF5::IconThemes KF5::I18n KF5::ConfigCore KF5::Parts Qt5::WebEngineWidgets)

install(TARGETS kimagemapeditor  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############


########### install files ###############

install( FILES  org.kde.kimagemapeditor.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install( FILES  org.kde.kimagemapeditor.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install( FILES  kimagemapeditorpart.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

if (NOT ECM_VERSION VERSION_LESS "5.59.0")
    install( FILES  kimagemapeditor.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR} )
else()
    install( FILES  kimagemapeditor.categories DESTINATION ${KDE_INSTALL_CONFDIR} )
endif()

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
