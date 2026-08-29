# Lesson: Loading the Entire Scene (Part 1)

We’re going to do something interesting: **learn to load the whole scene at once**.
We already know how to load individual objects and we have many resources, so let’s combine everything we’ve learned about resource loading and load it **as one big scene**.
Our main goal is to produce a copy (or something very close) to the scene we previously built by hand.

---

Before we start let's get back to `PhysicsComponent`. As you remeber we used local positions and rotations while updating object based on physics.
Let's fix that. Go to `GameObject` and add the next functions:

```cpp
void GameObject::SetWorldPosition(const glm::vec3& pos)
{
    if (m_parent)
    {
        glm::mat4 parentWorld = m_parent->GetWorldTransform();
        glm::mat4 invParentWorld = glm::inverse(parentWorld);
        glm::vec4 localPos = invParentWorld * glm::vec4(newWorldPos, 1.0f);
        SetPosition(glm::vec3(localPos) / localPos.w);
    }
    else
    {
        SetPosition(newWorldPos);
    }
}

glm::quat GameObject::GetWorldRotation()
{
    if (m_parent)
    {
        return m_parent->GetWorldRotation() * m_rotation;
    }
    else
    {
        return m_rotation;
    }
}

void GameObject::SetWorldRotation(const glm::quat& rot)
{
    if (m_parent)
    {
        glm::quat parentWorldRot = m_parent->GetWorldRotation();
        glm::quat invParentWorldRot = glm::inverse(parentWorldRot);
        glm::quat newLocalRot = invParentWorldRot * rot;
        SetRotation(newLocalRot);
    }
    else
    {
        SetRotation(worldRot);
    }
}
```

Then go to `PhysicsComponent::Init()` and write:

```cpp
const auto pos = m_owner->GetWorldPosition();
const auto rot = m_owner->GetWorldRotation();
```

Then go to `PhysicsComponent::Update()` and write:

```cpp
m_owner->SetWorldPosition(m_rigidBody->GetPosition());
m_owner->SetWorldRotation(m_rigidBody->GetRotation());
```
---

## 1) Start the usual way: add a `Load` function to `Scene`

Open the `Scene` class and add:

```cpp
// Scene.h
class Scene 
{
public:
    static std::shared_ptr<Scene> Load(const std::string& path);

private:
    // Will load a single object from JSON and attach it to 'parent' (or root if parent==nullptr)
    void LoadObject(const nlohmann::json& objectJson, GameObject* parent);
};
```

Now implement it the same way we did earlier loaders:

1. **Read file contents**:

```cpp
// Scene.cpp
#include <nlohmann/json.hpp>
using nlohmann::json;

std::shared_ptr<Scene> Scene::Load(const std::string& path) 
{
    const std::string contents = Engine::GetInstance().GetFileSystem().LoadAssetFileText(path);
    if (contents.empty())
    {
        return nullptr;
    }
```

2. **Parse JSON**:

```cpp
    json j = json::parse(contents);
    if (j.empty())
    {
        return nullptr;
    }
```

3. **Create the scene instance**:

```cpp
    auto result = std::make_shared<Scene>();
```

We have the JSON ready to use. Now let’s discuss the **scene format** before we continue.

---

## 2) Scene format (JSON)

We’ll store scenes in **JSON**, because that’s already our unified resource format in the engine.

* Go to `assets/`, create a subfolder `scenes/`.
* Put a file there, e.g. `scene.sc`.
* The root JSON object will have:

  * `name` — optional scene name, just in case.
  * `objects` — an array of scene objects (top-level objects that live directly in the scene).

Minimal scaffold:

```json
{
  "name": "MyScene",
  "objects": []
}
```

Let’s start describing it gradually. For now, `objects` is an array. We’ll come back to fill it.

Return to `Scene::Load` and read the root fields:

```cpp
    const std::string sceneName = j.value("name", "NoName");

    if (j.contains("objects") && j["objects"].is_array()) 
    {
        const auto& objects = j["objects"];
        // These are the top-level items (no parent)
        for (const auto& obj : objects) 
        {
            result->LoadObject(obj, nullptr);
        }
    }

    return result;
}
```

---

## 3) `LoadObject`: load a single object from JSON

Declare earlier:

```cpp
void LoadObject(const nlohmann::json& objectJson, GameObject* parent);
```

Now implement it. We’ll go step by step, descending into the object description itself.

**Name.** Every object should have a `name`.
In our JSON, for the first object, let’s just call it a random cube hanging in space:

```json
{
  "name": "Object_0"
}
```

Implementation outline:

```cpp
void Scene::LoadObject(const nlohmann::json& objectJson, GameObject* parent) 
{
    const std::string name = objectJson.value("name", "Object");

    GameObject* gameObject = nullptr;
```

We have **three object kinds** we’ll eventually support:

1. A **standard** object we create via the scene (`GameObject`).
2. An object **loaded from a GLTF file**.
3. A **custom** object that derives from `GameObject`.

We need to distinguish them. The simplest way is to add a `type` field to the JSON.
If `type` is absent, we treat it as a default `GameObject`.

```cpp
    if (objectJson.contains("type")) 
    {
        const std::string type = objectJson.value("type", "");
        // Non-default path (GLTF or custom) — we’ll implement this later.
        // Example placeholder:
        // gameObject = CreateObjectFromType(type, name, parent);
    } 
    else 
    {
        // Default object
        gameObject = CreateObject(name, parent);
    }

    if (!gameObject)
    {
        return; // could not create object — bail out and continue
    }
```

Next, load the **transform**, common to all game objects: **position**, **rotation**, **scale**.

We’ll extend our JSON with:

* `position` holding `{ x, y, z }`
* `rotation` holding `{ x, y, z, w }`
* `scale` holding `{ x, y, z }`

```cpp
    // Position
    if (objectJson.contains("position") && objectJson["position"].is_object()) 
    {
        const auto& p = objectJson["position"];
        glm::vec3 pos(
            p.value("x", 0.0f),
            p.value("y", 0.0f),
            p.value("z", 0.0f)
        );
        gameObject->SetPosition(pos);
    }

    // Rotation (quaternion)
    if (objectJson.contains("rotation") && objectJson["rotation"].is_object()) 
    {
        const auto& r = objectJson["rotation"];
        glm::quat rot(
            r.value("w", 1.0f), // glm::quat(w, x, y, z)
            r.value("x", 0.0f),
            r.value("y", 0.0f),
            r.value("z", 0.0f)
        );
        gameObject->SetRotation(rot);
    }

    // Scale
    if (objectJson.contains("scale") && objectJson["scale"].is_object()) 
    {
        const auto& s = objectJson["scale"];
        glm::vec3 scl(
            s.value("x", 1.0f),
            s.value("y", 1.0f),
            s.value("z", 1.0f)
        );
        gameObject->SetScale(scl);
    }
```

Finally, components.

---

## 4) Components in JSON and iteration

Each object can have components. In the JSON file add a field `components`, which is an array.
For our first simple box, it will have just one component: **`MeshComponent`**.

JSON example:

```json
{
  "name": "Object_0",
  "position": { "x": 10, "y": 0, "z": 0 },
  "rotation": { "x": 0, "y": 0, "z": 0, "w": 1 },
  "scale":    { "x": 1, "y": 1, "z": 1 },
  "components": [
    {
      "type": "MeshComponent"
      // material/mesh fields will be read in LoadProperties()
    }
  ]
}
```

Back in `LoadObject`, iterate and load:

```cpp
    if (objectJson.contains("components") && objectJson["components"].is_array()) 
    {
        const auto& comps = objectJson["components"];
        for (const auto& compJson : comps) 
        {
            const std::string type = compJson.value("type", "");
            // create compoent by type
        }
    }
}
```

To make this work, we must “teach” the system what component types exist and how to create them.
That leads us to a tiny **factory**.

---

## 5) Component factory (creator classes + registry)

We’ll build a small factory that can create components by their **type name**.

1. A base creator with a single pure virtual method:

```cpp
// Component.h
struct ComponentCreatorBase 
{
    virtual ~ComponentCreatorBase() = default;
    virtual Component* CreateComponent() = 0;
};
```

2. A templated creator for concrete types:

```cpp
template <typename T>
struct ComponentCreator : ComponentCreatorBase 
{
    Component* CreateComponent() override 
    { 
        return new T(); 
    }
};
```

3. The factory singleton itself:

```cpp
class ComponentFactory 
{
public:
    static ComponentFactory& GetInstance() 
    {
        static ComponentFactory instance;
        return instance;
    }

    template <typename T>
    void RegisterObject(const std::string& name) 
    {
        m_creators.emplace(name, std::make_unique<ComponentCreator<T>>());
    }

    Component* CreateComponent(const std::string& typeName) 
    {
        auto it = m_creators.find(typeName);
        if (it == m_creators.end())
            return nullptr;
        return it->second->CreateComponent();
    }

private:
    std::unordered_map<std::string, std::unique_ptr<ComponentCreatorBase>> m_creators;
};
```
---

## 6) Making component registration easy

Add a **static registration function** to each component type. You can reuse a small helper macro:

```cpp
// In each component header after the class definition:
#define COMPONENT(ComponentClass) \
public: \
    static size_t TypeId() { return Component::StaticTypeId<ComponentClass>(); } \
    size_t GetTypeId() const override { return TypeId(); } \
    static void Register() { eng::ComponentFactory::GetInstance().RegisterObject<ComponentClass>(std::string(#ComponentClass)); }
```

The idea: `Register()` is a **static** method that is called **once** during initialization; it tells the factory “when you see the string `"MeshComponent"`, construct `MeshComponent`”.

---

## 7) Default constructors + setters for components

Because our `ComponentCreator<T>` constructs components with a **default constructor**, all registered component types must have one.

* `MeshComponent`: previously required a material and a mesh in its constructor.
  Give it `MeshComponent() = default;` and add setters:

```cpp
void SetMaterial(std::shared_ptr<Material> m);
void SetMesh(std::shared_ptr<Mesh> mesh);
```

* `PhysicsComponent`: previously took a `Rigidbody`.
  Give it a default constructor and a setter:

```cpp
void SetRigidbody(const std::shared_ptr<Rigidbody>& body);
```

Go through your component list and ensure each one has a default constructor.

---

## 8) Loading per-component properties from JSON

Some components used to receive parameters via their constructors. We’ll compensate by adding a **virtual** load hook to the base class:

```cpp
// Component.h
class Component 
{
public:
    virtual ~Component() = default;
    virtual void LoadProperties(const nlohmann::json& j) { /* default: do nothing */ }
};
```

Be sure to `#include <nlohmann/json.hpp>` where needed.

If your game/CMake to include the engine’s third-party JSON headers, add:

```
# In the game's CMakeLists.txt
include_directories(engine/thirdparty/json)
```

Now, when we create a component via the factory, we immediately call `component->LoadProperties(compJson)` to read fields like mesh/material/rigidbody, etc.

---

## 9) Where to call registration (engine vs. client)

We need a place that runs **once** at startup to register all component types:

* On the **engine side**, add `static void Scene::RegisterTypes();` and call it from `Engine::Init()`.
* On the **client/application side**, let the app add its **own** types by overriding a virtual method.

```cpp
// Application.h
class Application 
{
public:
    virtual ~Application() = default;
    virtual void RegisterTypes() {} // client can override to register custom types
};
```

And in `Engine::Init()` (after you’ve confirmed `m_application` exists):

```cpp
Scene::RegisterTypes();        // register all built-in engine components
if (m_application)
    mApplication->RegisterTypes();  // let the client register its custom components
```

Now implement `Scene::RegisterTypes()` to include each built-in component and call its `Register()`:

```cpp
// Scene.cpp (or a dedicated registration TU)
#include "scene/components/AnimationComponent.h"
#include "scene/components/CameraComponent.h"
#include "scene/components/LightComponent.h"
#include "scene/components/MeshComponent.h"
#include "scene/components/PhysicsComponent.h"
#include "scene/components/PlayerControllerComponent.h"

void Scene::RegisterTypes() 
{
    AnimationComponent::Register();
    CameraComponent::Register();
    LightComponent::Register();
    MeshComponent::Register();
    PhysicsComponent::Register();
    PlayerControllerComponent::Register();
}
```

This ensures the factory knows how to create all built-in components **by name** during scene loading.

---

## 10) Back to object loading: creating components by `type`

Now that registration is in place, the component loading loop in `LoadObject` works:

```cpp
if (objectJson.contains("components") && objectJson["components"].is_array()) 
{
    for (const auto& compJson : objectJson["components"]) 
    {
        const std::string type = compJson.value("type", "");
        Component* component = ComponentFactory::GetInstance().CreateComponent(type);
        if (component) 
        {
            component->LoadProperties(compJson);     // per-component JSON
            gameObject->AddComponent(component);
        }
    }
}
```

`MeshComponent::LoadProperties()` will read `"mesh"` and `"material"` and call `SetMesh(...)`, `SetMaterial(...)`.

(Objects with `"type"` specified — e.g., GLTF or custom subclasses of `GameObject` — will be handled later when we implement that branch.)

---

## 12) Summary (what we achieved in this part)

* Added `Scene::Load(path)` that reads a JSON file and constructs the entire scene.
* Defined a **stable JSON format** for the scene: `name`, `objects` (each with `position`, `rotation`, `scale`, `components`).
* Implemented `LoadObject(json,parent)` that creates a default `GameObject` (non-default types deferred), applies transform, and loads components.
* Built a **ComponentFactory** with creators and a registry to instantiate components **by name**.
* Ensured components have **default constructors** + setters for previously constructor-injected data.
* Added `Component::LoadProperties(json)` to load per-component data.
* Introduced **engine-side** (`Scene::RegisterTypes`) and **client-side** (`Application::RegisterTypes`) registration, both called from `Engine::Init()`.

That’s it for now. **To be continued…** (GLTF/custom object creation will be tackled in the next part.)
