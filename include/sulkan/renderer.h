#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <sulkan/essentials.h>
#include <sulkan/vector.h>
#include <sulkan/hashmap.h>
#include <sulkan/window.h>
#include <cglm/cglm.h>
#include <sulkan/model.h>
#include <sulkan/ecs_api.h>

#define SK_FRAMES_IN_FLIGHT (2)
#define SK_MAX_RENDER_OBJECTS (1000)

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

    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;

    VkImage depthImage;
    VkImageView depthImageView;
    VkDeviceMemory depthImageMemory;

    mat4 viewTransform;

    skVector* renderObjects; // skRenderObject

    double startTime;

    VkInstance instance;
} skRenderer;

struct skRenderObject;

skRenderer skRenderer_Create(skWindow* window);
void skRenderer_InitializeVulkan(skRenderer* renderer, skWindow* window);
void skRenderer_InitializeUniformsAndDescriptors(skRenderer* renderer);
void skRenderer_DrawFrame(skRenderer* renderer);
void skRenderer_Destroy(skRenderer* renderer);

typedef struct skRenderObject
{
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
    u32 indexCount;

    VkDescriptorSet descriptorSets[SK_FRAMES_IN_FLIGHT];
    VkBuffer uniformBuffers[SK_FRAMES_IN_FLIGHT];
    VkDeviceMemory uniformBuffersMemory[SK_FRAMES_IN_FLIGHT];
    void* uniformBuffersMap[SK_FRAMES_IN_FLIGHT];

    mat4 transform;
} skRenderObject;

skRenderObject skRenderObject_CreateFromModel(skRenderer* renderer,
                                              skModel*    model,
                                              const char* texturePath);
void skRenderer_CreateDescriptorSetsForObject(skRenderer* renderer,
                                              skRenderObject* obj);
void skRenderer_AddRenderObject(skRenderer*     renderer,
                                skRenderObject* object);
