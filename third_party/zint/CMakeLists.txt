add_library(zint STATIC
    zint.c
    zint.h
    code128.c
    common.c
    render.c
    ps.c
    svg.c
    bmp.c
    gif.c
    png.c
    pcx.c
    emf.c
    tif.c
    dllversion.c
)
target_include_directories(zint PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
