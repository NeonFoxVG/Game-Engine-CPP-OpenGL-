### Creating a Window Using the Engine

Let’s now use our engine to create a **window** — the main canvas where all the action in your game (or any other application) takes place.

As you might remember, we previously used the **GLFW** library to create a window. So now, we’re going to integrate that window creation logic directly into our engine.

---

### Step 1: Moving GLFW Code into `Engine::Init`

Open the `Engine::Init()` function. After the check for a valid application instance (`if (!m_application)`), insert the GLFW window creation code you wrote earlier:

1. `glfwInit()`
2. `glfwWindowHint()` to specify the OpenGL version and profile
3. `glfwCreateWindow()` with width and height
4. `glfwMakeContextCurrent()`
5. `glewInit()` (or similar function to initialize GL function pointers)

To allow dynamic resolution, modify `Engine::Init()` to accept two parameters: `int width` and `int height`.

```cpp
bool Engine::Init(int width, int height)
```

---

### Step 2: Storing the Window in the Engine

Previously, the window was stored in a local variable. But within the engine, we want to **store it as a class member** so it can be accessed across the system.

Do the following:

* In `Engine.h`, before the `eng` namespace, forward-declare the GLFW struct:

```cpp
struct GLFWwindow;
```

* Inside the `Engine` class, declare a private member:

```cpp
GLFWwindow* m_window = nullptr;
```

* In `Engine::Init`, assign the result of `glfwCreateWindow` to `m_window`.

---

### Step 3: Cleaning Up in `Engine::Destroy`

After calling `m_application->Destroy()` and resetting the application instance, clean up GLFW:

```cpp
glfwTerminate();
m_window = nullptr;
```

---

### Step 4: Updating the Main Game Loop (`Engine::Run`)

Now we update the main loop to handle window events properly:

```cpp
void Engine::Run() 
{
    if (!m_application || !m_window) 
    {
        return;
    }

    m_lastTimePoint = std::chrono::high_resolution_clock::now();

    while (!glfwWindowShouldClose(m_window) && !m_application->needsToBeClosed()) 
    {
        glfwPollEvents(); // Handle OS/window input

        auto now = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(now - m_lastTimePoint).count();
        m_lastTimePoint = now;

        m_application->Update(deltaTime); // Game logic update

        glfwSwapBuffers(m_window); // Placeholder for rendering
    }
}
```

### Breakdown of the Main Loop:

* **`glfwPollEvents()`** — processes input/events.
* **`Update(deltaTime)`** — application/game logic.
* **`glfwSwapBuffers()`** — would handle rendering (even though we haven’t added rendering yet).

---

### Step 5: Update `main.cpp` to Provide Resolution

In your `main.cpp`, update the call to `engine.Init()` to pass the desired screen resolution, e.g., 1280×720:

```cpp
if (engine.Init(1280, 720)) 
{
    engine.Run();
}
```

Then build and run your project.

---

### Result

The application launches, a window opens, and your engine loop is working — ready to build on.

**Congratulations — you've just created the foundation for rendering and real-time updates!**