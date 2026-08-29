Now it’s time to answer an important question:
**What if we have many game objects with completely different behaviors?**

Imagine this — one object might be entirely different from another in every possible way. Now, imagine we have **thousands** of such unique objects in our game.

How can we manage this?

---

### The Answer: Components

We can break down unique behaviors and characteristics into separate **entities** called **components**.

So, each game object can:

* Have children (other objects)
* Contain a **set of components**, each representing a specific behavior or property

Examples:

* `MeshComponent` – defines how the object looks and what it’s made of
* `CarComponent` – handles road movement
* `AIComponent` – controls NPC behavior
* `PlayerComponent` – handles player controls
* `CameraComponent` – manages the camera
* `LightComponent` – controls lighting

This architecture is known as the **Component-Based System**, and it's widely used in modern game engines (like Unity and Unreal).

---

## Step-by-Step Implementation

### 1. Create the `Component` Base Class

In the `Scene` folder, create two files:

* `Component.h`
* `Component.cpp`

Add them to your `CMakeLists.txt` and include them in `eng.h`.

In `Component.h`:

```cpp
#pragma once

class GameObject; // Forward declaration

class Component 
{
public:
    virtual ~Component() = default;
    virtual void Update(float deltaTime) = 0;

    GameObject* GetOwner() const { return m_owner; }

protected:
    GameObject* m_owner = nullptr;

    friend class GameObject;
};
```

In `Component.cpp`, implement `GetOwner()` if needed (already trivial in header).

---

### 2. Add Component Support to `GameObject`

In `GameObject.h`, include the component header:

```cpp
#include "scene/component.h"
```

Add a container to store components:

```cpp
std::vector<std::unique_ptr<Component>> m_components;
```

In `Update()` method of `GameObject`, loop through all components:

```cpp
for (auto& component : m_components) 
{
    component->Update(deltaTime);
}
```

Add an `AddComponent` method:

```cpp
void AddComponent(Component* component) 
{
    m_components.emplace_back(component);
    component->m_owner = this;
}
```

---

### 3. Create `MeshComponent`

Inside `engine/source/scene`, create a new folder: `components`.

Inside that folder, create:

* `MeshComponent.h`
* `MeshComponent.cpp`

Add them to `CMakeLists.txt` and `eng.h`.

In `MeshComponent.h`:

```cpp
#pragma once

#include "scene/Component.h"
#include <memory>

class Material;
class Mesh;

class MeshComponent : public Component 
{
public:
    MeshComponent(std::shared_ptr<Material> material, std::shared_ptr<Mesh> mesh);
    void Update(float deltaTime) override;

private:
    std::shared_ptr<Material> m_material;
    std::shared_ptr<Mesh> m_mesh;
};
```

In `MeshComponent.cpp`:

```cpp
#include "scene/components/MeshComponent.h"
#include "render/Material.h"
#include "render/Mesh.h"
#include "render/RenderQueue.h"
#include "scene/GameObject.h"
#include "Engine.h"

MeshComponent::MeshComponent(std::shared_ptr<Material> material, std::shared_ptr<Mesh> mesh)
    : mMaterial(material), mMesh(mesh) {}

void MeshComponent::Update(float deltaTime) 
{
    if (!m_material || !m_mesh) 
    {
        return;
    }

    RenderCommand command;
    command.material = m_material.get();
    command.mesh = m_mesh.get();
    command.modelMatrix = GetOwner()->GetWorldTransform();

    auto& renderQueue = Engine::GetInstance().GetRenderQueue();
    renderQueue.Submit(command);
}
```

---

### 4. Refactor `TestObject` to Use `MeshComponent`

Now go into your `TestObject` class:

* Remove the `m_material` and `m_mesh` member variables
* Remove the manual creation of render commands in `Update()`

Instead, do the following in the constructor or `Init()`:

```cpp
auto material = std::make_shared<Material>();
auto mesh = std::make_shared<Mesh>();

AddComponent(new eng::MeshComponent(material, mesh));
```

---

### 5. Test the Implementation

Compile and run the game.
You’ll see the same result: a rectangle on screen, reacting to input and moving as before.

But now, all mesh and material rendering logic is cleanly separated into the `MeshComponent`.

---

### Summary

* You've learned what a **component** is and how it helps separate logic cleanly.
* You've added a base `Component` class.
* You updated `GameObject` to support multiple components.
* You built a `MeshComponent` that handles rendering.
* You've refactored your test object to use this new system.

> This structure gives you a powerful, flexible, and scalable foundation for working with thousands of unique objects in your game.