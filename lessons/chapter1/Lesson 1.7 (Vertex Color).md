Great! Now, in addition to the position attributes of each vertex, we can also add color attributes. Let's start by modifying the shader code.

#### Vertex Shader

Open the vertex shader and add a new input attribute:

```glsl
layout(location = 1) in vec3 color;
```

This line tells us that at location index 1, we will receive a `vec3` type color attribute.

Then declare an output variable:

```glsl
out vec3 vColor;
```

This means we will pass a `vec3` variable named `vColor` from the vertex shader to the fragment shader.

Inside the `main()` function, add:

```glsl
vColor = color;
```

#### Fragment Shader

Now go to the fragment shader and declare the input variable that comes from the vertex shader:

```glsl
in vec3 vColor;
```

Then change the line where the final color is set:

```glsl
FragColor = vec4(vColor, 1.0);
```

#### Adding Color Data to Vertex Array

Next, we need to add the color attributes to our vertex array. Open the vertex array and, for each vertex, add RGB color values.

Example:

```cpp
// Position         // Color
{  0.0f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f }, // Red
{ -0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f }, // Green
{  0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f }  // Blue
```

Now, configure the vertex array object (VAO) to understand how to read the new attribute. Add this after the position attribute setup:

```cpp
glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
glEnableVertexAttribArray(1);
```

Note:

* The stride is now `6 * sizeof(float)` because we have 3 position and 3 color values per vertex.
* The offset is `3 * sizeof(float)` because the color comes after the position.

Also, make sure to update the stride value for the position attribute to match this new layout.

#### Gradient via Interpolation

Now you can run the program and see a triangle colored with a gradient. This gradient appears because OpenGL interpolates the colors between the vertices across the surface of the triangle.
When we pass data (like color) from the **vertex shader** to the **fragment shader**, OpenGL **interpolates** it automatically.

That means:

* If one vertex is red, and another is green,
* OpenGL smoothly blends colors for the pixels in between.
* This creates **gradients** across surfaces, like triangles or rectangles.

This is a powerful feature of the graphics pipeline — and we get it for free!

#### Drawing a Rectangle with Indices

Let’s draw something a bit more complex — a rectangle. For the rectangle, define four vertices.
Previously, we had a `std::vector<float> vertices` that stored only the triangle's vertex positions and colors. We'll **reuse this same vector**, but modify it to represent **a rectangle with four vertices**:

```cpp
std::vector<float> vertices = {
    // Positions       // Colors (R, G, B)
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f, // Top Right (Red)
    -0.5f,  0.5f, 0.0f,   0.0f, 1.0f, 0.0f, // Top Left (Green)
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f, // Bottom Left (Blue)
     0.5f, -0.5f, 0.0f,   1.0f, 1.0f, 0.0f  // Bottom Right (Yellow)
};
```

We now have 4 vertices, each with 3 position values and 3 color values.


### Why Use Indices?

OpenGL renders everything as triangles. To render a rectangle, we must define it as **two triangles**. If we naively repeated shared vertices, we would duplicate data, which is inefficient — especially for complex models.

**Indices** solve this problem by allowing us to **reuse** vertex data. Instead of sending the same vertex multiple times, we reference it via its index in the array.

Example index list to form two triangles:

```cpp
std::vector<unsigned int> indices = {
    0, 1, 2,  // First triangle (Top Right - Top Left - Bottom Left)
    0, 2, 3   // Second triangle (Top Right - Bottom Left - Bottom Right)
};
```

This reduces memory usage and allows better GPU optimization (like vertex cache re-use).

OpenGL requires that the vertices in triangles be ordered counterclockwise.

---

### What is an EBO?

An **Element Buffer Object (EBO)** — also known as an **Index Buffer Object** — is similar to a **Vertex Buffer Object (VBO)**, but instead of storing vertex attributes like positions or colors, it stores **indices** that tell OpenGL **which vertices to draw and in what order**.

The EBO is bound using a different buffer type:

```cpp
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
```

This differs from VBOs, which are bound with:

```cpp
glBindBuffer(GL_ARRAY_BUFFER, vbo);
```

The `GL_ELEMENT_ARRAY_BUFFER` binding tells OpenGL: *"These are the indices I want to use when drawing elements."*

---

### Setting Up the EBO

```cpp
GLuint ebo;
glGenBuffers(1, &ebo);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
```

### Activating the EBO

We also need to activate it withing the active VAO. use `glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);`
Now the GPU knows how to reuse the vertices from the `vertices` array to draw our rectangle efficiently.

#### Rendering with Indices

Now we switch from `glDrawArrays()` to `glDrawElements()`:

```cpp
glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
```

Parameters:

* `GL_TRIANGLES`: primitive type.
* `6`: number of indices (2 triangles × 3 vertices each).
* `GL_UNSIGNED_INT`: data type of indices.
* `0`: starting offset in the EBO.

#### Uniforms for Color Manipulation

A `uniform` is a **global GPU variable** set from the CPU, shared across all processed vertices/fragments in a draw call.
Let's **tint** the whole shape.

In the fragment shader, declare a uniform:

```glsl
uniform vec4 uColor;
```

Then multiply the interpolated color by the uniform:

```glsl
FragColor = vec4(vColor, 1.0) * uColor;
```

On the CPU side, to set the uniform:

1. Get its location:

```cpp
GLint uColorLoc = glGetUniformLocation(shaderProgram, "uColor");
```

2. Use the shader program and set the value:

```cpp
glUseProgram(shaderProgram);
glUniform4f(uColorLoc, 0.0f, 1.0f, 0.0f, 1.0f); // Example: Green
```

Run the program and observe the final result — a rectangle drawn with interpolated vertex colors, influenced by a uniform color multiplier.
