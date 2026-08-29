### Integrating the Engine into a Real Project

Let’s now start using the engine code we’ve written in a real project.

---

### Step 1: Creating a Public Header for the Engine

To begin using the engine, we first need to expose the relevant headers to the user. We’ll create a central include file for the engine’s public interface.

1. Go to the `engine/source` folder.
2. Create a new file named `eng.h`.
3. In this file, include all the public headers you want to expose. At this stage, we’ll include two main classes:

```cpp
#include "Application.h"
#include "Engine.h"
```

---

### Step 2: Update the Main Project to Use the Engine

Open your main project’s `CMakeLists.txt` and make sure it knows about the engine module. Specifically, add the engine’s `source` directory to the include path:

```cmake
include_directories(engine/source)
```

This will allow us to include the engine headers from anywhere in the project.

---

### Step 3: Creating a Custom Application

Next, we’ll define our own application by extending the engine’s `Application` class.

1. Go to your project’s `source` folder.
2. Create two files: `Game.h` and `Game.cpp`.
3. Add them to the `CMakeLists.txt` under the list of project sources.

#### `Game.h`

```cpp
#include <eng.h>

class Game : public eng::Application 
{
public:
    bool Init() override;
    void Update(float deltaTime) override;
    void Destroy() override;
};
```

#### `Game.cpp`

```cpp
#include "Game.h"
#include <iostream>

bool Game::Init() 
{
    return true;
}

void Game::Update(float deltaTime) 
{
    std::cout << "Current delta: " << deltaTime << std::endl;
}

void Game::Destroy() 
{
    // Cleanup if needed
}
```

---

### Step 4: Writing the Entry Point (main.cpp)

Now let’s initialize and run the game.

#### `main.cpp`

```cpp
#include "Game.h"
#include <eng.h>

int main() 
{
    Game* game = new Game();                // Create game instance
    eng::Engine engine;                     // Create engine instance

    engine.SetApplication(game);           // Pass game instance to engine

    if (engine.Init())                      // Initialize engine and game
    {                                       
        engine.Run();                      // Start main game loop
    }

    engine.Destroy();                      // Clean up resources
    return 0;
}
```

In this setup:

* We create an instance of `Game` and `Engine`.
* We pass the game instance to the engine using `engine.SetApplication(...)`.
* We initialize the engine (and the game).
* If initialization is successful, we enter the main loop with `engine.Run()`.
* After exiting the loop, we call `engine.Destroy()` to free up resources.

---

### Step 5: Testing the Loop

To verify that the loop is working and the update function is being called, we added a simple `std::cout` statement inside `Game::Update`, which prints the current delta time to the console every frame.

Now, build and run the project.

**Voilà!** Everything works.