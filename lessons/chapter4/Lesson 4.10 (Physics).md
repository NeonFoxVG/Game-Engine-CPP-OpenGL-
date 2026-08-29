# Lesson: Integrating Bullet Physics into Our Engine

Time to recall Sir Isaac Newton and bring **physics** into our engine.
The most practical way is to integrate an existing, mature **physics engine** rather than reinventing it. Physics simulation is mathematically heavy, and industry-standard practice (in games, film, simulations) is to use a **third-party physics library**.

We will integrate [**Bullet Physics**](https://github.com/bulletphysics/bullet3). It is widely used, open-source, and reliable.

---

## 1) Adding Bullet to Our Project

I’ve already downloaded Bullet and placed it into our `thirdparty/` folder.
In `CMakeLists.txt` we adjust:

* Disable unnecessary parts:

  * `BUILD_UNIT_TESTS = OFF`
  * `BUILD_BULLET2_DEMOS = OFF`
  * `BUILD_EXTRAS = OFF`
  * `BUILD_SHARED_LIBS = OFF`
* Link only what we need (static libraries):

  * `BulletCollision`
  * `BulletDynamics`
  * `LinearMath`

This keeps the build lean and focused.

---

## 2) Physics Manager

We’ll create a **PhysicsManager** to own and control the Bullet world.

### 2.1 File setup

Create folder `source/physics/` add:

* `PhysicsManager.h`
* `PhysicsManager.cpp`

### 2.2 Class declaration

Forward declare Bullet types we need:

```cpp
class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
```

Then define:

```cpp
class PhysicsManager 
{
public:
    PhysicsManager();
    ~PhysicsManager();

    void Init();
    void Update(float deltaTime);

    btDiscreteDynamicsWorld* GetWorld() { return m_world.get(); }

private:
    std::unique_ptr<btBroadphaseInterface> m_broadphase;
    std::unique_ptr<btDefaultCollisionConfiguration> m_collisionConfig;
    std::unique_ptr<btCollisionDispatcher> m_dispatcher;
    std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
    std::unique_ptr<btDiscreteDynamicsWorld> m_world;
};
```

### 2.3 Implementation

In `PhysicsManager.cpp`:

```cpp
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
```

* `Init`:

  * Instantiate all Bullet subsystems in order.
  * Create `btDiscreteDynamicsWorld`.
  * Set gravity:

    ```cpp
    void PhysicsManager::Init()
    {
        // --- World setup ---
        m_broadphase = std::make_unique<btDbvtBroadphase>();
        m_collisionConfig = std::make_unique<btDefaultCollisionConfiguration>();
        m_dispatcher = std::make_unique<btCollisionDispatcher>(m_collisionConfig.get());
        m_solver = std::make_unique<btSequentialImpulseConstraintSolver>();
        m_world = std::make_unique<btDiscreteDynamicsWorld>(
            m_dispatcher.get(), m_broadphase.get(), m_solver.get(), m_collisionConfig.get());

        m_world->setGravity(btVector3(0, -9.81f, 0));
    }
    ```

* `Update`:

  ```cpp
  const btScalar fixedTimeStep = 1.0f / 60.0f;
  const int maxSubsteps = 4;
  m_world->stepSimulation(deltaTime, maxSubsteps, fixedTimeStep);
  ```

---

## 3) Engine Integration

Expose `PhysicsManager` via `Engine`:

* Add `PhysicsManager m_physicsManager;`
* Add `PhysicsManager& GetPhysicsManager();`
* In `Engine::Init()`, after initializing graphics, call:

  ```cpp
  m_physicsManager.Init();
  ```
* In `Engine::Run()` just after you've got the deltaTime before `m_application->Update()`
  call `m_physicsManager.Update(deltaTime)`

Now physics runs as part of the engine update loop.

---

## 4) Colliders

A **collider** is a simplified geometric shape used by the physics engine to calculate collisions.
Common collider types include:

* **Box Collider** (`btBoxShape`)

  * Defined by width, height, and length.
  * Perfect for crates, walls, doors, or any object with a blocky shape.

* **Sphere Collider** (`btSphereShape`)

  * Defined by a single radius.
  * Great for balls, projectiles, or any round object.

* **Capsule Collider** (`btCapsuleShape`)

  * A cylinder capped with two half-spheres.
  * Often used for characters, because it moves smoothly over uneven surfaces and doesn’t snag on edges.

Why these shapes?
Because they are:

* **Mathematically simple** → fast collision detection.
* **Stable** → less chance of glitches.
* **Good enough** → they approximate real gameplay behavior without wasting resources.

In professional games, even highly detailed characters and objects are represented by a few colliders.

We create a unified interface.

### 4.1 Files

`source/physics/Collider.h` and `Collider.cpp`.

### 4.2 Base class

```cpp
class btCollisionShape;

class Collider 
{
public:
    virtual ~Collider() { if (m_shape) delete m_shape; }
    btCollisionShape* GetShape() const { return m_shape; }

protected:
    btCollisionShape* m_shape = nullptr;
};
```

### 4.3 Derived shapes

* **BoxCollider**:

  ```cpp
  class BoxCollider : public Collider 
  {
  public:
      BoxCollider(const glm::vec3& extents) 
      {
          glm::vec3 halfExtents = extents * 0.5f;
          m_shape = new btBoxShape(btVector3(halfExtents.x, halfExtents.y, halfExtents.z));
      }
  };
  ```

* **SphereCollider**:

  ```cpp
  class SphereCollider : public Collider 
  {
  public:
      SphereCollider(float radius) 
      {
          m_shape = new btSphereShape(btScalar(radius));
      }
  };
  ```

* **CapsuleCollider**:

  ```cpp
  class CapsuleCollider : public Collider 
  {
  public:
      CapsuleCollider(float radius, float height) 
      {
          m_shape = new btCapsuleShape(btScalar(radius), btScalar(height));
      }
  };
  ```

---

## 5) Rigid Bodies

While colliders define **shape**, they don’t move by themselves.
To actually simulate physics, we need a **RigidBody**.

A **RigidBody** represents an object in the physical world. It combines:

* **A collider (shape)** → how the object collides.
* **Mass** → how heavy it is.
* **Friction** → how it resists sliding.
* **Motion state** → where it is and how it moves.

There are three main **RigidBody types**:

1. **Static**

   * Doesn’t move.
   * Example: the ground, walls, large immovable obstacles.
   * Other bodies collide against it.

2. **Dynamic**

   * Fully simulated by physics.
   * Example: a box that falls, a ball that rolls, a chair you can push.
   * Affected by forces like gravity and collisions.

3. **Kinematic**

   * Controlled manually (by code), not by physics.
   * Still collides with dynamic objects.
   * Example: moving platforms, elevators, player character capsule.

Think of it like this:

* **Static** = the environment.
* **Dynamic** = free-moving objects.
* **Kinematic** = scripted or controlled motion.

### 5.1 Declaration

Add `RigidBody.h` and `RigidBody.cpp` to `source/physics`
In `RigidBody.h`:

```cpp
enum class BodyType 
{ 
    Static, 
    Dynamic, 
    Kinematic 
};

class RigidBody 
{
public:
    RigidBody(BodyType type,
              const std::shared_ptr<Collider>& collider,
              float mass,
              float friction);
    ~RigidBody();

    btRigidBody* GetBody() { return m_body.get(); }

    BodyType GetType() const { return m_type; }

    void SetAddedToWorld(bool added);
    bool IsAddedToWorld() const;

    void SetPosition(const glm::vec3& pos);
    void SetRotation(const glm::quat& rot);
    glm::vec3 GetPosition() const;
    glm::quat GetRotation() const;

private:
    BodyType m_type = BodyType::Static;
    std::shared_ptr<Collider> m_collider;
    float m_mass = 0.0f;
    float m_friction = 0.5f;
    bool m_addedToWorld = false;

    std::unique_ptr<btRigidBody> m_body;
};
```

### 5.2 Behavior

* Dynamic bodies → mass > 0, inertia computed.

* Static/Kinematic → mass = 0.

```cpp
RigidBody::RigidBody(BodyType type, const std::shared_ptr<Collider>& collider, float mass, float friction)
    : m_type(type), m_collider(collider), m_mass(mass), m_friction(friction)
{
    btVector3 inertia(0, 0, 0);
    if (m_type == BodyType::Dynamic && mass > 0.0f && m_collider->GetShape())
    {
        m_collider->GetShape()->calculateLocalInertia(btScalar(mass), inertia);
    }

    btTransform transform;
    transform.setIdentity();
    btDefaultMotionState* motionState = new btDefaultMotionState(transform);

    btRigidBody::btRigidBodyConstructionInfo info(
        (m_type == BodyType::Dynamic) ? btScalar(mass) : btScalar(0.0),
        motionState, m_collider->GetShape(), inertia);

    m_body = std::make_unique<btRigidBody>(info);
    m_body->setFriction(btScalar(friction));

    // Kinematic:
    if (m_type == BodyType::Kinematic)
    {
        m_body->setCollisionFlags(m_body->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        m_body->setActivationState(DISABLE_DEACTIVATION);
    }
}

void RigidBody::SetAddedToWorld(bool added)
{
    m_isAddedToWorld = added;
}

bool RigidBody::IsAddedToWorld() const
{
    return m_isAddedToWorld;
}

```

* **Setters for transform** only valid **before adding to world**, because once simulated, Bullet overwrites them each step.

```cpp
void RigidBody::SetPosition(const glm::vec3& pos)
{
    if (!m_body)
    {
        return;
    }

    auto& tr = m_body->getWorldTransform();
    tr.setOrigin(btVector3(btScalar(pos.x), btScalar(pos.y), btScalar(pos.z)));
    if (m_body->getMotionState()) 
    {
        m_body->getMotionState()->setWorldTransform(tr);
    }
    m_body->setWorldTransform(tr);
}

glm::vec3 RigidBody::GetPosition() const
{
    const auto& pos = m_body->getWorldTransform().getOrigin();
    return glm::vec3(pos.x(), pos.y(), pos.z());
}

void RigidBody::SetRotation(const glm::quat& rot)
{
    if (!m_body)
    {
        return;
    }

    auto& tr = m_body->getWorldTransform();
    tr.setRotation(btQuaternion(rot.x, rot.y, rot.z, rot.w));
    if (m_body->getMotionState())
    {
        m_body->getMotionState()->setWorldTransform(tr);
    }
    m_body->setWorldTransform(tr);
}

glm::quat RigidBody::GetRotation() const
{
    const auto& rot = m_body->getWorldTransform().getRotation();
    return glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
}
```
---

### 5.3 Let PhysicsManager add/remove bodies

In `PhysicsManager.h`:

```cpp
class RigidBody;

public:
    void AddRigidBody(RigidBody* body);
    void RemoveRigidBody(RigidBody* body);
```

In `PhysicsManager.cpp`:

```cpp
#include "RigidBody.h"
#include <btBulletDynamicsCommon.h>

void PhysicsManager::AddRigidBody(RigidBody* body) 
{
    if (!body || !m_world)
    {
        return;
    }

    if (auto* rb = body->GetBody()) 
    {
        m_world->addRigidBody(rb,
            btBroadphaseProxy::StaticFilter,
            btBroadphaseProxy::AllFilter);
        body->SetAddedToWorld(true);
    }
}

void PhysicsManager::RemoveRigidBody(RigidBody* body) 
{
    if (!body || !m_world) 
    {
        return;
    }

    if (auto* rb = body->GetBody()) 
    {
        m_world->removeRigidBody(rb);
        body->SetAddedToWorld(false);
    }
}
```

And finally the destructor of `RigidBody`:

```cpp
RigidBody::~RigidBody()
{
    if (m_isAddedToWorld)
    {
        Engine::GetInstance().GetPhysicsManager().RemoveRigidBody(this);
    }
}
```

--

## 6) Physics Component
To tie rigid bodies to `GameObject`s, add `PhysicsComponent`.

### 6.1 Files
Create `scene/components/PhysicsComponent.h/.cpp`. It syncs transforms at init and, for dynamic bodies, pulls transforms back every frame.

### 6.2 Class

**PhysicsComponent.h**

```cpp
class PhysicsComponent : public Component 
{
public:
    PhysicsComponent(std::shared_ptr<RigidBody> body);

    void Init() override;
    void Update(float dt) override;

private:
    std::shared_ptr<RigidBody> m_rigidBody;
};
```

### 6.3 Implementation

* `Init`: sync initial transform from `GameObject` to `RigidBody`, then add body to physics world:
```cpp
void PhysicsComponent::Init() 
{
    if (!m_rigidBody) 
    {
        return;
    }

    const auto pos = m_owner->GetWorldPosition();
    const auto rot = m_owner->GetRotation();

    m_rigidBody->SetPosition(pos);
    m_rigidBody->SetRotation(rot);

    Engine::GetInstance().GetPhysicsManager().AddRigidBody(m_rigidBody.get());
}
```
* `Update`: for dynamic bodies, sync transform back from physics to `GameObject`:
```cpp
void PhysicsComponent::Update(float) 
{
    if (!m_rigidBody) 
    {
        return;
    }

    if (m_rigidBody->GetType() == BodyType::Dynamic) 
    {
        m_owner->SetPosition(m_rigidBody->GetPosition());
        m_owner->SetRotation(m_rigidBody->GetRotation());
    }
}
```

---

## 7) Update Component class
Go to `Component` class and add empty `virtual void Init() {}` function
Then go to `GameObject::AddComponent()` function and after everything is set call
`component->Init()`. This ensures that each component can be initialized after it's been added
to the object.

---

## 7) Example: Ground Plane + Falling Box

Let’s test physics by creating a simple scene: a static ground and a dynamic box that falls onto it.

### 7.1 Extend Mesh

Rename `Mesh::CreateCube` → `Mesh::CreateBox(const glm::vec3& extents = glm::vec3(1))`
When generating vertices, use **half-extents** to position the corners:

```cpp
const glm::vec3 half = extents * 0.5f;
// use half.x/half.y/half.z for the 8 corners
```

### 7.2 In `Game.cpp`

**Ground**:

```cpp
auto ground = m_scene->CreateObject("ground");
ground->SetPosition(glm::vec3(0, -5, 0));

glm::vec3 groundExtents(20, 2, 20);
auto groundMesh = eng::Mesh::CreateBox(groundExtents);
ground->AddComponent(new MeshComponent(material, groundMesh));

auto groundCollider = std::make_shared<eng::BoxCollider>(groundExtents);
auto groundBody = std::make_shared<eng::RigidBody>(
    eng::BodyType::Static, groundCollider, 0.0f, 0.5f);

ground->AddComponent(new PhysicsComponent(groundBody));
```

**Dynamic box**:

```cpp
auto boxObj = m_scene->CreateObject("fallingBox");
boxObj->AddComponent(new MeshComponent(material, mesh));

boxObj->SetPosition(glm::vec3(0, 2, 2));
boxObj->SetRotation(glm::quat(glm::vec3(1, 2, 0))); // rotated

auto boxCollider = std::make_shared<eng::BoxCollider>(glm::vec3(1));
auto boxBody = std::make_shared<eng::RigidBody>(
    eng::BodyType::Dynamic, boxCollider, 5.0f, 0.5f);

boxObj->AddComponent(new PhysicsComponent(boxBody));
```

**Player start position**:

```cpp
player->SetPosition(glm::vec3(0, 1, 7));
```

---

## 8) Result

* The ground is static and does not move.
* A box spawns above it and falls under gravity (`-9.81` on Y).
* On contact, Bullet resolves collision, and the box rests on the ground.
* Transform sync ensures game objects update visually to match physical simulation.

---

Congratulations!
You’ve successfully integrated **Bullet Physics** into our engine, built a clean abstraction (`Collider`, `RigidBody`, `PhysicsComponent`), and created your first physically simulated scene. From here, we can add more shapes, collision responses, character controllers, and interactive gameplay.