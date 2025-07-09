#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <sulkan/essentials.h>
#include <sulkan/vector.h>
#include <sulkan/window.h>
#include <cglm/cglm.h>

#define SK_FRAMES_IN_FLIGHT (2)

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

typedef struct skVertex 
{
    vec2 pos;
    vec3 colour;
    vec2 textureCoordinates;
} skVertex;

typedef struct skUniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} skUniformBufferObject;

typedef struct skRenderer
{
    skWindow* window;
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
    VkCommandPool commandPool;
    skVector* commandBuffers; // VkCommandBuffer
    skVector* imageAvailableSemaphores; // VkSemaphore
    skVector* renderFinishedSemaphores; // VkSemaphore
    skVector* inFlightFences; // VkFence
    u32 currentFrame;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;

    skVector* descriptorSets; // VkDescriptorSet
    skVector* uniformBuffers; // VkBuffer
    skVector* uniformBuffersMemory; // VkDeviceMemory
    skVector* uniformBuffersMap; // void*

    VkImage textureImage;
    VkImageView textureImageView;
    VkSampler textureSampler;
    VkDeviceMemory textureImageMemory;

    double startTime;

    VkInstance instance;
} skRenderer;

skRenderer skRenderer_Create(skWindow* window);
void skRenderer_DrawFrame(skRenderer* renderer);
void skRenderer_Destroy(skRenderer* renderer);
