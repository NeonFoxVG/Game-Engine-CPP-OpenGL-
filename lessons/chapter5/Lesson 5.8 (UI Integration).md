## Lesson — Building a Functional UI Menu (Play & Quit Buttons)

Alright, now let’s add the ability to **load and manage our newly created UI elements**, alongside the ones we’ve already learned to load earlier.

In other words — it’s time to make a **main menu** with working **Play** and **Quit** buttons.

---

### Step 1 — Setting up the scene

Go to the folder:

```
/assets/scenes/
```

and open the file:

```
scene.sc
```

Inside, let’s add **three objects**:

1. A root **Canvas** object — name it `MainCanvas`.
2. Inside it, add two child objects:

   * `PlayButton`
   * `QuitButton`

Each of them should have both a **ButtonComponent** and a child object with a **TextComponent** attached.
That will give us the structure of a simple main menu UI.

---

### Step 2 — Loading the active canvas

Now open `Scene::Load()` and make sure we can tell **which canvas is active** after loading.

At the end of the scene-loading routine, add the following:

```cpp
std::string activeCanvasName = json.value("activeCanvas", "");

for (auto& child : result->m_objects)
{
    if (auto canvasObject = child->FindChildByName(activeCanvasName))
    {
        if (auto component = canvasObject->GetComponent<CanvasComponent>())
        {
            Engine::GetInstance().GetUIInputSystem().SetCanvas(component);
            break;
        }
    }
}
```

Then, in your scene JSON file, specify:

```json
"activeCanvas": "MainCanvas"
```

This way, the engine knows **which canvas** should be active and responsive to user input.

---

### Step 3 — Canvas activation flag

Let’s make our `CanvasComponent` aware of whether it’s active or not.

In `CanvasComponent.h`, add:

```cpp
bool m_active = true;
```

And the following methods:

```cpp
void SetActive(bool active) { m_active = active; }
bool IsActive() const { return m_active; }
```

Now, override `LoadProperties()`:

```cpp
void CanvasComponent::LoadProperties(const nlohmann::json& jsonObject)
{
    bool active = jsonObject.value("active", true);
    SetActive(active);
}
```

---

### Step 4 — Respecting canvas activity in input

In `UIInputSystem::Update()`, add an extra condition at the start:

```cpp
if (!m_active || !m_activeCanvasComponent || !m_activeCanvasComponent->IsActive())
{
    return;
}
```

This ensures we only process input when the canvas itself is active.

---

### Step 5 — Grouping 3D objects under a root

Let’s organize our scene a bit.
Group all 3D objects under a single parent object called **`3DRoot`**.
This will make it easy to enable or disable the entire 3D world at once — for example, when switching between the game and the main menu.

---

### Step 6 — Finding objects by name

In `Scene.cpp`, add a helper function:

```cpp
GameObject* Scene::FindObjectByName(const std::string& name)
{
    for (auto& obj : m_objects)
    {
        if (auto child = obj->FindChildByName(name))
        {
            return child;
        }
    }
    return nullptr;
}
```

---

### Step 7 — Expanding input manager for Escape key

The Escape key has a higher code value, so we need to expand our input array.

In `InputManager`, change:

```cpp
std::array<bool, 256> m_keys;
```

to:

```cpp
std::array<bool, 512> m_keys;
```

Now we can safely process keys like `GLFW_KEY_ESCAPE`.

---

### Step 8 — Improving glyph rendering

Let’s improve font rendering accuracy.
In `Font.h`, update the `GlyphDescription` struct:

```cpp
int xOffset = 0;
int yOffset = 0;
```

In `FontaManager::GetFont()` add these offsets to glyph description:

```cpp
gd.xOffset  = static_cast<int>(face->glyph->bitmap_left);
gd.yOffset = static_cast<int>(face->glyph->bitmap_top);
```

Then in `TextComponent::Render()`, modify how we calculate Y coordinates:

```cpp
float x1 = static_cast<float>(xOrigin);
float y1 = static_cast<float>(yOrigin - desc.height + desc.yOffset);
float x2 = x1 + static_cast<float>(desc.width);
float y2 = y1 + static_cast<float>(desc.height);
```

This adjustment fixes vertical alignment differences between glyphs.

---

### Step 9 — Shader cache optimization

To avoid recreating the same shaders repeatedly, let’s add a simple **shader cache**.

In `GraphicsAPI`, define:

```cpp
struct ShaderKey 
{
    std::string vertexSource;
    std::string fragmentSource;

    bool operator==(const ShaderKey& other) const 
    {
        return vertexSource == other.vertexSource &&
               fragmentSource == other.fragmentSource;
    }
};

struct ShaderKeyHash 
{
    std::size_t operator()(const ShaderKey& key) const 
    {
        std::size_t h1 = std::hash<std::string>{}(key.vertexSource);
        std::size_t h2 = std::hash<std::string>{}(key.fragmentSource);
        return h1 ^ (h2 << 1);
    }
};
```

Now, inside the `GraphicsAPI` class:

```cpp
std::unordered_map<ShaderKey, std::shared_ptr<ShaderProgram>, ShaderKeyHash> m_shaderCache;
```

In `CreateShaderProgram()`:

```cpp
ShaderKey key{ vertexSource, fragmentSource };
auto it = m_shaderCache.find(key);
if (it != m_shaderCache.end())
{
    return it->second;
}

auto shaderProgram = std::make_shared<ShaderProgram>(vertexSource, fragmentSource);
m_shaderCache.emplace(key, shaderProgram);
return shaderProgram;
```

That’s it — now identical shaders will reuse existing instances.

---

### Step 10 — Updating engine scene ownership

In `Engine`, replace:

```cpp
std::unique_ptr<Scene> m_currentScene;
```

with:

```cpp
std::shared_ptr<Scene> m_currentScene;
```

This will let us dynamically switch scenes in the future without ownership conflicts. Also update `Get()` and `Set()` methods:

```cpp
void Engine::SetScene(const std::shared_ptr<Scene>& scene)
{
    m_currentScene = scene;
}

const std::shared_ptr<Scene>& Engine::GetScene()
{
    return m_currentScene;
}
```

---

### Step 11 — Proper cleanup

In `Engine::Run()`, after the main loop ends, add:

```cpp
m_application.reset(nullptr);
```

This ensures the game releases all memory and resources when it exits.

---

### Step 12 — Setting up in Game.cpp

Let’s declare a reference to our root 3D object:

```cpp
GameObject* m_3DRoot = nullptr;
```

Then, after loading the scene:

```cpp
m_3DRoot = m_scene->FindObjectByName("3DRoot");
if (m_3DRoot)
{
    m_3DRoot->SetActive(false);
}
```

Now, let’s connect our **Play** and **Quit** buttons:

```cpp
if (auto button = canvasComponent->GetOwner()->FindChildByName("PlayButton"))
{
    if (auto component = button->GetComponent<eng::ButtonComponent>())
    {
        component->onClick = [this]()
        {
            auto& engine = eng::Engine::GetInstance();
            engine.GetUIInputSystem().GetCanvas()->SetActive(false);
            engine.SetCursorEnabled(false);

            if (m_3DRoot)
            {
                m_3DRoot->SetActive(true);
            }
        };
    }
}
```

And for the **Quit** button:

```cpp
if (auto button = canvasComponent->GetOwner()->FindChildByName("QuitButton"))
{
    if (auto component = button->GetComponent<eng::ButtonComponent>())
    {
        component->onClick = [this]()
        {
            SetNeedsToBeClosed(true);
        };
    }
}
```

---

### 🏃 Step 13 — Handling Escape key

Inside `Game::Update()`, after `Scene::Update()`, add:

```cpp
if (eng::Engine::GetInstance().GetInputManager().IsKeyPressed(GLFW_KEY_ESCAPE))
{
    if (m_3DRoot && m_3DRoot->IsActive())
    {
        auto& engine = Engine::GetInstance();
        engine.GetUIInputSystem().GetCanvas()->SetActive(true);
        engine.SetCursorEnabled(true);
        m_3DRoot->SetActive(false);
    }
}
```

This lets us toggle between the **3D world** and the **UI menu** by pressing Escape —
exactly how it works in real games.

---

### Step 14 — Testing

Now, when you run the application:

* You’ll see your UI with **Play** and **Quit** buttons.
* Clicking **Play** hides the UI and enables the 3D scene.
* Clicking **Quit** exits the game.
* Pressing **Escape** while in-game brings the UI back.

---

### Summary

We’ve just built a **fully functional in-game menu system**:

* Loadable from JSON scene files
* Interactive UI elements with callbacks
* Proper input handling and activation logic
* Dynamic toggling between UI and game worlds
* Shader caching for performance

**Congratulations!**
Your engine now supports a complete **main menu** workflow — just like real games do.