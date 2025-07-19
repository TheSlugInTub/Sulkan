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
#include <sulkan/model.h>
#include <sulkan/ecs_api.h>

#define SK_FRAMES_IN_FLIGHT   (2)
#define SK_MAX_RENDER_OBJECTS (1000)

typedef struct skSwapchainDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    skVector*                formats;      // VkSurfaceFormatKHR
    skVector*                presentModes; // VkPresentModeKHR
} skSwapchainDetails;

typedef struct skQueueFamilyIndices
{
    u32  graphicsFamily;
    u32  presentFamily;
    Bool isValid;
} skQueueFamilyIndices;

#define SK_MAX_LIGHTS 10

typedef struct skLight
{
    vec3  position;
    vec3  color;
    float radius;
    float intensity;
} skLight;

typedef struct skUniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
} skUniformBufferObject;

typedef struct skGlobalUniformBufferObject
{
    int lightCount;
} skGlobalUniformBufferObject;

typedef struct skRenderer
{
    skWindow*                window;
    VkPhysicalDevice         physicalDevice;
    VkDevice                 device;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR             surface;
    VkQueue                  graphicsQueue;
    VkQueue                  presentQueue;
    VkSwapchainKHR           swapchain;
    VkFormat                 swapchainImageFormat;
    VkExtent2D               swapchainExtent;
    skVector*                swapchainImages;       // VkImage
    skVector*                swapchainImageViews;   // VkImageView
    skVector*                swapchainFramebuffers; // VkFramebuffer
    VkRenderPass             renderPass;
    VkPipelineLayout         pipelineLayout;
    VkPipeline               pipeline;
    VkCommandPool            commandPool;
    skVector*                commandBuffers; // VkCommandBuffer
    skVector*                imageAvailableSemaphores; // VkSemaphore
    skVector*                renderFinishedSemaphores; // VkSemaphore
    skVector*                inFlightFences;           // VkFence
    u32                      currentFrame;
    VkDescriptorSetLayout    descriptorSetLayout;
    VkDescriptorSetLayout    lightDescriptorSetLayout;
    VkDescriptorSetLayout    uniformDescriptorSetLayout;
    VkDescriptorPool         descriptorPool;
    VkImage                  depthImage;
    VkImageView              depthImageView;
    VkDeviceMemory           depthImageMemory;
    mat4                     viewTransform;
    skVector*                renderObjects; // skRenderObject
    skVector*                lights;        // skLight

    VkDescriptorSet lightDescriptorSets[SK_FRAMES_IN_FLIGHT];
    VkBuffer        storageBuffers[SK_FRAMES_IN_FLIGHT];
    VkDeviceMemory  storageBuffersMemory[SK_FRAMES_IN_FLIGHT];
    void*           storageBuffersMap[SK_FRAMES_IN_FLIGHT];

    VkDescriptorSet uniformDescriptorSets[SK_FRAMES_IN_FLIGHT];
    VkBuffer        uniformBuffers[SK_FRAMES_IN_FLIGHT];
    VkDeviceMemory  uniformBuffersMemory[SK_FRAMES_IN_FLIGHT];
    void*           uniformBuffersMap[SK_FRAMES_IN_FLIGHT];

    VkInstance instance;
} skRenderer;

struct skRenderObject;

struct skEditor;
typedef struct skEditor skEditor;

skRenderer skRenderer_Create(skWindow* window);
void skRenderer_DrawFrame(skRenderer* renderer, skEditor* editor);
void skRenderer_InitImGui(skRenderer* renderer);

void skRenderer_CreateBuffer(skRenderer* renderer, size_t size,
                             VkBufferUsageFlags    usage,
                             VkMemoryPropertyFlags properties,
                             VkBuffer*             buffer,
                             VkDeviceMemory*       bufferMemory);

VkVertexInputBindingDescription skVertex_GetBindingDescription(void);
char* skReadFile(const char* filePath, u32* len);

VkShaderModule      skCreateShaderModule(skRenderer* renderer,
                                         char* buffer, u32 len);
VkBool32 VKAPI_CALL skDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*                                       pUserData);
VkResult skCreateDebugUtilsMessengerEXT(
    VkInstance                                instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks*              pAllocator,
    VkDebugUtilsMessengerEXT*                 pDebugMessenger);
void skDestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator);
Bool skCheckValidationLayerSupport(void);
void skPopulateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT* createInfo);

VkSurfaceFormatKHR
skChooseSwapSurfaceFormat(skVector* availableFormats);
VkPresentModeKHR
skChooseSwapPresentMode(skVector* availablePresentModes);

u32        skClampU32(u32 val, u32 low, u32 high);
VkExtent2D skChooseSwapExtent(VkSurfaceCapabilitiesKHR* capabilities,
                              skWindow*                 window);
skSwapchainDetails skQuerySwapchainSupport(skRenderer* renderer);

skQueueFamilyIndices skFindQueueFamilies(VkPhysicalDevice device,
                                         VkSurfaceKHR     surface);
void                 skRenderer_CreateSwapchain(skRenderer* renderer,
                                                skWindow*   window);
void skRenderer_CreateGraphicsPipeline(skRenderer* renderer);
Bool skRenderer_CheckExtensionsSupported(VkPhysicalDevice device);
Bool skRenderer_IsDeviceSuitable(VkPhysicalDevice device,
                                 VkSurfaceKHR     surface);
void skRenderer_CreateImageViews(skRenderer* renderer);
void skRenderer_CreateRenderPass(skRenderer* renderer);
void skRenderer_CreateFramebuffers(skRenderer* renderer);
void skRenderer_CreateCommandBuffers(skRenderer* renderer);
void skRenderer_CreateCommandPool(skRenderer* renderer);
void skRenderer_RecordCommandBuffer(skRenderer*     renderer,
                                    VkCommandBuffer commandBuffer,
                                    u32 imageIndex, skEditor* editor);
void skRenderer_CreateSyncObjects(skRenderer* renderer);
void skRenderer_CleanSwapchain(skRenderer* renderer);
u32  skRenderer_FindMemoryType(skRenderer* renderer, u32 typeFilter,
                               VkMemoryPropertyFlags properties);

void        skRenderer_CreateImage(skRenderer* renderer, u32 width,
                                   u32 height, VkFormat format,
                                   VkImageTiling         tiling,
                                   VkImageUsageFlags     usage,
                                   VkMemoryPropertyFlags properties,
                                   VkImage*              image,
                                   VkDeviceMemory*       imageMemory);
VkImageView skRenderer_CreateImageView(skRenderer* renderer,
                                       VkImage image, VkFormat format,
                                       VkImageAspectFlags flags);

void skRenderer_CreateDepthResources(skRenderer* renderer);
void skRenderer_RecreateSwapchain(skRenderer* renderer,
                                  skWindow*   window);
void skRenderer_UpdateUniformBuffers(skRenderer* renderer);
VkCommandBuffer
     skRenderer_BeginSingleTimeCommands(skRenderer* renderer);
void skRenderer_EndSingleTimeCommands(skRenderer*     renderer,
                                      VkCommandBuffer commandBuffer);
void skRenderer_CopyBuffer(skRenderer* renderer, VkBuffer srcBuffer,
                           VkBuffer dstBuffer, VkDeviceSize size);
void skRenderer_CopyBufferToImage(skRenderer* renderer,
                                  VkBuffer buffer, VkImage image,
                                  u32 width, u32 height);
void skRenderer_TransitionImageLayout(skRenderer* renderer,
                                      VkImage image, VkFormat format,
                                      VkImageLayout oldLayout,
                                      VkImageLayout newLayout);
Bool skHasStencilComponent(VkFormat format);
void skRenderer_CreateDescriptorSetLayout(skRenderer* renderer);
void skRenderer_CreateDescriptorSets(skRenderer* renderer);
void skRenderer_CreateDescriptorPool(skRenderer* renderer);
void skRenderer_Destroy(skRenderer* renderer);

typedef struct skRenderObject
{
    VkBuffer       vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer       indexBuffer;
    VkDeviceMemory indexBufferMemory;

    VkImage        textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView    textureImageView;
    VkSampler      textureSampler;
    u32            indexCount;

    VkDescriptorSet descriptorSets[SK_FRAMES_IN_FLIGHT];
    VkBuffer        uniformBuffers[SK_FRAMES_IN_FLIGHT];
    VkDeviceMemory  uniformBuffersMemory[SK_FRAMES_IN_FLIGHT];
    void*           uniformBuffersMap[SK_FRAMES_IN_FLIGHT];

    mat4 transform;
} skRenderObject;

skRenderObject
skRenderObject_CreateFromModel(skRenderer* renderer, skModel* model,
                               const char* texturePath);
skRenderObject
     skRenderObject_CreateFromSprite(skRenderer* renderer,
                                     const char* texturePath);
void skRenderer_CreateDescriptorSetsForObject(skRenderer* renderer,
                                              skRenderObject* obj);
void skRenderer_AddRenderObject(skRenderer*     renderer,
                                skRenderObject* object);
void skRenderer_AddLight(skRenderer* renderer, skLight* light);
