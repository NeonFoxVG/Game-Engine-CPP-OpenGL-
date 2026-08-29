We are getting very close to actual rendering! Let’s take one more step toward the finish line.

---

### **Rendering Overview**

Here’s the general idea:
During the **update phase**, we will accumulate draw requests into a **render queue**. Once the scene has finished updating and the engine moves to the **rendering phase**, it will process the queue and issue all the necessary draw calls.

Let’s implement this system.

---

### Step 1: Create the Render Queue

In the `render` folder, create two files:

* `RenderQueue.h`
* `RenderQueue.cpp`

Add them to `CMakeLists` and include `RenderQueue.h` in `eng.h`.

---

### Step 2: Define the RenderQueue Class

Inside `RenderQueue`, we’ll store **draw commands**.

To represent each draw call, define a structure called `RenderCommand`:

```cpp
struct RenderCommand 
{
    Mesh* mesh;
    Material* material;
};
```

Now define the `RenderQueue` class with the following:

* A method `Submit()` to enqueue draw commands:

```cpp
void Submit(const RenderCommand& command);
```

* A container to store them:

```cpp
std::vector<RenderCommand> m_commands;
```

* A method to execute all queued draw calls:

```cpp
void Draw(GraphicsAPI& graphicsAPI);
```

---

### Step 3: Implement Submission and Drawing

In `Submit()`:

```cpp
m_commands.push_back(command);
```

In `Draw()`:

```cpp
for (const auto& command : m_commands) 
{
    graphicsAPI.BindMaterial(command.material);
    graphicsAPI.BindMesh(command.mesh);
    graphicsAPI.DrawMesh(command.mesh);
}
m_commands.clear();
```

---

### Step 4: Integrate with Engine

Open `Engine.h` and add a `RenderQueue` member:

```cpp
RenderQueue m_renderQueue;
```

Also add an accessor:

```cpp
RenderQueue& GetRenderQueue();
```

---

### Step 5: Rendering in Engine Loop

Open the `Engine::Run()` method. After the update logic and **before** `glfwSwapBuffers`, insert the rendering calls.

But first, we need to clear the screen and set a background color.

---

### Step 6: Add Buffer Clearing to GraphicsAPI

In `GraphicsAPI.h`, add:

```cpp
void SetClearColor(float r, float g, float b, float a);
void ClearBuffers();
```

Implement them as:

```cpp
void GraphicsAPI::SetClearColor(float r, float g, float b, float a) 
{
    glClearColor(r, g, b, a);
}

void GraphicsAPI::ClearBuffers() 
{
    glClear(GL_COLOR_BUFFER_BIT);
}
```

Now in `Engine::Run()`, insert before drawing:

```cpp
m_graphicsAPI.SetClearColor(1.0f, 1.0f, 1.0f, 1.0f); // White background
m_graphicsAPI.ClearBuffers();

m_renderQueue.Draw(m_graphicsAPI);
```

---

### Step 7: Submitting Render Commands from Game

Open `Game.cpp`, go to `Game::Update()` and prepare a render command:

```cpp
RenderCommand command;
command.mesh = m_mesh.get();
command.material = &m_material;

auto& renderQueue = eng::Engine::GetInstance().GetRenderQueue();
renderQueue.Submit(command);
```

---

### Step 8: Run and Verify

Build and run the application. If everything is working correctly, you should see your rectangle rendered on screen with smoothly interpolated vertex colors.

This confirms:

* The shader is working.
* The geometry is correctly defined.
* The rendering pipeline is functioning as intended.

---

### Summary

You now have:

* A **render queue** to organize draw calls.
* A way to submit **render commands** during update.
* A clean rendering phase that executes all commands.
* Clear screen functionality with customizable background color.