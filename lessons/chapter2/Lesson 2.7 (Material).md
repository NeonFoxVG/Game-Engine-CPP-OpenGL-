Now that we have a unified and reusable shader program, it's time to introduce an important concept — the **material**.

This concept is widely used in modern rendering systems, not only in game development but also in other fields where graphics are involved — such as CAD systems, simulations, or any software that renders 2D or 3D scenes.

As the name suggests, a *material* represents the surface characteristics of an object — just like in the physical world. Think of objects made from wood, metal, plastic, or glass. Each of these has distinct visual properties: some are shiny, some are rough, some are highly transparent, and so on.

In 3D graphics, the *Material* object serves the same purpose — it describes the unique visual properties of a rendered object. In other words, it tells the rendering system how an object should look.

Put simply, a **material** is a collection of settings that informs the graphics API how to draw a particular object. And, as you may have already guessed, the core component of a material is the **shader program** — the one we just implemented.

The shader program defines how the object is shaded, how lighting is applied, and how pixel colors are generated from geometry. Since we're working with 3D rendering, all visual output eventually boils down to how polygons are processed and how pixels are colored.

Alongside the shader program, a material can also store a set of **uniform parameters**. These parameters are passed to the shader program to define properties like color, glossiness, transparency, and more.

So, to summarize:

> A **Material** is essentially a shader program along with a set of uniquely defined parameters that determine how an object is rendered.

---

### Let’s move from theory to practice.

To better understand how this works, let’s implement the `Material` class.

1. Inside the `source/engine` directory, create a new folder called `render`.
2. Inside `render`, create two files: `Material.h` and `Material.cpp`.
3. Add both files to `CMakeLists` of the engine and include `Material.h` in `eng.h`.

Now, let’s define the `Material` class in `Material.h`.

Since the material must store a shader program, we start with:

```cpp
std::shared_ptr<ShaderProgram> m_ShaderProgram;
```

Then, since each material may include specific parameters, we’ll create a container for them. For now, let’s start with simple `float` parameters:

```cpp
std::unordered_map<std::string, float> m_floatParams;
```

In the future, this could be extended to support vectors, colors, textures, etc.

---

### Implementing Material Methods

Now let’s add a few methods:

#### 1. `SetShaderProgram()`

```cpp
void SetShaderProgram(const std::shared_ptr<ShaderProgram>& shaderProgram) 
{
    m_shaderProgram = shaderProgram;
}
```

#### 2. `SetParam()`

This sets a float uniform value:

```cpp
void SetParam(const std::string& name, float value) 
{
    m_floatParams[name] = value;
}
```

#### 3. `Bind()`

This method binds the shader and sets all the uniforms:

```cpp
void Bind() 
{
    if (!m_shaderProgram) 
    {
        return;
    }

    m_shaderProgram->Bind();

    for (const auto& param : m_floatParams) 
    {
        m_ShaderProgram->SetUniform(param.first, param.second);
    }
}
```

---

### Integrating with `GraphicsAPI`

Next, go to `GraphicsAPI.h` and add a method:

```cpp
void BindMaterial(Material* material);
```

And in its implementation:

```cpp
void GraphicsAPI::BindMaterial(Material* material) 
{
    if (material) 
    {
        material->Bind();
    }
}
```

---

### Using Material in the Game

Now let’s test it.

1. Open `Game.h` and add a new field:

```cpp
eng::Material m_material;
```

2. In `Game::Init()` (in `Game.cpp`), after creating the shader program, initialize the material:

```cpp
m_material.SetShaderProgram(shaderProgram);
```

Now your material is ready to be used in rendering. You can later expand it by adding texture support, multiple parameter types (like `vec3`, `mat4`, etc.).
