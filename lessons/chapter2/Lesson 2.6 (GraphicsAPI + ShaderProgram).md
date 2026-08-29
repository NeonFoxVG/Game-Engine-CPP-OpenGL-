Great! Now it’s time to dive into graphics. Let’s standardize our graphics handling and wrap it in a set of universal wrappers that our engine will use.

We’ll start with the shader program. Inside the `source` directory of our engine, create a new folder named `graphics`. Within that folder, create two files: `ShaderProgram.h` and `ShaderProgram.cpp`. Add these two files to `CMakeLists` of the engine and also include them in `eng.h`.

Perfect. Now open `ShaderProgram.h` and define the `ShaderProgram` class.

First, it will contain a field: `GLuint m_shaderProgramID`. This is the actual ID of the shader program created by OpenGL.

Next, we’ll add a `Bind()` method, which will internally call the OpenGL function `glUseProgram(m_shaderProgramID)`.

We'll also add a `GetUniformLocation()` method, which will use a local cache to avoid redundant OpenGL calls. To implement the cache, use:

```cpp
std::unordered_map<std::string, GLint> m_uniformLocationCache;
```

In `GetUniformLocation`, we’ll first check if the location is already cached:

```cpp
auto it = m_uniformLocationCache.find(name);
```

If not found, we get the location with:

```cpp
GLint location = glGetUniformLocation(m_shaderProgramID, name.c_str());
```

Then we store it in the cache:

```cpp
m_uniformLocationCache[name] = location;
```

If it *is* found, we simply return the cached value.

We’ll also implement a `SetUniform()` method that takes a `std::string` and a `float` value. This method will retrieve the location and, if valid, set the uniform:

```cpp
glUniform1f(location, value);
```

Next, add an explicit constructor that takes a `GLuint shaderProgramID`. Mark the default constructor and copy constructor as deleted. In the destructor, delete the shader program using `glDeleteProgram`.

We’re making the constructors deleted intentionally to avoid copying the shader object. It ensures that the shader program is created and destroyed exactly once, avoiding potential bugs or crashes.

---

Now, we need a way to *create* a shader program. I’ve decided to centralize the resource management for graphics components. Let’s create a class called `GraphicsAPI` which will serve as a wrapper around all core graphics operations.

In the same `graphics` folder, create two new files: `GraphicsAPI.h` and `GraphicsAPI.cpp`. Add these files to `CMakeLists` of the engine and include them in `eng.h`.

As mentioned, `GraphicsAPI` will serve as a centralized interface for rendering operations. Define the `GraphicsAPI` class accordingly.

Just like with `InputManager`, we’ll store a single instance of `GraphicsAPI` inside the `Engine` class. So, open `Engine.h`, declare a `GraphicsAPI` member, and add a `GetGraphicsAPI()` method that returns a reference to it.

Back in `GraphicsAPI`, implement a method to create a shader program:

```cpp
std::shared_ptr<ShaderProgram> CreateShaderProgram(const std::string& vertexSource, const std::string& fragmentSource);
```

This method will receive the source code for the vertex and fragment shaders, compile them, link them into a shader program, and return a new `ShaderProgram` instance.

After successful compilation and linking, we wrap the resulting program ID in a `ShaderProgram` object and return it via `std::make_shared`.

Next, add a method:

```cpp
void BindShaderProgram(ShaderProgram* shader);
```

This method simply calls `shader->Bind()`. This lets us bind shaders both directly and via the unified `GraphicsAPI`.

---

Let’s now test shader creation.

Open `Game.cpp` and go to the `Init()` function. Paste in your vertex and fragment shader source code (used earlier). Then write:

```cpp
auto& graphicsAPI = Engine::getInstance().getGraphicsAPI();
auto shaderProgram = graphicsAPI.createShaderProgram(vertexShaderSource, fragmentShaderSource);
```

Compile and run the application to confirm that the shader program is created successfully and holds valid data.

Done! The shader program is working, and everything looks great.
Thanks!