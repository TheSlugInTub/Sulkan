#pragma once

#include <sulkan/essentials.h>
#include <sulkan/state.h>

typedef struct skEditor
{
    u32 selectedEntityIndex;
    skEntityID selectedEntity;
    skECSState* ecsState;
    char sceneName[128];
    Bool playing;
} skEditor;

void skEditor_DrawHierarchy(skEditor* editor);
void skEditor_DrawInspector(skEditor* editor);
void skEditor_DrawTray(skEditor* editor);

void skEditor_SaveScene(skECSState* state);
void skEditor_LoadScene(skECSState* state);
