#define GLFW_INCLUDE_VULKAN
#include <sulkan/sulkan.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    skWindow window = skWindow_Create("Sulkan", 800, 600, false, false);

    skRenderer renderer = skRenderer_Create(&window);

    while (!skWindow_ShouldClose(&window))
    {
        skRenderer_DrawFrame(&renderer);

        skWindow_Update(&window);
    }

    skWindow_Close(&window);

    return 0;
}
