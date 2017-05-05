set(LIBTGA_SOURCES
    tga_private.h
    tga.c
    tgaread.c
    tgawrite.c
)

add_library(libtga
    ${LIBTGA_SOURCES}
)

set_target_properties(libtga
    PROPERTIES
        PREFIX ""
)

target_include_directories(libtga
    PUBLIC
        "${LIBTGA_PROJECT_PATH}/include"
)

if (TGA_DEBUG)
    target_compile_definitions(libtga
        PRIVATE
            "TGA_DEBUG"
    )
endif()


install(
    TARGETS libtga
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(
    FILES
        "${LIBTGA_PROJECT_PATH}/include/tga.h"
    DESTINATION
        include
)
