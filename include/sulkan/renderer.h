#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <sulkan/essentials.h>
#include <sulkan/vector.h>

typedef struct skRendererInfo
{
    skVector* validationLayers;
    VkPhysicalDevice physicalDevice;
    VkInstance instance;
} skRenderer;

skRenderer skRenderer_Create();
void skRenderer_Destroy();
