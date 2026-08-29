Now that we have a working **Material** system, let’s move on to creating the actual **renderable entity** — in other words, the **geometry** or the **3D object** that will be drawn on screen.

This brings us to the concept of a **Mesh**.

---

### What is a Mesh?

A *Mesh* is a container for 3D geometry data. In our case, it will hold a **collection of vertices and indices** that define a 3D object in space.

Let’s implement the `Mesh` class step by step.

---

### Step 1: Setup

Inside the `render` folder, create two files:

* `Mesh.h`
* `Mesh.cpp`

Add them to your `CMakeLists` for the engine and include `Mesh.h` in `eng.h`.

---

### Step 2: Define the Mesh Class

In `Mesh.h`, start by declaring the class `Mesh`.

Since it stores geometry, we’ll need:

```cpp
GLuint m_VBO = 0;  // Vertex Buffer Object
GLuint m_EBO = 0;  // Element Buffer Object (Index Buffer)
GLuint m_VAO = 0;  // Vertex Array Object
```

All initialized to `0` to indicate they are uninitialized.

---

### Step 3: Define Vertex Layout

To communicate vertex structure to the GPU, we need to define a **VertexLayout** — a description of the format of our vertex data.

Create a new file `VertexLayout.h` in the `graphics` folder.

#### Define `VertexElement`:

```cpp
struct VertexElement 
{
    GLuint index;      // Attribute location in shader (e.g., layout(location = 0))
    GLuint size;       // Number of components (e.g., 3 for position, 4 for color)
    GLuint type;       // Data type (e.g., GL_FLOAT)
    uint32_t offset;   // Byte offset from start of vertex
};
```

#### Define `VertexLayout`:

```cpp
struct VertexLayout 
{
    std::vector<VertexElement> elements;
    uint32_t stride = 0; // Total size of a single vertex in bytes
};
```

This layout tells OpenGL how to interpret raw vertex data.

---

### Step 4: Add Layout and Metadata to Mesh

In `Mesh.h`, include:

```cpp
VertexLayout m_VertexLayout;
size_t m_vertexCount = 0;
size_t m_indexCount = 0;
```

These track the geometry currently stored in GPU buffers.

---

### Step 5: Implement `Bind()` and `Draw()` Methods

* `Bind()` activates the VAO:

```cpp
void Bind() 
{
    glBindVertexArray(m_VAO);
}
```

* `Draw()` renders the mesh:

```cpp
void Draw() 
{
    if (m_indexCount > 0) 
    {
        glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    } 
    else 
    {
        glDrawArrays(GL_TRIANGLES, 0, m_vertexCount);
    }
}
```

---

### Step 6: Constructors and Restrictions

* **Disable copy and assignment** to avoid accidental resource duplication:

```cpp
Mesh(const Mesh&) = delete;
Mesh& operator=(const Mesh&) = delete;
```

* Add two constructors:

  1. Mesh **with indices**
  2. Mesh **without indices**

Signature for indexed version:

```cpp
Mesh(const VertexLayout& layout,
     const std::vector<float>& vertices,
     const std::vector<uint32_t>& indices);
```

And for non-indexed:

```cpp
Mesh(const VertexLayout& layout,
     const std::vector<float>& vertices);
```

---

### Step 7: Implement Constructor Logic

In the constructor:

1. Store the layout:

```cpp
m_vertexLayout = layout;
```

2. Get `GraphicsAPI` instance:

```cpp
auto& graphicsAPI = Engine::GetInstance().GetGraphicsAPI();
```

3. Create vertex and index buffers:

#### Add to `GraphicsAPI`:

* `GLuint CreateVertexBuffer(const std::vector<float>& vertices);`
* `GLuint CreateIndexBuffer(const std::vector<uint32_t>& indices);`

Implement these methods using standard OpenGL buffer creation logic.

4. Back in `Mesh`, initialize:

```cpp
m_VBO = graphicsAPI.CreateVertexBuffer(vertices);
m_EBO = graphicsAPI.CreateIndexBuffer(indices); // Only if using indices
```

5. Generate and bind the VAO:

```cpp
glGenVertexArrays(1, &m_VAO);
glBindVertexArray(m_VAO);
```

6. Bind VBO:

```cpp
glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
```

7. Set up vertex attributes using `VertexLayout`:

```cpp
for (const auto& element : m_vertexLayout.elements) 
{
    glVertexAttribPointer(element.index, element.size, element.type, GL_FALSE, m_vertexLayout.stride, (void*)(uintptr_t)element.offset);
    glEnableVertexAttribArray(element.index);
}
```

8. Bind EBO:

```cpp
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
```

9. Unbind everything:

```cpp
glBindVertexArray(0);
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
```

---

### Step 8: Count Vertices and Indices

* Vertex count is calculated as:

```cpp
m_vertexCount = (vertices.size() * sizeof(float)) / m_vertexLayout.stride;
```

* Index count:

```cpp
m_indexCount = indices.size();
```

---

### Step 9: Handle Non-Indexed Mesh

In the second constructor (without indices), skip index buffer creation. Only initialize VBO and VAO, and skip the call to `glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ...)`.