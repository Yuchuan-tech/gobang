add_executable(gobang main.c "board.h" "board.c" "logic.h" "logic.c" "stats.h" stats.c "ai.h" "ai.c")

# raylib路径（改成你的实际路径）
set(RAYLIB_PATH "D:/Microsoft Visual Studio/Tools/raylib-6.0")

target_include_directories(gobang PRIVATE ${RAYLIB_PATH}/include)
target_link_directories(gobang PRIVATE ${RAYLIB_PATH}/lib)
target_link_libraries(gobang raylib winmm gdi32)