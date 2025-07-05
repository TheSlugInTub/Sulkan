#include <sulkan/renderer.h>
#include <stdio.h>

Bool skRenderer_IsDeviceSuitable(skRenderer*      renderer,
                                 VkPhysicalDevice device)
{
    return true;
}

skRenderer skRenderer_Create()
{
    skRenderer renderer = {0};
    renderer.validationLayers =
        skVector_Create(sizeof(const char*), 10);

    VkApplicationInfo appInfo = {0};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Sulkan";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Sulkan";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;
    VkInstanceCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t     glfwExtensionCount = 0;
    const char** glfwExtensions;

    glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    if (vkCreateInstance(&createInfo, NULL, &renderer.instance) !=
        VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create Vulkan instance.\n");
    }
    
    u32 deviceCount = 0;

    skVector* physicalDevices;
    physicalDevices =
        skVector_Create(sizeof(VkPhysicalDevice), 1);
    vkEnumeratePhysicalDevices(renderer.instance, &deviceCount,
                               physicalDevices->data[0]);
    
    if (deviceCount == 0)
    {
        printf("SK ERROR: No physical device found.\n");
    }

    for (int i = 0; i < physicalDevices->size; i++)
    {
        VkPhysicalDevice* devicePtr = (VkPhysicalDevice*)skVector_Get(
            physicalDevices, i);
        VkPhysicalDevice device = *devicePtr;

        if (skRenderer_IsDeviceSuitable(&renderer, device))
        {
            renderer.physicalDevice = device;
        }
    }

    if (renderer.physicalDevice == VK_NULL_HANDLE)
    {
        printf("SK ERROR: Failed to find a suitable physical device.\n");
    }

    return renderer;
}

void skRenderer_Destroy(skRenderer* renderer)
{
    vkDestroyInstance(renderer->instance, NULL);
}
