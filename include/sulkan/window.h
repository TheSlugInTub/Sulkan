#pragma once

#include <GLFW/glfw3.h>
#include <sulkan/essentials.h>

typedef struct
{
    const char*        title;
    i16                width;
    i16                height;
    struct GLFWwindow* window;
    Bool framebufferResized;
} skWindow;

// Initialize a GLFW window and OpenGL context
skWindow skWindow_Create(const char* title, i16 width, i16 height,
                         Bool fullscreen, Bool maximize);

// Is the window closed or not? Useful for running a game loop
Bool skWindow_ShouldClose(skWindow* window);

// Swaps buffers, poll events
void skWindow_Update(skWindow* window);

// (float)width / (float)height
float skWindow_GetAspectRatio(skWindow* window);

void skWindow_Rename(skWindow* window, const char* newName);

// Closes the window
void skWindow_Close(skWindow* window);
