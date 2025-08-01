#include <sulkan/renderer.h>
#include <stdio.h>
#include <assert.h>
#include <synchapi.h>
#include <stb/stb_image.h>
#include <sulkan/imgui_layer.h>
#include <sulkan/editor.h>
#include <sulkan/basic_components.h>

static const Bool enableValidationLayers = true;

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
    VkVertexInputAttributeDescription descriptions[7];
} VkVertexInputAttributeDescriptions;

VkVertexInputAttributeDescriptions skVertex_GetAttributeDescription()
{
    VkVertexInputAttributeDescription descriptions[7] = {0};

    descriptions[0].binding = 0;
    descriptions[0].location = 0;
    descriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    descriptions[0].offset = offsetof(skVertex, position);

    descriptions[1].binding = 0;
    descriptions[1].location = 1;
    descriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    descriptions[1].offset = offsetof(skVertex, normal);

    descriptions[2].binding = 0;
    descriptions[2].location = 2;
    descriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    descriptions[2].offset = offsetof(skVertex, textureCoordinates);

    descriptions[3].binding = 0;
    descriptions[3].location = 3;
    descriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
    descriptions[3].offset = offsetof(skVertex, tangent);

    descriptions[4].binding = 0;
    descriptions[4].location = 4;
    descriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
    descriptions[4].offset = offsetof(skVertex, bitangent);

    descriptions[5].binding = 0;
    descriptions[5].location = 5;
    descriptions[5].format = VK_FORMAT_R32G32B32A32_SINT;
    descriptions[5].offset = offsetof(skVertex, boneIDs);

    descriptions[6].binding = 0;
    descriptions[6].location = 6;
    descriptions[6].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    descriptions[6].offset = offsetof(skVertex, weights);

    VkVertexInputAttributeDescriptions pair = {
        descriptions[0], descriptions[1], descriptions[2],
        descriptions[3], descriptions[4], descriptions[5],
        descriptions[6]};

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

static const u32 validationLayerCount =
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
    u32 layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);

    VkLayerProperties* availableLayers = (VkLayerProperties*)malloc(
        layerCount * sizeof(VkLayerProperties));
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

    for (u32 i = 0; i < validationLayerCount; i++)
    {
        Bool layerFound = false;

        for (u32 j = 0; j < layerCount; j++)
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

        if (availableMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
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

        VkExtent2D actualExtent = {(u32)(width), (u32)(height)};
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

    u32 formatCount;
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

    u32 presentModeCount;
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

    u32 queueFamilyCount = 0;
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

    u32 imageCount = details.capabilities.minImageCount + 1;
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
    u32 queueFamilyIndices[] = {indices.graphicsFamily,
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
    char* vertShaderCode = skReadFile("shaders/vert.spv", &vertLen);
    char* fragShaderCode = skReadFile("shaders/frag.spv", &fragLen);

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
    vertexInputInfo.vertexAttributeDescriptionCount = 7;
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
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
    pipelineLayoutInfo.setLayoutCount = 4;
    VkDescriptorSetLayout layouts[] = {
        renderer->descriptorSetLayout,
        renderer->lightDescriptorSetLayout,
        renderer->uniformDescriptorSetLayout,
        renderer->bonesDescriptorSetLayout};
    pipelineLayoutInfo.pSetLayouts = layouts;
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
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = renderer->pipelineLayout;

    pipelineInfo.renderPass = renderer->renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {0};
    depthStencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = (VkStencilOpState) {0};
    depthStencil.back = (VkStencilOpState) {0};

    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(renderer->device, VK_NULL_HANDLE, 1,
                                  &pipelineInfo, NULL,
                                  &renderer->pipeline) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create graphics pipeline.\n");
    }
}

void skRenderer_CreateSkyboxGraphicsPipeline(skRenderer* renderer)
{
    u32   vertLen, fragLen;
    char* vertShaderCode =
        skReadFile("shaders/skybox_vert.spv", &vertLen);
    char* fragShaderCode =
        skReadFile("shaders/skybox_frag.spv", &fragLen);

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
    vertexInputInfo.vertexAttributeDescriptionCount = 7;
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
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
    pipelineLayoutInfo.setLayoutCount = 3;
    VkDescriptorSetLayout layouts[] = {
        renderer->descriptorSetLayout,
        renderer->lightDescriptorSetLayout,
        renderer->uniformDescriptorSetLayout};
    pipelineLayoutInfo.pSetLayouts = layouts;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = NULL;

    if (vkCreatePipelineLayout(
            renderer->device, &pipelineLayoutInfo, NULL,
            &renderer->skyboxPipelineLayout) != VK_SUCCESS)
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
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;

    pipelineInfo.layout = renderer->skyboxPipelineLayout;

    pipelineInfo.renderPass = renderer->renderPass;
    pipelineInfo.subpass = 0;

    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex = -1;

    VkPipelineDepthStencilStateCreateInfo depthStencil = {0};
    depthStencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_FALSE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = (VkStencilOpState) {0};
    depthStencil.back = (VkStencilOpState) {0};

    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(
            renderer->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL,
            &renderer->skyboxPipeline) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create graphics pipeline.\n");
    }
}

void skRenderer_CreateLineGraphicsPipeline(skRenderer* renderer)
{
    u32   vertLen, fragLen;
    char* vertShaderCode =
        skReadFile("shaders/line_vert.spv", &vertLen);
    char* fragShaderCode =
        skReadFile("shaders/line_frag.spv", &fragLen);

    VkShaderModule vertMod =
        skCreateShaderModule(renderer, vertShaderCode, vertLen);
    VkShaderModule fragMod =
        skCreateShaderModule(renderer, fragShaderCode, fragLen);

    VkPipelineShaderStageCreateInfo vertStage = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertMod,
        .pName = "main"};

    VkPipelineShaderStageCreateInfo fragStage = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragMod,
        .pName = "main"};

    VkPipelineShaderStageCreateInfo stages[] = {vertStage, fragStage};

    VkVertexInputBindingDescription bindingDesc = {
        .binding = 0,
        .stride = sizeof(vec3),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};

    VkVertexInputAttributeDescription attributeDesc = {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = 0};

    VkPipelineVertexInputStateCreateInfo vertexInput = {
        .sType =
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDesc,
        .vertexAttributeDescriptionCount = 1,
        .pVertexAttributeDescriptions = &attributeDesc};

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
        .sType =
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
        .primitiveRestartEnable = VK_FALSE};

    VkPipelineViewportStateCreateInfo viewportState = {
        .sType =
            VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1};

    // Rasterizer (enable wide lines)
    VkPipelineRasterizationStateCreateInfo rasterizer = {
        .sType =
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE};

    VkPipelineMultisampleStateCreateInfo multisampling = {
        .sType =
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};

    VkPipelineColorBlendAttachmentState blendAttachment = {
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD};

    VkPipelineColorBlendStateCreateInfo colorBlending = {
        .sType =
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &blendAttachment};

    VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                      VK_DYNAMIC_STATE_SCISSOR,
                                      VK_DYNAMIC_STATE_LINE_WIDTH};

    VkPipelineDynamicStateCreateInfo dynamicState = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 3,
        .pDynamicStates = dynamicStates};

    VkPushConstantRange pushConstant = {
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(vec3)};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &renderer->descriptorSetLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstant};

    if (vkCreatePipelineLayout(renderer->device, &pipelineLayoutInfo,
                               NULL, &renderer->linePipelineLayout) !=
        VK_SUCCESS)
    {
        printf("Failed to create line pipeline layout\n");
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = stages,
        .pVertexInputState = &vertexInput,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = renderer->linePipelineLayout,
        .renderPass = renderer->renderPass,
        .subpass = 0};

    VkPipelineDepthStencilStateCreateInfo depthStencil = {0};
    depthStencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = (VkStencilOpState) {0};
    depthStencil.back = (VkStencilOpState) {0};

    pipelineInfo.pDepthStencilState = &depthStencil;

    if (vkCreateGraphicsPipelines(
            renderer->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL,
            &renderer->linePipeline) != VK_SUCCESS)
    {
        printf("Failed to create line pipeline\n");
    }

    vkDestroyShaderModule(renderer->device, vertMod, NULL);
    vkDestroyShaderModule(renderer->device, fragMod, NULL);
}

Bool skRenderer_CheckExtensionsSupported(VkPhysicalDevice device)
{
    u32 extensionCount;
    vkEnumerateDeviceExtensionProperties(device, NULL,
                                         &extensionCount, NULL);

    VkExtensionProperties* availableExtensions =
        (VkExtensionProperties*)malloc(extensionCount *
                                       sizeof(VkExtensionProperties));
    vkEnumerateDeviceExtensionProperties(
        device, NULL, &extensionCount, availableExtensions);

    const char* requiredExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    u32 requiredExtensionCount = 1;

    for (u32 i = 0; i < requiredExtensionCount; i++)
    {
        Bool found = false;
        for (u32 j = 0; j < extensionCount; j++)
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

    VkAttachmentDescription depthAttachment = {0};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {0};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkAttachmentDescription attachments[2] = {colorAttachment,
                                              depthAttachment};

    VkRenderPassCreateInfo renderPassInfo = {0};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask =
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(renderer->device, &renderPassInfo, NULL,
                           &renderer->renderPass) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create render pass.\n");
    }
}

void skRenderer_CreateFramebuffers(skRenderer* renderer)
{
    renderer->swapchainFramebuffers =
        skVector_Create(sizeof(VkFramebuffer), 1);

    for (size_t i = 0; i < renderer->swapchainImageViews->size; i++)
    {
        VkFramebuffer framebuf = {0};

        VkImageView attachments[2] = {0};
        attachments[0] = *(VkImageView*)skVector_Get(
            renderer->swapchainImageViews, i);
        attachments[1] = renderer->depthImageView;

        VkFramebufferCreateInfo framebufferInfo = {0};
        framebufferInfo.sType =
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderer->renderPass;
        framebufferInfo.attachmentCount = 2;
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
                                    u32 imageIndex, skEditor* editor)
{
    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
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

    VkClearValue clearColors[2] = {0};

    clearColors[0].color =
        (VkClearColorValue) {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearColors[1].depthStencil =
        (VkClearDepthStencilValue) {1.0f, 0.0f};

    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearColors;

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

    for (size_t i = 0; i < renderer->renderObjects->size; i++)
    {
        skRenderObject* obj =
            (skRenderObject*)skVector_Get(renderer->renderObjects, i);

        if (obj->boneTransforms != NULL)
        {
            for (int i = 0; i < 100; i++)
            {
                mat4* mat =
                    (mat4*)skVector_Get(obj->boneTransforms, i);
                memcpy(
                    (char*)renderer
                            ->boneBuffersMap[renderer->currentFrame] +
                        (i * sizeof(mat4)),
                    mat, sizeof(mat4));
            }
        }

        // Bind vertex and index buffers for this object
        VkBuffer     vertexBuffers[] = {obj->vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers,
                               offsets);
        vkCmdBindIndexBuffer(commandBuffer, obj->indexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);

        VkDescriptorSet sets[] = {
            obj->descriptorSets[renderer->currentFrame],
            renderer->lightDescriptorSets[renderer->currentFrame],
            renderer->uniformDescriptorSets[renderer->currentFrame],
            renderer->boneDescriptorSets[renderer->currentFrame],
        };

        // Bind descriptor set for this object
        vkCmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            renderer->pipelineLayout, 0, 4, sets, 0, NULL);

        // Draw this object
        vkCmdDrawIndexed(commandBuffer, obj->indexCount, 1, 0, 0, 0);
    }

    // Skybox rendering
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      renderer->skyboxPipeline);

    VkBuffer vertexBuffers[] = {renderer->skyboxObject.vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers,
                           offsets);
    vkCmdBindIndexBuffer(commandBuffer,
                         renderer->skyboxObject.indexBuffer, 0,
                         VK_INDEX_TYPE_UINT32);

    VkDescriptorSet sets[] = {
        renderer->skyboxObject.descriptorSets[renderer->currentFrame],
        renderer->lightDescriptorSets[renderer->currentFrame],
        renderer->uniformDescriptorSets[renderer->currentFrame]};

    vkCmdBindDescriptorSets(
        commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        renderer->skyboxPipelineLayout, 0, 3, sets, 0, NULL);

    vkCmdDrawIndexed(commandBuffer, renderer->skyboxObject.indexCount,
                     1, 0, 0, 0);

    // Line rendering
    if (renderer->lineObjects->size > 0)
    {
        vkCmdBindPipeline(commandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          renderer->linePipeline);

        vkCmdSetLineWidth(commandBuffer, 1.0f);

        for (size_t i = 0; i < renderer->lineObjects->size; i++)
        {
            skLineObject* line =
                (skLineObject*)skVector_Get(renderer->lineObjects, i);

            vkCmdSetLineWidth(commandBuffer, line->lineWidth);

            vkCmdBindDescriptorSets(
                commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                renderer->linePipelineLayout, 0, 1,
                &line->descriptorSets[renderer->currentFrame], 0,
                NULL);

            vkCmdPushConstants(commandBuffer,
                               renderer->linePipelineLayout,
                               VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                               sizeof(vec3), &line->color);

            VkBuffer     buffers[] = {line->vertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers,
                                   offsets);
            vkCmdBindIndexBuffer(commandBuffer, line->indexBuffer, 0,
                                 VK_INDEX_TYPE_UINT32);

            vkCmdDrawIndexed(commandBuffer, line->indexCount, 1, 0, 0, 0);
        }
    }

    if (editor != NULL)
    {
        skImGui_NewFrame();

        skEditor_DrawHierarchy(editor);
        skEditor_DrawInspector(editor);
        skEditor_DrawTray(editor);

        skImGui_EndFrame(commandBuffer);
    }

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

void skRenderer_CreateImage(skRenderer* renderer, u32 width,
                            u32 height, VkFormat format,
                            VkImageTiling         tiling,
                            VkImageUsageFlags     usage,
                            VkMemoryPropertyFlags properties,
                            VkImage*              image,
                            VkDeviceMemory*       imageMemory)
{
    VkImageCreateInfo imageInfo = {0};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(renderer->device, &imageInfo, NULL, image) !=
        VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create image.");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(renderer->device, *image,
                                 &memRequirements);

    VkMemoryAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = skRenderer_FindMemoryType(
        renderer, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(renderer->device, &allocInfo, NULL,
                         imageMemory) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to allocate image memory.");
    }

    vkBindImageMemory(renderer->device, *image, *imageMemory, 0);
}

VkImageView skRenderer_CreateImageView(skRenderer* renderer,
                                       VkImage image, VkFormat format,
                                       VkImageAspectFlags flags)
{
    VkImageViewCreateInfo viewInfo = {0};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = flags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(renderer->device, &viewInfo, NULL,
                          &imageView) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create texture image view.");
    }

    return imageView;
}

void skRenderer_CreateDepthResources(skRenderer* renderer)
{
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

    skRenderer_CreateImage(
        renderer, renderer->swapchainExtent.width,
        renderer->swapchainExtent.height, depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &renderer->depthImage,
        &renderer->depthImageMemory);
    renderer->depthImageView = skRenderer_CreateImageView(
        renderer, renderer->depthImage, depthFormat,
        VK_IMAGE_ASPECT_DEPTH_BIT);
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
    skRenderer_CreateDepthResources(renderer);
    skRenderer_CreateFramebuffers(renderer);

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

void skRenderer_UpdateUniformBuffers(skRenderer* renderer)
{
    for (size_t i = 0; i < renderer->renderObjects->size; i++)
    {
        skRenderObject* obj =
            (skRenderObject*)skVector_Get(renderer->renderObjects, i);

        skUniformBufferObject ubo = {0};

        glm_mat4_copy(obj->transform, ubo.model);

        glm_mat4_copy(renderer->viewTransform, ubo.view);

        mat4 proj;
        glm_perspective(glm_rad(80.0f),
                        renderer->swapchainExtent.width /
                            (float)renderer->swapchainExtent.height,
                        0.001f, 1000.0f, proj);
        proj[1][1] *= -1.0f;
        glm_mat4_copy(proj, ubo.proj);

        memcpy(obj->uniformBuffersMap[renderer->currentFrame], &ubo,
               sizeof(ubo));
    }

    char* mapped =
        (char*)renderer->storageBuffersMap[renderer->currentFrame];

    for (size_t i = 0; i < renderer->lights->size; i++)
    {
        skLight* light = (skLight*)skVector_Get(renderer->lights, i);
        char*    dest = mapped + i * 48; // Destination with padding

        // Copy position (12 bytes) + 4-byte padding
        memcpy(dest, &light->position, sizeof(vec3));
        dest += 16; // Advance 16 bytes (position + padding for
                    // Vulkan memory alignment for vec3s)

        // Copy color (12 bytes) + 4-byte padding
        memcpy(dest, &light->color, sizeof(vec3));

        // I really don't know why I have to advance 12 bytes here
        // instead of 16 because if I do, the radius gets copied
        // to the intensity variable in the float
        dest += 12; // Advance 12 bytes

        memcpy(dest, &light->radius, sizeof(float));
        dest += sizeof(float);

        memcpy(dest, &light->intensity, sizeof(float));
    }

    skGlobalUniformBufferObject ubo = {.lightCount =
                                           renderer->lights->size};
    glm_vec3_copy(renderer->viewPos, ubo.viewPos);

    memcpy(renderer->uniformBuffersMap[renderer->currentFrame], &ubo,
           sizeof(skGlobalUniformBufferObject));

    skUniformBufferObject skyboxUbo = {0};
    glm_mat4_copy(renderer->skyboxObject.transform, skyboxUbo.model);

    mat4 viewNoTranslation;
    glm_mat4_copy(renderer->viewTransform, viewNoTranslation);
    viewNoTranslation[3][0] = 0.0f; // Remove translation
    viewNoTranslation[3][1] = 0.0f;
    viewNoTranslation[3][2] = 0.0f;
    glm_mat4_copy(viewNoTranslation, skyboxUbo.view);

    // Projection (same as regular objects)
    mat4 proj;
    glm_perspective(glm_rad(80.0f),
                    renderer->swapchainExtent.width /
                        (float)renderer->swapchainExtent.height,
                    0.001f, 1000.0f, proj);
    proj[1][1] *= -1.0f;
    glm_mat4_copy(proj, skyboxUbo.proj);

    memcpy(renderer->skyboxObject
               .uniformBuffersMap[renderer->currentFrame],
           &skyboxUbo, sizeof(skyboxUbo));

    for (size_t i = 0; i < renderer->lineObjects->size; i++)
    {
        skLineObject* line =
            (skLineObject*)skVector_Get(renderer->lineObjects, i);

        skUniformBufferObject ubo = {0};

        glm_mat4_copy(line->transform, ubo.model);
        glm_mat4_print(line->transform, stdout);
        
        glm_mat4_copy(renderer->viewTransform, ubo.view);

        mat4 proj;
        glm_perspective(glm_rad(80.0f),
                        renderer->swapchainExtent.width /
                            (float)renderer->swapchainExtent.height,
                        0.001f, 1000.0f, proj);
        proj[1][1] *= -1.0f;
        glm_mat4_copy(proj, ubo.proj);

        memcpy(line->uniformBuffersMap[renderer->currentFrame], &ubo,
               sizeof(ubo));
    }
}

void skRenderer_DrawFrame(skRenderer* renderer, skEditor* editor)
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

    skRenderer_UpdateUniformBuffers(renderer);

    skRenderer_RecordCommandBuffer(renderer, cmdBuffer, imageIndex,
                                   editor);

    vkResetFences(renderer->device, 1, inFlightFence);

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

    u32          glfwExtensionCount = 0;
    const char** glfwExtensions;

    // Get required vulkan extensions from GLFW
    glfwExtensions =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // Create extensions array with space for other extensions
    u32          extensionCount = glfwExtensionCount;
    const char** extensions = (const char**)malloc(
        (glfwExtensionCount + 1) * sizeof(const char*));

    for (u32 i = 0; i < glfwExtensionCount; i++)
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

    u32 uniqueQueueFamilies[2];
    int uniqueCount = 0;

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
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
        VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME,
        VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME};

    deviceFeatures.wideLines = VK_TRUE;

    VkDeviceCreateInfo deviceCreateInfo = {0};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos =
        (VkDeviceQueueCreateInfo*)queueCreateInfos->data;
    deviceCreateInfo.queueCreateInfoCount =
        (u32)queueCreateInfos->size;
    deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

    // Enable swapchain extension
    deviceCreateInfo.enabledExtensionCount = 4;
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

VkCommandBuffer
skRenderer_BeginSingleTimeCommands(skRenderer* renderer)
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

    return commandBuffer;
}

void skRenderer_EndSingleTimeCommands(skRenderer*     renderer,
                                      VkCommandBuffer commandBuffer)
{
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

void skRenderer_CopyBuffer(skRenderer* renderer, VkBuffer srcBuffer,
                           VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer =
        skRenderer_BeginSingleTimeCommands(renderer);

    VkBufferCopy copyRegion = {0};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1,
                    &copyRegion);

    skRenderer_EndSingleTimeCommands(renderer, commandBuffer);
}

void skRenderer_CreateDescriptorSetLayout(skRenderer* renderer)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding = {0};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType =
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutBinding samplerLayoutBinding = {0};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = NULL;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding normalLayoutBinding = {0};
    normalLayoutBinding.binding = 2;
    normalLayoutBinding.descriptorCount = 1;
    normalLayoutBinding.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalLayoutBinding.pImmutableSamplers = NULL;
    normalLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding roughnessLayoutBinding = {0};
    roughnessLayoutBinding.binding = 3;
    roughnessLayoutBinding.descriptorCount = 1;
    roughnessLayoutBinding.descriptorType =
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    roughnessLayoutBinding.pImmutableSamplers = NULL;
    roughnessLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[] = {
        uboLayoutBinding, samplerLayoutBinding, normalLayoutBinding,
        roughnessLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo = {0};
    layoutInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 4;
    layoutInfo.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(
            renderer->device, &layoutInfo, NULL,
            &renderer->descriptorSetLayout) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create descriptor set layout.\n");
    }

    VkDescriptorSetLayoutBinding lightBufferBinding = {0};
    lightBufferBinding.binding = 0;
    lightBufferBinding.descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    lightBufferBinding.descriptorCount = 1;
    lightBufferBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    lightBufferBinding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo layoutInfo2 = {0};
    layoutInfo2.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo2.bindingCount = 1;
    layoutInfo2.pBindings = &lightBufferBinding;

    if (vkCreateDescriptorSetLayout(
            renderer->device, &layoutInfo2, NULL,
            &renderer->lightDescriptorSetLayout) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create descriptor set layout for "
               "lights.\n");
    }

    VkDescriptorSetLayoutBinding uniformBinding = {0};
    uniformBinding.binding = 0;
    uniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uniformBinding.descriptorCount = 1;
    uniformBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    uniformBinding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo layoutInfo3 = {0};
    layoutInfo3.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo3.bindingCount = 1;
    layoutInfo3.pBindings = &uniformBinding;

    if (vkCreateDescriptorSetLayout(
            renderer->device, &layoutInfo3, NULL,
            &renderer->uniformDescriptorSetLayout) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create descriptor set layout for "
               "global uniforms.\n");
    }

    VkDescriptorSetLayoutBinding bonesBufferBinding = {0};
    bonesBufferBinding.binding = 0;
    bonesBufferBinding.descriptorType =
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bonesBufferBinding.descriptorCount = 1;
    bonesBufferBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    bonesBufferBinding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo layoutInfo4 = {0};
    layoutInfo4.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo4.bindingCount = 1;
    layoutInfo4.pBindings = &bonesBufferBinding;

    if (vkCreateDescriptorSetLayout(
            renderer->device, &layoutInfo4, NULL,
            &renderer->bonesDescriptorSetLayout) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create descriptor set layout for "
               "bones.\n");
    }
}

void skRenderer_CreateDescriptorSets(skRenderer* renderer)
{
    VkDeviceSize bufferSize = 48 * SK_MAX_LIGHTS;
    VkDeviceSize boneSize = sizeof(mat4) * SK_MAX_BONES;
    VkDeviceSize uniformBufferSize =
        sizeof(skGlobalUniformBufferObject);

    for (int frame = 0; frame < SK_FRAMES_IN_FLIGHT; frame++)
    {
        skRenderer_CreateBuffer(
            renderer, boneSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &renderer->boneBuffers[frame],
            &renderer->boneBuffersMemory[frame]);

        vkMapMemory(renderer->device,
                    renderer->boneBuffersMemory[frame], 0, boneSize,
                    0, &renderer->boneBuffersMap[frame]);

        skRenderer_CreateBuffer(
            renderer, bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &renderer->storageBuffers[frame],
            &renderer->storageBuffersMemory[frame]);

        vkMapMemory(
            renderer->device, renderer->storageBuffersMemory[frame],
            0, bufferSize, 0, &renderer->storageBuffersMap[frame]);

        skRenderer_CreateBuffer(
            renderer, uniformBufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &renderer->uniformBuffers[frame],
            &renderer->uniformBuffersMemory[frame]);

        vkMapMemory(renderer->device,
                    renderer->uniformBuffersMemory[frame], 0,
                    uniformBufferSize, 0,
                    &renderer->uniformBuffersMap[frame]);
    }

    VkDescriptorSetLayout layouts[SK_FRAMES_IN_FLIGHT];
    for (int i = 0; i < SK_FRAMES_IN_FLIGHT; i++)
    {
        layouts[i] = renderer->lightDescriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = renderer->descriptorPool;
    allocInfo.descriptorSetCount = SK_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts;

    if (vkAllocateDescriptorSets(renderer->device, &allocInfo,
                                 renderer->lightDescriptorSets) !=
        VK_SUCCESS)
    {
        printf("SK ERROR: Failed to allocate descriptor sets for "
               "light.");
    }

    // Update descriptor sets
    for (int frame = 0; frame < SK_FRAMES_IN_FLIGHT; frame++)
    {
        VkDescriptorBufferInfo bufferInfo = {0};
        bufferInfo.buffer = renderer->storageBuffers[frame];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(skLight) * SK_MAX_LIGHTS;

        VkWriteDescriptorSet descriptorWrites[] = {{0}};
        descriptorWrites[0].sType =
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet =
            renderer->lightDescriptorSets[frame];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(renderer->device, 1, descriptorWrites,
                               0, NULL);
    }

    VkDescriptorSetLayout boneLayouts[SK_FRAMES_IN_FLIGHT];
    for (int i = 0; i < SK_FRAMES_IN_FLIGHT; i++)
    {
        boneLayouts[i] = renderer->bonesDescriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo boneAllocInfo = {0};
    boneAllocInfo.sType =
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    boneAllocInfo.descriptorPool = renderer->descriptorPool;
    boneAllocInfo.descriptorSetCount = SK_FRAMES_IN_FLIGHT;
    boneAllocInfo.pSetLayouts = boneLayouts;

    if (vkAllocateDescriptorSets(renderer->device, &boneAllocInfo,
                                 renderer->boneDescriptorSets) !=
        VK_SUCCESS)
    {
        printf("SK ERROR: Failed to allocate descriptor sets for "
               "bones.");
    }

    // Update descriptor sets
    for (int frame = 0; frame < SK_FRAMES_IN_FLIGHT; frame++)
    {
        VkDescriptorBufferInfo bufferInfo = {0};
        bufferInfo.buffer = renderer->boneBuffers[frame];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(mat4) * SK_MAX_BONES;

        VkWriteDescriptorSet descriptorWrites[] = {{0}};
        descriptorWrites[0].sType =
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet =
            renderer->boneDescriptorSets[frame];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType =
            VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(renderer->device, 1, descriptorWrites,
                               0, NULL);
    }

    VkDescriptorSetLayout uniformLayouts[SK_FRAMES_IN_FLIGHT];
    for (int i = 0; i < SK_FRAMES_IN_FLIGHT; i++)
    {
        uniformLayouts[i] = renderer->uniformDescriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo2 = {0};
    allocInfo2.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo2.descriptorPool = renderer->descriptorPool;
    allocInfo2.descriptorSetCount = SK_FRAMES_IN_FLIGHT;
    allocInfo2.pSetLayouts = uniformLayouts;

    if (vkAllocateDescriptorSets(renderer->device, &allocInfo2,
                                 renderer->uniformDescriptorSets) !=
        VK_SUCCESS)
    {
        printf("SK ERROR: Failed to allocate descriptor sets for "
               "light.");
    }

    // Update descriptor sets
    for (int frame = 0; frame < SK_FRAMES_IN_FLIGHT; frame++)
    {
        VkDescriptorBufferInfo bufferInfo = {0};
        bufferInfo.buffer = renderer->uniformBuffers[frame];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(skGlobalUniformBufferObject);

        VkWriteDescriptorSet descriptorWrites[] = {{0}};
        descriptorWrites[0].sType =
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet =
            renderer->uniformDescriptorSets[frame];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType =
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(renderer->device, 1, descriptorWrites,
                               0, NULL);
    }
}

void skRenderer_AddRenderObject(skRenderer*     renderer,
                                skRenderObject* object)
{
    VkDeviceSize bufferSize = sizeof(skUniformBufferObject);

    for (int frame = 0; frame < SK_FRAMES_IN_FLIGHT; frame++)
    {
        skRenderer_CreateBuffer(
            renderer, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &object->uniformBuffers[frame],
            &object->uniformBuffersMemory[frame]);

        vkMapMemory(renderer->device,
                    object->uniformBuffersMemory[frame], 0,
                    bufferSize, 0, &object->uniformBuffersMap[frame]);
    }

    skRenderer_CreateDescriptorSetsForObject(renderer, object);

    skVector_PushBack(renderer->renderObjects, object);
}

void skRenderer_AddLight(skRenderer* renderer, skLight* light)
{
    skVector_PushBack(renderer->lights, light);
}

void skRenderer_CreateDescriptorPool(skRenderer* renderer)
{
    VkDescriptorPoolSize poolSizes[] = {{0}, {0}, {0}, {0}};

    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount =
        (SK_MAX_RENDER_OBJECTS * SK_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount =
        (SK_MAX_RENDER_OBJECTS * SK_FRAMES_IN_FLIGHT * 2) +
        SK_FRAMES_IN_FLIGHT;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount =
        SK_MAX_LIGHTS * SK_FRAMES_IN_FLIGHT;
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[3].descriptorCount = SK_MAX_BONES * SK_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo poolInfo = {0};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags =
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.poolSizeCount = 4;
    poolInfo.pPoolSizes = poolSizes;
    poolInfo.maxSets = (SK_FRAMES_IN_FLIGHT * SK_MAX_RENDER_OBJECTS) +
                       SK_FRAMES_IN_FLIGHT;

    if (vkCreateDescriptorPool(renderer->device, &poolInfo, NULL,
                               &renderer->descriptorPool) !=
        VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create descriptor pool.");
    }
}

void skRenderer_CreateDescriptorSetsForObject(skRenderer* renderer,
                                              skRenderObject* obj)
{
    VkDescriptorSetLayout layouts[SK_FRAMES_IN_FLIGHT];
    for (int i = 0; i < SK_FRAMES_IN_FLIGHT; i++)
    {
        layouts[i] = renderer->descriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = renderer->descriptorPool;
    allocInfo.descriptorSetCount = SK_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts;

    if (vkAllocateDescriptorSets(renderer->device, &allocInfo,
                                 obj->descriptorSets) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to allocate descriptor sets for "
               "object.");
    }

    // Update descriptor sets
    for (int frame = 0; frame < SK_FRAMES_IN_FLIGHT; frame++)
    {
        VkDescriptorBufferInfo bufferInfo = {0};
        bufferInfo.buffer = obj->uniformBuffers[frame];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(skUniformBufferObject);

        VkDescriptorImageInfo imageInfo = {0};
        imageInfo.imageLayout =
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = obj->textureImageView;
        imageInfo.sampler = obj->textureSampler;

        VkDescriptorImageInfo normalImageInfo = {0};
        normalImageInfo.imageLayout =
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        normalImageInfo.imageView = obj->normalImageView;
        normalImageInfo.sampler = obj->normalSampler;

        VkDescriptorImageInfo roughnessImageInfo = {0};
        roughnessImageInfo.imageLayout =
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        roughnessImageInfo.imageView = obj->roughnessImageView;
        roughnessImageInfo.sampler = obj->roughnessSampler;

        VkWriteDescriptorSet descriptorWrites[] = {{0},
                                                   {0},
                                                   {0},
                                                   {0}};
        descriptorWrites[0].sType =
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = obj->descriptorSets[frame];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType =
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType =
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = obj->descriptorSets[frame];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        descriptorWrites[2].sType =
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[2].dstSet = obj->descriptorSets[frame];
        descriptorWrites[2].dstBinding = 2;
        descriptorWrites[2].dstArrayElement = 0;
        descriptorWrites[2].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[2].descriptorCount = 1;
        descriptorWrites[2].pImageInfo = &normalImageInfo;

        descriptorWrites[3].sType =
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[3].dstSet = obj->descriptorSets[frame];
        descriptorWrites[3].dstBinding = 3;
        descriptorWrites[3].dstArrayElement = 0;
        descriptorWrites[3].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[3].descriptorCount = 1;
        descriptorWrites[3].pImageInfo = &roughnessImageInfo;

        vkUpdateDescriptorSets(renderer->device, 4, descriptorWrites,
                               0, NULL);
    }
}

void skRenderer_CopyBufferToImage(skRenderer* renderer,
                                  VkBuffer buffer, VkImage image,
                                  u32 width, u32 height)
{
    VkCommandBuffer commandBuffer =
        skRenderer_BeginSingleTimeCommands(renderer);

    VkBufferImageCopy region = {0};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = (VkOffset3D) {0, 0, 0};
    region.imageExtent = (VkExtent3D) {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &region);

    skRenderer_EndSingleTimeCommands(renderer, commandBuffer);
}

void skRenderer_TransitionImageLayout(skRenderer* renderer,
                                      VkImage image, VkFormat format,
                                      VkImageLayout oldLayout,
                                      VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer =
        skRenderer_BeginSingleTimeCommands(renderer);

    VkImageMemoryBarrier barrier = {0};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        printf("SK ERROR: Unsupported layout transition.\n");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage,
                         0, 0, NULL, 0, NULL, 1, &barrier);

    skRenderer_EndSingleTimeCommands(renderer, commandBuffer);
}

Bool skHasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           format == VK_FORMAT_D24_UNORM_S8_UINT;
}

skRenderer skRenderer_Create(skWindow* window)
{
    skRenderer  renderer = {0};
    skRenderer* rendererPtr = &renderer;

    renderer.currentFrame = 0;
    renderer.window = window;

    renderer.renderObjects =
        skVector_Create(sizeof(skRenderObject), 10);
    renderer.lineObjects = skVector_Create(sizeof(skLineObject), 1);
    renderer.lights = skVector_Create(sizeof(skLight), 10);

    skRenderer_CreateInstance(rendererPtr);
    skRenderer_CreateSurface(rendererPtr, window);
    skRenderer_CreateDebugMessenger(rendererPtr);
    skRenderer_CreatePhysicalDevice(rendererPtr);
    skRenderer_CreateLogicalDevice(rendererPtr);
    skRenderer_CreateSwapchain(&renderer, window);
    skRenderer_CreateImageViews(&renderer);
    skRenderer_CreateRenderPass(&renderer);
    skRenderer_CreateDescriptorSetLayout(&renderer);
    skRenderer_CreateGraphicsPipeline(&renderer);
    skRenderer_CreateSkyboxGraphicsPipeline(&renderer);
    skRenderer_CreateLineGraphicsPipeline(&renderer);
    skRenderer_CreateCommandPool(&renderer);
    skRenderer_CreateDepthResources(&renderer);
    skRenderer_CreateFramebuffers(&renderer);
    skRenderer_CreateDescriptorPool(&renderer);
    skRenderer_CreateDescriptorSets(&renderer);
    skRenderer_CreateCommandBuffers(&renderer);
    skRenderer_CreateSyncObjects(&renderer);

    skModel model = skModel_Create();
    skModel_Load(&model, "res/models/sphere.fbx");

    renderer.skyboxObject = skRenderObject_CreateFromModel(
        &renderer, &model, 0, "res/textures/skybox.bmp",
        "res/textures/defualt_normal.bmp",
        "res/textures/default_roughness.bmp");

    for (int frame = 0; frame < SK_FRAMES_IN_FLIGHT; frame++)
    {
        skRenderer_CreateBuffer(
            &renderer, sizeof(skUniformBufferObject),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &renderer.skyboxObject.uniformBuffers[frame],
            &renderer.skyboxObject.uniformBuffersMemory[frame]);

        vkMapMemory(renderer.device,
                    renderer.skyboxObject.uniformBuffersMemory[frame],
                    0, sizeof(skUniformBufferObject), 0,
                    &renderer.skyboxObject.uniformBuffersMap[frame]);
    }

    skRenderer_CreateDescriptorSetsForObject(&renderer,
                                             &renderer.skyboxObject);

    mat4 trans = GLM_MAT4_IDENTITY_INIT;
    glm_translate(trans, (vec3) {0.0f, 0.0f, 0.0f});
    glm_quat_rotate(trans, (vec3) {0.0f, 0.0f, 0.0f}, trans);
    glm_scale(trans, (vec3) {100.0f, 100.0f, 100.0f});

    glm_mat4_copy(trans, renderer.skyboxObject.transform);

    return renderer;
}

void skRenderer_InitImGui(skRenderer* renderer)
{
    skImGui_Init(renderer->window->window, renderer->instance,
                 renderer->descriptorPool, renderer->renderPass,
                 renderer->physicalDevice, renderer->device,
                 renderer->commandPool, renderer->graphicsQueue);
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

void skRenderer_CreateTexture(skRenderer*    renderer,
                              VkImage        textureImage,
                              VkDeviceMemory textureImageMemory,
                              VkImageView    textureImageView,
                              VkSampler      textureSampler,
                              stbi_uc* pixels, VkDeviceSize imageSize,
                              int texWidth, int texHeight)
{
    VkBuffer       imageStagingBuffer;
    VkDeviceMemory imageStagingBufferMemory;

    skRenderer_CreateBuffer(
        renderer, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &imageStagingBuffer, &imageStagingBufferMemory);

    void* imageData;
    vkMapMemory(renderer->device, imageStagingBufferMemory, 0,
                imageSize, 0, &imageData);
    memcpy(imageData, pixels, imageSize);
    vkUnmapMemory(renderer->device, imageStagingBufferMemory);

    stbi_image_free(pixels);

    skRenderer_CreateImage(
        renderer, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &textureImage,
        &textureImageMemory);

    skRenderer_TransitionImageLayout(
        renderer, textureImage, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    skRenderer_CopyBufferToImage(renderer, imageStagingBuffer,
                                 textureImage, (u32)(texWidth),
                                 (u32)(texHeight));

    skRenderer_TransitionImageLayout(
        renderer, textureImage, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    textureImageView = skRenderer_CreateImageView(
        renderer, textureImage, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_ASPECT_COLOR_BIT);

    VkSamplerCreateInfo samplerInfo = {0};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;

    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 0;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(renderer->device, &samplerInfo, NULL,
                        &textureSampler) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to create texture sampler.");
    }
}

skRenderObject
skRenderObject_CreateFromModel(skRenderer* renderer, skModel* model,
                               int meshIndex, const char* texturePath,
                               const char* normalTexturePath,
                               const char* roughnessTexturePath)
{
    skRenderObject obj = {0};

    // Create vertex buffer

    skMesh* mesh = skVector_Get(model->meshes, 0);
    u16     numVertices = mesh->vertices->size;

    obj.indexCount = mesh->indices->size;

    size_t bufferSize = sizeof(skVertex) * numVertices;

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

    memcpy(data, mesh->vertices->data,
           sizeof(skVertex) * numVertices);

    vkUnmapMemory(renderer->device, stagingMemory);

    skRenderer_CreateBuffer(renderer, bufferSize,
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            &obj.vertexBuffer,
                            &obj.vertexBufferMemory);

    skRenderer_CopyBuffer(renderer, stagingBuffer, obj.vertexBuffer,
                          bufferSize);

    vkDestroyBuffer(renderer->device, stagingBuffer, NULL);
    vkFreeMemory(renderer->device, stagingMemory, NULL);

    // Create index buffer

    size_t indexBufferSize =
        sizeof(mesh->indices[0]) * obj.indexCount;

    VkBuffer       indexStagingBuffer;
    VkDeviceMemory indexStagingMemory;
    skRenderer_CreateBuffer(renderer, indexBufferSize,
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            &indexStagingBuffer, &indexStagingMemory);

    void* indexData;
    vkMapMemory(renderer->device, indexStagingMemory, 0,
                indexBufferSize, 0, &indexData);

    memcpy(indexData, mesh->indices->data,
           sizeof(u32) * obj.indexCount);

    vkUnmapMemory(renderer->device, indexStagingMemory);

    skRenderer_CreateBuffer(renderer, indexBufferSize,
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            &obj.indexBuffer, &obj.indexBufferMemory);

    skRenderer_CopyBuffer(renderer, indexStagingBuffer,
                          obj.indexBuffer, indexBufferSize);

    vkDestroyBuffer(renderer->device, indexStagingBuffer, NULL);
    vkFreeMemory(renderer->device, indexStagingMemory, NULL);

    // Create texture image

    {

        int      texWidth, texHeight, texChannels;
        stbi_uc* pixels =
            stbi_load(texturePath, &texWidth, &texHeight,
                      &texChannels, STBI_rgb_alpha);

        if (!pixels)
        {
            printf("SK ERROR: Failed to load texture image.");
            pixels =
                stbi_load("res/textures/image.bmp", &texWidth,
                          &texHeight, &texChannels, STBI_rgb_alpha);
        }
        
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        VkBuffer       imageStagingBuffer;
        VkDeviceMemory imageStagingBufferMemory;

        skRenderer_CreateBuffer(
            renderer, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &imageStagingBuffer, &imageStagingBufferMemory);

        void* imageData;
        vkMapMemory(renderer->device, imageStagingBufferMemory, 0,
                    imageSize, 0, &imageData);
        memcpy(imageData, pixels, imageSize);
        vkUnmapMemory(renderer->device, imageStagingBufferMemory);

        stbi_image_free(pixels);

        skRenderer_CreateImage(
            renderer, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &obj.textureImage,
            &obj.textureImageMemory);

        skRenderer_TransitionImageLayout(
            renderer, obj.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        skRenderer_CopyBufferToImage(
            renderer, imageStagingBuffer, obj.textureImage,
            (u32)(texWidth), (u32)(texHeight));

        skRenderer_TransitionImageLayout(
            renderer, obj.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        obj.textureImageView = skRenderer_CreateImageView(
            renderer, obj.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_ASPECT_COLOR_BIT);

        VkSamplerCreateInfo samplerInfo = {0};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 0;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(renderer->device, &samplerInfo, NULL,
                            &obj.textureSampler) != VK_SUCCESS)
        {
            printf("SK ERROR: Failed to create texture sampler.");
        }
    }

    glm_mat4_identity(obj.transform);

    {

        int      texWidth, texHeight, texChannels;
        stbi_uc* pixels =
            stbi_load(normalTexturePath, &texWidth, &texHeight,
                      &texChannels, STBI_rgb_alpha);

        if (!pixels)
        {
            printf("SK ERROR: Failed to load texture image.");
            pixels =
                stbi_load("res/textures/normal.bmp", &texWidth,
                          &texHeight, &texChannels, STBI_rgb_alpha);
        }

        VkDeviceSize imageSize = texWidth * texHeight * 4;

        VkBuffer       imageStagingBuffer;
        VkDeviceMemory imageStagingBufferMemory;

        skRenderer_CreateBuffer(
            renderer, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &imageStagingBuffer, &imageStagingBufferMemory);

        void* imageData;
        vkMapMemory(renderer->device, imageStagingBufferMemory, 0,
                    imageSize, 0, &imageData);
        memcpy(imageData, pixels, imageSize);
        vkUnmapMemory(renderer->device, imageStagingBufferMemory);

        stbi_image_free(pixels);

        skRenderer_CreateImage(
            renderer, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &obj.normalImage,
            &obj.normalImageMemory);

        skRenderer_TransitionImageLayout(
            renderer, obj.normalImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        skRenderer_CopyBufferToImage(renderer, imageStagingBuffer,
                                     obj.normalImage, (u32)(texWidth),
                                     (u32)(texHeight));

        skRenderer_TransitionImageLayout(
            renderer, obj.normalImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        obj.normalImageView = skRenderer_CreateImageView(
            renderer, obj.normalImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_ASPECT_COLOR_BIT);

        VkSamplerCreateInfo samplerInfo = {0};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 0;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(renderer->device, &samplerInfo, NULL,
                            &obj.normalSampler) != VK_SUCCESS)
        {
            printf("SK ERROR: Failed to create normal sampler.");
        }
    }

    {

        int      texWidth, texHeight, texChannels;
        stbi_uc* pixels =
            stbi_load(roughnessTexturePath, &texWidth, &texHeight,
                      &texChannels, STBI_rgb_alpha);

        if (!pixels)
        {
            printf(
                "SK ERROR: Failed to load roughness texture image.");
            pixels = stbi_load("res/textures/default_roughness.bmp",
                               &texWidth, &texHeight, &texChannels,
                               STBI_rgb_alpha);
        }

        VkDeviceSize imageSize = texWidth * texHeight * 4;

        VkBuffer       imageStagingBuffer;
        VkDeviceMemory imageStagingBufferMemory;

        skRenderer_CreateBuffer(
            renderer, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &imageStagingBuffer, &imageStagingBufferMemory);

        void* imageData;
        vkMapMemory(renderer->device, imageStagingBufferMemory, 0,
                    imageSize, 0, &imageData);
        memcpy(imageData, pixels, imageSize);
        vkUnmapMemory(renderer->device, imageStagingBufferMemory);

        stbi_image_free(pixels);

        skRenderer_CreateImage(
            renderer, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &obj.roughnessImage,
            &obj.roughnessImageMemory);

        skRenderer_TransitionImageLayout(
            renderer, obj.roughnessImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        skRenderer_CopyBufferToImage(
            renderer, imageStagingBuffer, obj.roughnessImage,
            (u32)(texWidth), (u32)(texHeight));

        skRenderer_TransitionImageLayout(
            renderer, obj.roughnessImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        obj.roughnessImageView = skRenderer_CreateImageView(
            renderer, obj.roughnessImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_ASPECT_COLOR_BIT);

        VkSamplerCreateInfo samplerInfo = {0};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 0;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(renderer->device, &samplerInfo, NULL,
                            &obj.roughnessSampler) != VK_SUCCESS)
        {
            printf("SK ERROR: Failed to create normal sampler.");
        }
    }

    return obj;
}

skRenderObject skRenderObject_CreateFromSprite(
    skRenderer* renderer, const char* texturePath,
    const char* normalTexturePath, const char* roughnessTexturePath)
{
    skRenderObject obj = {0};

    u16 numVertices = 4;

    const skVertex vertices[] = {
        {{-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{-0.5f, 0.0f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    };

    size_t vertexBufferSize = sizeof(skVertex) * numVertices;

    VkBuffer       vertexStagingBuffer;
    VkDeviceMemory vertexStagingMemory;
    skRenderer_CreateBuffer(
        renderer, vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &vertexStagingBuffer, &vertexStagingMemory);

    void* vertexData;
    vkMapMemory(renderer->device, vertexStagingMemory, 0,
                vertexBufferSize, 0, &vertexData);

    memcpy(vertexData, vertices, vertexBufferSize);

    vkUnmapMemory(renderer->device, vertexStagingMemory);

    skRenderer_CreateBuffer(renderer, vertexBufferSize,
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            &obj.vertexBuffer,
                            &obj.vertexBufferMemory);

    skRenderer_CopyBuffer(renderer, vertexStagingBuffer,
                          obj.vertexBuffer, vertexBufferSize);

    vkDestroyBuffer(renderer->device, vertexStagingBuffer, NULL);
    vkFreeMemory(renderer->device, vertexStagingMemory, NULL);

    // Create index buffer

    const u32 indices[] = {0, 2, 1, 2, 0, 3};

    u32 numIndices = 6;

    obj.indexCount = 6;

    size_t indexBufferSize = sizeof(indices[0]) * numIndices;

    VkBuffer       indexStagingBuffer;
    VkDeviceMemory indexStagingMemory;
    skRenderer_CreateBuffer(renderer, indexBufferSize,
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            &indexStagingBuffer, &indexStagingMemory);

    void* indexData;
    vkMapMemory(renderer->device, indexStagingMemory, 0,
                indexBufferSize, 0, &indexData);

    memcpy(indexData, indices, sizeof(indices));

    vkUnmapMemory(renderer->device, indexStagingMemory);

    skRenderer_CreateBuffer(renderer, indexBufferSize,
                            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                                VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            &obj.indexBuffer, &obj.indexBufferMemory);

    skRenderer_CopyBuffer(renderer, indexStagingBuffer,
                          obj.indexBuffer, indexBufferSize);

    vkDestroyBuffer(renderer->device, indexStagingBuffer, NULL);
    vkFreeMemory(renderer->device, indexStagingMemory, NULL);

    // Create texture image

    {

        int      texWidth, texHeight, texChannels;
        stbi_uc* pixels =
            stbi_load(texturePath, &texWidth, &texHeight,
                      &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels)
        {
            printf("SK ERROR: Failed to load texture image.");
        }

        VkBuffer       imageStagingBuffer;
        VkDeviceMemory imageStagingBufferMemory;

        skRenderer_CreateBuffer(
            renderer, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &imageStagingBuffer, &imageStagingBufferMemory);

        void* imageData;
        vkMapMemory(renderer->device, imageStagingBufferMemory, 0,
                    imageSize, 0, &imageData);
        memcpy(imageData, pixels, imageSize);
        vkUnmapMemory(renderer->device, imageStagingBufferMemory);

        stbi_image_free(pixels);

        skRenderer_CreateImage(
            renderer, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &obj.textureImage,
            &obj.textureImageMemory);

        skRenderer_TransitionImageLayout(
            renderer, obj.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        skRenderer_CopyBufferToImage(
            renderer, imageStagingBuffer, obj.textureImage,
            (u32)(texWidth), (u32)(texHeight));

        skRenderer_TransitionImageLayout(
            renderer, obj.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        obj.textureImageView = skRenderer_CreateImageView(
            renderer, obj.textureImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_ASPECT_COLOR_BIT);

        VkSamplerCreateInfo samplerInfo = {0};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 0;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(renderer->device, &samplerInfo, NULL,
                            &obj.textureSampler) != VK_SUCCESS)
        {
            printf("SK ERROR: Failed to create texture sampler.");
        }

        glm_mat4_identity(obj.transform);
    }

    {

        int      texWidth, texHeight, texChannels;
        stbi_uc* pixels =
            stbi_load(normalTexturePath, &texWidth, &texHeight,
                      &texChannels, STBI_rgb_alpha);

        if (!pixels)
        {
            printf("SK ERROR: Failed to load texture image.");
            pixels =
                stbi_load("res/textures/normal.bmp", &texWidth,
                          &texHeight, &texChannels, STBI_rgb_alpha);
        }

        VkDeviceSize imageSize = texWidth * texHeight * 4;

        VkBuffer       imageStagingBuffer;
        VkDeviceMemory imageStagingBufferMemory;

        skRenderer_CreateBuffer(
            renderer, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &imageStagingBuffer, &imageStagingBufferMemory);

        void* imageData;
        vkMapMemory(renderer->device, imageStagingBufferMemory, 0,
                    imageSize, 0, &imageData);
        memcpy(imageData, pixels, imageSize);
        vkUnmapMemory(renderer->device, imageStagingBufferMemory);

        stbi_image_free(pixels);

        skRenderer_CreateImage(
            renderer, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &obj.normalImage,
            &obj.normalImageMemory);

        skRenderer_TransitionImageLayout(
            renderer, obj.normalImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        skRenderer_CopyBufferToImage(renderer, imageStagingBuffer,
                                     obj.normalImage, (u32)(texWidth),
                                     (u32)(texHeight));

        skRenderer_TransitionImageLayout(
            renderer, obj.normalImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        obj.normalImageView = skRenderer_CreateImageView(
            renderer, obj.normalImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_ASPECT_COLOR_BIT);

        VkSamplerCreateInfo samplerInfo = {0};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 0;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(renderer->device, &samplerInfo, NULL,
                            &obj.normalSampler) != VK_SUCCESS)
        {
            printf("SK ERROR: Failed to create normal sampler.");
        }
    }

    {

        int      texWidth, texHeight, texChannels;
        stbi_uc* pixels =
            stbi_load(roughnessTexturePath, &texWidth, &texHeight,
                      &texChannels, STBI_rgb_alpha);

        if (!pixels)
        {
            printf(
                "SK ERROR: Failed to load roughness texture image.");
            pixels = stbi_load("res/textures/default_roughness.bmp",
                               &texWidth, &texHeight, &texChannels,
                               STBI_rgb_alpha);
        }

        VkDeviceSize imageSize = texWidth * texHeight * 4;

        VkBuffer       imageStagingBuffer;
        VkDeviceMemory imageStagingBufferMemory;

        skRenderer_CreateBuffer(
            renderer, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &imageStagingBuffer, &imageStagingBufferMemory);

        void* imageData;
        vkMapMemory(renderer->device, imageStagingBufferMemory, 0,
                    imageSize, 0, &imageData);
        memcpy(imageData, pixels, imageSize);
        vkUnmapMemory(renderer->device, imageStagingBufferMemory);

        stbi_image_free(pixels);

        skRenderer_CreateImage(
            renderer, texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &obj.roughnessImage,
            &obj.roughnessImageMemory);

        skRenderer_TransitionImageLayout(
            renderer, obj.roughnessImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_UNDEFINED,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        skRenderer_CopyBufferToImage(
            renderer, imageStagingBuffer, obj.roughnessImage,
            (u32)(texWidth), (u32)(texHeight));

        skRenderer_TransitionImageLayout(
            renderer, obj.roughnessImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        obj.roughnessImageView = skRenderer_CreateImageView(
            renderer, obj.roughnessImage, VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_ASPECT_COLOR_BIT);

        VkSamplerCreateInfo samplerInfo = {0};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 0;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(renderer->device, &samplerInfo, NULL,
                            &obj.roughnessSampler) != VK_SUCCESS)
        {
            printf("SK ERROR: Failed to create normal sampler.");
        }
    }

    return obj;
}

skLineObject skLineObject_Create(skRenderer* renderer, vec3* points,
                                 u32* indices, u32 pointCount,
                                 u32 indexCount, vec3 color,
                                 float width)
{
    skLineObject line = {0};
    line.vertexCount = pointCount;
    line.indexCount = indexCount;
    line.lineWidth = width;
    glm_vec3_copy(color, line.color);

    size_t bufferSize = sizeof(vec3) * pointCount;

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
    memcpy(data, points, bufferSize);
    vkUnmapMemory(renderer->device, stagingMemory);

    skRenderer_CreateBuffer(renderer, bufferSize,
                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            &line.vertexBuffer,
                            &line.vertexBufferMemory);

    skRenderer_CopyBuffer(renderer, stagingBuffer, line.vertexBuffer,
                          bufferSize);

    vkDestroyBuffer(renderer->device, stagingBuffer, NULL);
    vkFreeMemory(renderer->device, stagingMemory, NULL);

    size_t         indexBufferSize = sizeof(u32) * indexCount;
    VkBuffer       indexStagingBuffer;
    VkDeviceMemory indexStagingMemory;
    skRenderer_CreateBuffer(renderer, indexBufferSize,
                            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                            &indexStagingBuffer, &indexStagingMemory);

    void* indexData;
    vkMapMemory(renderer->device, indexStagingMemory, 0,
                indexBufferSize, 0, &indexData);
    memcpy(indexData, indices, indexBufferSize);
    vkUnmapMemory(renderer->device, indexStagingMemory);

    skRenderer_CreateBuffer(renderer, indexBufferSize,
                            VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
                                VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                            &line.indexBuffer,
                            &line.indexBufferMemory);

    skRenderer_CopyBuffer(renderer, indexStagingBuffer,
                          line.indexBuffer, indexBufferSize);

    vkDestroyBuffer(renderer->device, indexStagingBuffer, NULL);
    vkFreeMemory(renderer->device, indexStagingMemory, NULL);

    for (int frame = 0; frame < SK_FRAMES_IN_FLIGHT; frame++)
    {
        skRenderer_CreateBuffer(
            renderer, sizeof(skUniformBufferObject),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &line.uniformBuffers[frame],
            &line.uniformBuffersMemory[frame]);

        vkMapMemory(renderer->device,
                    line.uniformBuffersMemory[frame], 0,
                    sizeof(skUniformBufferObject), 0,
                    &line.uniformBuffersMap[frame]);
    }

    VkDescriptorSetLayout layouts[SK_FRAMES_IN_FLIGHT];
    for (int i = 0; i < SK_FRAMES_IN_FLIGHT; i++)
    {
        layouts[i] = renderer->descriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = renderer->descriptorPool;
    allocInfo.descriptorSetCount = SK_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = layouts;

    if (vkAllocateDescriptorSets(renderer->device, &allocInfo,
                                 line.descriptorSets) != VK_SUCCESS)
    {
        printf("SK ERROR: Failed to allocate descriptor sets for "
               "object.");
    }

    // Update descriptor sets
    for (int frame = 0; frame < SK_FRAMES_IN_FLIGHT; frame++)
    {
        VkDescriptorBufferInfo bufferInfo = {0};
        bufferInfo.buffer = line.uniformBuffers[frame];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(skUniformBufferObject);

        VkWriteDescriptorSet descriptorWrites[] = {{0}};

        descriptorWrites[0].sType =
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = line.descriptorSets[frame];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType =
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(renderer->device, 1, descriptorWrites,
                               0, NULL);
    }

    glm_mat4_identity(line.transform);

    return line;
}

void skRenderer_AddLineObject(skRenderer*   renderer,
                              skLineObject* line)
{
    skVector_PushBack(renderer->lineObjects, line);
}
