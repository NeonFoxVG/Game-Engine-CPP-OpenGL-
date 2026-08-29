## Lesson: Adding Interactivity to the UI — Implementing Buttons and Input Handling

So, we already have a **Text Component** that handles text rendering.
But UI isn’t just about drawing text or showing images — it’s also about **reacting to user input**.
So, to demonstrate this concept, let’s create another component that will do exactly that.

And, as you’ve probably guessed (or already know), the simplest and most fundamental interactive component is a **Button**.

### Step 1 — Extending the Base UI Element for Interactivity

Let’s start by extending the base class `UIElementComponent`, because interactivity shouldn’t be limited to buttons — other UI elements (like sliders, checkboxes, etc.) may also need user input later.

Open the base class **UIElementComponent** and add the following methods:

```cpp
virtual bool HitTest(const glm::vec2& pos) const;
virtual void OnPointerEnter();
virtual void OnPointerExit();
virtual void OnPointerUp();
virtual void OnPointerDown();
virtual void OnClick();
```

The `HitTest` method determines whether the mouse cursor is currently inside the element’s bounds — in other words, whether this element is being “hit”.
The rest of the methods (`OnPointerEnter`, `OnPointerExit`, etc.) define the reactions to different cursor and click events.

---

### Step 2 — Creating the Button Component

Now, inside the folder `scene/components/ui`, create two new files:

```
ButtonComponent.h  
ButtonComponent.cpp
```

Declare the class:

```cpp
class ButtonComponent : public UIElementComponent
```

Use the macro:

```cpp
COMPONENT_2(ButtonComponent, UIElementComponent)
```

The button will have two main fields:

```cpp
glm::vec2 m_rect;      // The button’s size (width and height)
glm::vec4 m_color = glm::vec4(1.0f); // Default color
```

Add getters and setters:

```cpp
void SetRect(const glm::vec2& rect);
const glm::vec2& GetRect() const;
void SetColor(const glm::vec4& color);
const glm::vec4& GetColor() const;
```

---

### Step 3 — Implementing the Virtual Methods

Now let’s implement all virtual methods inherited from `UIElementComponent`:
`HitTest`, `Render`, `OnPointerEnter`, `OnPointerExit`, `OnPointerUp`, `OnPointerDown`, `OnClick`.

#### `hitTest`

We need to determine whether the mouse position lies inside the rectangle of this button.

```cpp
bool ButtonComponent::HitTest(const glm::vec2& pos) const 
{
    auto ownerPos = m_owner->GetWorldPosition2D();

    float x1 = ownerPos.x - m_rect.x * m_pivot.x;
    float y1 = ownerPos.y - m_rect.y * m_pivot.y;
    float x2 = x1 + m_rect.x;
    float y2 = y1 + m_rect.y;

    return (x1 <= pos.x && x2 >= pos.x && y1 <= pos.y && y2 >= pos.y);
}
```

---

### Step 4 — Rendering the Button

Next, let’s draw the button.
Inside `ButtonComponent::Render()`:

```cpp
auto pos = m_owner->GetWorldPosition2D();
pos.x -= m_rect.x * m_pivot.x;
pos.y -= m_rect.y * m_pivot.y;

canvas->DrawRect(
    pos,
    pos + m_rect,
    m_color
);
```

But to make this work, we need a new helper function in `CanvasComponent`:

```cpp
void DrawRect(const glm::vec2& lowerLeft, const glm::vec2& upperRight, const glm::vec4& color)
{
    uint32_t base = static_cast<uint32_t>(m_vertices.size() / 8);

    m_vertices.insert(m_vertices.end(), {
        p2.x, p2.y, color.r, color.g, color.b, color.a, 1.0f, 1.0f,
        p1.x, p2.y, color.r, color.g, color.b, color.a, 0.0f, 1.0f,
        p1.x, p1.y, color.r, color.g, color.b, color.a, 0.0f, 0.0f,
        p2.x, p1.y, color.r, color.g, color.b, color.a, 1.0f, 0.0f,
        });
    m_indices.insert(m_indices.end(), { base, base + 1, base + 2, base, base + 2, base + 3 });

    UpdateBatches(nullptr);
}
```

This method is similar to the previous textured `DrawRect`,
except UV coordinates are set by default, and the texture pointer will be `nullptr`.
At the end of it, we’ll call `UpdateBatches(nullptr)`.

---

### Step 5 — Adding Hover and Press States

Now let’s handle mouse interactions — when the cursor enters, exits, or presses the button.

We’ll add more fields to `ButtonComponent`:

```cpp
glm::vec4 m_hoveredColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
glm::vec4 m_pressedColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
const glm::vec4* m_currentColor = &m_color;
```

Then, in `Render()`, we use:

```cpp
canvas->DrawRect(pos, pos + m_rect, *m_currentColor);
```

Add setters/getters for hovered and pressed colors.
Now we can implement interactivity:

```cpp
void ButtonComponent::OnPointerEnter() { m_currentColor = &m_hoveredColor; }
void ButtonComponent::OnPointerExit()  { m_currentColor = &m_color; }
void ButtonComponent::OnPointerUp()    { m_currentColor = &m_hoveredColor; }
void ButtonComponent::OnPointerDown()  { m_currentColor = &m_pressedColor; }
```

And for the click event:

```cpp
std::function<void()> onClick;

void ButtonComponent::OnClick() 
{
    if (onClick) 
    {
        onClick();
    }
}
```

---

### Step 6 — Creating the UI Input System

Now we need a system that will manage user input and distribute events to UI elements.

Create two new files:

```
UIInputSystem.h  
UIInputSystem.cpp
```

Declare:

```cpp
class UIInputSystem 
{
public:
    void SetActive(bool active);
    bool IsActive() const;
    void SetCanvas(CanvasComponent* canvas);
    void Update(float deltaTime);

private:
    bool m_active = false;
    CanvasComponent* m_activeCanvas = nullptr;
};
```

Add it to the engine (in `Engine` class) as a member:

```cpp
UIInputSystem m_uiInputSystem;
```

and expose it with a getter `GetUIInputSystem()`.

In `Engine::Run()` update it each frame after `m_physicsManager.Update(deltaTime);`:

```cpp
if (m_uiInputSystem.IsActive())
{
    m_uiInputSystem.Update(deltaTime);
}
```

---

### Step 7 — Connecting InputManager and Mouse Events

The `UIInputSystem` must know about mouse clicks and movement.
Let’s enhance our `InputManager` for that.

Add:

```cpp
std::array<bool, 16> m_mouseKeyPressed = { false };
std::array<bool, 16> m_mouseKeyReleased = { false };
```

Then implement:

```cpp
void SetMouseButtonWasPressed(int button, bool pressed);
bool WasMouseButtonPressed(int button) const;
void SetMouseButtonWasReleased(int button, bool released);
bool WasMouseButtonReleased(int button) const;
```

And also:

```cpp
void ClearStates() 
{
    SetMousePositionChanged(false);
    for (auto& k : m_mouseKeyPressed) 
    {
        WasMouseButtonPressed(k, false);
    } 
    for (auto& k : m_mouseKeyReleased)
    {
        WasMouseButtonReleased(k, false);
    }
}
```

Call `m_inputManager.ClearStates()` instead of `m_inputManager.SetMousePositionChanged(false)` at the end of each frame in `Engine::Run()`.

Now update GLFW callbacks:

```cpp
void mouseButtonCallback(GLFWwindow* window, int button, int action, int)
{
    auto& inputManager = eng::Engine::GetInstance().GetInputManager();
    if (action == GLFW_PRESS)
    {
        inputManager.SetMouseButtonPressed(button, true);
        inputManager.SetMouseButtonWasPressed(button, true);
    }
    else if (action == GLFW_RELEASE)
    {
        inputManager.SetMouseButtonPressed(button, false);
        inputManager.SetMouseButtonWasReleased(button, true);
    }
}
```

---

### Step 8 — Updating the UI Input System

Finally, in `UIInputSystem::Update()`:

```cpp
if (!m_active || !m_activeCanvas)
{
    return;
} 

auto& input = Engine::GetInstance().GetInputManager();
bool mouseDown     = input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
bool mousePressed  = input.WasMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
bool mouseReleased = input.WasMouseButtonReleased(GLFW_MOUSE_BUTTON_LEFT);

auto mousePos = input.GetMousePositionCurrent();
mousePos.y = Engine::GetInstance().GetGraphicsAPI().GetViewport().height - mousePos.y;
```

Now we have the full setup ready — our input system is active,
the base UI elements support interactivity,
and the button component can now respond visually and logically to user actions.

In the **next lesson**, we’ll handle how this system actually detects elements under the cursor,
dispatches pointer events, and triggers callbacks like `onClick`.

Don’t switch off — the fun part is coming next!