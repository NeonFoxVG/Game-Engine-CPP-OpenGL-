Great! Now we are ready to start using OpenGL in the window we just created and begin rendering.
Before we do that, we need to include the **GL Extension Wrangler** library. So, we add the following include directive:

```cpp
#include <GL/glew.h>
```

Next, we need to tell **GLFW** which version of OpenGL context we want to use. As you may remember, we decided to work with **OpenGL version 3.3**, so we need to pass that information to GLFW.

This is done using the `glfwWindowHint` function:

```cpp
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Set major version to 3
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Set minor version to 3
glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // OpenGL core profile

```

Great, now GLFW knows which OpenGL context we intend to use.

To make use of this context, we must specify which window it should be associated with. We do this using:

```cpp
glfwMakeContextCurrent(window);
```

Here, `window` is the window we created earlier.

Next, we need to initialize the **GLEW** library so that we can use OpenGL functions:

```cpp
if (glewInit() != GLEW_OK) 
{
    glfwTerminate();
    return -1;
}
```

Awesome! Now our window is ready for rendering.

Let’s do something simple first — like changing the background color.

### A Few Words About Color Representation

In all graphics APIs, any color is represented as a combination of **Red**, **Green**, and **Blue**, and sometimes an **Alpha** (transparency) channel.
Each of these components (channels) can have values ranging either from **0 to 1**, or from **0 to 255**, depending on the format.

This means that each color channel can have 256 different values (from 0 to 255), which can be stored in **8 bits**.
Since there are usually 4 channels (RGBA), one color typically takes **32 bits** (or 4 bytes) in memory.

Now that we know how colors are represented, let's set the **clear color** for our screen.

We use the function `glClearColor`, which takes 4 parameters: **Red**, **Green**, **Blue**, and **Alpha**.

Let’s set the screen to a fully opaque red color:

```cpp
glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
```

The first parameter is red (1.0), the second and third are green and blue (0.0), and the fourth is alpha (1.0), meaning fully opaque.

Now, let’s clear the screen using:

```cpp
glClear(GL_COLOR_BUFFER_BIT);
```

Let’s try running our program now\...

And we see... nothing changed!
Why? Because everything we’re doing — all our rendering — is happening in the **back buffer**.

### A Few Words About Double Buffering

Double buffering is a technique used in graphics APIs to prevent flickering and visual artifacts during rendering.
There are two buffers: the **front buffer** (displayed on the screen) and the **back buffer** (where rendering happens).

All drawing is done in the **back buffer**, and once everything is ready, we **swap** the buffers to show the rendered image.

So far, we’ve done all our drawing in the back buffer but didn’t swap it with the front buffer — so nothing was displayed.

To display the result, we need to **swap** the buffers using:

```cpp
glfwSwapBuffers(window);
```

This function takes the `window` as a parameter.

Now let’s run the program again. Voilà! You can see the window has changed its color.

You can play around and try different background colors. For example, you could set the green channel to 1.0:

```cpp
glClearColor(1.0f, 1.0f, 0.0f, 1.0f); // Red + Green = Yellow
```

Run it again — and you’ll see the background color change.

And this was your **first OpenGL rendering command**!

