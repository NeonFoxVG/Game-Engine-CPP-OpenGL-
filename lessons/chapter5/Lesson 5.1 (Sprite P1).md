### Lesson: Introducing 2D with Sprites

So, we now know how to fully work in 3D space. And, as you remember, we actually arrived at 3D from 2D. But there’s one small note here: that “2D space” we worked with earlier was just a **stepping stone** — a training stage that prepared us for the full-fledged 3D system we now have.

So yes, we can now build 3D worlds and 3D games. But here’s the question:
**What if we want to make a true 2D game, in actual 2D space?**

Well, the answer is surprisingly simple: we already have nearly everything we need. The only thing missing is a single foundational element that defines 2D rendering. That element is the **sprite**.

---

### What is a Sprite?

When I explain it, you’ll instantly recognize it. A **sprite** is just a **plane** — a rectangle with a texture applied on top. Sounds familiar, right? We’ve already drawn something very similar before.

But sprites do have a few special characteristics:

1. **Origin Point (Coordinate Start)**

   * The coordinate system for a sprite begins in the **bottom-left corner**, not the center.

2. **Size**

   * A sprite has width (X) and height (Y).

3. **UV Coordinates**

   * Sprites may use part of a texture (like a region in a sprite atlas) rather than the whole thing. So, two UV coordinates (lower-left and upper-right) define which section of the texture to use.

4. **Pivot Point**

   * This defines the point around which the sprite rotates.
   * By default, the pivot is in the center (0.5, 0.5).
   * Pivot values range from 0 to 1 relative to the sprite’s size.

5. **2D Transformations**

   * Unlike 3D objects, sprites only move along **X and Y**, and they rotate around the **Z axis** only.
   * This simplifies transforms dramatically: translation, scaling, and rotation are easier to manage in 2D.

---

### Implementation: Sprite Component

As you’ve guessed, we’ll implement sprites as a **new component**.

* Go into `scene/components` and create two files:

  * `SpriteComponent.h`
  * `SpriteComponent.cpp`

Declare the class:

```cpp
class SpriteComponent : public Component 
{
    ...
};
```

Don’t forget to add it to `CMakeLists` and to `eng.h`.

---

### Fields of SpriteComponent

A sprite stores:

* `std::shared_ptr<Texture> m_texture;`
  The texture applied to the sprite.

* `glm::vec4 m_color = glm::vec4(1.0f);`
  Default color is white (no tint).

* `glm::vec2 m_size = glm::vec2(100.0f);`
  Default size is 100x100.

* `glm::vec2 m_lowerLeftUV = glm::vec2(0.0f);`
  UV of bottom-left corner.

* `glm::vec2 m_upperRightUV = glm::vec2(1.0f);`
  UV of top-right corner.

* `glm::vec2 m_pivot = glm::vec2(0.5f);`
  Pivot point (center by default).

* `bool m_visible = true;`
  Controls sprite visibility.

Of course, we’ll also add **getters and setters** for all these fields:

```cpp
void SpriteComponent::SetTexture(const std::shared_ptr<Texture>& texture)
{
    m_texture = texture;
}

const std::shared_ptr<Texture>& SpriteComponent::GetTexture() const
{
    return m_texture;
}

void SpriteComponent::SetColor(const glm::vec4& color)
{
    m_color = color;
}

const glm::vec4& SpriteComponent::GetColor() const
{
    return m_color;
}

void SpriteComponent::SetSize(const glm::vec2& size)
{
    m_size = size;
}

const glm::vec2& SpriteComponent::GetSize() const
{
    return m_size;
}

void SpriteComponent::SetLowerLeftUV(const glm::vec2& uv)
{
    m_lowerLeftUV = uv;
}

const glm::vec2& SpriteComponent::GetLowerLeftUV() const
{
    return m_lowerLeftUV;
}

void SpriteComponent::SetUpperRightUV(const glm::vec2& uv)
{
    m_upperRightUV = uv;
}

const glm::vec2& SpriteComponent::GetUpperRightUV() const
{
    return m_upperRightUV;
}

void SpriteComponent::SetUV(const glm::vec2& lowerLeftUV, const glm::vec2& upperRightUV)
{
    m_lowerLeftUV = lowerLeftUV;
    m_upperRightUV = upperRightUV;
}

void SpriteComponent::SetPivot(const glm::vec2& pivot)
{
    m_pivot = pivot;
}

const glm::vec2& SpriteComponent::GetPivot() const
{
    return m_pivot;
}

void SpriteComponent::SetVisibile(bool visible)
{
    m_visible = visible;
}

bool SpriteComponent::IsVisible() const
{
    return m_visible;
}
```

---

### Loading from JSON

Override `LoadProperties` to load:

* Texture
* Color
* Size
* UVs
* Pivot

```cpp
void SpriteComponent::LoadProperties(const nlohmann::json& jsonObject)
{
    // Texture
    const std::string texturePath = jsonObject.value("texture", "");
    if (auto texture = Texture::Load(texturePath))
    {
        SetTexture(texture);
    }
    // color
    if (jsonObject.contains("color"))
    {
        auto& colorObj = jsonObject["color"];
        glm::vec4 color;
        color.r = colorObj.value("r", 1.0f);
        color.g = colorObj.value("g", 1.0f);
        color.b = colorObj.value("b", 1.0f);
        color.a = colorObj.value("a", 1.0f);
        SetColor(color);
    }
    // size
    if (jsonObject.contains("size"))
    {
        auto& sizeObj = jsonObject["size"];
        glm::vec2 size;
        size.x = sizeObj.value("x", 100.0f);
        size.y = sizeObj.value("y", 100.0f);
        SetSize(size);
    }
    // lowerLeftUV
    if (jsonObject.contains("lowerLeftUV"))
    {
        auto& uvObj = jsonObject["lowerLeftUV"];
        glm::vec2 uv;
        uv.x = uvObj.value("u", 0.0f);
        uv.y = uvObj.value("v", 0.0f);
        SetLowerLeftUV(uv);
    }
    // upperRightUV
    if (jsonObject.contains("upperRightUV"))
    {
        auto& uvObj = jsonObject["upperRightUV"];
        glm::vec2 uv;
        uv.x = uvObj.value("u", 1.0f);
        uv.y = uvObj.value("v", 1.0f);
        SetUpperRightUV(uv);
    }
    // pivot
    if (jsonObject.contains("pivot"))
    {
        auto& pivotObj = jsonObject["pivot"];
        glm::vec2 pivot;
        pivot.x = pivotObj.value("x", 0.5f);
        pivot.y = pivotObj.value("y", 0.5f);
        SetPivot(pivot);
    }
}
```

Also override `Update`:

```cpp
void SpriteComponent::Update(float deltaTime) override 
{
    if (!m_texture || !m_visible) return;
    // For now, we leave it empty
}
```

---

### Registering the Component

In `Scene::RegisterTypes`, register it:

```cpp
SpriteComponent::Register();
```

---

### Convenience: 2D Transforms in GameObject

Working in 2D should also be convenient. So, let’s add some helper methods in `GameObject`.

For positions:

```cpp
glm::vec2 GetPosition2D()
{
    return glm::vec2(m_position);
}

void SetPosition2D(const glm::vec2& pos)
{
    m_position = glm::vec3(pos, 0.0f);
}

```

For rotation (only around Z):

```cpp
float GetRotation2D()
{
    return glm::angle(m_rotation);
}

void SetRotation2D(float rotation)
{
    m_rotation = glm::angleAxis(rotation, glm::vec3(0.0f, 0.0f, 1.0f));
}
```

For scale:

```cpp
glm::vec2 GetScale2D()
{
    return glm::vec2(m_scale);
}

void SetScale2D(const glm::vec2& scale)
{
    m_scale = glm::vec3(scale, 1.0f);
}
```

And transforms:

```cpp
glm::mat4 GetLocalTransform2D() const
{
    glm::mat4 mat = glm::mat4(1.0f);

    const auto rotationZ = GetRotation2D();
    float c = cos(rotationZ);
    float s = sin(rotationZ);

    mat[0][0] = m_scale.x * c;
    mat[0][1] = m_scale.x * s;
    mat[1][0] = -m_scale.y * s;
    mat[1][1] = m_scale.y * c;
    mat[3][0] = m_position.x;
    mat[3][1] = m_position.y;

    return mat;
}

glm::mat4 GameObject::GetWorldTransform2D() const
{
    if (m_parent)
    {
        return m_parent->GetWorldTransform2D() * GetLocalTransform2D();
    }
    else
    {
        return GetLocalTransform2D();
    }
}
```

And finally:

```cpp
glm::vec2 GetWorldPosition2D() const
{
    glm::vec4 hom = GetWorldTransform2D() * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    return glm::vec2(hom) / hom.w;
}
```

Inside these, we simply adapt the logic of the 3D equivalents but **ignore Z**.

---

### Rendering Sprites

Now we need to actually render the sprite. Just like `MeshComponent` handles 3D meshes, `SpriteComponent` will handle 2D sprites.

In `RenderQueue.h`, define a new structure:

```cpp
struct RenderCommand2D 
{
    glm::mat4 modelMatrix;
    Texture* texture = nullptr;
    glm::vec4 color;
    glm::vec2 size;
    glm::vec2 lowerLeftUV;
    glm::vec2 upperRightUV;
    glm::vec2 pivot;
};
```

In `RenderQueue`, add:

* A new vector `std::vector<RenderCommand2D> m_commands2D;`
* A new `Submit(const RenderCommand2D& command)` method that pushes commands into it.

And in `DrawAll()`, after rendering the 3D world, **iterate over `m_commands2D`** and draw all sprites:

```cpp
for (auto& command : m_comands2D)
{

}
```

---

### Geometry for Sprites

What geometry does a sprite need? Just a **plane**.

So, in `Mesh`, add:

```cpp
static std::shared_ptr<Mesh> CreatePlane();
```

This creates a simple rectangle with four vertices:

```cpp
std::shared_ptr<Mesh> Mesh::CreatePlane()
{
    std::vector<float> vertices =
    {
        1.0f, 1.0f,
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f
    };

    std::vector<uint32_t> indices =
    {
        0, 1, 2,
        0, 2, 3
    };

    eng::VertexLayout vertexLayout;

    // Postion
    vertexLayout.elements.push_back({
        VertexElement::PositionIndex,
        2,
        GL_FLOAT,
        0
        });
    vertexLayout.stride = sizeof(float) * 2;

    auto result = std::make_shared<eng::Mesh>(vertexLayout, vertices, indices);

    return result;
}
```

In `RenderQueue`, add:

```cpp
std::shared_ptr<Mesh> m_mesh2D;
```

Initialize it in a new method `RenderQueue::Init()`:

```cpp
m_mesh2D = Mesh::CreatePlane();
```

And in `Engine::Init()`, call:

```cpp
m_renderQueue.Init();
```

When drawing 2D commands:

* Bind `m_mesh2D`
* Draw it for each `RenderCommand2D`
* Unbind it afterwards

```cpp
m_mesh2D->Bind();
for (auto& command : m_comands2D)
{
    //...
    m_mesh2D->Draw();
}
m_mesh2D->Unbind();
```

---

### Shader for Sprites

Finally, we need a **2D shader**.

In `GraphicsAPI`, add:

```cpp
std::shared_ptr<ShaderProgram> m_default2DShaderProgram;
const std::shared_ptr<ShaderProgram>& GetDefault2DShaderProgram() const;
```

This shader will take a texture and a color and render the sprite accordingly.

---

And that’s it for **part one** of sprites.
In the next part, we’ll continue building out the 2D rendering system.