In the previous chapter, you learned the basics of building a game from scratch. You created a simple game that responds to keyboard input, plays a basic animation, and displays some text. Great!

In this chapter, we’ll begin building a real game engine around the knowledge you’ve gained so far. In other words, we’ll start wrapping your existing code and experience into the foundation of an actual game engine that can be used for future game development.

We’ll start where most engine developers begin — with the **main game loop**.

To implement it, we need to operate with two key entities:

1. **Engine** – This is the base class for the entire game engine. It will handle initialization, updating, and cleanup. Essentially, this will be the main controller of the engine.

2. **Application** (or **Game**) – This is the class you, as the developer, will directly interact with. In the next chapter, we’ll dive into building this class. I chose to call it `Application` rather than `Game`, because the engine we’re building could be used not only for games, but also for other types of interactive software like presentations, simulations, and more.

### Basic Structure

The `Engine` class will have three core functions:

* `Init()`
* `Run()`
* `Destroy()`

Internally, the engine will hold an instance of the `Application` class. In each of its methods, it will call the corresponding method in the `Application`. For example:

* `Engine::Init()` will call `Application::Init()`
* `Engine::Destroy()` will call `Application::Destroy()`

The `Run()` method is where the actual game loop happens. Inside it, we’ll use a `while` loop that calculates delta time, and calls something like `m_application.Update(deltaTime)`.

Since we’re now working on implementing an actual engine, it’s time to split it into a separate module.

### Project Structure

Let’s do the following:

1. Inside the root of your source code, create a new folder named `engine`.
2. Inside `engine`, create another folder called `source`.
3. In the `source` folder, create the following files:

   * `Engine.h`, `Engine.cpp`
   * `Application.h`, `Application.cpp`

Also, inside the `engine` folder, create a `CMakeLists.txt` file to define the engine module. Name the project `project(Engine)` and include the source files we just created. Don’t forget to set the `include_directories` to `source`, just like we did with the game project.

Unlike the game executable, we will compile the engine as a **library**, not an executable. So instead of `add_executable`, we’ll use `add_library`.

Next, go back to the main CMakeLists file of the game and remove the direct dependencies on GLFW and GLTF. These libraries should now be included through the engine module, so the user doesn't have to deal with them directly.

Then, move the actual source code for these third-party libraries (the `thirdparty` folder) into the `engine` directory. Make sure the engine module links to them.

### Final Steps

Now our engine is packaged as a proper module. We can start using it in the main game code by linking to the `engine` library, just like we previously linked to GLFW or GLTF.

Let’s regenerate the project, reload it, and try compiling.

Excellent — the project compiles successfully. The foundation of our engine is ready. Let’s move forward!