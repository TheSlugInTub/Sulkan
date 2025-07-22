#include <sulkan/extra_systems.h>

float lastFrame = 0.0f;
float currentTime = 0.0f;

void skDeltaTimeSystem(skECSState* state)
{
    currentTime = (float)glfwGetTime();
    state->deltaTime = currentTime - lastFrame;
    lastFrame = currentTime;
}
