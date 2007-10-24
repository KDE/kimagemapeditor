project(kimagemapeditor)

add_subdirectory( pics )

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

target_link_libraries(kimagemapeditor_bin  ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KHTML_LIBS} )

install(TARGETS kimagemapeditor_bin  DESTINATION ${BIN_INSTALL_DIR} )


########### next target ###############

set(kimagemapeditor_PART_SRCS ${libkimagemapeditorcommon_SRC})


kde4_add_plugin(kimagemapeditor WITH_PREFIX ${kimagemapeditor_PART_SRCS})



target_link_libraries(kimagemapeditor  ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KHTML_LIBS} )

install(TARGETS kimagemapeditor  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############


########### install files ###############

install( FILES  kimagemapeditor.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES  kimagemapeditorpart.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  kimagemapeditorui.rc            kimagemapeditorpartui.rc  DESTINATION ${DATA_INSTALL_DIR}/kimagemapeditor)

