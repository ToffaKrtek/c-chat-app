#include <GLFW/glfw3.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CLAY_IMPLEMENTATION
#include "clay/clay.h"

static GLFWwindow* window = NULL;
static int window_width = 1200;
static int window_height = 720;

static void error_callback(int error, const char* description) {
  fprintf(stderr, "GLFW Error: %s\n", description);
}

static void mouse_button_callback(GLFWwindow* window, int button, int action,
                                  int mods) {
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  Clay_SetPointerState((Clay_Vector2){(float)x, (float)y},
                       action == GLFW_PRESS);
}

static void scroll_callback(GLFWwindow* window, double xoffset,
                            double yoffset) {
  Clay_UpdateScrollContainers(
      true, (Clay_Vector2){(float)xoffset, (float)yoffset}, 0.1);
}

int run_app(void) {
  glfwSetErrorCallback(error_callback);
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return EXIT_FAILURE;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  window = glfwCreateWindow(window_width, window_height, "C-Chat", NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    return EXIT_FAILURE;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetScrollCallback(window, scroll_callback);

  uint32_t min_size = Clay_MinMemorySize();
  void* memory = malloc(min_size);
  if (!memory) {
    fprintf(stderr, "Failed to allocate memory\n");
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }
  Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(min_size, memory);

  Clay_Context* ctx = Clay_Initialize(
      arena, (Clay_Dimensions){(float)window_width, (float)window_height},
      (Clay_ErrorHandler){.errorHandlerFunction = NULL});
  if (!ctx) {
    fprintf(stderr, "Failed to initialize Clay\n");
    free(memory);
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_FAILURE;
  }
  while (!glfwWindowShouldClose(window)) {
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    window_width = width;
    window_height = height;

    glfwPollEvents();

    Clay_SetLayoutDimensions((Clay_Dimensions){(float)width, (float)height});
    Clay_BeginLayout();
    CLAY(CLAY_ID("OuterContainer"),
         {.layout = {.sizing = {.width = CLAY_SIZING_GROW(0),
                                .height = CLAY_SIZING_GROW(0)},
                     .padding = {16, 16, 16, 16},
                     .childGap = 16},
          .backgroundColor = {200, 200, 200, 255}}) {}

    Clay_EndLayout(0.1);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
  }
  glfwDestroyWindow(window);
  glfwTerminate();
  free(memory);
  return EXIT_SUCCESS;
}
