add_executable(${PROJECT_NAME}
    main.cpp
    memory/ArenaAllocator.h
)

target_link_libraries(${PROJECT_NAME} PRIVATE vendor)