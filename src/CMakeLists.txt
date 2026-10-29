add_executable(${PROJECT_NAME}
    app/platform/win64/win64App.cpp
)

target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/CMakeLists.txt
    ${CMAKE_SOURCE_DIR}/CMakePresets.json
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    platformConfig/platformConfigMacros.h
    platformConfig/platformConfigIncludes.h
    platformConfig/platform/win64/win64PlatformIncludes.h
    platformConfig/platform/win64/win64PlatformDefines.h
    platformConfig/platform/win64/win64PlatformTypes.h
    app/appIncludes.h
    app/platform/win64/win64App.h
    app/platform/win64/win64AppTypes.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE vendor)

get_target_property(_srcs ${PROJECT_NAME} SOURCES)
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${_srcs})