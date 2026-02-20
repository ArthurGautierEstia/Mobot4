cmake_minimum_required(VERSION 3.16)
project(MoBot4 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ========== Qt6 ==========
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# ========== Configuration des chemins SDKs ==========
# Par défaut, cherche dans C:/Dev/SDK
# Peut être surchargé avec -DSDK_ROOT=<chemin>
if(NOT DEFINED SDK_ROOT)
    if(EXISTS "C:/Dev/SDK")
        set(SDK_ROOT "C:/Dev/SDK" CACHE PATH "Path to SDK root folder")
    elseif(DEFINED ENV{MOBOT_SDK_PATH})
        set(SDK_ROOT $ENV{MOBOT_SDK_PATH} CACHE PATH "Path to SDK root folder")
    else()
        set(SDK_ROOT "" CACHE PATH "Path to SDK root folder")
        message(WARNING "SDK_ROOT not defined. SDKs will not be available.")
    endif()
endif()

message(STATUS "SDK Root: ${SDK_ROOT}")

# ========== OptiTrack NatNet SDK ==========
set(NATNET_SDK_PATH "${SDK_ROOT}/NatNetSDK")

if(EXISTS "${NATNET_SDK_PATH}/include/NatNetTypes.h")
    message(STATUS "✓ OptiTrack NatNet SDK found at: ${NATNET_SDK_PATH}")
    add_compile_definitions(OPTITRACK_AVAILABLE)
    include_directories("${NATNET_SDK_PATH}/include")
    link_directories("${NATNET_SDK_PATH}/lib/x64")
    set(NATNET_LIBS NatNetLib)
    set(NATNET_DLL "${NATNET_SDK_PATH}/lib/x64/NatNetLib.dll")
else()
    message(WARNING "✗ OptiTrack NatNet SDK not found - OptiTrack support disabled")
    set(NATNET_LIBS "")
    set(NATNET_DLL "")
endif()

# ========== Vicon DataStream SDK ==========
set(VICON_SDK_PATH "${SDK_ROOT}/DataStream SDK/Win64/CPP")

if(EXISTS "${VICON_SDK_PATH}/DataStreamClient.h")
    message(STATUS "✓ Vicon DataStream SDK found at: ${VICON_SDK_PATH}")
    add_compile_definitions(VICON_AVAILABLE)
    include_directories("${VICON_SDK_PATH}")
    link_directories("${VICON_SDK_PATH}")
    set(VICON_LIBS ViconDataStreamSDK_CPP)
    
    # Collecter les DLLs Vicon
    file(GLOB VICON_DLLS 
        "${VICON_SDK_PATH}/ViconDataStreamSDK_CPP.dll"
        "${VICON_SDK_PATH}/boost_*.dll"
    )
else()
    message(WARNING "✗ Vicon DataStream SDK not found - Vicon support disabled")
    set(VICON_LIBS "")
    set(VICON_DLLS "")
endif()

# ========== Qualisys SDK ==========
set(QUALISYS_SDK_PATH "${SDK_ROOT}/qualisys_cpp_sdk")

if(EXISTS "${QUALISYS_SDK_PATH}/RTProtocol.h")
    message(STATUS "✓ Qualisys SDK found at: ${QUALISYS_SDK_PATH}")
    add_compile_definitions(QUALISYS_AVAILABLE)
    include_directories("${QUALISYS_SDK_PATH}")
    link_directories("${QUALISYS_SDK_PATH}/Debug/x64")
    set(QUALISYS_LIBS RTClientSDK)
    set(QUALISYS_DLL "${QUALISYS_SDK_PATH}/Debug/x64/RTClientSDK.dll")
else()
    message(WARNING "✗ Qualisys SDK not found - Qualisys support disabled")
    set(QUALISYS_LIBS "")
    set(QUALISYS_DLL "")
endif()

# ========== Eigen (header-only, optionnel) ==========
set(EIGEN_PATH "${SDK_ROOT}/Eigen/src")

if(EXISTS "${EIGEN_PATH}")
    message(STATUS "✓ Eigen found at: ${EIGEN_PATH}")
    include_directories("${EIGEN_PATH}")
else()
    message(STATUS "  Eigen not found (optional)")
endif()

# ========== Collecter les sources du projet ==========
include_directories(${CMAKE_SOURCE_DIR}/src)

# Core files
file(GLOB CORE_SOURCES
    "src/core/abstract/*.cpp"
    "src/core/utils/*.cpp"
)

file(GLOB CORE_HEADERS
    "src/core/abstract/*.h"
    "src/core/types/*.h"
    "src/core/utils/*.h"
)

# Factory
file(GLOB FACTORY_SOURCES "src/factories/*.cpp")
file(GLOB FACTORY_HEADERS "src/factories/*.h")

# Measurement systems
set(SYSTEMS_SOURCES "")
set(SYSTEMS_HEADERS "")

if(DEFINED OPTITRACK_AVAILABLE)
    file(GLOB OPTITRACK_SOURCES "src/measurement_systems/optitrack/*.cpp")
    file(GLOB OPTITRACK_HEADERS "src/measurement_systems/optitrack/*.h")
    list(APPEND SYSTEMS_SOURCES ${OPTITRACK_SOURCES})
    list(APPEND SYSTEMS_HEADERS ${OPTITRACK_HEADERS})
endif()

# TODO: Ajouter Vicon, Qualisys, RSI quand implémentés
# if(DEFINED VICON_AVAILABLE)
#     file(GLOB VICON_SOURCES "src/measurement_systems/vicon/*.cpp")
#     ...
# endif()

# UI (pour plus tard)
file(GLOB UI_SOURCES "src/ui/*.cpp")
file(GLOB UI_HEADERS "src/ui/*.h")
file(GLOB UI_FORMS "src/ui/*.ui")

# Main
set(MAIN_SOURCE "src/main.cpp")

# ========== Créer l'exécutable ==========
add_executable(MoBot4 
    ${MAIN_SOURCE}
    ${CORE_SOURCES}
    ${CORE_HEADERS}
    ${FACTORY_SOURCES}
    ${FACTORY_HEADERS}
    ${SYSTEMS_SOURCES}
    ${SYSTEMS_HEADERS}
    ${UI_SOURCES}
    ${UI_HEADERS}
    ${UI_FORMS}
)

# ========== Lier les bibliothèques ==========
target_link_libraries(MoBot4 PRIVATE 
    Qt6::Core 
    Qt6::Widgets
    ${NATNET_LIBS}
    ${VICON_LIBS}
    ${QUALISYS_LIBS}
)

# ========== Installation (déploiement) ==========
install(TARGETS MoBot4
    RUNTIME DESTINATION bin
)

# Copier les DLLs nécessaires
if(WIN32)
    # OptiTrack
    if(NATNET_DLL AND EXISTS "${NATNET_DLL}")
        install(FILES "${NATNET_DLL}" DESTINATION bin)
    endif()
    
    # Vicon
    foreach(DLL ${VICON_DLLS})
        if(EXISTS "${DLL}")
            install(FILES "${DLL}" DESTINATION bin)
        endif()
    endforeach()
    
    # Qualisys
    if(QUALISYS_DLL AND EXISTS "${QUALISYS_DLL}")
        install(FILES "${QUALISYS_DLL}" DESTINATION bin)
    endif()
endif()

# ========== Copie automatique des DLLs pour le développement ==========
if(WIN32)
    # OptiTrack
    if(NATNET_DLL AND EXISTS "${NATNET_DLL}")
        add_custom_command(TARGET MoBot4 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${NATNET_DLL}"
            $<TARGET_FILE_DIR:MoBot4>
            COMMENT "Copying NatNetLib.dll"
        )
    endif()
    
    # Vicon
    foreach(DLL ${VICON_DLLS})
        if(EXISTS "${DLL}")
            add_custom_command(TARGET MoBot4 POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL}"
                $<TARGET_FILE_DIR:MoBot4>
                COMMENT "Copying ${DLL}"
            )
        endif()
    endforeach()
    
    # Qualisys
    if(QUALISYS_DLL AND EXISTS "${QUALISYS_DLL}")
        add_custom_command(TARGET MoBot4 POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${QUALISYS_DLL}"
            $<TARGET_FILE_DIR:MoBot4>
            COMMENT "Copying RTClientSDK.dll"
        )
    endif()
    
    # Déployer Qt automatiquement (windeployqt)
    find_program(WINDEPLOYQT windeployqt HINTS ${Qt6_DIR}/../../../bin)
    if(WINDEPLOYQT)
        add_custom_command(TARGET MoBot4 POST_BUILD
            COMMAND ${WINDEPLOYQT} $<TARGET_FILE:MoBot4> --no-translations
            COMMENT "Running windeployqt..."
        )
    endif()
endif()

# ========== Afficher le résumé de configuration ==========
message(STATUS "")
message(STATUS "========== MoBot4 Configuration Summary ==========")
message(STATUS "  Version:        ${PROJECT_VERSION}")
message(STATUS "  Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard:   ${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "  Available SDKs:")
if(DEFINED OPTITRACK_AVAILABLE)
    message(STATUS "    ✓ OptiTrack NatNet")
else()
    message(STATUS "    ✗ OptiTrack NatNet")
endif()
if(DEFINED VICON_AVAILABLE)
    message(STATUS "    ✓ Vicon DataStream")
else()
    message(STATUS "    ✗ Vicon DataStream")
endif()
if(DEFINED QUALISYS_AVAILABLE)
    message(STATUS "    ✓ Qualisys RTClient")
else()
    message(STATUS "    ✗ Qualisys RTClient")
endif()
message(STATUS "===================================================")
message(STATUS "")
