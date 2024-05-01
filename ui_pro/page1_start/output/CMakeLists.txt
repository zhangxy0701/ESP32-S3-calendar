SET(SOURCES screens/ui_page1_Screen1_start.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    images/ui_img_images_long_png.c
    fonts/ui_font_Font20px.c)

add_library(ui ${SOURCES})
