add_executable(${PROJECT_NAME}
    main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE vendor)

#show these in Solution Explorer
target_sources(${PROJECT_NAME} PRIVATE
    sandbox/diastey/memory/ArenaAllocator.h
    sandbox/sav/memory/ArenaAllocator.h
    sandbox/sav/memory/ArenaAllocator.cpp
)