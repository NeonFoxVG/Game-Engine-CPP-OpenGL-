1. How to retrieve components of a specific type using `GetComponent<T>()`
2. How to implement a lightweight component type identification system (without RTTI)
3. How a 3D camera works: `FOV`, `nearPlane`, `farPlane`, and `aspect ratio`
4. How to pass camera projection and view matrices to shaders
5. How to render your scene with a proper camera and perspective

---

## Part 1: Accessing Components by Type

### Why do we need this?

We want to be able to do this:

```cpp
auto* camera = gameObject->GetComponent<CameraComponent>();
```

To make that possible, we need:

* A type-safe way to retrieve a component
* A fast system to identify component types at runtime (without using C++'s built-in RTTI)

---

### Step 1: Add `GetComponent<T>()` to `GameObject`

```cpp
template <typename T>
T* GetComponent() 
{
    size_t typeId = Component::StaticTypeId<T>();

    for (const auto& component : m_components) 
    {
        if (component->GetTypeId() == typeId) 
        {
            return static_cast<T*>(component.get());
        }
    }

    return nullptr;
}
```

---

### Step 2: Custom Type Identification System

We store components as `std::unique_ptr<Component>`. So we need a way to determine what *actual* type each component is — without relying on RTTI.

In `Component`:

```cpp
class Component 
{
protected:
    GameObject* m_owner = nullptr;

private:
    static size_t nextId;

public:
    virtual ~Component() = default;
    virtual void Update(float deltaTime) = 0;
    virtual size_t GetTypeId() const = 0;

    template<typename T>
    static size_t StaticTypeId() 
    {
        static size_t typeId = nextId++;
        return typeId;
    }

    GameObject* GetOwner() const { return m_owner; }
};
```

In `Component.cpp`:

```cpp
size_t Component::nextId = 1;
```

---

### 🔁 Step 3: Use a Macro to Avoid Repetitive Code

To avoid writing the same boilerplate in every component class:

```cpp
#define COMPONENT(ComponentClass) \
public: \
    static size_t TypeId() { return Component::StaticTypeId<ComponentClass>(); } \
    size_t GetTypeId() const override { return TypeId(); }
```

Now, in your component classes:

```cpp
class CameraComponent : public Component 
{
    COMPONENT(CameraComponent)
    // ...
};
```

And:

```cpp
class MeshComponent : public Component 
{
    COMPONENT(MeshComponent)
    // ...
};
```

---

## Part 2: How a 3D Camera Works

A **camera** in a game engine is a virtual eye. It determines what part of the scene is visible and how it's projected onto the screen.

There are two essential matrices involved:

| Matrix     | Role                                       |
| ---------- | ------------------------------------------ |
| View       | Positions the world relative to the camera |
| Projection | Projects the 3D world into 2D screen space |

Together with the **Model Matrix**, they form the MVP chain:

```cpp
gl_Position = uProjection * uView * uModel * vec4(position, 1.0);
```

---

## Camera Projection Concepts

### 1. **FOV (Field of View)**

* Describes the vertical angle of the camera’s view cone.
* Measured in degrees.
* Wide FOV: more visible, more distortion
* Narrow FOV: zoomed-in feel

| FOV (degrees) | Effect                |
| ------------- | --------------------- |
| 30–45         | Zoom / Telephoto lens |
| 60 (default)  | Balanced perspective  |
| 90–120        | Wide angle / fisheye  |

---

### 2. **Near and Far Planes**

* The camera only renders what’s **between** these two distances.
* Everything outside of them is **clipped**.
* This defines the **view frustum** — a truncated pyramid in 3D space.

| Plane       | Description                                 |
| ----------- | ------------------------------------------- |
| `nearPlane` | How close objects must be to be visible     |
| `farPlane`  | How far objects can be to still be rendered |

---

### 3. **Aspect Ratio**

* Width / Height of the screen
* Affects how the FOV is applied horizontally

If it's wrong, the scene will appear squashed or stretched.

---

## Part 3: Implementing Projection Matrix

### In `CameraComponent`:

#### Fields

```cpp
float m_fov = 60.0f;
float m_nearPlane = 0.1f;
float m_farPlane = 1000.0f;
```

#### Methods

```cpp
glm::mat4 GetViewMatrix() const 
{
    return glm::inverse(m_owner->GetWorldTransform());
}

glm::mat4 GetProjectionMatrix(float aspectRatio) const 
{
    return glm::perspective(glm::radians(m_fov), aspectRatio, m_nearPlane, m_farPlane);
}
```

---

## Part 4: Hooking It Up in the Engine

### In `Engine::Run()`:

```cpp
CameraData cameraData;

int width = 0, height = 0;
glfwGetWindowSize(mWindow, &width, &height);
float aspect = static_cast<float>(width) / static_cast<float>(height);

if (m_currentScene) 
{
    if (auto* cameraObj = m_currentScene->GetMainCamera()) 
    {
        if (auto* camera = cameraObj->GetComponent<CameraComponent>()) 
        {
            cameraData.viewMatrix = camera->GetViewMatrix();
            cameraData.projectionMatrix = camera->GetProjectionMatrix(aspect);
        }
    }
}

m_renderQueue.Draw(m_graphicsAPI, cameraData);
```

---

### In `Game::Init()`

```cpp
auto camera = m_scene->CreateObject("Camera");
camera->AddComponent<CameraComponent>();
camera->SetPosition(glm::vec3(0.0f, 0.0f, 2.0f)); // Move the camera back a bit

m_scene->SetMainCamera(camera);
eng::Engine::GetInstance().SetScene(m_scene);
```

---

## Part 5: Updating the Shader

### Vertex Shader (GLSL)

```glsl
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main() 
{
    gl_Position = uProjection * uView * uModel * vec4(position, 1.0);
}
```

Now the shader applies all three transforms in order: model → view → projection.

---

## Final Result

You’ve now added a real, working **camera system** with:

* Field of view (FOV)
* Near and far clipping planes
* Aspect ratio calculation
* Integration with your rendering pipeline
* Component type retrieval system for future expansion

And you’ve done it without relying on RTTI!

---

## Summary

| Feature                  | Description                                            |
| ------------------------ | ------------------------------------------------------ |
| `GetComponent<T>()`      | Retrieve components in a type-safe, fast way           |
| `StaticTypeId()`         | Unique ID system for component types                   |
| `FOV`                    | Controls how wide the camera sees                      |
| `nearPlane` / `farPlane` | Limits visible range of the camera                     |
| `aspectRatio`            | Maintains correct proportions on different resolutions |
| `View Matrix`            | Transforms world relative to camera                    |
| `Projection Matrix`      | Transforms 3D world into 2D space                      |

---