### Implementing Core Classes

Let’s start implementing the classes. First, open `Engine.h` and define the basic structure of the `Engine` class. Since the engine lives in its own module, let’s place it inside its own namespace — for example, `namespace eng`.

#### `Engine.h`

Define the class `Engine` with the following three basic methods:

```cpp
bool Init();
void Run();
void Destroy();
```

#### `Engine.cpp`

Now move to `Engine.cpp`, add `namespace eng`, and provide empty implementations:

```cpp
bool Engine::Init() { return false; }
void Engine::Run() {}
void Engine::Destroy() {}
```

---

### Defining the Application Interface

Next, let’s define the base structure of the `Application` class in `Application.h`. Wrap everything inside `namespace eng`.

As you remember, the application should have three core methods: `Init`, `Update`, and `Destroy`. Since these will be implemented in a derived class, we’ll make them **pure virtual**:

```cpp
class Application 
{
public:
    virtual bool Init() = 0;
    virtual void Update(float deltaTime) = 0; // deltaTime in seconds
    virtual void Destroy() = 0;

    void SetNeedsToBeClosed(bool value);
    bool NeedsToBeClosed() const;

protected:
    bool m_needsToBeClosed = false;
};
```

#### `Application.cpp`

Now implement the non-virtual methods:

```cpp
void Application::SetNeedsToBeClosed(bool value) 
{
    m_needsToBeClosed = value;
}

bool Application::NeedsToBeClosed() const 
{
    return m_needsToBeClosed;
}
```

---

### Connecting Engine to Application

Back in `Engine.h`, we need the `Engine` to **own and manage** an instance of `Application`.

1. Add a **forward declaration** of the `Application` class before declaring `Engine`.
2. Include `<memory>` for `std::unique_ptr`.

Inside the `Engine` class, add:

```cpp
void SetApplication(Application* app);
Application* GetApplication();

private:
    std::unique_ptr<Application> m_application;
```

#### `Engine.cpp`

Implement the getter and setter:

```cpp
void Engine::SetApplication(Application* app) 
{
    m_application.reset(app);
}

Application* Engine::GetApplication() 
{
    return m_application.get();
}
```

---

### Implementing the Engine Lifecycle

#### `Engine::Init`

```cpp
bool Engine::Init() 
{
    if (!m_application)
    {
        return false;
    }
    return m_application->init();
}
```

#### `Engine::Destroy`

```cpp
void Engine::Destroy() 
{
    if (m_application) 
    {
        m_application->Destroy();
        m_application.reset();
    }
}
```

#### `Engine::Run`

This is where the **main game loop** will live.

```cpp
void Engine::Run() 
{
    if (!m_application) 
    {
        return;
    }

    // Initialize time tracking
    m_lastTimePoint = std::chrono::high_resolution_clock::now();

    while (!m_application->NeedsToBeClosed()) 
    {
        auto now = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(now - m_lastTimePoint).count();
        m_lastTimePoint = now;

        m_application->Update(deltaTime);
    }
}
```

In `Engine.h`, declare the time-tracking variable:

```cpp
#include <chrono>
std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTimePoint;
```

---

Now, the basic structure and lifecycle of your engine and application are ready. This is a solid foundation for expanding your game engine in the next chapters.