#pragma once

#include "include/sulkan/basic_components.h"
#include "include/sulkan/render_association.h"
#include "include/sulkan/light_association.h"
#include "include/sulkan/physics_3d.h"
#include "include/sulkan/imgui_layer.h"
#include "include/sulkan/state.h"
#include "include/sulkan/json_api.h"

// micah.h
// Procedurally generated header file for the Sulkan game engine
// This contains drawer/serializer/deserializer function declarations for all registered components

void skName_DrawComponent(skName* object, skECSState* state);

skJson skName_SaveComponent(skName* object);

void skName_LoadComponent(skName* object, skJson j);


void skRenderAssociation_DrawComponent(skRenderAssociation* object, skECSState* state);

skJson skRenderAssociation_SaveComponent(skRenderAssociation* object);

void skRenderAssociation_LoadComponent(skRenderAssociation* object, skJson j);


void skLightAssociation_DrawComponent(skLightAssociation* object, skECSState* state);

skJson skLightAssociation_SaveComponent(skLightAssociation* object);

void skLightAssociation_LoadComponent(skLightAssociation* object, skJson j);


void skRigidbody3D_DrawComponent(skRigidbody3D* object, skECSState* state);

skJson skRigidbody3D_SaveComponent(skRigidbody3D* object);

void skRigidbody3D_LoadComponent(skRigidbody3D* object, skJson j);

void Micah_DrawAllComponents(skECSState* state, skEntityID ent);

skJson Micah_SaveAllComponents(skECSState* state);

void Micah_LoadAllComponents(skECSState* state, skJson j);

void Micah_ComponentAddMenu(skECSState* state, skEntityID ent);

