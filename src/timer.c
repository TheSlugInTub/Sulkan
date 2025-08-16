#include <sulkan/timer.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

void skTimer_Start(skTimer* timer)
{
    timer->startTime = glfwGetTime();
}

void skTimer_PrintSeconds(const char* string, skTimer timer)
{
    double currentTime = glfwGetTime();
    printf("%s %f\n", string, currentTime - timer.startTime);
}

double skTimer_GetSeconds(skTimer timer)
{
    return timer.startTime;
}
