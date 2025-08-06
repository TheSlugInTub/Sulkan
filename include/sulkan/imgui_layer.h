#pragma once

#include <cglm/cglm.h>
#include <stdarg.h>
#define VK_USE_PLATFORM_WIN32_KHR
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef void*                    skImGuiTextureID;
typedef struct skImGuiPayload_t* skImGuiPayload;

void skImGui_Init(struct GLFWwindow* window, VkInstance instance, 
        VkDescriptorPool descriptorPool,
        VkRenderPass renderPass, VkPhysicalDevice physicalDevice, VkDevice device,
        VkCommandPool pool, VkQueue graphicsQueue);
void skImGui_NewFrame();
void skImGui_EndFrame(VkCommandBuffer commandBuffer);
void skImGui_Terminate();
void skImGui_Theme1();

bool skImGui_Begin(const char* name);
void skImGui_End();

void skImGui_DebugWindow();
void skImGui_DemoWindow();

bool skImGui_DragFloat(const char* name, float* val, float speed);
bool skImGui_DragFloat2(const char* name, float* val, float speed);
bool skImGui_DragFloat3(const char* name, float* val, float speed);
bool skImGui_DragFloat4(const char* name, float* val, float speed);
bool skImGui_DragFloat16(const char* name, float* val, float speed);

bool skImGui_InputInt(const char* name, int* val);
bool skImGui_InputHex(const char* name, unsigned int* val);

bool skImGui_ComboBox(const char* name, const char** types,
                      int* currentType, int typeSize);

bool skImGui_Checkbox(const char* name, bool* val);
bool skImGui_SliderInt(const char* name, int* currentType, int min,
                       int max);
bool skImGui_Button(const char* name);
bool skImGui_ImageButton(skImGuiTextureID tex, vec2 size);
bool skImGui_InputText(const char* name, char* buffer, size_t size,
                       int flags);
bool skImGui_InputTextMultiline(const char* name, char* buffer, size_t size,
                       int flags);

bool skImGui_ColorEdit4(const char* name, float* val);

bool skImGui_IsWindowHovered();
bool skImGui_CollapsingHeader(const char* name);
bool skImGui_ColorPicker(const char* name, vec4 color);
void skImGui_TextLong(const char* val);
void skImGui_Text(const char* val);

bool skImGui_MenuItem(const char* name);
void skImGui_PushID(int id);
void skImGui_PopID();
bool skImGui_Selectable(const char* name, bool selected);
bool skImGui_BeginDragDropSource(int flags);
void skImGui_SetDragDropPayload(const char* name, const void* data,
                                size_t size);
void skImGui_EndDragDropSource();

bool skImGui_BeginDragDropTarget();
void skImGui_EndDragDropTarget();

void skImGui_Separator();

skImGuiPayload skImGui_AcceptDragDropPayload(const char* name);
void*          skImGuiPayload_GetData(skImGuiPayload payload);
int            skImGuiPayload_GetDataSize(skImGuiPayload payload);

bool skImGui_BeginPopupContextWindow();
void skImGui_EndPopup();

#ifdef __cplusplus
}
#endif
