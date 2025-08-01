#pragma once

#include <stdbool.h>
#include <sulkan/window.h>

// Alphabet keys
#define SK_KEY_A GLFW_KEY_A
#define SK_KEY_B GLFW_KEY_B
#define SK_KEY_C GLFW_KEY_C
#define SK_KEY_D GLFW_KEY_D
#define SK_KEY_E GLFW_KEY_E
#define SK_KEY_F GLFW_KEY_F
#define SK_KEY_G GLFW_KEY_G
#define SK_KEY_H GLFW_KEY_H
#define SK_KEY_I GLFW_KEY_I
#define SK_KEY_J GLFW_KEY_J
#define SK_KEY_K GLFW_KEY_K
#define SK_KEY_L GLFW_KEY_L
#define SK_KEY_M GLFW_KEY_M
#define SK_KEY_N GLFW_KEY_N
#define SK_KEY_O GLFW_KEY_O
#define SK_KEY_P GLFW_KEY_P
#define SK_KEY_Q GLFW_KEY_Q
#define SK_KEY_R GLFW_KEY_R
#define SK_KEY_S GLFW_KEY_S
#define SK_KEY_T GLFW_KEY_T
#define SK_KEY_U GLFW_KEY_U
#define SK_KEY_V GLFW_KEY_V
#define SK_KEY_W GLFW_KEY_W
#define SK_KEY_X GLFW_KEY_X
#define SK_KEY_Y GLFW_KEY_Y
#define SK_KEY_Z GLFW_KEY_Z

// Number keys
#define SK_KEY_0 GLFW_KEY_0
#define SK_KEY_1 GLFW_KEY_1
#define SK_KEY_2 GLFW_KEY_2
#define SK_KEY_3 GLFW_KEY_3
#define SK_KEY_4 GLFW_KEY_4
#define SK_KEY_5 GLFW_KEY_5
#define SK_KEY_6 GLFW_KEY_6
#define SK_KEY_7 GLFW_KEY_7
#define SK_KEY_8 GLFW_KEY_8
#define SK_KEY_9 GLFW_KEY_9

// Function keys
#define SK_KEY_F1  GLFW_KEY_F1
#define SK_KEY_F2  GLFW_KEY_F2
#define SK_KEY_F3  GLFW_KEY_F3
#define SK_KEY_F4  GLFW_KEY_F4
#define SK_KEY_F5  GLFW_KEY_F5
#define SK_KEY_F6  GLFW_KEY_F6
#define SK_KEY_F7  GLFW_KEY_F7
#define SK_KEY_F8  GLFW_KEY_F8
#define SK_KEY_F9  GLFW_KEY_F9
#define SK_KEY_F10 GLFW_KEY_F10
#define SK_KEY_F11 GLFW_KEY_F11
#define SK_KEY_F12 GLFW_KEY_F12

// Arrow keys
#define SK_KEY_UP    GLFW_KEY_UP
#define SK_KEY_DOWN  GLFW_KEY_DOWN
#define SK_KEY_LEFT  GLFW_KEY_LEFT
#define SK_KEY_RIGHT GLFW_KEY_RIGHT

// Modifier keys
#define SK_KEY_LEFT_SHIFT    GLFW_KEY_LEFT_SHIFT
#define SK_KEY_RIGHT_SHIFT   GLFW_KEY_RIGHT_SHIFT
#define SK_KEY_LEFT_CONTROL  GLFW_KEY_LEFT_CONTROL
#define SK_KEY_RIGHT_CONTROL GLFW_KEY_RIGHT_CONTROL
#define SK_KEY_LEFT_ALT      GLFW_KEY_LEFT_ALT
#define SK_KEY_RIGHT_ALT     GLFW_KEY_RIGHT_ALT
#define SK_KEY_LEFT_SUPER    GLFW_KEY_LEFT_SUPER
#define SK_KEY_RIGHT_SUPER   GLFW_KEY_RIGHT_SUPER

// Navigation keys
#define SK_KEY_HOME      GLFW_KEY_HOME
#define SK_KEY_END       GLFW_KEY_END
#define SK_KEY_PAGE_UP   GLFW_KEY_PAGE_UP
#define SK_KEY_PAGE_DOWN GLFW_KEY_PAGE_DOWN
#define SK_KEY_INSERT    GLFW_KEY_INSERT
#define SK_KEY_DELETE    GLFW_KEY_DELETE
#define SK_KEY_BACKSPACE GLFW_KEY_BACKSPACE
#define SK_KEY_TAB       GLFW_KEY_TAB
#define SK_KEY_ENTER     GLFW_KEY_ENTER
#define SK_KEY_ESCAPE    GLFW_KEY_ESCAPE

// Keypad keys
#define SK_KEY_KP_0        GLFW_KEY_KP_0
#define SK_KEY_KP_1        GLFW_KEY_KP_1
#define SK_KEY_KP_2        GLFW_KEY_KP_2
#define SK_KEY_KP_3        GLFW_KEY_KP_3
#define SK_KEY_KP_4        GLFW_KEY_KP_4
#define SK_KEY_KP_5        GLFW_KEY_KP_5
#define SK_KEY_KP_6        GLFW_KEY_KP_6
#define SK_KEY_KP_7        GLFW_KEY_KP_7
#define SK_KEY_KP_8        GLFW_KEY_KP_8
#define SK_KEY_KP_9        GLFW_KEY_KP_9
#define SK_KEY_KP_DECIMAL  GLFW_KEY_KP_DECIMAL
#define SK_KEY_KP_DIVIDE   GLFW_KEY_KP_DIVIDE
#define SK_KEY_KP_MULTIPLY GLFW_KEY_KP_MULTIPLY
#define SK_KEY_KP_SUBTRACT GLFW_KEY_KP_SUBTRACT
#define SK_KEY_KP_ADD      GLFW_KEY_KP_ADD
#define SK_KEY_KP_ENTER    GLFW_KEY_KP_ENTER
#define SK_KEY_KP_EQUAL    GLFW_KEY_KP_EQUAL

// Special character keys
#define SK_KEY_SPACE         GLFW_KEY_SPACE
#define SK_KEY_APOSTROPHE    GLFW_KEY_APOSTROPHE    // '
#define SK_KEY_COMMA         GLFW_KEY_COMMA         // ,
#define SK_KEY_MINUS         GLFW_KEY_MINUS         // -
#define SK_KEY_PERIOD        GLFW_KEY_PERIOD        // .
#define SK_KEY_SLASH         GLFW_KEY_SLASH         // /
#define SK_KEY_SEMICOLON     GLFW_KEY_SEMICOLON     // ;
#define SK_KEY_EQUAL         GLFW_KEY_EQUAL         // =
#define SK_KEY_LEFT_BRACKET  GLFW_KEY_LEFT_BRACKET  // [
#define SK_KEY_BACKSLASH     GLFW_KEY_BACKSLASH     // forward slash
#define SK_KEY_RIGHT_BRACKET GLFW_KEY_RIGHT_BRACKET // ]
#define SK_KEY_GRAVE_ACCENT  GLFW_KEY_GRAVE_ACCENT  // `

// Lock keys
#define SK_KEY_CAPS_LOCK    GLFW_KEY_CAPS_LOCK
#define SK_KEY_SCROLL_LOCK  GLFW_KEY_SCROLL_LOCK
#define SK_KEY_NUM_LOCK     GLFW_KEY_NUM_LOCK
#define SK_KEY_PRINT_SCREEN GLFW_KEY_PRINT_SCREEN
#define SK_KEY_PAUSE        GLFW_KEY_PAUSE

#define SK_MOUSE_BUTTON_LEFT   GLFW_MOUSE_BUTTON_LEFT
#define SK_MOUSE_BUTTON_RIGHT  GLFW_MOUSE_BUTTON_RIGHT
#define SK_MOUSE_BUTTON_MIDDLE GLFW_MOUSE_BUTTON_MIDDLE

// Additional mouse buttons (many mice have these extra buttons)
#define SK_MOUSE_BUTTON_4 GLFW_MOUSE_BUTTON_4
#define SK_MOUSE_BUTTON_5 GLFW_MOUSE_BUTTON_5
#define SK_MOUSE_BUTTON_6 GLFW_MOUSE_BUTTON_6
#define SK_MOUSE_BUTTON_7 GLFW_MOUSE_BUTTON_7
#define SK_MOUSE_BUTTON_8 GLFW_MOUSE_BUTTON_8

// Mouse button count definition
#define SK_MOUSE_BUTTON_COUNT (GLFW_MOUSE_BUTTON_LAST + 1)

typedef int skKey;

// Only returns true on the first frame that a key is pressed
bool skInput_GetKeyDown(skWindow* window, skKey key);
// Only returns true on the first frame that a key is released
bool skInput_GetKeyUp(skWindow* window, skKey key);
// Returns true if the key is pressed
bool skInput_GetKey(skWindow* window, skKey key);

// Much like the key functions but with mouse input

bool skInput_GetMouseButtonDown(skWindow* window, skKey mouseKey);
bool skInput_GetMouseButton(skWindow* window, skKey mouseKey);

// These functions get the screen-space position of the mouse

int skInput_GetMouseInputHorizontal(skWindow* window);
int skInput_GetMouseInputVertical(skWindow* window);
