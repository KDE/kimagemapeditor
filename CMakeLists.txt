project(kimagemapeditor)

cmake_minimum_required (VERSION 2.8.12 FATAL_ERROR)
set (QT_MIN_VERSION "5.7.0")
set (KF5_MIN_VERSION "5.15.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS
    Gui
    Widgets
)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    ConfigWidgets
    CoreAddons
    Crash
    DBusAddons
    DocTools
    GuiAddons
    KHtml
    Parts
    I18n
    WidgetsAddons
    XmlGui
    KDELibs4Support
)

include(FeatureSummary)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMOptionalAddSubdirectory)
include(ECMQtDeclareLoggingCategory)

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

target_link_libraries(kimagemapeditor_bin KF5::KHtml KF5::KDELibs4Support)

install(TARGETS kimagemapeditor_bin  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

set(kimagemapeditor_PART_SRCS ${libkimagemapeditorcommon_SRC})


add_library(kimagemapeditor MODULE ${kimagemapeditor_PART_SRCS})


target_link_libraries(kimagemapeditor KF5::KHtml KF5::KDELibs4Support)

install(TARGETS kimagemapeditor  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############


########### install files ###############

install( FILES  org.kde.kimagemapeditor.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install( FILES  org.kde.kimagemapeditor.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install( FILES  kimagemapeditorpart.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install( FILES  kimagemapeditorui.rc kimagemapeditorpartui.rc  DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kimagemapeditor)
install( FILES  kimagemapeditor.categories DESTINATION ${KDE_INSTALL_CONFDIR} )

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
