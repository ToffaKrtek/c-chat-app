cmake_minimum_required(VERSION 3.10)
project(c-chat-app VERSION 1.0.0 LANGUAGES C)

# ------------------- Настройки компиляции -------------------
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(OpenGL_GL_PREFERENCE GLVND)

# ------------------- Пути к заголовкам (глобально) ---------
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/chat
    ${CMAKE_CURRENT_SOURCE_DIR}/src/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
)

# ------------------- Исходные файлы -------------------------
set(SOURCES
    src/main.c

    # CHAT
    src/chat/chat_api.c

    # GUI
    src/gui/layout.c
)

# ------------------- Подключение зависимостей ---------------
include(FetchContent)

# NanoVG (рендеринг)
FetchContent_Declare(
    nanovg
    GIT_REPOSITORY https://github.com/memononen/nanovg.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(nanovg)

# Clay (UI, header‑only)
FetchContent_Declare(
    clay
    GIT_REPOSITORY https://github.com/nicbarker/clay.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(clay)

# Системные библиотеки: OpenGL и GLFW
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# ------------------- Исполняемый файл -----------------------
add_executable(${PROJECT_NAME} ${SOURCES})

# Линковка с библиотеками
target_link_libraries(${PROJECT_NAME}
    OpenGL::GL
    glfw
    nanovg
)

# Clay – header‑only, добавляем путь к заголовкам
target_include_directories(${PROJECT_NAME} PRIVATE ${clay_SOURCE_DIR})

# ------------------- Флаги компиляции -----------------------
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE 
        -Wall -Wextra -Wpedantic 
        -g -O2
    )
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
endif()

# ------------------- Тесты и установка (опционально) --------
enable_testing()
# add_subdirectory(tests)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CRITERION REQUIRED criterion)

# Макрос для создания теста модуля
macro(add_module_test MODULE_NAME)
    set(TEST_NAME "test_${MODULE_NAME}")
    add_executable(${TEST_NAME} ${ARGN})
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/${MODULE_NAME}
        ${CRITERION_INCLUDE_DIRS}  
    )
    target_link_libraries(${TEST_NAME}
        ${CRITERION_LIBRARIES}
    )
    if(CRITERION_CFLAGS_OTHER)
      target_compile_options(${TEST_NAME} PRIVATE ${CRITERION_CFLAGS_OTHER})
    endif()
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endmacro()

# Теперь для каждого модуля вызываем макрос
add_module_test(chat
    src/chat/tests/test_chat_api.c
    src/chat/chat_api.c
)

# add_module_test(gui
#     src/gui/tests/test_layout.c
#     src/gui/layout.c
#     # ...
# )

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
install(DIRECTORY assets/ DESTINATION share/${PROJECT_NAME}/assets)
