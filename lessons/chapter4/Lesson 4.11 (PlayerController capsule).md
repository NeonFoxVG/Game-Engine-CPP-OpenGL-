# Player Controller: Turning, Physics, and Character Controller

Now let’s make our **player controller** more comfortable to use.

## 1) Continuous rotation with the mouse

Open `PlayerControllerComponent::Update()`.
Currently, we rotate only while the left mouse button is held. That’s not how modern shooters work. Let’s remove that check — now rotation will always follow the mouse.

If we run the game, we’ll notice another issue: rotation stops once the cursor leaves the screen. Let’s fix this.

In `Engine::Init()`, after setting up all callbacks, add:

```cpp
glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
```

This disables the OS cursor and locks mouse movement to our window. Perfect.

## 2) Mouse movement flag in InputManager

We’ll improve `InputManager` so we can easily detect if the mouse moved this frame.
Go to `InputManager.h`

Add a field:

```cpp
bool m_mousePositionChanged = false;
```

And two methods:

```cpp
void SetMousePositionChanged(bool changed);
bool IsMousePositionChanged() const;
```

### Update in `Engine.cpp` in `cursorPositionCallback`

At the end of the callback:

```cpp
glm::vec2 currentPos{ static_cast<float>(xpos), static_cast<float>(ypos) };
mInputManager.SetMousePositionCurrent(currentPos);
mInputManager.SetMousePositionChanged(true);
```

So each time the mouse moves, the flag is set.

### Reset in `Engine::Run()`

At the end of each frame:

```cpp
m_inputManager.SetMousePositionChanged(false);
```

This gives us a “mouse moved this frame” event.

## 3) Improved player rotation

In `PlayerControllerComponent`, instead of checking mouse buttons, now we do:

```cpp
if (inputManager.IsMousePositionChanged()) 
{
    // apply rotation
}
```

### Rotation logic

* Rotate around **Y axis** for yaw (turn left/right).
* Rotate around **X axis** for pitch (look up/down).
* Clamp pitch to avoid flipping the camera.

```cpp
// Yaw
float yDeltaAngle = -deltaX * m_sensitivity * deltaTime;
m_yRot += yDeltaAngle;
glm::quat yRot = glm::angleAxis(glm::radians(m_yRot), glm::vec3(0, 1, 0));

// Pitch
float xDeltaAngle = -deltaY * m_sensitivity * deltaTime;
m_xRot += xDeltaAngle;
m_xRot = std::clamp(m_xRot, -89.0f, 89.0f);
glm::quat xRot = glm::angleAxis(glm::radians(m_xRot), glm::vec3(1, 0, 0));

// Final rotation
rotation = glm::normalize(yRot * xRot);
m_owner->SetRotation(rotation);
```

Now we rotate smoothly, always within FPS limits.

## 4) Integrating with Physics

Since we already have a physics world, it makes sense for the **player** to interact with it. Instead of a plain transform, we’ll use a **kinematic body**.

Bullet provides a ready-made helper:
**`btKinematicCharacterController`**.

We’ll wrap it into our own class.

## 5) KinematicCharacterController wrapper

Create `physics/KinematicCharacterController.h/.cpp`.

### Header

```cpp
#pragma once
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class btPairCachingGhostObject;
class btKinematicCharacterController;

class KinematicCharacterController 
{
public:
    KinematicCharacterController(float radius, float height);
    ~KinematicCharacterController();

    glm::vec3 GetPosition() const;
    glm::quat GetRotation() const;

    void Walk(const glm::vec3& direction);
    void Jump(const glm::vec3& direction);
    bool OnGround() const;

private:
    float m_height = 1.2f;
    float m_radius = 0.4f;

    std::unique_ptr<btPairCachingGhostObject> m_ghost;
    std::unique_ptr<btKinematicCharacterController> m_controller;
};
```

### Implementation

```cpp
#include "KinematicCharacterController.h"
#include "Engine.h"

#include <btBulletDynamicsCommon.h>
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

KinematicCharacterController::KinematicCharacterController(float radius, float height)
    : m_height(height), m_radius(radius)
{
    auto world = Engine::GetInstance().GetPhysicsManager().GetWorld();

    // Capsule collider (standard for characters)
    auto capsule = new btCapsuleShape(m_radius, m_height);

    m_ghost = std::make_unique<btPairCachingGhostObject>();
    btTransform start;
    start.setIdentity();
    start.setOrigin(btVector3(0.0f, 2.0f, 0.0f)); // spawn above ground
    m_ghost->setWorldTransform(start);
    m_ghost->setCollisionShape(capsule);
    m_ghost->setCollisionFlags(m_ghost->getCollisionFlags() | btCollisionObject::CF_CHARACTER_OBJECT);

    world->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(new btGhostPairCallback());

    const btScalar stepHeight = 0.35f;
    m_controller = std::make_unique<btKinematicCharacterController>(m_ghost.get(), capsule, stepHeight);

    m_controller->setMaxSlope(btRadians(50.0f));
    m_controller->setGravity(world->getGravity());

    world->addCollisionObject(m_ghost.get(),
        btBroadphaseProxy::CharacterFilter,
        btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::SensorTrigger);
    world->addAction(m_controller.get());
}

KinematicCharacterController::~KinematicCharacterController() 
{
    auto world = Engine::GetInstance().GetPhysicsManager().GetWorld();
    if (m_controller) 
    {
        world->removeAction(m_controller.get());
    }
    if (m_ghost)     
    {
        world->removeCollisionObject(m_ghost.get());
    } 
}

glm::vec3 KinematicCharacterController::GetPosition() const 
{
    const auto& pos = m_ghost->getWorldTransform().getOrigin();
    // Offset upwards: camera is not at capsule center
    const glm::vec3 offset(0.0f, m_height * 0.5f + m_radius, 0.0f);
    return glm::vec3(pos.x(), pos.y(), pos.z()) + offset;
}

glm::quat KinematicCharacterController::GetRotation() const 
{
    const auto& rot = m_ghost->getWorldTransform().getRotation();
    return glm::quat(rot.w(), rot.x(), rot.y(), rot.z());
}

void KinematicCharacterController::Walk(const glm::vec3& dir) 
{
    m_controller->setWalkDirection(btVector3(dir.x, dir.y, dir.z));
}

void KinematicCharacterController::Jump(const glm::vec3& dir) 
{
    if (m_controller->onGround()) 
    {
        m_controller->jump(btVector3(dir.x, dir.y, dir.z));
    }
}

bool KinematicCharacterController::OnGround() const 
{
    return m_controller->onGround();
}
```

## 6) Hook into PlayerController

Add to `PlayerControllerComponent`:

```cpp
std::unique_ptr<KinematicCharacterController> m_kinematicController;
```

```cpp
void PlayerControllerComponent::Init()
{
    m_kinematicController = std::make_unique<KinematicCharacterController>(0.4f, 1.2f);
}
```

After handling rotation, build `front` and `right` vectors. Then handle input:

```cpp
glm::vec3 move(0.0f);
if (input.IsKeyPressed(GLFW_KEY_A)) 
{
    move -= right;
}
else if (input.IsKeyPressed(GLFW_KEY_D)) 
{
    move += right;
}
if (input.IsKeyPressed(GLFW_KEY_W)) 
{
    move += front;
}
else if (input.IsKeyPressed(GLFW_KEY_S)) 
{
    move -= front;
}

if (glm::dot(move, move) > 0)
{
    move = glm::normalize(move);
}
m_kinematicController->Walk(move * moveSpeed * deltaTime);

if (input.IsKeyPressed(GLFW_KEY_SPACE)) 
{
    m_kinematicController->Jump(glm::vec3(0.0f, 5.0f, 0.0f));
}

// Sync scene object with physics controller
m_owner->SetPosition(m_kinematicController->GetPosition());
```

Now our player walks with WASD, rotates with the mouse, and jumps with space.

## 7) Dedicated Player class

Let’s wrap everything into a `Player` game object.

Create `Player.h/.cpp`:

**Player.h**

```cpp
#pragma once
#include <eng.h>

class AnimationComponent;

class Player : public GameObject 
{
public:
    void Init();
    void Update(float dt) override;

private:
    AnimationComponent* m_animationComponent = nullptr;
};
```

**Player.cpp**

```cpp
#include "Player.h"
#include <GLFW/glfw3.h>

void Player::Init() 
{
    AddComponent(new eng::CameraComponent());
    SetPosition(glm::vec3(0.0f, 0.0f, 2.0f));
    AddComponent(new eng::PlayerControllerComponent());

    auto gun = eng::GameObject::LoadGLTF("models/sten_gunmachine_carbine/scene.gltf");
    gun->SetParent(this);
    gun->SetPosition(glm::vec3(0.75f, -0.5f, -0.75f));
    gun->SetScale(glm::vec3(-1.0f, 1.0f, 1.0f));

    if (auto anim = gun->GetComponent<eng::AnimationComponent>())
    {
        if (auto bullet = gun->FindChildByName("bullet_33"))
        {
            bullet->SetActive(false);
        }

        if (auto fire = gun->FindChildByName("BOOM_35"))
        {
            fire->SetActive(false);
        }

        anim->Play("shoot", false);
    }
    m_animationComponent = gun->GetComponent<eng::AnimationComponent>();
}

void Player::Update(float dt) 
{
    GameObject::Update(dt);

    auto& input = Engine::GetInstance().GetInputManager();
    if (input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) 
    {
        if (m_animationComponent && !m_animationComponent->IsPlaying()) 
        {
            m_animationComponent->Play("shoot", false);
        }
    }
}
```

And also add `AnimationComponent::IsPlaying()` function.

Finaly in `Game.cpp` and remove code for creating camera and loading gun (they are now living in `Player`)
```cpp
auto player = m_scene->CreateObject<Player>("Player");
player->Init();
m_scene->SetMainCamera(player);
```

Now the player encapsulates camera, controls, and weapon animation.

## 8) Run the game

* Cursor locked and hidden
* Smooth FPS-style rotation
* Player capsule collides with ground and boxes
* Player walks around with WASD, no clipping through objects
* Jump with spacebar
* Shooting animation on left mouse click

**Bravo!** We now have a true FPS-style character controller with physics support. This is a big step toward a real playable prototype.
