## Lesson — Implementing UI Input Logic (Interactive Buttons)

Alright, now let’s continue by implementing the **actual interaction logic**.
We’ll make our UI respond to user input — clicks, hovers, and other mouse interactions.

---

### Step 1 — Preparing the groundwork

Let’s start by declaring:

```cpp
UIElementComponent* hit = nullptr;
```

This pointer will later hold a reference to whichever UI element our cursor is currently hovering over.

Next, we need to **collect all UI elements** that exist inside the currently active canvas, and then **iterate through them**, checking whether each one was hit by the mouse cursor using their `HitTest` function.

Let’s implement a helper function for that:

```cpp
std::vector<UIElementComponent*> CollectUI(CanvasComponent* canvas);
```

Inside, we’ll write:

```cpp
std::vector<UIElementComponent*> result;
GameObject* canvasObject = canvas->GetOwner();
const auto& children = canvasObject->GetChildren();

for (const auto& child : children)
{
    if (auto component = child->GetComponent<UIElementComponent>())
    {
        // We’ll handle recursion below
    }
}
```

---

### Step 2 — Recursive collection of elements

To make the traversal recursive, we’ll add a helper function **inside `CanvasComponent`**:

```cpp
void CollectUI(UIElementComponent* element, std::vector<UIElementComponent*>& out);
```

Implementation:

```cpp
void CanvasComponent::CollectUI(UIElementComponent* element, std::vector<UIElementComponent*>& out)
{
    out.push_back(element);

    const auto& children = element->GetOwner()->GetChildren();
    for (const auto& child : children)
    {
        if (auto component = child->GetComponent<UIElementComponent>())
        {
            CollectUI(component, out);
        }
    }
}
```

So this function simply walks through every child object recursively and gathers all UI components.

Back in our `UIInputSystem`, we can now call:

```cpp
for (const auto& child : children)
{
    auto comp = child->GetComponent<UIElementComponent>();
    if (comp)
    {
        canvas->CollectUI(comp, result);
    }
}

return result;
```

This gives us a **complete, recursive list** of all UI elements in the current active canvas.

---

### Step 3 — Tracking hovered and pressed elements

In the `UIInputSystem`, let’s add two new fields:

```cpp
UIElementComponent* m_hovered = nullptr;
UIElementComponent* m_pressed = nullptr;
```

* `m_hovered` — the element currently under the cursor
* `m_pressed` — the element that was last clicked

Now, inside the `Update` function, we can begin processing input:

```cpp
auto uiElements = CollectUI(m_activeCanvasComponent);
```

Then, we’ll iterate through all elements and check for hits:

```cpp
for (auto element : uiElements)
{
    if (element->HitTest(mousePos))
    {
        hit = element;
        break;
    }
}
```

Once we’ve found the first element that’s under the mouse, we stop processing the rest.

---

### Step 4 — Managing hover state

Now, if our `hit` element differs from the one stored in `m_hovered`, we need to notify both the old and the new ones:

```cpp
if (m_hovered != hit)
{
    if (m_hovered)
    {
        m_hovered->OnPointerExit();
    }

    m_hovered = hit;

    if (m_hovered)
    {
        m_hovered->OnPointerEnter();
    }
    m_pressed = nullptr;
}
```

So now the system knows when the mouse **enters** and **leaves** a widget.

---

### Step 5 — Mouse button interaction

Next, we’ll handle pressing and releasing the left mouse button.

```cpp
if (!m_pressed)
{
    // No active click right now
    if (mousePressed && m_hovered)
    {
        m_pressed = m_hovered;
        m_pressed->OnPointerDown();
    }
}
```

And when we release the button:

```cpp
if (mouseReleased)
{
    if (m_pressed)
    {
        m_pressed->OnPointerUp();

        if (m_pressed == m_hovered)
        {
            m_pressed->OnClick();
        }
    }

    m_pressed = nullptr;
}
```

Beautiful — this covers the **full mouse interaction lifecycle**:
`OnPointerEnter → OnPointerDown → OnPointerUp → OnClick → OnPointerExit`.

Before we start actual testing we need to have a functionality for enabling/disabling cursor. Go to `Engine` class and add a method:

```cpp
void SetCursorEnabled(bool enabled)
{
    glfwSetInputMode(m_window, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}
```
In `Engine::Init()`, after setting up all callbacks, remove this line:

```cpp
glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
```

So we have our cursor back
---

### Step 6 — Registering and testing our button

Don’t forget to register your `ButtonComponent` in the **Scene** initialization code:

```cpp
ButtonComponent::Register();
```

Now let’s test everything. Go to `Game.cpp`
After creating your `Canvas`, write the following:

```cpp
auto& uiInput = Engine::GetInstance().GetUIInputSystem();
uiInput.SetActive(true);
uiInput.SetCanvasComponent(canvasComponent);
```

This enables the input system and assigns the currently active canvas.

---

### Step 7 — Creating a test button

We’ll now create a simple button with a text label on it (using the `TextComponent` we implemented earlier):

```cpp
auto button = m_scene->CreateObject("Button", canvas);
button->SetPosition2D(glm::vec2(300.0f, 300.0f));

auto buttonComponent = new eng::ButtonComponent();
buttonComponent->SetRect(glm::vec2(150, 50));
buttonComponent->SetColor(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
button->AddComponent(buttonComponent);
```

Run the application.

Now, on the screen, you should see your text rendered over a rectangular area — that’s your button.

If you hover your mouse over it, the color will change (the `OnPointerEnter` state).
If you click it, it’ll darken further (the `OnPointerDown` and `OnClick` states).

That means the button component is fully interactive!

---

### Step 8 — Final note

And of course, don’t forget to implement the **`LoadProperties`** method for the `ButtonComponent`,
just like we did previously for the `TextComponent`, so it can read its properties from JSON:

```cpp
void ButtonComponent::LoadProperties(const nlohmann::json& jsonObject)
{
    if (jsonObject.contains("rect"))
    {
        auto& rectObj = jsonObject["rect"];
        SetRect(glm::vec2(
            rectObj.value("x", 1.0f),
            rectObj.value("y", 1.0f)
        ));
    }
    if (jsonObject.contains("color"))
    {
        auto& colorObj = jsonObject["color"];
        SetColor(glm::vec4(
            colorObj.value("r", 1.0f),
            colorObj.value("g", 1.0f),
            colorObj.value("b", 1.0f),
            colorObj.value("a", 1.0f)
        ));
    }
    if (jsonObject.contains("hovered"))
    {
        auto& colorObj = jsonObject["hovered"];
        SetHoveredColor(glm::vec4(
            colorObj.value("r", 0.5f),
            colorObj.value("g", 0.5f),
            colorObj.value("b", 0.5f),
            colorObj.value("a", 0.5f)
        ));
    }
    if (jsonObject.contains("pressed"))
    {
        auto& colorObj = jsonObject["pressed"];
        SetPressedColor(glm::vec4(
            colorObj.value("r", 0.2f),
            colorObj.value("g", 0.2f),
            colorObj.value("b", 0.2f),
            colorObj.value("a", 0.2f)
        ));
    }
}
```

---

### Summary

We’ve just built a **basic interactive UI system**:

* Recursively traversing all UI components in the canvas,
* Detecting mouse position and hit-testing,
* Handling hover, press, release, and click events,
* Reacting visually with color changes.

**Congratulations!**
You’ve now officially added **user input** and **interaction** to your custom UI system —
a huge milestone for any engine.