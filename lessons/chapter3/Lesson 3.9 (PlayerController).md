Right now, our camera can display different parts of the scene, but it’s fixed.
We want to make it **move** and **rotate**, so we can fly through the world —
exactly like in level editors or spectator mode in games.

The easiest way to do this is to make a **Player Controller** component.
This component will:

* Read input from the keyboard and mouse
* Apply movement and rotation to its **owner** object
* Work with any object (including one with a `CameraComponent`)

If we attach this component to an object with a camera,
we’ll have a **first-person view** we can control.

---

## Step 1: Creating the PlayerControllerComponent

In the `scene/components` folder:

* Create `PlayerControllerComponent.h`
* Create `PlayerControllerComponent.cpp`
* Add both to **CMakeLists.txt**
* Include them in `eng.h`

We’ll use the `COMPONENT` macro from before so the engine knows what this component is.

---

### PlayerControllerComponent.h

```cpp
#pragma once
#include "scene/component.h"

namespace eng 
{

class PlayerControllerComponent : public Component 
{
    COMPONENT(PlayerControllerComponent) // Identifies this component type

private:
    float m_sensitivity = 0.1f; // Mouse rotation sensitivity
    float m_moveSpeed = 1.0f;   // Movement speed

public:
    void Update(float deltaTime) override;
};

} // namespace eng
```

*We give our controller two important settings:*

* **m_sensitivity**: How quickly we turn when moving the mouse.
* **m_moveSpeed**: How fast we move when pressing movement keys.

---

## Step 2: Expanding InputManager for Mouse Input

Our `InputManager` already handles keyboard keys.
Now we need it to handle **mouse buttons** and **mouse position**.

Why?

* **Mouse buttons**: So we can rotate the camera only when the left button is pressed (or any button we choose).
* **Mouse position**: So we can measure how far the mouse has moved since last frame —
  this distance will determine how much we rotate.

---

### In `InputManager.h`

```cpp
std::array<bool, 16> m_mouseKeys = { false }; // Enough for any mouse buttons

glm::vec2 m_mousePositionOld { 0.0f, 0.0f };      // Last frame position
glm::vec2 m_mousePositionCurrent { 0.0f, 0.0f };  // Current frame position
```

We also create **get/set** methods, just like for keyboard keys.

---

## Step 3: Adding GLFW Callbacks for Mouse

We need two callbacks:

1. **Mouse Button Callback** — called when a mouse button is pressed/released
2. **Mouse Position Callback** — called whenever the cursor moves

---

### Mouse Button Callback

```cpp
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) 
{
    auto& inputManager = Engine::getInstance().getInputManager();
    inputManager.SetMouseKey(button, action != GLFW_RELEASE);
}
```

---

### Mouse Position Callback

```cpp
void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) 
{
    auto& inputManager = Engine::GetInstance().GetInputManager();

    // Save current as old before updating
    inputManager.SetMousePositionOld(inputManager.GetMousePositionCurrent());

    // Set new current position
    glm::vec2 currentPos(static_cast<float>(xpos), static_cast<float>(ypos));
    inputManager.SetMousePositionCurrent(currentPos);
}
```

---

### Hooking into Engine::init()

```cpp
glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
glfwSetCursorPosCallback(m_window, cursorPositionCallback);
```

---

### Resetting Old Mouse Position Each Frame

In `Engine::run()`, after `glfwSwapBuffers`:

```cpp
inputManager.SetMousePositionOld(inputManager.GetMousePositionCurrent());
```

💬 *This is important so we don’t keep getting the same delta over and over when the mouse isn’t moving.*

---

## Step 4: Clean Up Old Movement Code

In `TestObject::Update()`, remove any code that changes position directly.
Now, movement will be handled by `PlayerControllerComponent`.

---

## Step 5: Mouse Look in PlayerControllerComponent

```cpp
void PlayerControllerComponent::Update(float deltaTime) 
{
    auto& input = Engine::getInstance().GetInputManager();
    auto rotation = m_owner->GetRotation();

    // Rotate only when left mouse button is pressed
    if (input.IsMouseKeyPressed(GLFW_MOUSE_BUTTON_LEFT)) 
    {
        glm::vec2 oldPos = input.GetMousePositionOld();
        glm::vec2 curPos = input.GetMousePositionCurrent();

        float deltaX = curPos.x - oldPos.x;
        float deltaY = curPos.y - oldPos.y;

        // Yaw rotation (look left/right) — opposite direction to mouse movement
        rotation.y -= deltaX * mSensitivity * deltaTime;

        // Pitch rotation (look up/down)
        rotation.x -= deltaY * mSensitivity * deltaTime;
    }

    m_owner->SetRotation(rotation);
```

*We rotate around the Y-axis for horizontal mouse movement (yaw)
and around the X-axis for vertical movement (pitch).*

---

## Step 6: Moving with the Keyboard

To move **relative to the camera’s rotation**,
we first calculate the **forward** and **right** vectors from the rotation.

```cpp
    // Create a rotation-only matrix
    glm::mat4 rotMat(1.0f);
    glm::vec3 rot = m_owner->getRotation();

    rotMat = glm::rotate(rotMat, rot.x, glm::vec3(1, 0, 0));
    rotMat = glm::rotate(rotMat, rot.y, glm::vec3(0, 1, 0));
    rotMat = glm::rotate(rotMat, rot.z, glm::vec3(0, 0, 1));

    glm::vec3 forward = glm::normalize(glm::vec3(rotMat * glm::vec4(0, 0, -1, 0)));
    glm::vec3 right   = glm::normalize(glm::vec3(rotMat * glm::vec4(1, 0, 0, 0)));

    auto position = m_owner->getPosition();

    if (input.IsKeyPressed(GLFW_KEY_W))
    {
        position += forward * mMoveSpeed * deltaTime;
    }
    else if (input.IsKeyPressed(GLFW_KEY_S)) 
    {
        position -= forward * mMoveSpeed * deltaTime;
    }

    if (input.IsKeyPressed(GLFW_KEY_D)) 
    {
        position += right * mMoveSpeed * deltaTime;
    }
    else if (input.IsKeyPressed(GLFW_KEY_A)) 
    {
        position -= right * mMoveSpeed * deltaTime;
    }

    m_owner->SetPosition(position);
}
```

*The key thing here is we’re not moving in fixed world axes —
we move forward relative to where the camera is facing.*

---

## Step 7: Putting It All Together

In your game setup:

```cpp
camera->AddComponent(new PlayerControllerComponent());
```

---

## Final Result

* Move mouse (with left button pressed) → camera rotates
* Press **W/S** → move forward/backward relative to where you’re looking
* Press **A/D** → strafe left/right
* Movement and rotation are smooth because they scale with `deltaTime`
* Speeds and sensitivity can be tuned in the component

---

## Summary

| Feature                | Why It Matters                                 |
| ---------------------- | ---------------------------------------------- |
| Mouse delta tracking   | Detects how far mouse moved since last frame   |
| Mouse button checks    | Lets us rotate only when a button is held      |
| Sensitivity setting    | Controls rotation responsiveness               |
| Movement speed setting | Controls how quickly we move                   |
| Forward/right vectors  | Makes movement relative to camera rotation     |
| Delta time usage       | Keeps movement smooth regardless of frame rate |