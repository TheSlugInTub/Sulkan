#include "include/sulkan/basic_components.h"
#include "include/sulkan/imgui_layer.h"

void skSystemInfo_DrawComponent(skSystemInfo* object)
{
}


void skName_DrawComponent(skName* object)
{
    skImGui_InputText("name", object->name, 128, 0);
}


void skTransform_DrawComponent(skTransform* object)
{
}

