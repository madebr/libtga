set(EXAMPLES
    decode
    encode
    tgadump
)

foreach(EXAMPLE ${EXAMPLES})
    add_executable(${EXAMPLE}
        "${EXAMPLE}.c"
    )

    target_link_libraries(${EXAMPLE}
        libtga
    )

    install(
        TARGETS ${EXAMPLE}
        RUNTIME DESTINATION bin
    )
endforeach()
