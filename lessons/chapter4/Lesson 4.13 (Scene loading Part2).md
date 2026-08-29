# Lesson: Loading the Entire Scene (Part 2)

In the previous part, we stopped at the point where we had added a `LoadProperties` method to `Component`. Let’s continue from there.

---

## 1) Back to `scene.sc`: describing a MeshComponent

We return to `scene.sc` and our basic objects. As agreed, we’re loading a `MeshComponent`.
So we write:

```json
"type": "MeshComponent"
```

Next, we need to describe its **mesh** and **material**.

* For the material: we already know how to load it, so simply specify a path, for example:

```json
"material": "materials/brick.mat"
```

* For the mesh: we add a `mesh` field, which will itself be an object.
  Inside, it will contain a `type` (for now just `"box"`), and box meshes are described by **extents** (XYZ values).

Example:

```json
"mesh": {
  "type": "box",
  "x": 2,
  "y": 1,
  "z": 5
}
```

So a `MeshComponent` entry in JSON looks like this:

```json
{
  "type": "MeshComponent",
  "material": "materials/brick.mat",
  "mesh": {
    "type": "box",
    "x": 2,
    "y": 1,
    "z": 5
  }
}
```

---

## 2) Implementing `LoadProperties` in `MeshComponent`

Open `MeshComponent.h` and add:

```cpp
void LoadProperties(const nlohmann::json& j) override;
```

Implementation:

```cpp
void MeshComponent::LoadProperties(const nlohmann::json& j) 
{
    // Material
    if (j.contains("material")) 
    {
        const std::string matPath = j.value("material", "");
        auto material = Material::Load(matPath);
        if (material)
        {
            SetMaterial(material);
        }
    }

    // Mesh
    if (j.contains("mesh")) 
    {
        const auto& m = j["mesh"];
        const std::string type = m.value("type", "box");
        if (type == "box") 
        {
            glm::vec3 extents(
                m.value("x", 1.0f),
                m.value("y", 1.0f),
                m.value("z", 1.0f)
            );
            auto mesh = Mesh::CreateBox(extents);
            SetMesh(mesh);
        }
    }
}
```

Good — now our `MeshComponent` knows how to load its own data.

---

## 3) Back to `Scene::LoadObject`: object-level properties and children

Remember: objects may also have **custom properties**, just like components.
So let’s add a similar mechanism for `GameObject`.

1. In `GameObject`, add:

```cpp
virtual void LoadProperties(const nlohmann::json& j) { }
virtual void Init() { }
```

By default these do nothing, but user-defined objects can override them.

2. In `Scene::LoadObject`, after reading transforms and before loading components:

```cpp
gameObject->LoadProperties(objectJson);
```

This guarantees the order:

* Create object
* Read its properties
* Then read components

3. After components, check for **children**:

```cpp
if (objectJson.contains("children") && objectJson["children"].is_array()) 
{
    for (const auto& childJson : objectJson["children"]) 
    {
        LoadObject(childJson, gameObject); // recursive
    }
}
```

4. Finally, after all children are read:

```cpp
gameObject->Init();
```

This allows objects to perform custom initialization once their hierarchy and components are ready.

---

## 4) Next type of object: the Player

Now let’s add a new object to the scene — the **main player**.

In JSON:

```json
{
  "name": "MainPlayer",
  "type": "Player",
  "position": { "x": 0, "y": 2, "z": 7 },
  "components": [
    { "type": "CameraComponent" },
    { "type": "PlayerControllerComponent" }
  ],
  "children": [
    {
      "name": "Gun",
      "type": "gltf",
      "path": "models/sten_gun_machine_carbine/scene.gltf",
      "position": { "x": 0.75, "y": -0.5, "z": -0.75 },
      "scale":    { "x": -1, "y": 1, "z": 1 }
    }
  ]
}
```

---

## 5) Creating custom objects (Player)

Back in `LoadObject`, recall:

```cpp
if (objectJson.contains("type")) 
{
    std::string type = objectJson.value("type", "");
    if (type == "gltf") 
    {
        // GLTF branch (see later)
    } 
    else 
    {
        // Custom user-defined objects
    }
}
```

We need to support user-defined object types like `"Player"`.
We’ll solve it the same way as with components: with a **factory**.

---

## 6) GameObject Factory

1. Base creator:

```cpp
struct ObjectCreatorBase 
{
    virtual ~ObjectCreatorBase() = default;
    virtual GameObject* CreateGameObject() = 0;
};
```

2. Template creator:

```cpp
template <typename T>
struct ObjectCreator : ObjectCreatorBase 
{
    GameObject* CreateGameObject() override 
    { 
        return new T(); 
    }
};
```

3. Factory singleton:

```cpp
class GameObjectFactory 
{
public:
    static GameObjectFactory& GetInstance() 
    {
        static GameObjectFactory instance;
        return instance;
    }

    template <typename T>
    void RegisterObject(const std::string& name) 
    {
        m_creators.emplace(name, std::make_unique<ObjectCreator<T>>());
    }

    GameObject* CreateGameObject(const std::string& typeName) 
    {
        auto it = m_creators.find(typeName);
        if (it == m_creators.end())
            return nullptr;
        return it->second->CreateGameObject();
    }

private:
    std::unordered_map<std::string, std::unique_ptr<ObjectCreatorBase>> m_creators;
};
```

4. Macro for convenience:

```cpp
#define GAMEOBJECT(ObjectClass) \
public: \
    static void Register() { eng::GameObjectFactory::GetInstance().RegisterObject<ObjectClass>(std::string(#ObjectClass)); }
```

Now in `Player.h`:

```cpp
class Player : public eng::GameObject 
{
    GAMEOBJECT(Player)
public:
    Player() = default;
    // ...
};
```

---

## 7) Registering custom objects

The client game must override `Application::RegisterTypes()` to register custom game objects:

```cpp
void Game::RegisterTypes() override 
{
    Player::Register();
}
```

This will be called by the engine during `Engine::Init()`, after engine-side types are registered.

---

## 8) Scene method to create objects by type

In `Scene`, add:

```cpp
GameObject* CreateObject(const std::string& type,
                         const std::string& name,
                         GameObject* parent = nullptr) 
{
    GameObject* obj = GameObjectFactory::GetInstance().CreateGameObject(type);
    if (obj) 
    {
        obj->SetName(name);
        obj->m_scene = this;
        obj->SetParent(parent);
    }
    return obj;
}
```

Now, in `LoadObject` branch:

```cpp
gameObject = CreateObject(type, name, parent);
```

And we can create custom user types like `"Player"`.

---

## 9) GLTF objects

Children like the `"Gun"` object are of type `"gltf"`.
We need to load them from file paths.

In `LoadObject`:

```cpp
else if (type == "gltf") 
{
    std::string path = objectJson.value("path", "");
    gameObject = GameObject::LoadGLTF(path, this); // pass scene
    if (gameObject) 
    {
        gameObject->SetParent(parent);
        gameObject->SetName(name);
    }
}
```

Note: `GameObject::LoadGLTF` should take a `Scene*` argument, so the loaded nodes can be attached to it.
If the scene pointer is null, return `nullptr`.

Inside `LoadGLTF`, instead of getting a scene from engine manually, call `scene->CreateObject(...)` to ensure proper parent/scene linkage.

---

## 10) Using `Init` for post-load adjustments

Remember why we added `Init()`. For example, `Player::Init()` can:

* Hide unnecessary nodes,
* Grab its `AnimationComponent`,
* Setup player-specific state.

This happens **after** the object and all its children have been loaded.

---

We also need to pass the position of the parent object to the `KinematicCharacterController`. Add `const glm::vec3& position` parameter to
`KinematicCharacterController` consturtor and pass position to the tranform. Then in `PlayerControllerComponent::Init()` pass `m_owner->GetWorldPosition`
to the constructor.

---

So far, we created the **Player** object. The player is connected to physics, but right now it will simply **fall into the void** because there is no ground to land on.
Let’s fix that by creating a **Ground object**.

---

## 11) Ground object in JSON

In `scene.cs`, add:

```json
{
  "name": "Ground",
  "position": { "x": 0, "y": -5, "z": 0 },
  "components": [
    {
      "type": "MeshComponent",
      "mesh": {
        "type": "box",
        "x": 20,
        "y": 2,
        "z": 20
      },
      "material": "materials/brick.mat"
    },
    {
      "type": "PhysicsComponent",
      "collider": {
        "type": "box",
        "x": 20,
        "y": 2,
        "z": 20
      },
      "body": {
        "mass": 0,
        "friction": 0.5,
        "type": "static"
      }
    }
  ]
}
```

* The mesh is a **20 × 2 × 20 box**, large enough to act as a platform.
* The physics component has:

  * a box collider of the same size,
  * a static rigid body (mass = 0, friction = 0.5).

---

## 12) Implementing `PhysicsComponent::LoadProperties`

Now let’s make `PhysicsComponent` able to load itself from JSON.

```cpp
void PhysicsComponent::LoadProperties(const nlohmann::json& j) 
{
    std::shared_ptr<Collider> collider;

    // Collider
    if (j.contains("collider")) 
    {
        const auto& col = j["collider"];
        std::string type = col.value("type", "");

        if (type == "box") 
        {
            glm::vec3 extents(
                col.value("x", 1.0f),
                col.value("y", 1.0f),
                col.value("z", 1.0f)
            );
            collider = std::make_shared<BoxCollider>(extents);
        }
        else if (type == "sphere") 
        {
            float radius = col.value("r", 1.0f);
            collider = std::make_shared<SphereCollider>(radius);
        }
        else if (type == "capsule") 
        {
            float radius = col.value("r", 1.0f);
            float height = col.value("h", 1.0f);
            collider = std::make_shared<CapsuleCollider>(radius, height);
        }
    }

    if (!collider)
    {
        return;
    }

    // RigidBody
    std::shared_ptr<Rigidbody> rigidbody;
    if (j.contains("body")) 
    {
        const auto& body = j["body"];

        float mass = body.value("mass", 0.0f);
        float friction = body.value("friction", 0.5f);
        std::string typeStr = body.value("type", "static");

        BodyType type = BodyType::Static;
        if (typeStr == "dynamic") type = BodyType::Dynamic;
        else if (typeStr == "kinematic") type = BodyType::Kinematic;

        rigidbody = std::make_shared<Rigidbody>(type, collider, mass, friction);
    }

    if (rigidbody)
    {
        SetRigidbody(rigidbody);
    }
}
```

Now the `PhysicsComponent` can load both a collider and a rigid body from JSON.

---

## 13) Testing with a falling object

Let’s add another object to test collisions — a simple cube that falls onto the ground.

```json
{
  "name": "ObjectCollide",
  "position": { "x": 0, "y": 7, "z": 0 },
  "rotation": { "x": 1, "y": 2, "z": 0, "w": 1 },
  "components": [
    {
      "type": "MeshComponent",
      "mesh": {
        "type": "box",
        "x": 1,
        "y": 1,
        "z": 1
      },
      "material": "materials/brick.mat"
    },
    {
      "type": "PhysicsComponent",
      "collider": {
        "type": "box",
        "x": 1,
        "y": 1,
        "z": 1
      },
      "body": {
        "mass": 5,
        "friction": 0.5,
        "type": "dynamic"
      }
    }
  ]
}
```

* Mesh: 1×1×1 box.
* Physics: box collider of the same size, dynamic rigid body (mass = 5).
  This cube will fall and collide with the ground platform.

---

## 14) Adding a Light object

Let’s add a light source into the scene.

In `scene.json`:

```json
{
  "name": "Light",
  "position": { "x": 0, "y": 5, "z": 0 },
  "components": [
    {
      "type": "LightComponent",
      "color": { "r": 1, "g": 1, "b": 1 }
    }
  ]
}
```

In `LightComponent` add:

```cpp
void LoadProperties(const nlohmann::json& j) override 
{
    if (j.contains("color")) 
    {
        auto c = j["color"];
        glm::vec3 color(
            c.value("r", 1.0f),
            c.value("g", 1.0f),
            c.value("b", 1.0f)
        );
        SetColor(color);
    }
}
```

---

## 15) Testing the scene

Now let’s test it.

In `Game::Init()`, comment out all the manual object creation code we wrote earlier, and replace it with:

```cpp
auto scene = Scene::Load("scenes/scene.sc");
m_scene = scene;
```

Also replace `Scene*` with `std::shared_ptr<Scene>` in `Game.h`

And there is another step. Open `scene.sc` and after `children` block add `"camera": "MainPlayer"`. Then go back to `Scene::Load()`
And before returning the result write:

```cpp
if (json.contains("camera"))
{
    std::string cameraObjName = json.value("camera", "");
    for (const auto& child : result->m_objects)
    {
        if (auto object = child->FindChildByName(cameraObjName))
        {
            result->SetMainCamera(object);
            break;
        }
    }
}
```

This will find and set the main camera for the scene


In `Player::Init()`, clean up old initialization code and leave only what we actually need — hiding nodes, grabbing `AnimationComponent`.

Now run the game:

* The scene loads from JSON.
* Objects appear.
* Animations trigger.
* Character controller works.

Congratulations! This is a huge step forward.

You can now design different scenes by hand in JSON, with different components and object hierarchies. That’s powerful and exciting!