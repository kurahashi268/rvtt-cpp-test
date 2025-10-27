# FindPortAudio-Windows.cmake
# Helper module to find PortAudio on Windows (especially for cross-compilation)
#
# Sets:
#   PORTAUDIO_FOUND
#   PORTAUDIO_INCLUDE_DIR
#   PORTAUDIO_LIBRARY
#
# Environment variables:
#   PORTAUDIO_ROOT - Root directory of PortAudio installation

if(WIN32 OR MINGW)
    # Search paths
    set(PORTAUDIO_SEARCH_PATHS
        ${PORTAUDIO_ROOT}
        $ENV{PORTAUDIO_ROOT}
        ${CMAKE_PREFIX_PATH}
        ${CMAKE_INSTALL_PREFIX}
    )

    # Find include directory
    find_path(PORTAUDIO_INCLUDE_DIR
        NAMES portaudio.h
        PATHS ${PORTAUDIO_SEARCH_PATHS}
        PATH_SUFFIXES include portaudio
        DOC "PortAudio include directory"
    )

    # Find library (prefer static)
    find_library(PORTAUDIO_LIBRARY
        NAMES 
            libportaudio.a      # Static library (preferred)
            portaudio_static
            portaudio
            libportaudio
        PATHS ${PORTAUDIO_SEARCH_PATHS}
        PATH_SUFFIXES lib lib64 bin
        DOC "PortAudio library"
    )

    # Handle standard args
    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(PortAudio
        REQUIRED_VARS PORTAUDIO_LIBRARY PORTAUDIO_INCLUDE_DIR
    )

    # Create imported target
    if(PORTAUDIO_FOUND AND NOT TARGET PortAudio::PortAudio)
        add_library(PortAudio::PortAudio UNKNOWN IMPORTED)
        set_target_properties(PortAudio::PortAudio PROPERTIES
            IMPORTED_LOCATION "${PORTAUDIO_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${PORTAUDIO_INCLUDE_DIR}"
        )
    endif()

    mark_as_advanced(PORTAUDIO_INCLUDE_DIR PORTAUDIO_LIBRARY)
endif()

