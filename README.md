# Sulkan

Sulkan is a game engine in C.
The engine uses Vulkan for rendering and uses an ECS paradigm.

There is documentation for the engine when you start it.

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
[joltc](https://github.com/amerkoleci/joltc) - For 3d physics.

# TODO

- [ ] Make a new descriptor set layout for the lines.
- [ ] Seperate line rendering, skybox rendering and object rendering in 
skRenderer_RecordCommandBuffers
- [ ] Fix error handling
- [ ] Add custom physics layers and interactions in physics_3d
- [ ] Make physics_3d conversion between rigidbody and render association more performant
- [ ] Clean everything up
- [ ] Improve editor interface for components

# Quirks

If you have a rigidbody with a rotation with the component w set to zero, it will crash
the program if it collides with another rigidbody.
