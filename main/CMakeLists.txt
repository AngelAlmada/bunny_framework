file(GLOB_RECURSE APP_SOURCES
    "*.cpp"
    "*.c"
)

idf_component_register(
    SRCS
        ${APP_SOURCES}
    INCLUDE_DIRS
        "."
    REQUIRES
        bunny
        display
)

