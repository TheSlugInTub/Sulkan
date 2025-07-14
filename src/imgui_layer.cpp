#include <sulkan/imgui_layer.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>
#include <GLFW/glfw3.h>

#define SK_FRAMES_IN_FLIGHT (2)
#define SK_MAX_RENDER_OBJECTS (1000)

struct skImGuiPayload_t
{
    const ImGuiPayload* payload;
};

ImFont* mainfont = nullptr;

extern "C"
{

void skImGui_Init(struct GLFWwindow* window, VkInstance instance, 
        VkDescriptorPool descriptorPool,
        VkRenderPass renderPass, VkPhysicalDevice physicalDevice, VkDevice device,
        VkCommandPool pool, VkQueue graphicsQueue)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    ImGui_ImplGlfw_InitForVulkan(window, true);

    ImGui_ImplVulkan_InitInfo info = {0};
    info.Instance = instance;
    info.DescriptorPool = descriptorPool;
    info.Device = device;
    info.RenderPass = renderPass;
    info.PhysicalDevice = physicalDevice;
    info.ImageCount = SK_MAX_RENDER_OBJECTS * SK_FRAMES_IN_FLIGHT;
    info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    info.Subpass = 0;
    info.QueueFamily = ImGui_ImplVulkanH_SelectQueueFamilyIndex(physicalDevice);
    info.Queue = graphicsQueue;
    info.MinImageCount = 2;

    ImGui_ImplVulkan_LoadFunctions(
    VK_API_VERSION_1_0,
    [](const char* function_name, void* user_data) -> PFN_vkVoidFunction {
        return vkGetInstanceProcAddr(*(VkInstance*)user_data, function_name);
    }, &instance);

    ImGui_ImplVulkan_Init(&info);

    VkCommandBufferAllocateInfo allocInfo = {0};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = pool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo,
                             &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {0};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {0};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo,
                  VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, pool, 1,
                         &commandBuffer);

    vkDeviceWaitIdle(device);
}

void skImGui_NewFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void skImGui_EndFrame(VkCommandBuffer commandBuffer)
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void skImGui_Terminate()
{
}

void skImGui_Theme1()
{
    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextDisabled] =
        ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
    colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
    colors[ImGuiCol_BorderShadow] =
        ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_FrameBgHovered] =
        ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_FrameBgActive] =
        ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive] =
        ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] =
        ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ScrollbarGrab] =
        ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabHovered] =
        ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
    colors[ImGuiCol_ScrollbarGrabActive] =
        ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
    colors[ImGuiCol_SliderGrabActive] =
        ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
    colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
    colors[ImGuiCol_ButtonHovered] =
        ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
    colors[ImGuiCol_ButtonActive] =
        ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.30f, 0.30f, 0.30f, 0.71f);
    colors[ImGuiCol_HeaderHovered] =
        ImVec4(0.34f, 0.34f, 0.34f, 0.36f);
    colors[ImGuiCol_HeaderActive] =
        ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
    colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_SeparatorHovered] =
        ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_SeparatorActive] =
        ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_ResizeGripHovered] =
        ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
    colors[ImGuiCol_ResizeGripActive] =
        ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
    colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
    colors[ImGuiCol_TabUnfocused] =
        ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TabUnfocusedActive] =
        ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_DockingPreview] =
        ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_DockingEmptyBg] =
        ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] =
        ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram] =
        ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] =
        ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] =
        ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderStrong] =
        ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
    colors[ImGuiCol_TableBorderLight] =
        ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] =
        ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextSelectedBg] =
        ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
    colors[ImGuiCol_DragDropTarget] =
        ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
    colors[ImGuiCol_NavHighlight] =
        ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] =
        ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] =
        ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] =
        ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2(8.00f, 8.00f);
    style.FramePadding = ImVec2(5.00f, 2.00f);
    style.CellPadding = ImVec2(6.00f, 6.00f);
    style.ItemSpacing = ImVec2(6.00f, 6.00f);
    style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
    style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
    style.IndentSpacing = 25;
    style.ScrollbarSize = 15;
    style.GrabMinSize = 10;
    style.WindowBorderSize = 1;
    style.ChildBorderSize = 1;
    style.PopupBorderSize = 1;
    style.FrameBorderSize = 1;
    style.TabBorderSize = 1;
    style.WindowRounding = 7;
    style.ChildRounding = 4;
    style.FrameRounding = 3;
    style.PopupRounding = 4;
    style.ScrollbarRounding = 9;
    style.GrabRounding = 3;
    style.LogSliderDeadzone = 4;
    style.TabRounding = 4;
}

bool skImGui_Begin(const char* name)
{
    return ImGui::Begin(name);
}

void skImGui_End()
{
    ImGui::End();
}

void skImGui_DebugWindow()
{
    ImGui::ShowStyleEditor();
}

void skImGui_DemoWindow()
{
    ImGui::ShowDemoWindow();
}

bool skImGui_DragFloat(const char* name, float* val, float speed)
{
    return ImGui::DragFloat(name, val, speed);
}

bool skImGui_DragFloat2(const char* name, float* val, float speed)
{
    return ImGui::DragFloat2(name, val, speed);
}

bool skImGui_DragFloat3(const char* name, float* val, float speed)
{
    return ImGui::DragFloat3(name, val, speed);
}

bool skImGui_DragFloat4(const char* name, float* val, float speed)
{
    return ImGui::DragFloat4(name, val, speed);
}

bool skImGui_InputInt(const char* name, int* val)
{
    return ImGui::InputInt(name, val);
}

bool skImGui_InputHex(const char* name, unsigned int* val)
{
    return ImGui::InputScalar(name, ImGuiDataType_U32, val,
                              NULL, NULL, "%08X",
                              ImGuiInputTextFlags_CharsHexadecimal);
}

bool skImGui_ComboBox(const char* name, const char** types,
                      int* currentType, int typeSize)
{
    return ImGui::Combo("Body Type", currentType, types, 3);
}

bool skImGui_Checkbox(const char* name, bool* val)
{
    return ImGui::Checkbox(name, val);
}

bool skImGui_SliderInt(const char* name, int* currentType, int min,
                       int max)
{
    return ImGui::SliderInt(name, currentType, min, max);
}

bool skImGui_Button(const char* name)
{
    return ImGui::Button(name);
}

// bool skImGui_ImageButton(skImGuiTextureID tex, vec2 size)
// {
//     return ImGui::ImageButton(tex, ImVec2(size[0], size[1]));
// }

bool skImGui_InputText(const char* name, char* buffer, size_t size,
                       int flags)
{
    return ImGui::InputText(name, buffer, size, flags);
}

bool skImGui_InputTextMultiline(const char* name, char* buffer, size_t size,
                       int flags)
{
    return ImGui::InputTextMultiline(name, buffer, size, 
                             ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), 
                             ImGuiInputTextFlags_AllowTabInput);
}

bool skImGui_ColorEdit4(const char* name, float* val)
{
    return ImGui::ColorEdit4(name, val);
}

bool skImGui_IsWindowHovered()
{
    return ImGui::IsWindowHovered();
}

bool skImGui_CollapsingHeader(const char* name)
{
    return ImGui::CollapsingHeader(name);
}

bool skImGui_ColorPicker(const char* name, vec4 color)
{
    return ImGui::ColorPicker4(name, color);
}

void skImGui_Textf(const char* val, ...)
{
    char buffer[256]; // Adjust size as needed
    // Initialize variable argument list
    va_list args;
    va_start(args, val);
    // Use vsnprintf to safely format the string
    vsnprintf(buffer, sizeof(buffer), val, args);
    // Clean up the variable argument list
    va_end(args);
    // Call ImGui::Text with the formatted name and value
    ImGui::Text("%s: %s", val, buffer);
}

void skImGui_Text(const char* val)
{
    ImGui::Text(val);
}

bool skImGui_MenuItem(const char* name)
{
    return ImGui::MenuItem(name);
}

void skImGui_PushID(int id)
{
    ImGui::PushID(id);
}

void skImGui_PopID()
{
    ImGui::PopID();
}

bool skImGui_Selectable(const char* name, bool selected)
{
    return ImGui::Selectable(name, selected);
}

bool skImGui_BeginDragDropSource(int flags)
{
    return ImGui::BeginDragDropSource(flags);
}

void skImGui_SetDragDropPayload(const char* name, const void* data,
                                size_t size)
{
    ImGui::SetDragDropPayload(name, data, size);
}

void skImGui_EndDragDropSource()
{
    ImGui::EndDragDropSource();
}

void skImGui_EndDragDropTarget()
{
    ImGui::EndDragDropTarget();
}

bool skImGui_BeginDragDropTarget()
{
    return ImGui::BeginDragDropTarget();
}

void skImGui_Separator()
{
    ImGui::Separator();
}

skImGuiPayload skImGui_AcceptDragDropPayload(const char* name)
{
    return skImGuiPayload(ImGui::AcceptDragDropPayload(name));
}

void* skImGuiPayload_GetData(skImGuiPayload payload)
{
    return payload->payload->Data;
}

int skImGuiPayload_GetDataSize(skImGuiPayload payload)
{
    return payload->payload->DataSize;
}

bool skImGui_BeginPopupContextWindow()
{
    return ImGui::BeginPopupContextWindow();
}

void skImGui_EndPopup()
{
    ImGui::EndPopup();
}
}
