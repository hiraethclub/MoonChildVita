set(CMAKE_SYSTEM_NAME Windows)

if(NOT DEFINED MOONCHILD_TARGET_ARCH)
    set(MOONCHILD_TARGET_ARCH "WindowsX64")
endif()

# Windows x64
if(MOONCHILD_TARGET_ARCH STREQUAL "WindowsX64")
    set(CMAKE_SYSTEM_PROCESSOR AMD64)

# Windows x86
elseif(MOONCHILD_TARGET_ARCH STREQUAL "WindowsX86")
    set(CMAKE_SYSTEM_PROCESSOR X86)

    get_filename_component(_moonchild_c_compiler_name "${CMAKE_C_COMPILER}" NAME_WE)
    get_filename_component(_moonchild_cxx_compiler_name "${CMAKE_CXX_COMPILER}" NAME_WE)
    if(NOT _moonchild_c_compiler_name STREQUAL "cl" AND NOT _moonchild_cxx_compiler_name STREQUAL "cl")
        set(CMAKE_C_FLAGS_INIT "--target=i686-pc-windows-msvc")
        set(CMAKE_CXX_FLAGS_INIT "--target=i686-pc-windows-msvc")
        set(CMAKE_EXE_LINKER_FLAGS_INIT "--target=i686-pc-windows-msvc")
        set(CMAKE_SHARED_LINKER_FLAGS_INIT "--target=i686-pc-windows-msvc")
    endif()
else()
    message(FATAL_ERROR "Unsupported Windows target architecture: ${MOONCHILD_TARGET_ARCH}")
endif()
