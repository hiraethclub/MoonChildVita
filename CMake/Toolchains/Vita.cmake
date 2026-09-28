# PS Vita, via VitaSDK (https://vitasdk.org)
# Defers to the toolchain file shipped with VitaSDK, which sets VITA to true

if(NOT DEFINED MOONCHILD_TARGET_ARCH)
    set(MOONCHILD_TARGET_ARCH "Vita")
endif()

if(DEFINED ENV{VITASDK})
    set(MOONCHILD_VITASDK_ROOT "$ENV{VITASDK}")
else()
    set(MOONCHILD_VITASDK_ROOT "/usr/local/vitasdk")
endif()

if(NOT EXISTS "${MOONCHILD_VITASDK_ROOT}/share/vita.toolchain.cmake")
    message(FATAL_ERROR "Could not find VitaSDK at ${MOONCHILD_VITASDK_ROOT}! Set the VITASDK environment variable.")
endif()

include("${MOONCHILD_VITASDK_ROOT}/share/vita.toolchain.cmake")
