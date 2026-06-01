add_executable(gobang 
    main.c
    board.c
    logic.c
    stats.c
    ai.c
    challenge.c
    musicplayer.c
)


# raylib路径（改成你的实际路径）
set(RAYLIB_PATH "D:/Microsoft Visual Studio/Tools/raylib-6.0")

target_include_directories(gobang PRIVATE ${RAYLIB_PATH}/include)
target_link_directories(gobang PRIVATE ${RAYLIB_PATH}/lib)
target_link_libraries(gobang raylib winmm gdi32)

add_custom_command(TARGET gobang POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/music
        $<TARGET_FILE_DIR:gobang>/music
)