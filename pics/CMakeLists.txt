



########### install files ###############

install_files( ${DATA_INSTALL_DIR}/kimagemapeditor FILES  addpointcursor.png freehandcursor.png polygoncursor.png removepointcursor.png )

kde4_install_icons( ${ICON_INSTALL_DIR}  hicolor )




#original Makefile.am contents follow:

#KDE_ICON = kimagemapeditor addpoint arrow circle circle2 freehand lower polygon raise rectangle removepoint 
#
#picturesdir = $(kde_datadir)/kimagemapeditor
#pictures_DATA = addpointcursor.png freehandcursor.png polygoncursor.png removepointcursor.png
#
##	kimagemapeditor \
##	drawcircle \
##	arrow \
##	drawpolygon \
##	drawrectangle \
##	raise \
##	lower
#
