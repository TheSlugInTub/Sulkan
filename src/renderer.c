#include <sulkan/renderer.h>
#include <stdio.h>
#include <assert.h>
#include <synchapi.h>

#ifdef DEBUG
static const Bool enableValidationLayers = true;
#else
static const Bool enableValidationLayers = true;
#endif

VkVertexInputBindingDescription skVertex_GetBindingDescription()
{
    VkVertexInputBindingDescription description = {0};

    description.binding = 0;
    description.stride = sizeof(skVertex);
    description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return description;
}

typedef struct VkVertexInputAttributeDescriptions
{
    VkVertexInputAttributeDescription descriptions[2];
} VkVertexInputAttributeDescriptions;

VkVertexInputAttributeDescriptions skVertex_GetAttributeDescription()
{
    VkVertexInputAttributeDescription descriptions[2] = {0};

    descriptions[0].binding = 0;
    descriptions[0].location = 0;
    descriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    descriptions[0].offset = offsetof(skVertex, pos);

    descriptions[1].binding = 0;
    descriptions[1].location = 1;
    descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    descriptions[1].offset = offsetof(skVertex, colour);

    VkVertexInputAttributeDescriptions pair = {descriptions[0],
                                               descriptions[1]};

    return pair;
}

char* skReadFile(const char* filePath, u32* len)
{
    FILE* shaderStream = fopen(filePath, "rb");
    if (shaderStream == NULL)
    {
        printf("SK ERROR: Failed to open file.");
    }

    fseek(shaderStream, 0, SEEK_END);
    size_t length = ftell(shaderStream);
    fseek(shaderStream, 0, SEEK_SET);

    char* shaderCode = (char*)malloc(length);
    fread(shaderCode, sizeof(char), length, shaderStream);
    fclose(shaderStream);

    *len = length;
    return shaderCode;
}

VkShaderModule skCreateShaderModule(skRenderer* renderer,
                                    char* buffer, u32 len)
{
    VkShaderModuleCreateInfo createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = len;
    createInfo.pCode = (const u32*)buffer;

    if (len % 4 != 0)
    {
        printf("SK ERROR: Shader code size must be multiple of 4 "
               "bytes for SPIR-V\n");
        return VK_NULL_HANDLE;
    }

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(renderer->device, &createInfo, NULL,
                             &shaderModule) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create shader module.");
    }

    return shaderModule;
}

// Standard validation layer
static const char* validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"};

static const uint32_t validationLayerCount =
    sizeof(validationLayers) / sizeof(validationLayers[0]);

// Debug callback function
static VKAPI_ATTR VkBool32 VKAPI_CALL skDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT             messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void*                                       pUserData)
{

    printf("VALIDATION LAYER: %s\n", pCallbackData->pMessage);
    return VK_FALSE;
}

// Helper function to create debug messenger
VkResult skCreateDebugUtilsMessengerEXT(
    VkInstance                                instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks*              pAllocator,
    VkDebugUtilsMessengerEXT*                 pDebugMessenger)
{
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != NULL)
    {
        return func(instance, pCreateInfo, pAllocator,
                    pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

// Helper function to destroy debug messenger
void skDestroyDebugUtilsMessengerEXT(
    VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator)
{
    PFN_vkDestroyDebugUtilsMessengerEXT func =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

// Function to check if validation layers are available
Bool skCheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties* availableLayers = (VkLayerProperties*)malloc(
        layerCount * sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for (uint32_t i = 0; i < validationLayerCount; i++)
    {
        Bool layerFound = false;

        for (uint32_t j = 0; j < layerCount; j++)
        {
            if (strcmp(validationLayers[i],
                       availableLayers[j].layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            free(availableLayers);
            return false;
        }
    }

    free(availableLayers);
    return true;
}

// Function to populate debug messenger create info
void skPopulateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT* createInfo)
{
    createInfo->sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo->messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo->messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo->pfnUserCallback = skDebugCallback;
    createInfo->pUserData = NULL;
}

VkSurfaceFormatKHR
skChooseSwapSurfaceFormat(skVector* availableFormats)
{
    for (int i = 0; i < availableFormats->size; i++)
    {
        VkSurfaceFormatKHR* formatPtr =
            (VkSurfaceFormatKHR*)skVector_Get(availableFormats, i);
        VkSurfaceFormatKHR availableFormat = *formatPtr;

        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace ==
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return *((VkSurfaceFormatKHR*)skVector_Get(availableFormats, 0));
}

VkPresentModeKHR
skChooseSwapPresentMode(skVector* availablePresentModes)
{
    for (int i = 0; i < availablePresentModes->size; i++)
    {
        VkPresentModeKHR* formatPtr =
            (VkPresentModeKHR*)skVector_Get(availablePresentModes, i);
        VkPresentModeKHR availableMode = *formatPtr;

        if (availableMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availableMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

#include <limits.h>

u32 skClampU32(u32 val, u32 low, u32 high)
{
    if (val < low)
    {
        return low;
    }
    else if (val > high)
    {
        return high;
    }

    return val;
}

VkExtent2D skChooseSwapExtent(VkSurfaceCapabilitiesKHR* capabilities,
                              skWindow*                 window)
{
    if (capabilities->currentExtent.width != UINT32_MAX)
    {
        return capabilities->currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window->window, &width, &height);

        VkExtent2D actualExtent = {(uint32_t)(width),
                                   (uint32_t)(height)};
        actualExtent.width = skClampU32(
            actualExtent.width, capabilities->minImageExtent.width,
            capabilities->maxImageExtent.width);
        actualExtent.height = skClampU32(
            actualExtent.height, capabilities->minImageExtent.height,
            capabilities->maxImageExtent.height);

        return actualExtent;
    }
}

skSwapchainDetails skQuerySwapchainSupport(skRenderer* renderer)
{
    skSwapchainDetails details = {0};

    details.formats = skVector_Create(sizeof(VkSurfaceFormatKHR), 1);
    details.presentModes =
        skVector_Create(sizeof(VkPresentModeKHR), 1);

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        renderer->physicalDevice, renderer->surface,
        &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(renderer->physicalDevice,
                                         renderer->surface,
                                         &formatCount, NULL);

    if (formatCount != 0)
    {
        skVector_Resize(details.formats, formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            renderer->physicalDevice, renderer->surface, &formatCount,
            (VkSurfaceFormatKHR*)details.formats->data);
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        renderer->physicalDevice, renderer->surface,
        &presentModeCount, NULL);

    if (presentModeCount != 0)
    {
        skVector_Resize(details.presentModes, presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            renderer->physicalDevice, renderer->surface,
            &presentModeCount,
            (VkPresentModeKHR*)details.presentModes->data);
    }

    return details;
}

skQueueFamilyIndices skFindQueueFamilies(VkPhysicalDevice device,
                                         VkSurfaceKHR     surface)
{
    skQueueFamilyIndices indices = {0};

    indices.isValid = false;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device,
                                             &queueFamilyCount, NULL);

    skVector* queueFamilies = skVector_Create(
        sizeof(VkQueueFamilyProperties), queueFamilyCount);
    queueFamilies->size = queueFamilyCount;

    vkGetPhysicalDeviceQueueFamilyProperties(
        device, &queueFamilyCount,
        (VkQueueFamilyProperties*)queueFamilies->data);

    for (int i = 0; i < queueFamilies->size; i++)
    {
        VkQueueFamilyProperties* prop =
            (VkQueueFamilyProperties*)skVector_Get(queueFamilies, i);

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface,
                                             &presentSupport);

        if ((prop->queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
            presentSupport)
        {
            indices.graphicsFamily = i;
            indices.presentFamily = i;
            indices.isValid = true;
        }
    }

    return indices;
}

void skRenderer_CreateSwapchain(skRenderer* renderer,
                                skWindow*   window)
{
    skSwapchainDetails details = skQuerySwapchainSupport(renderer);
    VkSurfaceFormatKHR format =
        skChooseSwapSurfaceFormat(details.formats);
    VkPresentModeKHR mode =
        skChooseSwapPresentMode(details.presentModes);
    VkExtent2D extent =
        skChooseSwapExtent(&details.capabilities, window);

    uint32_t imageCount = details.capabilities.minImageCount + 1;
    if (details.capabilities.maxImageCount > 0 &&
        imageCount > details.capabilities.maxImageCount)
    {
        imageCount = details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {0};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = renderer->surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = format.format;
    createInfo.imageColorSpace = format.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    skQueueFamilyIndices indices = skFindQueueFamilies(
        renderer->physicalDevice, renderer->surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily,
                                     indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;  // Optional
        createInfo.pQueueFamilyIndices = NULL; // Optional
    }

    createInfo.preTransform = details.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = mode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(renderer->device, &createInfo, NULL,
                             &renderer->swapchain) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create swap chain.");
    }

    renderer->swapchainImageFormat = format.format;
    renderer->swapchainExtent = extent;

    renderer->swapchainImages = skVector_Create(sizeof(VkImage), 3);

    vkGetSwapchainImagesKHR(renderer->device, renderer->swapchain,
                            &imageCount, NULL);
    skVector_Resize(renderer->swapchainImages, imageCount);
    vkGetSwapchainImagesKHR(
        renderer->device, renderer->swapchain, &imageCount,
        (VkImage*)renderer->swapchainImages->data);
}

void skRenderer_CreateGraphicsPipeline(skRenderer* renderer)
{
    u32   vertLen, fragLen;
    char* vertShaderCode =
        skReadFile("D:/Repos/Sulkan/vert.spv", &vertLen);
    char* fragShaderCode =
        skReadFile("D:/Repos/Sulkan/frag.spv", &fragLen);

    VkShaderModule vertMod =
        skCreateShaderModule(renderer, vertShaderCode, vertLen);
    VkShaderModule fragMod =
        skCreateShaderModule(renderer, fragShaderCode, fragLen);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
    vertShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

    vertShaderStageInfo.module = vertMod;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
    fragShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragMod;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {
        vertShaderStageInfo, fragShaderStageInfo};

    VkVertexInputBindingDescription bindingDescription =
        skVertex_GetBindingDescription();
    VkVertexInputAttributeDescriptions attributeDescriptions =
        skVertex_GetAttributeDescription();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = 2;
    vertexInputInfo.pVertexAttributeDescriptions =
        attributeDescriptions.descriptions;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

    VkDynamicState dynamicStates[2] = {VK_DYNAMIC_STATE_VIEWPORT,
                                       VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState = {0};
    dynamicState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = 2;
    dynamicState.pDynamicStates = dynamicStates;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)renderer->swapchainExtent.width;
    viewport.height = (float)renderer->swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D) {0, 0};
    scissor.extent = renderer->swapchainExtent;

    VkPipelineViewportStateCreateInfo viewportState = {0};
    viewportState.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = NULL;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor =
        VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {0};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;
    colorBlending.blendConstants[1] = 0.0f;
    colorBlending.blendConstants[2] = 0.0f;
    colorBlending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
    pipelineLayoutInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = NULL;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = NULL;

    if (vkCreatePipelineLayout(renderer->device, &pipelineLayoutInfo,
                               NULL, &renderer->pipelineLayout) !=
        VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create pipeline layout.\n");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {0};
    pipelineInfo.sType =
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;

    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = NULL;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = renderer->pipelineLayout;

    pipelineInfo.renderPass = renderer->renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(renderer->device, VK_NULL_HANDLE, 1,
                                  &pipelineInfo, NULL,
                                  &renderer->pipeline) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create graphics pipeline.\n");
    }
}

Bool skRenderer_CheckExtensionsSupported(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL,
                                         &extensionCount, NULL);

    VkExtensionProperties* availableExtensions =
        (VkExtensionProperties*)malloc(extensionCount *
                                       sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(
        device, NULL, &extensionCount, availableExtensions);

    const char* requiredExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    uint32_t requiredExtensionCount = 1;

    for (uint32_t i = 0; i < requiredExtensionCount; i++)
    {
        Bool found = false;
        for (uint32_t j = 0; j < extensionCount; j++)
        {
            if (strcmp(requiredExtensions[i],
                       availableExtensions[j].extensionName) == 0)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            free(availableExtensions);
            return false;
        }
    }

    free(availableExtensions);
    return true;
}

Bool skRenderer_IsDeviceSuitable(VkPhysicalDevice device,
                                 VkSurfaceKHR     surface)
{
    skQueueFamilyIndices indices =
        skFindQueueFamilies(device, surface);
    return indices.isValid &&
           skRenderer_CheckExtensionsSupported(device);
}

void skRenderer_CreateImageViews(skRenderer* renderer)
{
    renderer->swapchainImageViews = skVector_Create(
        sizeof(VkImageView), renderer->swapchainImages->size);

    VkImageView imageView = {0};

    for (int i = 0; i < renderer->swapchainImages->size; i++)
    {
        skVector_PushBack(renderer->swapchainImageViews, &imageView);

        VkImage* swapchainImage =
            (VkImage*)skVector_Get(renderer->swapchainImages, i);

        VkImageView* swapchainImageView = (VkImageView*)skVector_Get(
            renderer->swapchainImageViews, i);

        VkImageViewCreateInfo createInfo = {0};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = *swapchainImage;

        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = renderer->swapchainImageFormat;

        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask =
            VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(renderer->device, &createInfo, NULL,
                              swapchainImageView) != VK_SUCCESS)
        {
            printf("SK ERROR: Failed to create image views.");
        }
    }
}

void skRenderer_CreateRenderPass(skRenderer* renderer)
{
    VkAttachmentDescription colorAttachment = {0};
    colorAttachment.format = renderer->swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {0};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout =
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(renderer->device, &renderPassInfo, NULL,
                           &renderer->renderPass) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create render pass.\n");
    }
}

void skRenderer_CreateFrameBuffers(skRenderer* renderer)
{
    renderer->swapchainFramebuffers =
        skVector_Create(sizeof(VkFramebuffer), 1);

    for (size_t i = 0; i < renderer->swapchainImageViews->size; i++)
    {
        VkFramebuffer framebuf = {0};

        VkImageView attachments[1] = {0};
        attachments[0] = *(VkImageView*)skVector_Get(
            renderer->swapchainImageViews, i);

        VkFramebufferCreateInfo framebufferInfo = {0};
        framebufferInfo.sType =
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderer->renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = renderer->swapchainExtent.width;
        framebufferInfo.height = renderer->swapchainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(renderer->device, &framebufferInfo,
                                NULL, &framebuf) != VK_SUCCESS)
        {
            printf("SK ERROR: Failed to create framebuffer.");
        }

        skVector_PushBack(renderer->swapchainFramebuffers, &framebuf);
    }
}

void skRenderer_CreateCommandBuffers(skRenderer* renderer)
{
    renderer->commandBuffers =
        skVector_Create(sizeof(VkCommandBuffer), 2);
    skVector_Resize(renderer->commandBuffers, SK_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = renderer->commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = SK_FRAMES_IN_FLIGHT;

    if (vkAllocateCommandBuffers(
            renderer->device, &allocInfo,
            (VkCommandBuffer*)renderer->commandBuffers->data) !=
        VK_SUCCESS)
    {
        printf("SK ERROR: Failed to allocate command buffers.");
    }
}

void skRenderer_CreateCommandPool(skRenderer* renderer)
{
    skQueueFamilyIndices indices = skFindQueueFamilies(
        renderer->physicalDevice, renderer->surface);

    VkCommandPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = indices.graphicsFamily;

    if (vkCreateCommandPool(renderer->device, &poolInfo, NULL,
                            &renderer->commandPool) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create command pool.");
    }
}

void skRenderer_RecordCommandBuffer(skRenderer*     renderer,
                                    VkCommandBuffer commandBuffer,
                                    u32             imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;
    beginInfo.pInheritanceInfo = NULL;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to begin recording command buffer.");
    }

    VkFramebuffer framebuf = *(VkFramebuffer*)skVector_Get(
        renderer->swapchainFramebuffers, imageIndex);

    VkRenderPassBeginInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderer->renderPass;
    renderPassInfo.framebuffer = framebuf;

    renderPassInfo.renderArea.offset = (VkOffset2D) {0, 0};
    renderPassInfo.renderArea.extent = renderer->swapchainExtent;

    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      renderer->pipeline);

    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)(renderer->swapchainExtent.width);
    viewport.height = (float)(renderer->swapchainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D) {0, 0};
    scissor.extent = renderer->swapchainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer     vertexBuffers[] = {renderer->vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers,
                           offsets);

    vkCmdBindIndexBuffer(commandBuffer, renderer->indexBuffer, 0,
                         VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(commandBuffer, 6, 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to record command buffer.");
    }
}

void skRenderer_CreateSyncObjects(skRenderer* renderer)
{
    renderer->imageAvailableSemaphores =
        skVector_Create(sizeof(VkSemaphore), SK_FRAMES_IN_FLIGHT);
    skVector_Resize(renderer->imageAvailableSemaphores,
                    SK_FRAMES_IN_FLIGHT);
    renderer->renderFinishedSemaphores =
        skVector_Create(sizeof(VkSemaphore), SK_FRAMES_IN_FLIGHT);
    skVector_Resize(renderer->renderFinishedSemaphores,
                    SK_FRAMES_IN_FLIGHT);
    renderer->inFlightFences =
        skVector_Create(sizeof(VkFence), SK_FRAMES_IN_FLIGHT);
    skVector_Resize(renderer->inFlightFences, SK_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {0};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < SK_FRAMES_IN_FLIGHT; i++)
    {
        VkFence* fence = skVector_Get(renderer->inFlightFences, i);
        VkSemaphore* sema1 =
            skVector_Get(renderer->imageAvailableSemaphores, i);
        VkSemaphore* sema2 =
            skVector_Get(renderer->renderFinishedSemaphores, i);

        if (vkCreateSemaphore(renderer->device, &semaphoreInfo, NULL,
                              sema1) != VK_SUCCESS ||
            vkCreateSemaphore(renderer->device, &semaphoreInfo, NULL,
                              sema2) != VK_SUCCESS ||
            vkCreateFence(renderer->device, &fenceInfo, NULL,
                          fence) != VK_SUCCESS)
        {
            printf("SK ERROR: Failed to create semaphores.\n");
        }
    }
}

void skRenderer_CleanSwapchain(skRenderer* renderer)
{
    // Destroy framebuffers
    for (int i = 0; i < renderer->swapchainFramebuffers->size; i++)
    {
        VkFramebuffer framebuffer = *(VkFramebuffer*)skVector_Get(
            renderer->swapchainFramebuffers, i);
        vkDestroyFramebuffer(renderer->device, framebuffer, NULL);
    }
    skVector_Clear(renderer->swapchainFramebuffers); // Clear vector

    // Destroy pipeline and render pass if they exist
    if (renderer->pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(renderer->device, renderer->pipeline, NULL);
        renderer->pipeline = VK_NULL_HANDLE;
    }

    if (renderer->renderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(renderer->device, renderer->renderPass,
                            NULL);
        renderer->renderPass = VK_NULL_HANDLE;
    }

    // Destroy image views
    for (int i = 0; i < renderer->swapchainImageViews->size; i++)
    {
        VkImageView imageView = *(VkImageView*)skVector_Get(
            renderer->swapchainImageViews, i);
        vkDestroyImageView(renderer->device, imageView, NULL);
    }
    skVector_Clear(renderer->swapchainImageViews);

    // Destroy swapchain
    if (renderer->swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(renderer->device, renderer->swapchain,
                              NULL);
        renderer->swapchain = VK_NULL_HANDLE;
    }
}

void skRenderer_RecreateSwapchain(skRenderer* renderer,
                                  skWindow*   window)
{
    vkDeviceWaitIdle(renderer->device);
    skRenderer_CleanSwapchain(renderer);

    // Recreate core swapchain resources
    skRenderer_CreateSwapchain(renderer, window);
    skRenderer_CreateImageViews(renderer);
    skRenderer_CreateRenderPass(renderer);
    skRenderer_CreateGraphicsPipeline(renderer);
    skRenderer_CreateFrameBuffers(renderer);

    // Recreate command buffers
    if (renderer->commandBuffers &&
        renderer->commandBuffers->size > 0)
    {
        vkFreeCommandBuffers(
            renderer->device, renderer->commandPool,
            renderer->commandBuffers->size,
            (VkCommandBuffer*)renderer->commandBuffers->data);
        renderer->commandBuffers->size = 0; // Reset vector
    }
    skRenderer_CreateCommandBuffers(renderer);
}

void skRenderer_DrawFrame(skRenderer* renderer)
{
    u32 currentFrame = renderer->currentFrame;

    VkFence* inFlightFence = (VkFence*)skVector_Get(
        renderer->inFlightFences, currentFrame);
    VkSemaphore* imageAvailableSemaphore = (VkSemaphore*)skVector_Get(
        renderer->imageAvailableSemaphores, currentFrame);
    VkSemaphore* renderFinishedSemaphore = (VkSemaphore*)skVector_Get(
        renderer->renderFinishedSemaphores, currentFrame);
    VkCommandBuffer* commandBuffer = (VkCommandBuffer*)skVector_Get(
        renderer->commandBuffers, currentFrame);
    VkCommandBuffer cmdBuffer = *commandBuffer;

    vkWaitForFences(renderer->device, 1, inFlightFence, VK_TRUE,
                    UINT64_MAX);

    u32      imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        renderer->device, renderer->swapchain, UINT64_MAX,
        *imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    vkResetFences(renderer->device, 1, inFlightFence);

    vkResetCommandBuffer(cmdBuffer, 0);

    skRenderer_RecordCommandBuffer(renderer, cmdBuffer, imageIndex);

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {*imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = commandBuffer;

    VkSemaphore signalSemaphores[] = {*renderFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(renderer->graphicsQueue, 1, &submitInfo,
                      *inFlightFence) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to submit draw command buffer.");
    }

    VkPresentInfoKHR presentInfo = {0};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {renderer->swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL;

    result = vkQueuePresentKHR(renderer->presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR ||
        result == VK_SUBOPTIMAL_KHR ||
        renderer->window->framebufferResized)
    {
        renderer->window->framebufferResized = false;
        skRenderer_RecreateSwapchain(renderer, renderer->window);
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        printf("SK ERROR: Failed to acquire swapchain image.\n");
    }

    renderer->currentFrame = (currentFrame + 1) % SK_FRAMES_IN_FLIGHT;
}

void skRenderer_CreateInstance(skRenderer* renderer)
{
    if (enableValidationLayers && !skCheckValidationLayerSupport())
    {
        printf("SK ERROR: Validation layers requested, but not "
               "available.\n");
    }

    // Send information about our application to the Vulkan API
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

    // Get required vulkan extensions from GLFW
    glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // Create extensions array with space for other extensions
    uint32_t     extensionCount = glfwExtensionCount;
    const char** extensions = (const char**)malloc(
        (glfwExtensionCount + 1) * sizeof(const char*));

    for (uint32_t i = 0; i < glfwExtensionCount; i++)
    {
        extensions[i] = glfwExtensions[i];
    }

    if (enableValidationLayers)
    {
        // Enable the validation layer extension
        extensions[extensionCount++] =
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }

    createInfo.enabledExtensionCount = extensionCount;
    createInfo.ppEnabledExtensionNames = extensions;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {0};
    if (enableValidationLayers)
    {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;

        skPopulateDebugMessengerCreateInfo(&debugCreateInfo);
        createInfo.pNext =
            (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = NULL;
    }

    // Create the instance
    if (vkCreateInstance(&createInfo, NULL, &renderer->instance) !=
        VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create Vulkan instance.\n");
    }

    free(extensions);
}

void skRenderer_CreateSurface(skRenderer* renderer, skWindow* window)
{
    if (glfwCreateWindowSurface(renderer->instance, window->window,
                                NULL,
                                &renderer->surface) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create window surface.");
    }
}

void skRenderer_CreatePhysicalDevice(skRenderer* renderer)
{
    u32 deviceCount = 0;

    // First call: get the number of devices
    vkEnumeratePhysicalDevices(renderer->instance, &deviceCount,
                               NULL);

    if (deviceCount == 0)
    {
        printf("SK ERROR: No physical device found.\n");
        exit(1);
    }

    // Create vector with the correct size
    skVector* physicalDevices =
        skVector_Create(sizeof(VkPhysicalDevice), deviceCount);

    // Second call: get the actual devices
    vkEnumeratePhysicalDevices(
        renderer->instance, &deviceCount,
        (VkPhysicalDevice*)physicalDevices->data);

    // Update the vector's size to reflect the actual number of
    // devices
    physicalDevices->size = deviceCount;

    for (int i = 0; i < physicalDevices->size; i++)
    {
        VkPhysicalDevice* devicePtr =
            (VkPhysicalDevice*)skVector_Get(physicalDevices, i);
        VkPhysicalDevice device = *devicePtr;

        if (skRenderer_IsDeviceSuitable(device, renderer->surface))
        {
            renderer->physicalDevice = device;
            break; // Found a suitable device, no need to continue
        }
    }

    if (renderer->physicalDevice == VK_NULL_HANDLE)
    {
        printf(
            "SK ERROR: Failed to find a suitable physical device.\n");
    }
}

void skRenderer_CreateLogicalDevice(skRenderer* renderer)
{
    skQueueFamilyIndices indices = skFindQueueFamilies(
        renderer->physicalDevice, renderer->surface);

    // Create queue create infos for unique queue families
    skVector* queueCreateInfos =
        skVector_Create(sizeof(VkDeviceQueueCreateInfo), 2);

    uint32_t uniqueQueueFamilies[2];
    int      uniqueCount = 0;

    // Add graphics family
    uniqueQueueFamilies[uniqueCount++] = indices.graphicsFamily;

    // Add present family only if it's different from graphics family
    if (indices.presentFamily != indices.graphicsFamily)
    {
        uniqueQueueFamilies[uniqueCount++] = indices.presentFamily;
    }

    float queuePriority = 1.0f;
    for (int i = 0; i < uniqueCount; i++)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {0};
        queueCreateInfo.sType =
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        skVector_PushBack(queueCreateInfos, &queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {0};

    // Define required device extensions
    const char* deviceExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo deviceCreateInfo = {0};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos =
        (VkDeviceQueueCreateInfo*)queueCreateInfos->data;
    deviceCreateInfo.queueCreateInfoCount =
        (uint32_t)queueCreateInfos->size;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    // Enable swapchain extension
    deviceCreateInfo.enabledExtensionCount = 1;
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;

    if (enableValidationLayers)
    {
        deviceCreateInfo.enabledLayerCount = validationLayerCount;
        deviceCreateInfo.ppEnabledLayerNames = validationLayers;
    }
    else
    {
        deviceCreateInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(renderer->physicalDevice, &deviceCreateInfo,
                       NULL, &renderer->device) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create logical device.");
    }

    // Get the queue handles
    vkGetDeviceQueue(renderer->device, indices.graphicsFamily, 0,
                     &renderer->graphicsQueue);
    vkGetDeviceQueue(renderer->device, indices.presentFamily, 0,
                     &renderer->presentQueue);

    // Clean up
    skVector_Free(queueCreateInfos);
}

void skRenderer_CreateDebugMessenger(skRenderer* renderer)
{
    // Setup debug messenger
    if (enableValidationLayers)
    {
        VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo =
            {0};
        skPopulateDebugMessengerCreateInfo(&debugMessengerCreateInfo);

        if (skCreateDebugUtilsMessengerEXT(
                renderer->instance, &debugMessengerCreateInfo, NULL,
                &renderer->debugMessenger) != VK_SUCCESS)
        {
            printf("SK ERROR: Failed to set up debug messenger!\n");
        }
    }
}

u32 skRenderer_FindMemoryType(skRenderer* renderer, u32 typeFilter,
                              VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(renderer->physicalDevice,
                                        &memProperties);

    for (u32 i = 0; i < memProperties.memoryTypeCount; i++)
    {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags &
             properties) == properties)
        {
            return i;
        }
    }

    printf("SK ERROR: Failed to find suitable memory type.\n");
    return -1;
}

void skRenderer_CreateBuffer(skRenderer* renderer, size_t size,
                             VkBufferUsageFlags    usage,
                             VkMemoryPropertyFlags properties,
                             VkBuffer*             buffer,
                             VkDeviceMemory*       bufferMemory)
{
    VkBufferCreateInfo bufferInfo = {0};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(renderer->device, &bufferInfo, NULL, buffer) !=
        VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create buffer.\n");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(renderer->device, *buffer,
                                  &memRequirements);

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = skRenderer_FindMemoryType(
        renderer, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(renderer->device, &allocInfo, NULL,
                         bufferMemory) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to allocate buffer memory.");
    }

    vkBindBufferMemory(renderer->device, *buffer, *bufferMemory, 0);
}

void skRenderer_CopyBuffer(skRenderer* renderer, VkBuffer srcBuffer,
                           VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = renderer->commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(renderer->device, &allocInfo,
                             &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {0};
    copyRegion.srcOffset = 0; // Optional
    copyRegion.dstOffset = 0; // Optional
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1,
                    &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(renderer->graphicsQueue, 1, &submitInfo,
                  VK_NULL_HANDLE);
    vkQueueWaitIdle(renderer->graphicsQueue);

    vkFreeCommandBuffers(renderer->device, renderer->commandPool, 1,
                         &commandBuffer);
}

void skRenderer_CreateVertexBuffer(skRenderer* renderer)
{
    const skVertex vertices[] = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                 {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
                                 {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
                                 {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}};

    size_t bufferSize = sizeof(vertices[0]) * 4;

    VkBuffer       stagingBuffer;
    VkDeviceMemory stagingMemory;
    skRenderer_CreateBuffer(renderer, bufferSize,
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            &stagingBuffer, &stagingMemory);

    void* data;
    vkMapMemory(renderer->device, stagingMemory, 0, bufferSize, 0,
                &data);

    memcpy(data, vertices, sizeof(vertices[0]) * 4);

    vkUnmapMemory(renderer->device, stagingMemory);

    skRenderer_CreateBuffer(renderer, bufferSize,
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            &renderer->vertexBuffer,
                            &renderer->vertexBufferMemory);

    skRenderer_CopyBuffer(renderer, stagingBuffer,
                          renderer->vertexBuffer, bufferSize);

    vkDestroyBuffer(renderer->device, stagingBuffer, NULL);
    vkFreeMemory(renderer->device, stagingMemory, NULL);
}

void skRenderer_CreateIndexBuffer(skRenderer* renderer)
{
    const u16 indices[] = {0, 1, 2, 2, 3, 0};

    size_t bufferSize = sizeof(indices[0]) * 6;

    VkBuffer       stagingBuffer;
    VkDeviceMemory stagingMemory;
    skRenderer_CreateBuffer(renderer, bufferSize,
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            &stagingBuffer, &stagingMemory);

    void* data;
    vkMapMemory(renderer->device, stagingMemory, 0, bufferSize, 0,
                &data);

    memcpy(data, indices, sizeof(indices[0]) * 6);

    vkUnmapMemory(renderer->device, stagingMemory);

    skRenderer_CreateBuffer(renderer, bufferSize,
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            &renderer->indexBuffer,
                            &renderer->indexBufferMemory);

    skRenderer_CopyBuffer(renderer, stagingBuffer,
                          renderer->indexBuffer, bufferSize);

    vkDestroyBuffer(renderer->device, stagingBuffer, NULL);
    vkFreeMemory(renderer->device, stagingMemory, NULL);
}

skRenderer skRenderer_Create(skWindow* window)
{
    skRenderer  renderer = {0};
    skRenderer* rendererPtr = &renderer;

    renderer.currentFrame = 0;
    renderer.window = window;

    skRenderer_CreateInstance(rendererPtr);
    skRenderer_CreateSurface(rendererPtr, window);
    skRenderer_CreateDebugMessenger(rendererPtr);
    skRenderer_CreatePhysicalDevice(rendererPtr);
    skRenderer_CreateLogicalDevice(rendererPtr);
    skRenderer_CreateSwapchain(&renderer, window);
    skRenderer_CreateImageViews(&renderer);
    skRenderer_CreateRenderPass(&renderer);
    skRenderer_CreateGraphicsPipeline(&renderer);
    skRenderer_CreateFrameBuffers(&renderer);
    skRenderer_CreateCommandPool(&renderer);
    skRenderer_CreateCommandBuffers(&renderer);
    skRenderer_CreateVertexBuffer(&renderer);
    skRenderer_CreateIndexBuffer(&renderer);
    skRenderer_CreateSyncObjects(&renderer);

    return renderer;
}

void skRenderer_Destroy(skRenderer* renderer)
{
    if (enableValidationLayers)
    {
        skDestroyDebugUtilsMessengerEXT(
            renderer->instance, renderer->debugMessenger, NULL);
    }

    vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
    vkDestroyDevice(renderer->device, NULL);
    vkDestroyInstance(renderer->instance, NULL);
    vkDestroyPipelineLayout(renderer->device,
                            renderer->pipelineLayout, NULL);
    vkDestroyPipeline(renderer->device, renderer->pipeline, NULL);
    vkDestroyRenderPass(renderer->device, renderer->renderPass, NULL);
}
