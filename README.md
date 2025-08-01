# Sulkan

Sulkan is a game engine in C.
The engine uses Vulkan for rendering and uses an ECS paradigm.

# Building

```bash
git clone https://github.com/TheSlugInTub/Sulkan.git
cd Sulkan 
mkdir bld
cmake -S . -B bld
cmake --build bld/ --config Release
```

Go to bld/Release and you will find the executable.
You will need to have git and cmake to compile the project.

# Libraries used

[GLFW](https://github.com/glfw/glfw) - For window creation and management. \
[Assimp](https://github.com/assimp/assimp) - For model loading. \
[cglm](https://github.com/recp/cglm) - For math and linear algebra. \
[stb_image](https://github.com/nothings/stb) - For loading images. \
[LunarG Vulkan SDK](https://github.com/LunarG/VulkanTools/releases/tag/sdk-1.0.33.0) - For graphics rendering.
[ImGui](https://github.com/ocornut/imgui) - For graphics rendering.

# Design

This section is going to cover the design of Sulkan.

## Renderer

The renderer is completely seperate from the ECS, I originally wanted to have a 
skMeshRenderer or skSpriteRenderer components but I realized that looping 
through the current scene with a scene view to check for mesh renderers or sprite 
renderers every frame wouldn't be very performant, so I decided to have a vector
of skRenderObjects in the renderer class. You create a renderer object, then you 
intialize it, then you add render objects to it with skRenderer_AddRenderObject,
then in the main loop you draw the scene with skRenderer_DrawFrame.

The render objects are very basic structs with just a vertex buffer, index buffer
and texture image in them, you need to load them up using skRenderObject_LoadFrom...
functions before adding them to the renderer.

## ECS

There's an ECS API for Sulkan. You can make an skScene, and make skEntityIDs.
You can also make systems and loop over entities with specific components using 
iterators. You can then add this system using skECS_AddSystem. You can also specify
whether this is a start system or a regular systems, both of which have seperate vectors
and you can call both vectors with a function.
There's also an ECS state struct you need to create to call the systems.

# TODO

- [ ] Make a new descriptor set layout for the lines.
- [ ] Seperate line rendering, skybox rendering and object rendering in 
skRenderer_RecordCommandBuffers
- [ ] Fix error handling
- [ ] Fix object layers and broad phase layers in physics_3d
- [ ] Split skPhysics3dState_CreateBody function into multiple types of colliders
- [ ] Fix micah formatting
