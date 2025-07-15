#pragma once

#include <sulkan/essentials.h>
#include <sulkan/state.h>

typedef struct skEditor
{
    u32 selectedEntityIndex;
    skEntityID selectedEntity;
    skECSState* ecsState;
} skEditor;

void skEditor_DrawHierarchy(skEditor* editor);
void skEditor_DrawInspector(skEditor* editor);
void skEditor_DrawTray(skEditor* editor);
