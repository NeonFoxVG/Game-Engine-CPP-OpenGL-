Alright—let’s finally render our text. We’ll go into `TextComponent::Render` and finish the implementation step by step, staying super practical and close to how we’ve drawn sprites.

---

## TextComponent::Render — per-glyph quads

First, grab the **font atlas** dimensions:

```cpp
int width  = m_font->GetTexture()->GetWidth();
int height = m_font->GetTexture()->GetHeight();
```

Next, compute where to start drawing, taking the pivot into account:

```cpp
auto pos = GetPivotPos(); // our helper from the previous lesson
float xOrigin = pos.x;
float yOrigin = pos.y;
```

Now iterate over each glyph in the string and draw it as a textured rectangle (just like a sprite):

```cpp
for (const auto c : m_text) 
{
    const auto& desc = m_font->GetGlyphDescription(c);

    float x1 = static_cast<float>(xOrigin);
    float y1 = static_cast<float>(yOrigin);
    float x2 = static_cast<float>(xOrigin + desc.width);
    float y2 = static_cast<float>(yOrigin + desc.height);

    float u1 = static_cast<float>(desc.x0) / static_cast<float>(width);
    float v1 = static_cast<float>(desc.y0) / static_cast<float>(height);
    float u2 = static_cast<float>(desc.x1) / static_cast<float>(width);
    float v2 = static_cast<float>(desc.y1) / static_cast<float>(height);

    // Advance pen position for the next glyph
    xOrigin += desc.advance;
}
```

Let's go to `CanvasComponent` and declare an empty function (for now):

```cpp
void DrawRect(
    const glm::vec2& lowerLeftPos, const glm::vec2& upperRightPos,
    const glm::vec2& lowerLeftUV, const glm::vec2& upperRightUV,
    Texture* texture, const glm::vec4& color);
```

Go back to `TextComponent::Render()`:

```cpp
    // Draw the glyph via the canvas
    // Note: we swap v1/v2 because our UI Y-origin differs (bottom-left vs top-left)
    canvas->DrawRect(
        glm::vec2(x1, y1), glm::vec2(x2, y2),
        glm::vec2(u1, v2), glm::vec2(u2, v1),
        m_font->GetTexture().get(), m_color);
```

Why are `v1` and `v2` swapped? By convention, many UI systems use **top-left** as the origin, while our world space uses **bottom-left**. Swapping V fixes the orientation when sampling the atlas.

That’s it for `TextComponent` rendering.

---

## Canvas: the UI shader and how we’ll draw

Before implementing the low-level draw, let’s establish the **default UI shader** in `GraphicsAPI`.

Add:

```cpp
std::shared_ptr<ShaderProgram> m_defaultUIShaderProgram;

const std::shared_ptr<ShaderProgram>& GetDefaultUIShaderProgram();
```

### Vertex shader

We only need **position**, **color**, and **UV**, plus a single **projection** matrix (orthographic). No world/view matrices; UI is screen-space:

```glsl
layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 uv;

out vec2 vUV;
out vec4 vColor;

uniform mat4 uProjection;

void main() 
{
    vUV = uv;
    vColor = color;
    gl_Position = uProjection * vec4(position, 0.0, 1.0);
}
```

### Fragment shader

We’ll optionally sample a texture and multiply by vertex color; or render flat color:

```glsl
in vec2 vUV;
in vec4 vColor;

uniform sampler2D uTex;
uniform int uUseTexture;

out vec4 FragColor;

void main() 
{
    vec4 src = (uUseTexture != 0) ? texture(uTex, vUV) * vColor : vColor;
    FragColor = src;
}
```

---

## Strategy: dynamic vertex buffer + batching

For UI we won’t create a separate mesh per element—that’s inefficient for lots of small widgets.
Instead, we **accumulate all UI geometry each frame** into a single large **vertex/index** buffer and upload it once per frame. This is standard practice and usually performs better than managing many tiny meshes.

We’ll also add **batching**: merge geometry that shares the same shader state (notably, same “use texture?” setting and the **same texture**). Elements in the same batch can be drawn in a **single draw call**.

* Example: `Text1` and `Text2` both use the same font texture and color → one batch.
* A button with no texture (flat color) → separate batch.
* Another text with a **different** font texture → another batch.

---

## Command & data structures

In `Common.h`, declare:

```cpp
struct UIBatch 
{
    Texture* texture;
    uint32_t indexCount = 0;
};
```

In `CanvasComponent`:

```cpp
std::vector<UIBatch> m_batches;
std::vector<float>   m_vertices; // interleaved: pos(2) + color(4) + uv(2) = 8 floats/vertex
std::vector<uint32_t> m_indices;
std::shared_ptr<Mesh> m_mesh;
```

### Canvas initialization

In `CanvasComponent::Init()` set up the vertex layout to match the shader:

```cpp
VertexLayout layout;
layout.elements.push_back({VertexElement::PositionIndex, 2, GL_FLOAT, 0});
layout.elements.push_back({VertexElement::ColorIndex, 4, GL_FLOAT, sizeof(float) * 2});
layout.elements.push_back({VertexElement::UVIndex, 2, GL_FLOAT, sizeof(float) * 6});
layout.stride = sizeof(float) * 8;

m_mesh = std::make_shared<Mesh>(layout, m_vertices, m_indices);
```

---

## Canvas::DrawRect

Append 4 vertices + 6 indices and update batching:

```cpp
uint32_t base = static_cast<uint32_t>(m_vertices.size() / 8);

// p1 = lower-left, p2 = upper-right
m_vertices.insert(m_vertices.end(), {
    p2.x, p2.y, color.r, color.g, color.b, color.a, uv2.x, uv2.y, // UR
    p1.x, p2.y, color.r, color.g, color.b, color.a, uv1.x, uv2.y, // UL
    p1.x, p1.y, color.r, color.g, color.b, color.a, uv1.x, uv1.y, // LL
    p2.x, p1.y, color.r, color.g, color.b, color.a, uv2.x, uv1.y  // LR
});

m_indices.insert(m_indices.end(), { base, base+1, base+2, base, base+2, base+3 });

UpdateBatches(texture)
```

```cpp
// batching: start new batch if empty or texture differs; otherwise grow last batch
void CanvasComponent::UpdateBatches(Texture* texture)
{
    if (m_batches.empty() || m_batches.back().texture != texture)
    {
        m_batches.push_back({ texture, 6 });
    }
    else
    {
        m_batches.back().indexCount += 6;
    }
}
```

---

## Begin & Flush

At the **start** of `CanvasComponent::Update()` call:

```cpp
void CanvasComponent::BeginRendering() 
{
    m_vertices.clear();
    m_indices.clear();
    m_batches.clear();
}
```

Go to `GraphicsAPI` and declare a struct:

```cpp
struct Rect
{
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};
```

Also define a member function and Get/Set methods:

```cpp
class GraphicsAPI
{
    //...
   void SetViewport(int x, int y, int width, int height)
   {
       glViewport(x, y, width, height);
       m_viewport.x = x;
       m_viewport.y = y;
       m_viewport.width = width;
       m_viewport.height = height;
   }

   const Rect& GraphicsAPI::GetViewport() const
   {
       return m_viewport;
   }

private:
    Rect m_viewport;
    //...
};
```

Then in `Engine::Init()` call `m_graphicsAPI.SetViewport(0, 0, width, height);`

Also add a callback for listening to screen size changes:

```cpp
void windowSizeCallback(GLFWwindow* window, int width, int height)
{
    eng::Engine::GetInstance().GetGraphicsAPI().SetViewport(0, 0, width, height);
}

glfwSetWindowSizeCallback(m_window, windowSizeCallback);
```

After the recursive UI traversal (which calls `DrawRect` many times), call:

```cpp
void CanvasComponent::Flush()
{
    m_mesh->UpdateDynamic(m_vertices, m_indices);
    auto& gfx = Engine::GetInstance()->GetGraphicsAPI();
    const auto& viewport = gfx.GetViewport();
    RenderCommandUI cmd;
    cmd.mesh = m_mesh.get();
    cmd.shaderProgram = gfx.GetDefaultUIShaderProgram().get();
    cmd.batches = m_batches;
    cmd.screenWidth = viewport.width;
    cmd.screenHeight = viewport.height;
    Engine::GetInstance()->GetRenderQueue().submit(cmd);
}
```

---

## Mesh updates (dynamic)

Add to `Mesh`:

```cpp
void Mesh::UpdateDynamic(const std::vector<float>& vertices)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_vertexCout = (vertices.size() * sizeof(float)) / m_vertexLayout.stride;
}

void Mesh::UpdateDynamic(const std::vector<float>& vertices, const std::vector<uint32_t>& indices)
{
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    m_vertexCout = (vertices.size() * sizeof(float)) / m_vertexLayout.stride;

    if (m_EBO == 0)
    {
        Engine::GetInstance().GetGraphicsAPI().CreateIndexBuffer(indices);
    }
    else
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint32_t), indices.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    m_indexCount = indices.size();
}
```

And a **partial draw** helper:

```cpp
void Mesh::DrawIndexedRange(uint32_t startIndex, uint32_t indexCount) 
{
    if (indexCount == 0) return;
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT,
                   reinterpret_cast<void*>(static_cast<size_t>(startIndex) * sizeof(uint32_t)));
}
```

---

## RenderQueue — UI pass

Add a command to `RenderQueue`:

```cpp
struct RenderCommandUI 
{
    Mesh* mesh;
    ShaderProgram* shaderProgram;
    size_t screenWidth = 0;
    size_t screenHeight = 0;
    std::vector<UIBatch> batches;
};
```

A container and submit:

```cpp
std::vector<RenderCommandUI> m_commandsUI;

void RenderQueue::submit(const RenderCommandUI& command) 
{
    m_commandsUI.push_back(command);
}
```

In `RenderQueue::Draw()` add the **UI pass** (after 3D passes):

```cpp
graphicsAPI.SetDepthTestEnabled(false);
graphicsAPI.SetBlendMode(BlendMode::Alpha);

for (auto& cmd : m_commandsUI) 
{
    glm::mat4 ortho = glm::ortho(
        0.0f, static_cast<float>(cmd.screenWidth),
        0.0f, static_cast<float>(cmd.screenHeight)
    );
    cmd.shaderProgram->Bind();
    cmd.shaderProgram->SetUniform("uProjection", ortho);

    cmd.mesh->Bind();

    uint32_t indexBase = 0;
    for (auto& batch : cmd.batches) 
    {
        if (batch.texture) 
        {
            cmd.shaderProgram->SetUniform("uUseTexture", 1);
            cmd.shaderProgram->SetTexture("uTex", batch.texture);
        } 
        else 
        {
            cmd.shaderProgram->SetUniform("uUseTexture", 0);
        }

        cmd.mesh->DrawIndexedRange(indexBase, batch.indexCount);
        indexBase += batch.indexCount;
    }

    cmd.mesh->Unbind();
}

graphicsAPI.SetBlendMode(BlendMode::Disabled);
graphicsAPI.SetDepthTestEnabled(true);
m_commandsUI.clear();
```

--- 

Then got to `ShaderProgram` and add another `SetUniform` method accepting itegers:

```cpp
void ShaderProgram::SetUniform(const std::string& name, int value)
{
    auto location = GetUniformLocation(name);
    glUniform1i(location, value);
}
```

---

## Quick test

In `Game.cpp`:

```cpp
// Create a canvas root
auto canvas = m_scene->CreateObject("Canvas");
auto canvasComp = new eng::CanvasComponent();
canvas->AddComponent(canvasComp);

// Create a text object
auto text = m_scene->CreateObject("Text", canvas); // parent under canvas if you like
text->SetPosition2D(glm::vec2(300.0f, 300.0f));

auto textComp = new eng::TextComponent();
text->AddComponent(textComp);

textComp->SetText("Some Text");
textComp->SetFont("fonts/arial.ttf", 24);
textComp->SetColor(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
```

Run it.

You should see **“Some Text”** on the screen — clear evidence that **FreeType** is working: it loads the font at the requested size, rasterizes glyphs into a texture atlas, we fetch correct metrics, place quads with the right UVs, and render the result in our UI pipeline.

**Congrats!** This is a serious step forward for the engine. We’ve now added **text rendering** and a robust **UI batching** path on top of our 2D/3D foundation.
