Now let’s implement **keyboard input** handling inside our engine.

---

### **Background**

In a previous chapter, we controlled object movement using direct keyboard input — shifting the object horizontally and vertically. Now we’ll do the same, but using our engine’s input and material APIs.

---

### Step 1: Update ShaderProgram – Add `SetUniform` for `vec2`

First, open `ShaderProgram` and add an **overloaded** `SetUniform()` method that takes **two `float` parameters**:

```cpp
void SetUniform(const std::string& name, float v0, float v1);
```

In its implementation, call:

```cpp
glUniform2f(location, v0, v1);
```

This handles uniforms of type `vec2`.

---

### Step 2: Update Material – Store and Pass `vec2` Uniforms

Open the `Material` class and:

1. Add a new container for `vec2` parameters:

```cpp
std::unordered_map<std::string, std::pair<float, float>> m_float2Params;
```

2. Add an overloaded `SetParam()` method:

```cpp
void SetParam(const std::string& name, float v0, float v1);
```

In its implementation:

```cpp
m_float2Params[name] = {v0, v1};
```

3. Update the `Bind()` method in `Material`:

After setting all float uniforms, iterate over `m_float2Params`:

```cpp
for (const auto& param : m_float2Params) 
{
    m_ShaderProgram->SetUniform(param.first, param.second.first, param.second.second);
}
```

Now your material system supports `vec2` uniforms.

---

### Step 3: Add Offset Variables in Game

In `Game.h`, declare:

```cpp
float m_offsetX = 0.0f;
float m_offsetY = 0.0f;
```

---

### Step 4: Modify Vertex Shader to Use Offset

Update your **vertex shader source code** (in `Game.cpp`) to include the new uniform:

```glsl
uniform vec2 uOffset;

void main() 
{
    gl_Position = vec4(position.x + uOffset.x, position.y + uOffset.y, position.z, 1.0);
}
```

---

### Step 5: Handle Key Input in `Game::Update()`

Inside `Game::Update()`, use the engine’s input manager to check for key presses:

```cpp
auto& inputManager = eng::Engine::GetInstance().GetInputManager();
if (inputManager.IsKeyPressed(GLFW_KEY_A)) 
{
    m_offsetX -= 0.01f;
} 
else if (inputManager.IsKeyPressed(GLFW_KEY_D)) 
{
    m_offsetX += 0.01f;
}

if (inputManager.IsKeyPressed(GLFW_KEY_W)) 
{
    m_offsetY += 0.01f;
} 
else if (inputManager.IsKeyPressed(GLFW_KEY_S)) 
{
    m_offsetY -= 0.01f;
}
```

---

### Step 6: Update the Material with New Offset

Still in `Game::Update()`, after handling input:

```cpp
m_material.SetParam("uOffset", m_offsetX, m_offsetY);
```

---

### Step 7: Run and Test

Build and run your application. You should now be able to move your rendered object using:

* **W / S** to move vertically
* **A / D** to move horizontally

If the object shifts as expected — **congratulations**!

---

### Final Notes

This time, you’re not just moving a hardcoded shape. You’ve:

* Used your **custom shader system**
* Passed uniforms through the **Material API**
* Handled input with the **InputManager**
* Used a proper **rendering pipeline** with reusable components

What used to be a single spaghetti script is now a structured and expandable mini game engine.

**Well done — this is real engine architecture in action.** 👏