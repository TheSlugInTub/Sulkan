# Vulkan

Vulkan is a graphics API much like OpenGL or DirectX.

A graphics API is an interface that allows us to communicate with the GPU and make it do work.

To first understand Vulkan, we need to understand GPUs first.

# GPUs

The GPU is mounted on an area known as the PCB, this is where all the components are mounted.

The real meat of the GPU is the graphics processing unit, this is a chip in which most of the 
area is taken up by the processing cores. These processing cores are called GPCs or graphics
processingclusters, and in them are 12 steaming multiprocessors, and in those are 4 Warps and
1 ray tracing core and in those warps, are 32 cuda cores and one tensor core.

Across the GPU are 10,752 cuda cores, 336 tensor cores and 84 ray tracing cores.

CUDA cores are the simplest of the bunch, they're essentially simple binary calculators, with 
addition, multiplication and a few other operations and are used the most in graphics 
applications.

Tensor cores are used for linear algebra and matrix multiplication.

And ray tracing cores are used for ray tracing.

# Overview

This section is dedicated to an overview of drawing something onto the screen in Vulkan.

## Instance

To interact with the Vulkan API, we need an instance. We will create a `VkInstance`.

## Surface

To draw something onto the screen, we need to interact with a `VkSurface` object.
The `VkSurface` object is an abstraction of a window, you'll need to provide a native 
window handle like HWND on Windows to create it.
You can get the window handle on any platform using GLFW.

## Physical device

Then, we will need to get the GPU that we want to use.
Vulkan handles this with a `VkPhysicalDevice` object that we need to create.
We can get the number of devices we have with `vkEnumeratePhysicalDevices` and we can them 
in an array using the same function call but with the last parameter pointing to an array.
We can then loop through this array and choose which device is best suited to our needs.
We can check what extensions the graphics card is capable of using 
`vkEnumerateDeviceExtensionProperties` and check if we are ok with the extensions it has.

### Queue Families

In Vulkan, almost every operation is performed through a queue. A draw call is pushed onto 
a queue for rendering, a request call for an object's properties is also pushed back on a 
queue.
A queue family is a set of related queues, for example for rendering or presentation.
We can get the queue families of a physical device using 
`vkGetPhysicalDeviceQueueFamilyProperties` using two calls, one for the count and one for the
array.
We can loop through these queue families to see what kind they are, if we find a graphics
or presentation queue family, we need to store its index for later use.

## Logical device

A logical device is a mini API for the physical device. The physical device is a 
representation of your actual GPU whereas a logical device is how you interface with it.
We will create two queues for the graphics and presnetation queue families, these will be 
useful for their respective tasks.

## Swapchain

The swapchain is a collection of render targets. Its purpose is to ensure that the image
currently shown on screen is different from the one that we're rendering.
Whenever you want to render something onto a render target, you'll need to request a render
target from the swapchain.

When creating a swapchain, you'll need to pick an image format and a presentation mode that
your GPU supports. You'll also want to store each image of the swapchain into a vector for 
later rendering (The swapchain is a list of `VkImage`s).

## Image views and Framebuffers

To properly interact with a `VkImage`, we first need to create its corresponding 
`VkImageView`. You'll want to create a `VkImageView` for every image of the swapchain you 
have stored.

You'll also want to create a `VkFramebuffer` for every image since a framebuffer stores
stencil, depth and color information and you'll need it for rendering.

## Render passes

A render pass is a collection of images that will be used to render to, you can add a color
attachment to the render pass to make it use color and be informed about what formats and 
options you want it to use. The render pass will be used during rendering

## Graphics pipeline

A `VkGraphicsPipeline` object in Vulkan describes the state of the GPU, the configurable
options like viewport width, viewport height, shaders, configuration of the input assembly,
configuration of rasterization, multisampling, color blend modes and more. The render pass
will also need to be referenced in the pipeline's creation to let Vulkan know what render 
targets we will be using. The graphics pipeline will need to be binded when issuing a draw 
call.

When trying to change a graphics setting like multisampling, shaders or blend modes, an 
entirely new `VkGraphicsPipeline` object needs to be made.

## Command pools and Command buffers

To draw something onto the screen, we will need to fill a `VkCommandBuffer` object with the
adequate commands to draw something onto the screen. For example, a command buffer can have 
the commands of binding the pipeline, beginning the render pass, setting the viewport and 
ending the render pass. At the end, it will end the command buffer to stop recording these
commands into the buffer any longer.

## Semaphores and Fences

In Vulkan (And other applications), there are things called semaphores and fences that are 
used for timing parallel operations.

Semaphores are pretty much booleans. They can be signaled or unsignaled. We can start an 
operation on the GPU, such as drawing to a render target, and have it signal a semaphore when 
finished, another operation, such as drawing to a render target will have to wait for the 
semaphore to become signaled to begin work. Semaphores are completely on the GPU and are very 
useful for timing. For example, you can use it to time draw commands in a queue to make sure 
that two parallel operations aren't drawing to the same target.

Fences on the other hand, are a lot like semaphores except they are also on the CPU, which
means that the CPU will also have to wait for the fence to become signaled and will halt the
thread. For example, you can use fences to have to wait for the GPU to finish rendering to 
show the final image onto the screen.

We will create two semaphores, one for the image, and one for the presentation.
We will also create a fence to signal that the GPU has finished rendering the image and that
we are ready to present it onto the screen.

## Rendering

To start rendering, we will wait for the fence that we created, and once it becomes signaled,
we reset it and aquire a render target from the swapchain. We reset the command buffer and 
then record it. We then create a `VkSubmitInfo` struct and load it with the appropriate 
semaphores and our command buffer. We then submit our submit info onto the graphics queue that
we made earlier. And after submitting the command buffer to be drawn, we present it with
`vkQueuePresentKHR`, this doesn't immediately present the potentially half drawn image onto
the screen of course, it waits for our semaphore to be signalled and then presents it.
