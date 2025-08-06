README

Please use middle mouse click to pan the camera around the scene if you have the default 
set up. WASD to move the camera.

There is already a sample scene in the build if you have downloaded this from github
releases. Please press load in the tray to load the scene.

DOCUMENTATION

Sulkan is a game engine/framework written in C.

It is a codebase which you can drag into a project and include the sulkan.h file to 
include the API. From there, you can make a window, add render objects and render
graphics onto the screen.

---
HIGH LEVEL OVERVIEW
---

RENDERER
---

The renderer is a struct which is created in your application and it is used to render
stuff. The struct has a lot of Vulkan info and structs inside such as physical devices,
logical devices, descriptor set layouts and more.

RENDER OBJECTS
---

The renderer struct also has a vector of render objects in it. A render object is a 
struct which contains a vertex buffer, index buffer, and descriptor sets for diffuse,
normal and roughness textures.
You can make a render object from a model using the skRenderObject_CreateFroModel 
function, you can then add this object to the vector of render objects in the renderer
struct using skRenderer_AddRenderObject. All the render objects are drawn when you call
the skRenderer_DrawFrame command.

ECS
---

There is an ECS implementation in Sulkan that is written in C++ but there is an API for
calling it from C. Everything begins from a scene which is a struct you create.
It has entities which are u64s that you can create with skScene_CreateEntity.
You can associate a component with
an entity by calling SK_ECS_ASSIGN which is a macro which makes it easier to add
components to entities. You can get the component from an entity using SK_ECS_GET.

There is something called a scene view which lets you iterate through all entities in
a scene with a certain component or components. You will usually use these scene views
in the ECS concept known as a system which are functions in the engine. You can assign
a system with skECS_AddSystem(bool start). Whether you label this as a start system
will get it put into two different vectors. You can call the regular systems with
skECS_UpdateSystems and start systems with skECS_StartStartSystems.

There are a number of default systems in the engine which you will have to register
manually.

There is also a struct known as an skECSState which has various members that you will 
have to fill up to provide this struct as an argument for calling all the systems.
All the systems have an skECSState parameter as argument by the way.

EDITOR
---

There is an editor you can enable in the engine. You simply have to make an skEditor
struct and pass it over to the skRenderer_DrawFrame function.

The editor has four panels, a hierarchy panel in which all entities in the scene are 
displayed, an inspector panel in which all the components of the selected entity are
displayed and whose values can be tweaked, a tray panel for saving and loading the 
scene, and finally a documentation panel in which you are very likely reading this
from.

The editor is drawn with ImGui which is a C++ library but I have made a C API for it.

MICAH
---

There is a program in the project that is called micah. You'll have to supply it all
the files in which you have components as the arguments (including the engine filesl),
and it will create two files called micah.h and micah.c. The first file has all the 
function declarations of drawers, serializers and deserializers for every component
and the second file defines all of these functions. These functions are procedurally
generated from the struct definition in the files you have provided it. You will need
to add a // COMPONENT comment above the struct in order for micah to regonize it as a
component. It will only serialize a few basic types such as int, float, char, char[],
vec2, vec3, vec4, mat4 and double. You will unfortunately have to recompile the engine
when these files are generated again.
If you put an // IGNORE comment above a function in the micah.c file, it won't 
regenerate it.

RENDER AND LIGHT ASSOCIATIONS
---

There is a component called a render association, this is purely for the editor's sake.
In it, you can define a render object and generate it with the "Create render object"
button in the GUI for it.

The same is true for the light association but with a different component.

---
TUTORIAL
---

There is an example of how to get stuff up and running in Sulkan in the main.c file 
of the engine.
