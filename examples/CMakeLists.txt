add_library(example_utils
    utils.c
    utils.h
)

set(EXAMPLES
    decode
    encode
    sane
    tgadump
)

foreach(EXAMPLE ${EXAMPLES})
    add_executable(${EXAMPLE}
        "${EXAMPLE}.c"
    )

    target_link_libraries(${EXAMPLE}
        example_utils
        libtga
    )

    install(
        TARGETS ${EXAMPLE}
        RUNTIME DESTINATION bin
    )
endforeach()
