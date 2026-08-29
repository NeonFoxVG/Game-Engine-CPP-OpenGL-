So far, we’ve been able to render a single object in the scene.
But what if we want to render **many** objects? And what if those objects are spread out across a **huge 3D world** — so large that the entire scene can't possibly fit on the screen?

This is where we need to bring in a crucial concept:

### The Camera.

---

### Why Do We Need a Camera?

Think of a camera in filmmaking.
A large movie set might have buildings, props, actors... but what the audience sees depends on **where the camera is**, **what it’s pointed at**, and **how it’s configured**.

The same idea applies in game development.

* We have a **scene** with many **objects**.
* To decide what the player sees, we need to simulate a **camera**.

This virtual camera lets us:

* Move around the world
* Rotate to look in different directions
* Zoom in and out (via projection)
* And ultimately control what appears on screen

---

### From World Space to Screen Space

Before, we only used the **Model Matrix**, which converts local coordinates of an object to world coordinates. That was enough to move objects around. But to render large 3D scenes properly, we need to introduce two more transformations:

1. **Model Matrix**
   Transforms object from **local space** to **world space**.

2. **View Matrix**
   Transforms the world into **camera space** (like moving the world in front of a stationary camera).

3. **Projection Matrix**
   Transforms the camera space into **screen space**, applying perspective or orthographic projection.

So in the vertex shader, the transformation pipeline becomes:

```glsl
gl_Position = uProjection * uView * uModel * vec4(position, 1.0);
```

Each matrix serves a different purpose:

| Matrix     | Purpose                                 |
| ---------- | --------------------------------------- |
| Model      | Places the object in the world          |
| View       | Moves the world to simulate camera view |
| Projection | Projects 3D coordinates to 2D screen    |

> 🔁 Note: The view matrix is the **inverse** of the camera’s transform. We simulate moving the *world* instead of moving the *camera*.

---

## Step-by-Step Implementation

---

### 1. Create the `CameraComponent`

Inside `scene/components`, create:

* `CameraComponent.h`
* `CameraComponent.cpp`

#### CameraComponent.h

```cpp
#pragma once

#include "scene/component.h"
#include <glm/glm.hpp>

class CameraComponent : public Component 
{
public:
    // Empty update - the camera doesn't change over time in this simple case
    void Update(float deltaTime) override {}

    // Returns a view matrix that simulates the camera's perspective
    glm::mat4 GetViewMatrix() const;

    // Returns the projection matrix (perspective or orthographic)
    glm::mat4 GetProjectionMatrix() const;
};
```

#### CameraComponent.cpp

```cpp
#include "CameraComponent.h"
#include "scene/GameObject.h"
#include <glm/gtc/matrix_transform.hpp>

glm::mat4 CameraComponent::getViewMatrix() const 
{
    // Get the world transform of the camera and invert it
    return glm::inverse(m_owner->GetWorldTransform());
}

// We'll define this later depending on whether we want perspective or orthographic projection
glm::mat4 CameraComponent::GetProjectionMatrix() const 
{
    // Placeholder for now
    return glm::mat4(1.0f);
}
```

> ✅ The camera is just another object — it has a transform like anything else.
> ❗ But unlike other objects, we **invert** its transform when building the view matrix.

---

### 2. Add a "Main Camera" to the Scene

In the `Scene` class, add a field and two methods:

```cpp
// Only one active camera per scene
GameObject* m_mainCamera = nullptr;

void setMainCamera(GameObject* camera) 
{
    m_mainCamera = camera;
}

GameObject* getMainCamera() const 
{
    return m_mainCamera;
}
```

> This gives us a single "active" camera — useful when rendering, because you usually only render from one perspective at a time.

---

### 3. Pass View and Projection Matrices to the Shader

In `RenderQueue.h`:

```cpp
struct CameraData 
{
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
};
```

Update the `Draw` method:

```cpp
void Draw(GraphicsAPI& graphics, const CameraData& cameraData);
```

In the implementation, pass the camera matrices to the shader:

```cpp
shader->setUniform("uView", cameraData.viewMatrix);
shader->setUniform("uProjection", cameraData.projectionMatrix);
```

> 🧠 You’re now giving the shader all the data it needs to simulate a camera and project 3D to 2D.

---

### 4. Update Your Vertex Shader

In the vertex shader:

```glsl
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() 
{
    gl_Position = uProjection * uView * uModel * vec4(position, 1.0);
}
```

This performs the **MVP** transformation.

---

### 5. Hook It All Up in the Engine

In the `Engine` class:

```cpp
std::unique_ptr<Scene> m_currentScene;

void SetScene(std::unique_ptr<Scene> scene);
Scene* GetScene() const;
```

In `Engine::Run()`:

```cpp
CameraData cameraData;

// If there's an active scene...
if (m_currentScene) 
{
    if (auto* cameraObject = m_currentScene->getMainCamera()) 
    {
        // Try to get the camera component
        if (auto* camera = cameraObject->GetComponent<CameraComponent>()) 
        {
        }
    }
}

// Now pass cameraData into the render queue
m_renderQueue.Draw(m_graphicsAPI, cameraData);
```

---

## Summary

In this lesson, you learned:

* Why we need a **camera** to render large 3D scenes
* How to compute the **View Matrix** by inverting the camera’s transform
* How to pass **Model → View → Projection** matrices to the shader
* How to structure your rendering system to be **camera-aware**

> Now you can render large 3D environments, move through them with a camera, and simulate a real-world perspective.