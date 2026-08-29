## Lesson: Building the UI Foundation and Type Hierarchy System

Alright — since we already have physics, rendering, and 2D/3D logic in place, it’s time to expand our horizons again.
A natural next step is to understand **user interface (UI)** and **user input systems**.

UI elements are special: they’re essentially 2D, but they have strong constraints — for instance, they **don’t rotate**, and they’re almost always **rectangular**.
So let’s agree on one thing first: all UI elements in our engine will **inherit from a single base component**.

---

### Step 1: Create the Base UI Component

Let’s start by creating that base class.

1. Go to the folder: `scene/components/`.
2. Inside it, create a new folder: **ui** — this will store all UI-related components.
3. Inside it, create two new files:

   * `UIElementComponent.h`
   * `UIElementComponent.cpp`

In these files, declare:

```cpp
class UIElementComponent : public Component 
{
protected:
    glm::vec2 m_pivot = glm::vec2(0.5f);
};
```

This pivot works just like the one we used for sprites — it defines the point around which our UI element is positioned.

Don’t forget to add these files to **CMake** and to your **engine include paths**.

Great — now we have a foundation for all UI elements.

---

### Step 2: The Canvas — Root of All UI

Since UI elements are special, they’ll all live under a **dedicated subtree** in the scene hierarchy — a single root object that manages everything UI-related.

This is a common design pattern.
It allows us to manage multiple UI screens (menus, HUDs, etc.) independently — activating, hiding, or disabling entire UI sets as needed.

So let’s create a **Canvas** component — the root of our UI system.

In the same `ui` folder, create two more files:

* `CanvasComponent.h`
* `CanvasComponent.cpp`

Inside, declare:

```cpp
class CanvasComponent : public Component 
{
public:
    void Update(float deltaTime) override;
    void Render(UIElementComponent* element);
};
```

---

### Step 3: Canvas Logic — Recursive Rendering

Now, in the `Update()` method, we’ll make the canvas loop through all its child objects and render any that contain a `UIElementComponent`.

```cpp
void CanvasComponent::Update(float deltaTime) 
{
    const auto children = m_owner->GetChildren();
    for (const auto& child : children) {
        if (auto comp = child->GetComponent<UIElementComponent>()) 
        {
            Render(comp);
        }
    }
}
```

Go to `GameObject` and implement `GetChildren()` method:

```cpp
const std::vector<std::unique_ptr<GameObject>>& GameObject::GetChildren() const
{
    return m_children;
}
```

Now let’s implement `Render()`:

```cpp
void CanvasComponent::Render(UIElementComponent* element) 
{
    if (!element) 
    {
        return;
    }

    // Render this element
    element->Render(this);

    // Then recursively render all children that are also UI elements
    const auto children = element->GetOwner()->GetChildren();
    for (const auto& child : children) 
    {
        if (auto comp = child->getComponent<UIElementComponent>()) 
        {
            Render(comp);
        }
    }
}
```

So, `CanvasComponent` now acts as the central manager — it traverses the scene graph, finds all UI elements, and calls their `Render()` methods recursively.

---

### Step 4: Extend `UIElementComponent` with a Virtual Render Function

Let’s go back to `UIElementComponent` and add a **virtual** render function, so all derived UI components can implement their own drawing logic:

```cpp
virtual void Render(CanvasComponent* canvas) {}
```

Now any subclass — buttons, text, images — can override this to draw themselves.

---

### Step 5: The First Real UI Element — TextComponent

Let’s implement our first UI component: **TextComponent**.

In `scene/components/ui/`, create:

* `TextComponent.h`
* `TextComponent.cpp`

Don’t forget to register them in **CMake** and include files.

In the header, declare:

```cpp
class TextComponent : public UIElementComponent 
{
public:
    COMPONENT(TextComponent)

    void Render(CanvasComponent* canvas) override;

private:
    std::string m_text;

public:
    void SetText(const std::string& text) { m_text = text; }
    const std::string& GetText() const { return m_text; }
};
```

For now, the `Render()` method can remain empty — we’ll fill it later when we start actually drawing text.

---

### Step 6: Expanding Type Hierarchy — The Parent Registry System

Now, let’s address an important question:
When the canvas iterates through scene objects, it calls `GetComponent<UIElementComponent>()`.
But our `TextComponent` is **derived** from `UIElementComponent`.
How can the system automatically recognize that a `TextComponent` **is a kind of** `UIElementComponent`?

To solve this, we need a **parent-type hierarchy** inside our `ComponentFactory`.

---

### Step 7: Implementing the Parent Map

We’ll store parent-child relationships between types using a map.

In `ComponentFactory`, add:

```cpp
std::unordered_map<size_t, std::vector<size_t>> m_parentMap;
```

* **Key:** Type ID of the component.
* **Value:** Vector of Type IDs representing its direct parents.

This way, we can traverse up the inheritance chain to see if a component is derived from another.

---

### Step 8: Extending Registration

When we register a new component type, by default it’s a child of `Component`.
But sometimes, we want to explicitly specify a **different parent** (like `UIElementComponent`).

So let’s extend the registration system.

Add a second templated registration method:

```cpp
template<typename T, typename ParentT>
void RegisterObject() 
{
    m_creators.emplace(name, std::make_unique<ComponentCreator<T>>());
    m_parentMap[T::TypeId()].push_back(Component::StaticTypeId<ParentT>());
}
```

Also add registration in Parent map to the basic `RegisterObject` function:
```cpp
template<typename T>
void RegisterObject() 
{
    m_creators.emplace(name, std::make_unique<ComponentCreator<T>>());
    m_parentMap[T::TypeId()].push_back(Component::StaticTypeId<Component>());
}
```


Now we can register a component with a **specific parent**.

---

### Step 9: The New Macro: `COMPONENT_2`

Let’s introduce a new macro for components that specify a parent type:

```cpp
#define COMPONENT_2(ComponentClass, ParentComponentClass) \
public: \
    static size_t TypeId() { return eng::Component::StaticTypeId<ComponentClass>(); } \
    size_t GetTypeId() const override { return TypeId(); } \
    static void Register() { eng::ComponentFactory::GetInstance().RegisterComponent<ComponentClass, ParentComponentClass>(std::string(#ComponentClass)); }
```

This works just like the regular `COMPONENT` macro, but explicitly links a class to its parent.

Now, in `TextComponent`, replace the old macro with this one:

```cpp
COMPONENT_2(TextComponent, UIElementComponent)
```

---

Also make `Component::Update()` function from pure virtual to default implementation:

```cpp
void Component::Update(float deltaTime)
{
}
```

---

### Step 10: Recursive Parent Checking — `HasParent`

In `ComponentFactory`, add:

```cpp
bool HasParent(size_t objectType, size_t parentType);
```

Implementation:

```cpp
bool ComponentFactory::HasParent(size_t objectType, size_t parentType) 
{
    auto record = m_parentMap.find(objectType);
    if (record == m_parentMap.end()) 
    {
        return false;
    }

    auto& parents = record->second;
    if (std::find(parents.begin(), parents.end(), parentType) != parents.end())
    {
        return true;
    }

    for (auto p : parents) 
    {
        if (HasParent(p, parentType)) 
        {
            return true;
        }
    }

    return false;
}
```

This function climbs the inheritance tree recursively until it finds (or fails to find) the requested parent.

---

### Step 11: Integrating the Hierarchy into `GameObject::GetComponent`

Finally, in the `GetComponent<T>()` method of `GameObject`, update the check:

```cpp
if (component->GetTypeId() == typeId ||
    ComponentFactory::GetInstance().HasParent(component->getTypeId(), typeId)) 
    {
    return static_cast<T*>(component.get());
}
```

Now `GetComponent<UIElementComponent>()` will correctly return a `TextComponent`.

---

### Step 12: Register Everything

In `Scene::RegisterTypes()`, register your new components:

```cpp
CanvasComponent::RegisterType();
UIElementComponent::RegisterType();
TextComponent::RegisterType();
```

---

### Step 13: Understanding the Result

Let’s summarize how this system now works.

Imagine you create a `TextComponent` and attach it to a `GameObject`.

* It’s registered as a subclass of `UIElementComponent`.
* When `CanvasComponent` iterates through children, it looks for `UIElementComponent`.
* The `ComponentFactory` hierarchy tells it that `TextComponent` **is derived from** `UIElementComponent`.
* The canvas calls `Render()` on it.

That’s the full chain of logic — and it works recursively all the way up to the base `Component`.

---

### Final Thoughts

Now we’ve built the entire **foundation for our UI system**:

* We understand what UI elements are and how they fit into the scene hierarchy.
* We created a `CanvasComponent` that manages them.
* We implemented a flexible type hierarchy that supports inheritance checks.
* We prepared our first UI element — `TextComponent`.

In the **next lesson**, we’ll finally start implementing the actual rendering logic for text on screen.

Stay tuned — it’s going to get visual!