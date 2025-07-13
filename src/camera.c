#include <sulkan/camera.h>
#include <sulkan/state.h>
#include <sulkan/input.h>

skCamera skCamera_Create(vec3 position, vec3 up, float yaw,
                         float pitch, float FOV)
{
    skCamera camera;
    camera.pitch = pitch;
    camera.yaw = yaw;
    camera.roll = 0.0f;
    camera.FOV = FOV;
    camera.zoom = 0.0f;
    camera.position[0] = position[0];
    camera.position[1] = position[1];
    camera.position[2] = position[2];
    camera.up[0] = up[0];
    camera.up[1] = up[1];
    camera.up[2] = up[2];
    camera.worldUp[0] = 0.0f;
    camera.worldUp[1] = 1.0f;
    camera.worldUp[2] = 0.0f;
    camera.front[0] = 0.0f;
    camera.front[1] = 0.0f;
    camera.front[2] = -1.0f;
    return camera;
}

void skCamera_UpdateVectors(skCamera* camera)
{
    camera->front[0] =
        cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    camera->front[1] = sin(glm_rad(camera->pitch));
    camera->front[2] =
        sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    glm_normalize(camera->front);

    glm_cross(camera->front, camera->worldUp, camera->right);
    glm_cross(camera->right, camera->front, camera->up);
    glm_normalize(camera->right);
    glm_normalize(camera->up);

    if (camera->roll != 0.0f)
    {
        float cosRoll = cos(camera->roll);
        float sinRoll = sin(camera->roll);

        // Store original right vector
        vec3 originalRight;
        glm_vec3_copy(camera->right, originalRight);

        // Rotate right vector around front
        camera->right[0] =
            cosRoll * originalRight[0] - sinRoll * camera->up[0];
        camera->right[1] =
            cosRoll * originalRight[1] - sinRoll * camera->up[1];
        camera->right[2] =
            cosRoll * originalRight[2] - sinRoll * camera->up[2];

        // Recalculate up vector to be perpendicular to front and
        // right
        glm_cross(camera->right, camera->front, camera->up);
        glm_normalize(camera->right);
        glm_normalize(camera->up);
    }
}

void skCamera_GetViewMatrix(skCamera* camera, mat4 view)
{
    glm_mat4_identity(view);
    vec3 center;
    glm_vec3_add(camera->position, camera->front, center);

    glm_lookat(camera->position, center, camera->up, view);
}

bool  firstMouse = true;
float lastX = 0.0f, lastY = 0.0f;

void skCamera_Sys(skECSState* state)
{
    // Calculate movement speed
    float speed = 0.001f;

    // Calculate movement vectors based on camera orientation
    vec3 forward, right;

    glm_vec3_copy(state->camera->front, forward);

    // Right vector is perpendicular to forward on the XZ plane
    glm_cross((vec3) {0.0f, 1.0f, 0.0f}, forward, right);
    glm_normalize(right);

    if (skInput_GetKey(state->window, SK_KEY_W))
    {
        state->camera->position[0] += forward[0] * speed;
        state->camera->position[1] += forward[1] * speed;
        state->camera->position[2] += forward[2] * speed;
    }
    if (skInput_GetKey(state->window, SK_KEY_S))
    {
        state->camera->position[0] -= forward[0] * speed;
        state->camera->position[1] -= forward[1] * speed;
        state->camera->position[2] -= forward[2] * speed;
    }

    // Strafe left/right (along right vector)
    if (skInput_GetKey(state->window, SK_KEY_A))
    {
        state->camera->position[0] += right[0] * speed;
        state->camera->position[2] += right[2] * speed;
    }
    if (skInput_GetKey(state->window, SK_KEY_D))
    {
        state->camera->position[0] -= right[0] * speed;
        state->camera->position[2] -= right[2] * speed;
    }

    if (skInput_GetMouseButton(state->window, SK_MOUSE_BUTTON_MIDDLE))
    {
        double xpos, ypos;
        glfwGetCursorPos(state->window->window, &xpos, &ypos);

        // If this is the first frame of middle mouse press, just
        // store position
        if (!firstMouse)
        {
            lastX = (float)xpos;
            lastY = (float)ypos;
            firstMouse = true;
        }
        else
        {
            // Calculate offset
            float xoffset = (float)xpos - lastX;
            float yoffset = lastY - (float)ypos;

            // Update last position
            lastX = (float)xpos;
            lastY = (float)ypos;

            // Apply sensitivity
            const float sensitivity = 0.1f;
            xoffset *= sensitivity;
            yoffset *= sensitivity;

            // Update camera angles
            state->camera->yaw += xoffset;
            state->camera->pitch += yoffset;

            // Constrain pitch to prevent flipping
            if (state->camera->pitch > 89.0f)
                state->camera->pitch = 89.0f;
            if (state->camera->pitch < -89.0f)
                state->camera->pitch = -89.0f;

            // Update camera vectors
            skCamera_UpdateVectors(state->camera);
        }
    }
    else
    {
        // Reset the flag when middle mouse is released
        firstMouse = false;
    }

    mat4 view;
    skCamera_GetViewMatrix(state->camera, view);
    glm_mat4_copy(view, state->renderer->viewTransform);
}
