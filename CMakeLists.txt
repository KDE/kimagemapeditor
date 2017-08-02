project(kimagemapeditor)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

add_definitions( ${QT_QTDBUS_DEFINITIONS} ${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDE4_INCLUDES})

add_subdirectory( pics )
macro_optional_add_subdirectory(doc)

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


########### next target ###############

set(kimagemapeditor_bin_SRCS main.cpp kimeshell.cpp ${libkimagemapeditorcommon_SRC})


kde4_add_executable(kimagemapeditor_bin ${kimagemapeditor_bin_SRCS})

set_target_properties(kimagemapeditor_bin PROPERTIES OUTPUT_NAME kimagemapeditor)

target_link_libraries(kimagemapeditor_bin ${KDE4_KHTML_LIBS} )

install(TARGETS kimagemapeditor_bin  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### next target ###############

set(kimagemapeditor_PART_SRCS ${libkimagemapeditorcommon_SRC})


kde4_add_plugin(kimagemapeditor WITH_PREFIX ${kimagemapeditor_PART_SRCS})



target_link_libraries(kimagemapeditor ${KDE4_KHTML_LIBS} )

install(TARGETS kimagemapeditor  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############


########### install files ###############

install( FILES  kimagemapeditor.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES  kimagemapeditorpart.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  kimagemapeditorui.rc            kimagemapeditorpartui.rc  DESTINATION ${DATA_INSTALL_DIR}/kimagemapeditor)

