### Lesson: Building the 2D Shader and Rendering Sprites

Alright, let’s now implement our shader for sprite rendering.

---

### Vertex Shader

The vertex shader will take:

* Position (`layout(location = 0) in vec2 position;`)
* UV coordinates to pass to the fragment shader (`out vec2 vUV;`)
* Three matrices: **Model**, **View**, and **Projection**:
```glsl
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;
```
* Pivot and Size (to calculate sprite geometry):
```glsl
uniform vec2 uPivot;
uniform vec2 uSize;
```
* Lower-left and upper-right UV coordinates:
```glsl
uniform vec2 uUVMin;      // lowerLeftCornerUV
uniform vec2 uUVMax;      // upperRightCornerUV
```

In the `main()` function, here’s what happens:

1. **Convert coordinates**

   ```glsl
   vec2 local = (position - uPivot) * uSize;
   ```

   * We offset by the pivot and scale by the size.

2. **Recalculate UVs**

   ```glsl
   vUV = mix(uUVMin, uUVMax, position);
   ```

   * This interpolates UV coordinates based on vertex position.

3. **Output final position**

   ```glsl
   gl_Position = uProjection * uView * uModel * vec4(local, 0.0, 1.0);
   ```

And that’s the entire **vertex shader**.

---

### Fragment Shader

The fragment shader receives:

* Interpolated UVs (`in vec2 vUV;`)
* Uniform color (`uniform vec4 uColor;`)
* Uniform texture sampler (`uniform sampler2D uTex;`)
* Output color (`out vec4 FragColor;`)

Implementation:

```glsl
vec4 src = texture(uTex, vUV) * uColor;
FragColor = src;
```

That’s it — the fragment shader multiplies the sampled texture color by the uniform tint color.

---

### Hooking into RenderQueue

Now let’s integrate this into our rendering pipeline.

* In `RenderQueue::Draw`, before calling `m_mesh2D->Bind()`, activate the shader:

  ```cpp
  const auto shaderProgram2D = graphicsAPI.GetDefault2DShaderProgram();
  shaderProgram2D->Bind();
  ```

* Set all required uniforms:

  * `uModel`
  ```cpp
  shaderProgram2D->SetUniform("uModel", command.modelMatrix);
  ```
  * `uView`
  ```cpp
  shaderProgram2D->SetUniform("uView", cameraData.viewMatrix);
  ```
  * `uSize`
  ```cpp
  shaderProgram2D->SetUniform("uSize", command.size.x, command.size.y);
  ```
  * `uPivot`
  ```cpp
  shaderProgram2D->SetUniform("uPivot", command.pivot.x, command.pivot.y);
  ```
  * `uUVMin`
  ```cpp
  shaderProgram2D->SetUniform("uUVMin", command.lowerLeftUV.x, command.lowerLeftUV.y);
  ```
  * `uUVMax`
  ```cpp
  shaderProgram2D->SetUniform("uUVMax", command.upperRightUV.x, command.upperRightUV.y);
  ```
  * Go to `ShaderProgram.h` and add:
  ```cpp
  void ShaderProgram::SetUniform(const std::string& name, const glm::vec4& value)
  {
      auto location = GetUniformLocation(name);
      glUniform4fv(location, 1, glm::value_ptr(value));
  }
  ```
  * `uColor`
  ```cpp
  shaderProgram2D->SetUniform("uColor", command.color);
  ```
  * `uTex`
  ```cpp
  shaderProgram2D->SetTexture("uTex", command.texture);
  ```

---

### Orthographic Projection

Unlike 3D rendering, which uses **perspective projection** (to simulate the eye), 2D uses an **orthographic projection**.

To do this:

1. In `CameraData` (from `Common.h`), add a field:

   ```cpp
   glm::mat4 orthoMatrix;
   ```

2. In `RenderQueue`, set this matrix as `uProjection` when drawing 2D:
   ```cpp
   shaderProgram2D->SetUniform("uProjection", cameraData.orthoMatrix);
   ```

3. In `Engine::Run`, when filling `CameraData`:

   ```cpp
   cameraData.orthographicMatrix = glm::ortho(
       0.0f, static_cast<float>(width),
       0.0f, static_cast<float>(height)
   );
   ```

Now 2D rendering works with an orthographic camera.

---

### Depth Test and Transparency

There’s an issue: if we render 2D objects at `z = 0`, they might conflict with 3D objects. To prevent this:

1. Add to `GraphicsAPI`:

   ```cpp
   void SetDepthTestEnabled(bool enabled);
   ```

   * Enables/disables OpenGL depth testing:
   ```cpp
   void GraphicsAPI::SetDepthTestEnabled(bool enabled)
   {
       if (enabled)
       {
           glEnable(GL_DEPTH_TEST);
       }
       else
       {
           glDisable(GL_DEPTH_TEST);
       }
   }
   ```

2. In `RenderQueue::Draw`:

   * Before rendering 2D: `graphicsAPI.SetDepthTestEnabled(false);`
   * After rendering: `graphicsAPI.SetDepthTestEnabled(true);`

This ensures 3D depth doesn’t interfere with 2D.

---

#### Transparency and Blending

Sprites often require transparency. Let’s add blending modes.

In `GraphicsAPI`:

```cpp
enum class BlendMode 
{
    Disabled,
    Alpha,
    Additive,
    Multiply
};

void SetBlendMode(BlendMode mode);
```

Implementation (simplified):

* **Disabled** → `glDisable(GL_BLEND)`
* **Alpha** → `glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);`
* **Additive** → `glEnable(GL_BLEND); glBlendFunc(GL_ONE, GL_ONE);`
* **Multiply** → `glEnable(GL_BLEND); glBlendFunc(GL_DST_COLOR, GL_ZERO);`

And of course, default back to **Disabled** if nothing is set:

```cpp
void GraphicsAPI::SetBlendMode(BlendMode mode)
{
    switch (mode)
    {
    case BlendMode::Disabled:
    {
        glDisable(GL_BLEND);
    }
    break;
    case BlendMode::Alpha:
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    break;
    case BlendMode::Additive:
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
    }
    break;
    case BlendMode::Multiply:
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
    }
    break;
    default:
    {
        glDisable(GL_BLEND);
    }
        break;
    }
}
```

---

In `RenderQueue::Draw`:

   * Before rendering 2D: `graphicsAPI.SetBlendMode(BlendMode::Alpha);`
   * After rendering: `graphicsAPI.SetBlendMode(BlendMode::Disabled);`


---

### Testing the Sprite System

Time to test!

In `Game.cpp`:

1. Comment out old scene loading.
2. Create a new empty scene:

```cpp
m_scene = std::make_shared<Scene>();
eng::Engine::GetInstance().SetScene(m_scene.get());
```

3. Create a sprite:

```cpp
auto sprite = m_scene->CreateObject("sprite");
auto spriteComponent = new eng::SpriteComponent();

auto texture = eng::Texture::Load("textures/brick.png");
spriteComponent->setTexture(texture);

sprite->AddComponent(spriteComponent);
sprite->SetPosition2D(glm::vec2(500, 500));
spriteComponent->SetSize(glm::vec2(200, 100));
spriteComponent->SetUpperRightUV(glm::vec2(2.0f, 1.0f));
sprite->SetRotation2D(glm::radians(45.0f));
```

4. Create a camera:

```cpp
auto camera = m_scene->CreateObject("camera");
auto cameraComponent = new eng::CameraComponent();
camera->AddComponent(cameraComponent);
m_scene->SetMainCamera(camera);
```

---

Before testing go to `SpriteComponent::Update()` and submit a render command:

```cpp
void SpriteComponent::Update(float deltaTime)
{
    if (!m_texture || !m_visible)
    {
        return;
    }

    RenderCommand2D command;
    command.modelMatrix = GetOwner()->GetWorldTransform2D();
    command.texture = m_texture.get();
    command.color = m_color;
    command.size = m_size;
    command.lowerLeftUV = m_lowerLeftUV;
    command.upperRightUV = m_upperRightUV;
    command.pivot = m_pivot;

    auto& renderQueue = Engine::GetInstance().GetRenderQueue();
    renderQueue.Submit(command);
}
```

---

### Result

Run the program. You’ll see:

* A sprite rendered at position `(500, 500)`
* With the **brick texture**
* Proper size `(200x100)`
* Rotated **45°**
* Correct UVs

---

### Conclusion

Congratulations!

We started with 2D as a preparation stage, then built a full **3D engine** with components, scenes, and objects. And now we’ve circled back: we’ve reintroduced **true 2D rendering** with sprites — but this time inside the modern engine infrastructure.

This gives you the freedom to work in both **3D worlds** and **2D games**, seamlessly.

**Bravo!**