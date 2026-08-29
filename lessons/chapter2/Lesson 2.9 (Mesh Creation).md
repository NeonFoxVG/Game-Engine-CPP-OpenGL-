Now let’s try creating an instance of our newly implemented **Mesh** within the game.

---

### Step 1: Declare the Mesh Pointer

Open `Game.h` and declare a pointer to the `Mesh`:

```cpp
std::unique_ptr<Mesh> m_mesh;
```

---

### Step 2: Initialize Geometry in `Game::Init()`

Go to `Game.cpp`, inside the `Init()` method. Start by declaring the vertex and index arrays for a simple **rectangle**.

We’ll reuse data from earlier examples: a rectangle composed of two triangles, with vertex attributes for **position** and **color**.

#### Example:

```cpp
std::vector<float> vertices = 
{
    // positions       // colors
    0.5f, 0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // top-right, red
    -0.5f, 0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // top-left, green
    -0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  // bottom-left, blue
    0.5f,  -0.5f, 0.0f,  1.0f, 1.0f, 0.0f   // bottom-right, yellow
};

std::vector<uint32_t> indices = 
{
    0, 1, 2,  // first triangle
    0, 2, 3   // second triangle
};
```

---

### Step 3: Define Vertex Layout

Create a `VertexLayout` object that describes how the vertex data is structured:

```cpp
VertexLayout vertexLayout;

// Position attribute (location = 0)
vertexLayout.elements.push_back({ 
    0,                      // index in shader
    3,                      // size (x, y, z)
    GL_FLOAT,               // type
    0                       // offset
});

// Color attribute (location = 1)
vertexLayout.elements.push_back({ 
    1,                      // index in shader
    3,                      // size (r, g, b)
    GL_FLOAT,               // type
    sizeof(float) * 3       // offset after position
});

vertexLayout.stride = sizeof(float) * 6; // 3 for position + 3 for color
```

---

### Step 4: Create Mesh Instance

Now that we have our vertex data, index data, and layout defined, we can create a mesh:

```cpp
m_mesh = std::make_unique<Mesh>(vertexLayout, vertices, indices);
```

At this point, the mesh will be fully initialized with VBO, EBO, and VAO. You can test your code by running the program — the mesh should be created without errors and ready to render.

---

### Step 5: Add `BindMesh()` and `DrawMesh()` to `GraphicsAPI`

To unify mesh operations, let’s add two helper methods to `GraphicsAPI`:

#### In `GraphicsAPI.h`:

```cpp
void BindMesh(Mesh* mesh);
void DrawMesh(Mesh* mesh);
```

#### In `GraphicsAPI.cpp`:

```cpp
void GraphicsAPI::BindMesh(Mesh* mesh) 
{
    if (mesh) 
    {
        mesh->Bind();
    }
}

void GraphicsAPI::drawMesh(Mesh* mesh) 
{
    if (mesh) 
    {
        mesh->Draw();
    }
}
```

These methods follow the same pattern as `BindShaderProgram()` and `BindMaterial()`, offering a clean and consistent interface for rendering.

---

### Summary

You now have a fully working render pipeline:

* A **ShaderProgram** tells OpenGL how to shade pixels.
* A **Material** stores the shader and any uniform values.
* A **Mesh** holds the actual geometry to draw.
* The **GraphicsAPI** manages unified access to these components.