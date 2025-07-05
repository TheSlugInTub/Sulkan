#define GLFW_INCLUDE_VULKAN
#include <sulkan/sulkan.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    skWindow window = skWindow_Create("Sulkan", 800, 600, false, false);

    unsigned int extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

    printf("Extensions support: %d\n", extensionCount);

    skRenderer renderer = skRenderer_Create();

    while (!skWindow_ShouldClose(&window))
    {
        

        skWindow_Update(&window);
    }

    skWindow_Close(&window);

    return 0;
}
