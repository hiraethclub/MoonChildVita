include("${VITASDK}/share/vita.cmake" REQUIRED)

# Vita port versioning, separate from the game's own version
set(MOONCHILD_VITA_VERSION "0.1.0" CACHE STRING "Version of the Vita port")
set(MOONCHILD_VITA_TITLE_ID "MCFE01997" CACHE STRING "Vita homebrew title ID (4 letters + 5 digits)")
set(MOONCHILD_GIT_HASH "" CACHE STRING "Short git hash to embed, detected automatically if empty")

if(MOONCHILD_GIT_HASH STREQUAL "")
    find_package(Git QUIET)
    if(GIT_FOUND)
        # safe.directory is needed when the repo is mounted into a container as another user
        execute_process(
            COMMAND ${GIT_EXECUTABLE} -c safe.directory=* rev-parse --short=7 HEAD
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE MOONCHILD_GIT_HASH_DETECTED
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
    endif()
    if(NOT MOONCHILD_GIT_HASH_DETECTED)
        set(MOONCHILD_GIT_HASH_DETECTED "unknown")
    endif()
    set(MOONCHILD_GIT_HASH_EFFECTIVE "${MOONCHILD_GIT_HASH_DETECTED}")
else()
    set(MOONCHILD_GIT_HASH_EFFECTIVE "${MOONCHILD_GIT_HASH}")
endif()

# param.sfo wants APP_VER as ##.##, so 0.1.0 becomes 00.10
string(REGEX MATCH "^([0-9]+)\\.([0-9])\\.([0-9])$" MOONCHILD_VITA_VERSION_OK "${MOONCHILD_VITA_VERSION}")
if(NOT MOONCHILD_VITA_VERSION_OK)
    message(FATAL_ERROR "MOONCHILD_VITA_VERSION must look like X.Y.Z with single digit Y and Z, got ${MOONCHILD_VITA_VERSION}")
endif()
if(CMAKE_MATCH_1 LESS 10)
    set(MOONCHILD_VITA_APP_VER "0${CMAKE_MATCH_1}.${CMAKE_MATCH_2}${CMAKE_MATCH_3}")
else()
    set(MOONCHILD_VITA_APP_VER "${CMAKE_MATCH_1}.${CMAKE_MATCH_2}${CMAKE_MATCH_3}")
endif()

message(STATUS "Vita port ${MOONCHILD_VITA_VERSION} (APP_VER ${MOONCHILD_VITA_APP_VER}), git ${MOONCHILD_GIT_HASH_EFFECTIVE}, title ID ${MOONCHILD_VITA_TITLE_ID}")

target_sources(${EXECUTABLE_NAME} PRIVATE
    ${MOONCHILD_PLATFORM_DIR}/Vita/VitaLog.cpp
    ${MOONCHILD_PLATFORM_DIR}/Vita/VitaPlatform.cpp
)

target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${MOONCHILD_PLATFORM_DIR}/Vita
)

target_compile_definitions(${EXECUTABLE_NAME} PRIVATE
    "MOONCHILD_VITA_VERSION_STR=\"${MOONCHILD_VITA_VERSION}\""
    "MOONCHILD_GIT_HASH_STR=\"${MOONCHILD_GIT_HASH_EFFECTIVE}\""
)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE pthread)

vita_create_self(eboot.bin ${EXECUTABLE_NAME})

vita_create_vpk(${EXECUTABLE_NAME}.vpk ${MOONCHILD_VITA_TITLE_ID} eboot.bin
    VERSION ${MOONCHILD_VITA_APP_VER}
    NAME "${MOONCHILD_OUTPUT_NAME}"
    FILE ${MOONCHILD_PLATFORM_DIR}/Vita/sce_sys/icon0.png sce_sys/icon0.png
    FILE ${CMAKE_SOURCE_DIR}/Data data
    FILE ${CMAKE_SOURCE_DIR}/LICENSE LICENSE
    FILE ${CMAKE_SOURCE_DIR}/README.md README.md
)

# Read by Scripts/BuildGameVita.sh to name the VPK
file(WRITE ${CMAKE_BINARY_DIR}/VitaBuildInfo.txt
    "VERSION=${MOONCHILD_VITA_VERSION}\nHASH=${MOONCHILD_GIT_HASH_EFFECTIVE}\nTITLE_ID=${MOONCHILD_VITA_TITLE_ID}\n")
