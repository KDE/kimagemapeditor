cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

# KDE Application Version, managed by release script
set(RELEASE_SERVICE_VERSION_MAJOR "24")
set(RELEASE_SERVICE_VERSION_MINOR "08")
set(RELEASE_SERVICE_VERSION_MICRO "3")
set(RELEASE_SERVICE_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}.${RELEASE_SERVICE_VERSION_MINOR}.${RELEASE_SERVICE_VERSION_MICRO}")

project(kimagemapeditor VERSION ${RELEASE_SERVICE_VERSION})

set (QT_MIN_VERSION "6.5.0")
set (KF_MIN_VERSION "6.0.0")
set (KDE_COMPILERSETTINGS_LEVEL "5.83.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

add_definitions(-DQURL_NO_CAST_FROM_STRING)

include(FeatureSummary)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS
    Gui
    Widgets
    WebEngineWidgets
)
find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    Config
    ConfigWidgets
    CoreAddons
    Crash
    DBusAddons
    DocTools
    GuiAddons
    Parts
    I18n
    WidgetsAddons
    XmlGui
)

ecm_setup_version(${RELEASE_SERVICE_VERSION} VARIABLE_PREFIX KIMAGEMAPEDITOR VERSION_HEADER kimagemapeditor_version.h)

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} )

add_subdirectory( pics )
add_subdirectory( doc )

ecm_qt_declare_logging_category(kimagemapeditor_LOG_SRC
    HEADER kimagemapeditor_debug.h
    IDENTIFIER KIMAGEMAPEDITOR_LOG
    CATEGORY_NAME org.kde.kimagemapeditor
    DEFAULT_SEVERITY Warning
    DESCRIPTION "KImageMapEditor"
    EXPORT KIMAGEMAPEDITOR
)

########### next target ###############

add_executable(kimagemapeditor_bin)

set_target_properties(kimagemapeditor_bin PROPERTIES OUTPUT_NAME kimagemapeditor)

target_sources(kimagemapeditor_bin PRIVATE
    main.cpp
    kimeshell.cpp
    ${kimagemapeditor_LOG_SRC}

    kimagemapeditor.qrc
)

target_link_libraries(kimagemapeditor_bin
    KF6::CoreAddons
    KF6::I18n
    KF6::ConfigCore
    KF6::Parts
    Qt::WebEngineWidgets
)

install(TARGETS kimagemapeditor_bin  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

configure_file(kimagemapeditorpart.json.in ${CMAKE_CURRENT_BINARY_DIR}/kimagemapeditorpart.json)

add_library(kimagemapeditorpart MODULE)

target_sources(kimagemapeditorpart PRIVATE
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
    ${kimagemapeditor_LOG_SRC}

    kimagemapeditorpart.qrc
)

target_link_libraries(kimagemapeditorpart
    KF6::CoreAddons
    KF6::I18n
    KF6::ConfigCore
    KF6::Parts
    Qt::WebEngineWidgets
)

install(TARGETS kimagemapeditorpart DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/parts)

########### install files ###############

install( FILES  org.kde.kimagemapeditor.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install( FILES  org.kde.kimagemapeditor.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

ki18n_install(po)
kdoctools_install(po)

ecm_qt_install_logging_categories(
    EXPORT KIMAGEMAPEDITOR
    FILE kimagemapeditor.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
