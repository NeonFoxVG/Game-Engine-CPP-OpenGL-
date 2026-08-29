Great! Now it's time to move the shapes we've drawn. Let's get familiar with the basic settings and control capabilities. We'll start with the keyboard.

Keyboard input is handled via events. What does that mean? When you press a key, it sends a signal to the operating system, which triggers an event you can listen for. GLFW provides a unified way to handle such events.

To handle keypress events in GLFW, you need to set a key callback function using `glfwSetKeyCallback`. This function takes two arguments: the `window` and the `keyCallback` function pointer.

Let's declare the `keyCallback` function:

```cpp
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
```

Parameter explanation:

* `window`: the window the event came from;
* `key`: the code of the key that was pressed (e.g., spacebar, arrow keys, Escape);
* `scancode`: the physical scan code of the key;
* `action`: the type of action (pressed, released, etc.);
* `mods`: any modifier keys (Shift, Ctrl, etc.).

We check if a key was pressed:

```cpp
if (action == GLFW_PRESS) 
{
    switch (key) 
    {
        case GLFW_KEY_UP:
            std::cout << "GLFW_KEY_UP" << std::endl;
            break;
        case GLFW_KEY_DOWN:
            std::cout << "GLFW_KEY_DOWN" << std::endl;
            break;
        case GLFW_KEY_LEFT:
            std::cout << "GLFW_KEY_LEFT" << std::endl;
            break;
        case GLFW_KEY_RIGHT:
            std::cout << "GLFW_KEY_RIGHT" << std::endl;
            break;
        default:
            break;
    }
}
```

Now let's do something more interesting — make our shape move. We’ll add an offset to the vertex coordinates via the vertex shader.

First, define a structure:

```cpp
struct Vec2 
{
    float x = 0.0f;
    float y = 0.0f;
};
```

Then declare a uniform in the vertex shader:

```glsl
uniform vec2 uOffset;
```

Apply the offset in the vertex shader:

```glsl
gl_Position = vec4(position.x + uOffset.x, position.y + uOffset.y, position.z, 1.0);
```

Get the uniform location in code:

```cpp
GLint uOffsetLoc = glGetUniformLocation(shaderProgram, "uOffset");
```

Create the variable:

```cpp
Vec2 offset;
```

Send it to the shader program:

```cpp
glUniform2f(uOffsetLoc, offset.x, offset.y);
```

Finally, update the offset when arrow keys are pressed:

```cpp
case GLFW_KEY_UP:
    offset.y += 0.01f;
    break;
case GLFW_KEY_DOWN:
    offset.y -= 0.01f;
    break;
case GLFW_KEY_LEFT:
    offset.x -= 0.01f;
    break;
case GLFW_KEY_RIGHT:
    offset.x += 0.01f;
    break;
```

Now, when you press the arrow keys, the shape will move accordingly on the screen. Great job! We're getting closer to creating our first game.
