add_executable(${PROJECT_NAME}
    main.cpp
    platformConfig/platformConfigMacros.cpp
)

target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/CMakeLists.txt
    ${CMAKE_SOURCE_DIR}/CMakePresets.json
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
)

target_link_libraries(${PROJECT_NAME} PRIVATE vendor)

get_target_property(_srcs ${PROJECT_NAME} SOURCES)
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${_srcs})