#include <sulkan/input.h>

typedef unsigned char Bool;

Bool keyState[400] = {};
Bool prevKeyState[400] = {};

bool skInput_GetKeyDown(skWindow* window, skKey key)
{
    prevKeyState[key] = keyState[key];
    keyState[key] =
        glfwGetKey(window->window, key) == GLFW_PRESS;
    return keyState[key] && !prevKeyState[key];
}

bool skInput_GetKeyUp(skWindow* window, skKey key)
{
    bool result =
        glfwGetKey(window->window, key) != GLFW_PRESS &&
        prevKeyState[key];
    prevKeyState[key] = keyState[key];
    keyState[key] =
        glfwGetKey(window->window, key) == GLFW_PRESS;
    return result;
}

bool skInput_GetKey(skWindow* window, skKey key)
{
    return glfwGetKey(window->window, key) == GLFW_PRESS;
}

bool skInput_GetMouseButtonDown(skWindow* window, skKey mouseKey)
{
    prevKeyState[mouseKey] = keyState[mouseKey];
    keyState[mouseKey] = glfwGetMouseButton(window->window,
                                            mouseKey) == GLFW_PRESS;
    return keyState[mouseKey] && !prevKeyState[mouseKey];
}

bool skInput_GetMouseButton(skWindow* window, skKey mouseKey)
{
    return glfwGetMouseButton(window->window, mouseKey) ==
           GLFW_PRESS;
}

int skInput_GetMouseInputHorizontal(skWindow* window)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window->window, &mouseX, &mouseY);
    return mouseX;
}

int skInput_GetMouseInputVertical(skWindow* window)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window->window, &mouseX, &mouseY);
    return mouseY;
}
