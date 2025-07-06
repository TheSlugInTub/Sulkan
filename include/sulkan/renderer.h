#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <sulkan/essentials.h>
#include <sulkan/vector.h>
#include <sulkan/window.h>

typedef struct skSwapchainDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    skVector* formats; // VkSurfaceFormatKHR
    skVector* presentModes; // VkPresentModeKHR
} skSwapchainDetails;

typedef struct skQueueFamilyIndices
{
    u32 graphicsFamily;
    u32 presentFamily;
    Bool isValid;
} skQueueFamilyIndices;

typedef enum skShaderType
{
    skShaderType_Vertex,
    skShaderType_Fragment,
    skShaderType_Geometry
} skShaderType;

typedef struct skShader
{
    skShaderType type;
} skShader;

void skShader_Create(const char* filePath);

void skShader_ReadFile();

typedef struct skRenderer
{
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSwapchainKHR swapchain;
    VkFormat swapchainImageFormat;
    VkExtent2D swapchainExtent;
    skVector* swapchainImages; // VkImage
    skVector* swapchainImageViews; // VkImageView
    skVector* swapchainFramebuffers; // VkFramebuffer
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    VkPipeline pipeline;
    VkInstance instance;
} skRenderer;

skRenderer skRenderer_Create(skWindow* window);
void skRenderer_Destroy();
