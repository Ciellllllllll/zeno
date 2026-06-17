get_filename_component(_ZENOENGINE_PREFIX "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)

if(NOT TARGET ZenoEngine::zeno_abi_rust)
    add_library(ZenoEngine::zeno_abi_rust SHARED IMPORTED)
    set_target_properties(ZenoEngine::zeno_abi_rust PROPERTIES
        IMPORTED_CONFIGURATIONS "Debug;Release"
        IMPORTED_IMPLIB_DEBUG "${_ZENOENGINE_PREFIX}/lib/Debug/zeno_abi.dll.lib"
        IMPORTED_LOCATION_DEBUG "${_ZENOENGINE_PREFIX}/bin/Debug/zeno_abi.dll"
        IMPORTED_IMPLIB_RELEASE "${_ZENOENGINE_PREFIX}/lib/Release/zeno_abi.dll.lib"
        IMPORTED_LOCATION_RELEASE "${_ZENOENGINE_PREFIX}/bin/Release/zeno_abi.dll"
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
        MAP_IMPORTED_CONFIG_MINSIZEREL Release
        INTERFACE_INCLUDE_DIRECTORIES "${_ZENOENGINE_PREFIX}/include"
    )
endif()

if(NOT TARGET ZenoEngine::zeno_native)
    add_library(ZenoEngine::zeno_native STATIC IMPORTED)
    set_target_properties(ZenoEngine::zeno_native PROPERTIES
        IMPORTED_CONFIGURATIONS "Debug;Release"
        IMPORTED_LOCATION_DEBUG "${_ZENOENGINE_PREFIX}/lib/Debug/zeno_native.lib"
        IMPORTED_LOCATION_RELEASE "${_ZENOENGINE_PREFIX}/lib/Release/zeno_native.lib"
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
        MAP_IMPORTED_CONFIG_MINSIZEREL Release
        INTERFACE_INCLUDE_DIRECTORIES "${_ZENOENGINE_PREFIX}/include"
        INTERFACE_LINK_LIBRARIES "ZenoEngine::zeno_abi_rust;user32;d3d11;dxgi;d3dcompiler;windowscodecs;ole32;xaudio2"
    )
endif()

if(NOT TARGET ZenoEngine::zeno_sdk_cpp)
    add_library(ZenoEngine::zeno_sdk_cpp STATIC IMPORTED)
    set_target_properties(ZenoEngine::zeno_sdk_cpp PROPERTIES
        IMPORTED_CONFIGURATIONS "Debug;Release"
        IMPORTED_LOCATION_DEBUG "${_ZENOENGINE_PREFIX}/lib/Debug/zeno_sdk_cpp.lib"
        IMPORTED_LOCATION_RELEASE "${_ZENOENGINE_PREFIX}/lib/Release/zeno_sdk_cpp.lib"
        MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
        MAP_IMPORTED_CONFIG_MINSIZEREL Release
        INTERFACE_INCLUDE_DIRECTORIES "${_ZENOENGINE_PREFIX}/include"
        INTERFACE_LINK_LIBRARIES "ZenoEngine::zeno_native;ZenoEngine::zeno_abi_rust"
    )
endif()

if(NOT TARGET ZENO::zeno_abi_rust)
    add_library(ZENO::zeno_abi_rust INTERFACE IMPORTED)
    set_target_properties(ZENO::zeno_abi_rust PROPERTIES
        INTERFACE_LINK_LIBRARIES ZenoEngine::zeno_abi_rust
    )
endif()

if(NOT TARGET ZENO::zeno_native)
    add_library(ZENO::zeno_native INTERFACE IMPORTED)
    set_target_properties(ZENO::zeno_native PROPERTIES
        INTERFACE_LINK_LIBRARIES ZenoEngine::zeno_native
    )
endif()

if(NOT TARGET ZENO::zeno_sdk_cpp)
    add_library(ZENO::zeno_sdk_cpp INTERFACE IMPORTED)
    set_target_properties(ZENO::zeno_sdk_cpp PROPERTIES
        INTERFACE_LINK_LIBRARIES ZenoEngine::zeno_sdk_cpp
    )
endif()

set(ZenoEngine_INCLUDE_DIR "${_ZENOENGINE_PREFIX}/include")
set(ZenoEngine_BIN_DIR "${_ZENOENGINE_PREFIX}/bin")
set(ZenoEngine_LIB_DIR "${_ZENOENGINE_PREFIX}/lib")
set(ZenoEngine_VERSION "0.1.0-dev")
set(ZenoEngine_FOUND TRUE)

set(ZENO_INCLUDE_DIR "${ZenoEngine_INCLUDE_DIR}")
set(ZENO_BIN_DIR "${ZenoEngine_BIN_DIR}")
set(ZENO_LIB_DIR "${ZenoEngine_LIB_DIR}")
set(ZENO_VERSION "${ZenoEngine_VERSION}")
set(ZENO_FOUND TRUE)
