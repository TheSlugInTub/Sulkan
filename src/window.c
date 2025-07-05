#include <sulkan/window.h>
#include <stdio.h>
#include <stdlib.h>

skWindow skWindow_Create(const char* title, int width, int height,
                         Bool fullscreen, Bool maximize)
{
    skWindow window;

    // Glfw: Initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // Glfw window creation
    // --------------------
    window.window = glfwCreateWindow(
        width, height, title,
        fullscreen ? glfwGetPrimaryMonitor() : NULL, NULL);
    if (window.window == NULL)
    {
        printf("SK ERROR: Failed to create GLFW window.\n");
        glfwTerminate();
        exit(1);
    }

    glfwMakeContextCurrent(window.window);
    
    if (maximize)
        glfwMaximizeWindow(window.window);

    window.width = width;
    window.height = height;

    return window;
}

Bool skWindow_ShouldClose(skWindow* window)
{
    return glfwWindowShouldClose(window->window);
}

void skWindow_Update(skWindow* window)
{
    glfwSwapBuffers(window->window);
    glfwPollEvents();
}

float skWindow_GetAspectRatio(skWindow* window)
{
    if (window->width == 0 || window->height == 0)
    {
        // Handle the minimized window case
        return 1.0f;
    }

    return (float)window->width / (float)window->height;
}

void skWindow_Close(skWindow* window)
{
    glfwTerminate();
}
