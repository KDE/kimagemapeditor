install( FILES  addpointcursor.png freehandcursor.png polygoncursor.png removepointcursor.png  DESTINATION ${DATA_INSTALL_DIR}/kimagemapeditor)

ecm_install_icons(ICONS
    16-apps-kimagemapeditor.png
    22-actions-addpoint.png
    22-actions-arrow.png
    22-actions-circle.png
    22-actions-circle2.png
    22-actions-freehand.png
    22-actions-lower.png
    22-actions-polygon.png
    22-actions-raise.png
    22-actions-rectangle.png
    22-actions-removepoint.png
    32-apps-kimagemapeditor.png
    48-apps-kimagemapeditor.png
    64-apps-kimagemapeditor.png
    128-apps-kimagemapeditor.png
    sc-apps-kimagemapeditor.svgz
    DESTINATION ${ICON_INSTALL_DIR}
    THEME hicolor
)

