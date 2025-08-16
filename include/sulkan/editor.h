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
    const char* documentationText;
} skEditor;

skEditor skEditor_Create(skECSState* state, const char* scenePath,
                         const char* documentationPath);
void skEditor_DrawHierarchy(skEditor* editor);
void skEditor_DrawInspector(skEditor* editor);
void skEditor_DrawTray(skEditor* editor);

void skEditor_SaveScene(skECSState* state);
void skEditor_LoadScene(skECSState* state, skSceneHandle scene,
                        const char* filepath);
