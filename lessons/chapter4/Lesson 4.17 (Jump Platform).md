# Lesson: Jump Platform and Collision Events

Since we already have **physics**, let’s add something more fun.
We’ll create a **jump platform**:

* It will be painted **red**,
* When the player steps on it, they’ll be catapulted upwards (like in many shooters).

To do this, we need to detect **contacts (collisions)** in our physics engine.

---

## 1. Detecting Collisions in PhysicsManager

Open `PhysicsManager::Update()`.
After calling `stepSimulation`, we can query all collisions that happened during this frame.

Bullet Physics allows us to attach **user pointers** to rigid bodies or controllers.
These pointers can be anything we want (we’ll use them to point back to our **CollisionObject**).

So here’s the plan:

1. Retrieve collision manifolds,
2. Extract both bodies,
3. Convert their user pointers into `CollisionObject*`,
4. Extract contact points and normals,
5. Notify both objects that a collision happened.

---

## 2. Base Class: CollisionObject

Let’s create a new base class for all collidable objects.

Files:

* `CollisionObject.h`
* `CollisionObject.cpp`

**CollisionObject.h**

```cpp
class IContactListener;

enum class CollisionObjectType 
{
    RigidBody,
    KinematicCharacterController
};

class CollisionObject 
{
public:
    CollisionObjectType GetCollisionObjectType() const { return m_type; }

    void AddContactListener(IContactListener* listener);
    void RemoveContactListener(IContactListener* listener);

protected:
    void DispatchContactEvent(CollisionObject* obj,
                              const glm::vec3& position,
                              const glm::vec3& normal);

    CollisionObjectType m_type;
    std::vector<IContactListener*> m_contactListeners;

    friend class PhysicsManager;
};

class IContactListener
{
public:
    virtual void OnContact(CollisionObject* obj, const glm::vec3& position, const glm::vec3& normal) = 0;
};
```

**CollisionObject.cpp**

```cpp
void CollisionObject::AddContactListener(IContactListener* listener) 
{
    m_contactListeners.push_back(listener);
}

void CollisionObject::RemoveContactListener(IContactListener* listener) 
{
    auto it = std::find(m_contactListeners.begin(), m_contactListeners.end(), listener);
    if (it != m_contactListeners.end())
    {
        m_contactListeners.erase(it);
    }
}

void CollisionObject::DispatchContactEvent(CollisionObject* obj,
                                           const glm::vec3& pos,
                                           const glm::vec3& normal) 
{
    for (auto listener : m_contactListeners) 
    {
        if (listener) 
        {
            listener->OnContact(obj, pos, normal);
        }
    }
}
```

---

## 3. PhysicsManager Updates

In `PhysicsManager::Update`, loop over all manifolds:

```cpp
auto dispatcher = m_world->getDispatcher();
const auto numManifolds = dispatcher->getNumManifolds();
for (int i = 0; i < numManifolds; i++) 
{
    btPersistentManifold* manifold = dispatcher->getManifoldByIndexInternal(i);

    auto bodyA = reinterpret_cast<CollisionObject*>(manifold->getBody0()->getUserPointer());
    auto bodyB = reinterpret_cast<CollisionObject*>(manifold->getBody1()->getUserPointer());

    if (!bodyA || !bodyB) continue;

    for (int j = 0; j < manifold->getNumContacts(); j++) 
    {
        const btManifoldPoint& pt = manifold->getContactPoint(j);

        glm::vec3 pos(pt.getPositionWorldOnB().x(), pt.getPositionWorldOnB().y(), pt.getPositionWorldOnB().z());
        glm::vec3 normal(pt.m_normalWorldOnB.x(), pt.m_normalWorldOnB.y(), pt.m_normalWorldOnB.z());

        bodyA->DispatchContactEvent(bodyB, pos, normal);
        bodyB->DispatchContactEvent(bodyA, pos, normal);
    }
}
```

Now every collision notifies both objects.

---

## 4. Extending RigidBody and CharacterController

Both `RigidBody` and `KinematicCharacterController` should inherit from `CollisionObject`.

* In `RigidBody` constructor:

  ```cpp
  m_type = CollisionObjectType::RigidBody;
  m_body->setUserPointer(this);
  ```

* In `KinematicCharacterController` constructor:

  ```cpp
  m_type = CollisionObjectType::KinematicCharacterController;
  m_ghost->setUserPointer(this);
  ```

Now both are valid `CollisionObject`s.

---

## 5. Creating Jump Platform

Let’s implement our special **JumpPlatform**.

Files:

* `JumpPlatform.h`
* `JumpPlatform.cpp`

**JumpPlatform.h**

```cpp
class JumpPlatform : public eng::GameObject,
                     public eng::IContactListener 
{
    GAMEOBJECT(JumpPlatform)
public:
    void Init() override;
    void OnContact(emg::CollisionObject* obj,
                   const glm::vec3& position,
                   const glm::vec3& normal) override;
};
```

**JumpPlatform.cpp**

```cpp
void JumpPlatform::Init() 
{
    auto physics = GetComponent<PhysicsComponent>();
    if (physics) 
    {
        auto rigidBody = physics->GetRigidBody();
        if (rigidBody) 
        {
            rigidBody->AddContactListener(this);
        }
    }
}

void JumpPlatform::OnContact(CollisionObject* obj,
                             const glm::vec3& position,
                             const glm::vec3& normal) 
{
    if (obj->GetCollisionObjectType() == CollisionObjectType::KinematicCharacterController) 
    {
        auto controller = static_cast<KinematicCharacterController*>(obj);
        if (controller) 
        {
            controller->Jump(glm::vec3(0, 20, 0));
        }
    }
}
```

---

## 6. Registering the Object

In `Game.cpp`:

```cpp
JumpPlatform::Register();
```

In `scene.sc`, define the object:

```json
{
  "name": "JumpPlatform",
  "type": "JumpPlatform",
  "position": { "x": -7, "y": 1.75, "z": 1 },
  "components": [
    {
      "type": "MeshComponent",
      "mesh": { "type": "box", "x": 2, "y": 0.2, "z": 2 },
      "material": {
        "path": "materials/checker.mat",
        "params": { "float3": [ { "name": "color", "value0": 1, "value1": 0, "value2": 0 } ] }
      }
    },
    {
      "type": "PhysicsComponent",
      "collider": { "type": "box", "x": 2, "y": 0.2, "z": 2 },
      "body": { "mass": 0, "friction": 0.5, "type": "static" }
    }
  ]
}
```

This makes a static red platform that acts as a jump pad.

---

## 7. Adding Spheres

To add variety, I also placed a few **colored spheres**.

In `MeshComponent::LoadProperties`, extend the mesh type loader:

```cpp
if (type == "box") 
{
    // existing
} 
else if (type == "sphere") 
{
    float r = meshObj.value("r", 1.0f);
    auto mesh = Mesh::CreateSphere(r, 16, 16);
    SetMesh(mesh);
}
```

Do the same for `PhysicsComponent`:
support `"sphere"` colliders alongside box and capsule.

Now you can spawn colored spheres that roll and react to bullets.

---

## Result

* We now have a **Jump Platform** (red box) that launches the player upwards,
* Collisions are detected and dispatched using **listeners**,
* Both `RigidBody` and `CharacterController` integrate into the system,
* We added support for **sphere meshes** and colliders,
* The scene contains boxes, spheres, and the jump pad — all interactive.

When you run the game:

* Stepping on the platform catapults the player,
* Shooting spheres and boxes makes them react,
* The scene feels more alive and dynamic.

Awesome progress! Now you can extend your game world with interactive physics-driven elements.