### Handling User Input via `InputManager`

Let’s implement support for **user input** in our engine. For this, we’ll create a new component called `InputManager`.

---

### Step 1: File and Folder Setup

1. Go to `engine/source/` and create a new folder called `input`.

2. Inside this folder, create two files:

   * `InputManager.h`
   * `InputManager.cpp`

3. Open `eng.h` and add `#include "input/InputManager.h"` so the input system is exposed to engine users.

4. In the engine’s `CMakeLists.txt`, include both `InputManager.h` and `InputManager.cpp` in the build target.

---

### Step 2: Implementing the `InputManager` Class

This class will only be created and owned by the **engine** itself. To enforce that, we’ll do the following:

* Make all constructors private: default, copy, move.
* Declare `Engine` as a `friend class` so only it can construct and access `InputManager`.

#### Example (in `InputManager.h`):

```cpp
#pragma once
#include <array>

namespace eng 
{

    class InputManager 
    {
        friend class Engine;

    private:
        InputManager() = default;
        InputManager(const InputManager&) = delete;
        InputManager& operator=(const InputManager&) = delete;
        InputManager(InputManager&&) = delete;
        InputManager& operator=(InputManager&&) = delete;

    public:
        void SetKeyPressed(int key, bool pressed);
        bool IsKeyPressed(int key) const;

    private:
        std::array<bool, 256> mKeys = { false };
    };

}
```

#### `InputManager.cpp`:

```cpp
#include "InputManager.h"

namespace eng 
{

    void InputManager::SetKeyPressed(int key, bool pressed) 
    {
        if (key < 0 || key >= static_cast<int>(mKeys.size()))
        { 
            return; 
        }
        mKeys[key] = pressed;
    }

    bool InputManager::IsKeyPressed(int key) const 
    {
        if (key < 0 || key >= static_cast<int>(mKeys.size())) 
        {
            return false;
        }
        return mKeys[key];
    }

}
```

---

### Step 3: Integrating with `Engine`

In `Engine.h`, add:

```cpp
#include "input/InputManager.h"

private:
    InputManager m_inputManager;

public:
    InputManager& GetInputManager();
```

In `Engine.cpp`:

```cpp
InputManager& Engine::GetInputManager() {
    return m_inputManager;
}
```

---

### Step 4: Making `Engine` Globally Accessible

To use input anywhere in the game code, we’ll make `Engine` a **singleton**. Modify the `Engine` class:

```cpp
class Engine 
{
public:
    static Engine& GetInstance();

private:
    Engine() = default;
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
    Engine(Engine&&) = delete;
    Engine& operator=(Engine&&) = delete;
    //...
};
```

Implementation in `Engine.cpp`:

```cpp
Engine& Engine::GetInstance() 
{
    static Engine instance;
    return instance;
}
```

Now you can access the engine globally via `Engine::GetInstance()`.

Update `main.cpp` to use the singleton instead of creating the engine manually:

```cpp
eng::Engine& engine = eng::Engine::GetInstance();
```

---

### Step 5: Registering the GLFW Key Callback

In `Engine.cpp`, inside `Engine::init()`, after creating the window, add:

```cpp
void keyCallback(GLFWwindow*, int key, int, int action, int) 
{
    auto& inputManager = eng::Engine::GetInstance().GetInputManager();
    if (action == GLFW_PRESS)
    {
        inputManager.SetKeyPressed(key, true);
    }
    else if (action == GLFW_RELEASE)
    {
        inputManager.SetKeyPressed(key, false);
    
}
```

This lambda directly updates the key state in `InputManager`.

---

### Step 6: Testing Input Handling

In `Game.cpp`, inside `Game::Update`, you can now check input like this:

```cpp
#include <GLFW/glfw3.h>
#include <iostream>
#include <eng.h>

void Game::Update(float deltaTime) 
{
    auto& input = eng::Engine::GetInstance().GetInputManager();

    if (input.IsKeyPressed(GLFW_KEY_A)) 
    {
        std::cout << "A button pressed" << std::endl;
    }
}
```

Build and run your game. When you press the **A** key, you should see a message in the console.

---

### Recap

You now have:

* A global `InputManager` for key press tracking
* A singleton-based `Engine` class for global access
* Proper key state handling via GLFW callbacks
* Input querying inside your game update loop