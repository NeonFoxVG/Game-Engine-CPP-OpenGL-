Great! Now we need to upload our triangle data—specifically, the vertex data—into the GPU memory. In OpenGL, this is done using buffers.

To create a buffer, we call the function `glGenBuffers` and pass in a reference to a variable, typically named `vbo`. Once the buffer is generated, we need to make it active by calling `glBindBuffer` with the `GL_ARRAY_BUFFER` target. The `GL_ARRAY_BUFFER` parameter indicates that this buffer will store an array of vertex data.

Now that the buffer is active, we can transfer our vertex data from the system memory to the GPU memory. This is done using the function `glBufferData`. It takes the following parameters:

1. The target buffer type — in our case, `GL_ARRAY_BUFFER`.
2. The size of the data we want to transfer — typically `verticesSize * sizeof(float)`.
3. A pointer to the beginning of our data.
4. The usage pattern — in our case, `GL_STATIC_DRAW`. This indicates that the data will be uploaded once and will not change, so it will stay in the GPU memory permanently.

Excellent. The data has now been uploaded. We can unbind the buffer by calling `glBindBuffer` again with the same target, `GL_ARRAY_BUFFER`, and passing `0` as the buffer ID.

Now the vertex data resides in GPU memory.

Next, we need to tell our graphics pipeline, or more precisely, our shader program, how to interpret this data when it reaches the vertex shader.

OpenGL provides a concept called **Vertex Array Objects (VAOs)**. A VAO stores the configuration of how vertex attributes are laid out in the buffer and how they map to shader inputs.

Let’s create a VAO. This is done using the function `glGenVertexArrays`. Once created, we need to activate it using `glBindVertexArray`.

Now the VAO is active.

The next step is to bind the buffer that we previously uploaded data into. We do this by calling `glBindBuffer(GL_ARRAY_BUFFER, vbo)`. This tells OpenGL that within the currently bound VAO, we want to associate this buffer as the source of vertex data.

Next, we specify how exactly the data in the buffer should be interpreted. This is done with the function `glVertexAttribPointer`.

Here’s how the parameters work:

1. The **attribute index** expected by the vertex shader. If you recall the shader code, we defined the position at location 0, so we use index 0.
2. The **number of components** per attribute. For position, we use `x`, `y`, and `z`, so that’s 3.
3. The **data type** — in our case, `GL_FLOAT`.
4. Whether the data should be **normalized** — we set this to `GL_FALSE`, as our data is already in the desired format.
5. The **stride** — this is the total size in bytes of a single vertex's attributes. Since each vertex only contains a position (3 floats), the stride is `3 * sizeof(float)`.
6. The **offset** — this tells OpenGL where to start reading the data. Since we’re only passing position data and it starts at the beginning, we pass `(void*)0`.

After specifying how to read the data, we enable the attribute array using `glEnableVertexAttribArray`, passing in the same index (0).

Great. Now the shader knows how to access the vertex data.

We can now unbind the buffer with `glBindBuffer(GL_ARRAY_BUFFER, 0)` and also unbind the VAO by calling `glBindVertexArray(0)`.

Now, our vertex data is fully configured and ready to be drawn.

Let’s move on to the rendering loop. Inside the `while` loop, right after the call to `glClear`, we do the following:

1. Activate our shader program using `glUseProgram(shaderProgram)`.
2. Bind the VAO containing our vertex data layout using `glBindVertexArray`.

Now everything is set up to draw.

To render the triangle, we call `glDrawArrays`. The parameters are:

1. The **primitive type** — `GL_TRIANGLES`, because we are drawing a triangle.
2. The **starting vertex** in the buffer — we start from 0.
3. The **number of vertices** — since we are drawing one triangle, we use 3.

```cpp
glDrawArrays(GL_TRIANGLES, 0, 3);
```

Perfect. Run your application, and if everything was done correctly…

**Congratulations on rendering your first triangle!**
