# Sulkan

Sulkan is a Vulkan game engine in C.

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
