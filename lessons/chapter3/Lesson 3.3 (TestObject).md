Now let’s use our newly created **Scene** and **GameObject** system inside the game.

---

### Step 1: Create a Test Object

Inside your game code, create a new class to test scene functionality. Add two files:

* `TestObject.h`
* `TestObject.cpp`

Also, add them to your `CMakeLists.txt`.

---

### Step 2: Define the `TestObject` Class

In `TestObject.h`:

```cpp
#include "eng.h"

class TestObject : public eng::GameObject 
{
public:
    TestObject();

    void Update(float deltaTime) override;

private:
    std::shared_ptr<eng::Mesh> m_mesh;
    eng::Material m_material;
    float m_offsetX = 0.0f;
    float m_offsetY = 0.0f;
};
```

This class inherits from our engine’s `GameObject`.

---

### Step 3: Implement the Constructor

In `TestObject.cpp`, move **all logic previously in `Game`** (such as mesh and material setup) into the constructor of `TestObject`.

```cpp
TestObject::TestObject() 
{
    // Setup shader program
    // Setup geometry (vertices + indices)
    // Create VertexLayout
    // Create Mesh
    // Setup Material and bind shader
}
```

Use `std::make_shared` to initialize the mesh as a `shared_ptr`.

---

### Step 4: Implement `Update()`

Inside `TestObject::Update()`:

1. **Call base update first**:

```cpp
eng::GameObject::Update(deltaTime);
```

2. Move the previous logic from `Game::Update` here

---

### Step 5: Add Scene to the Game

Open `Game.h` and add:

```cpp
eng::Scene m_scene;
```

In `Game.cpp`, inside `Game::Init()`:

1. Include `TestObject.h`:

```cpp
#include "TestObject.h"
```

2. Create the object through the scene:

```cpp
m_scene.createObject<TestObject>("TestObject");
```

---

### Step 6: Update the Scene in Game Loop

In `Game::Update()`:

```cpp
m_scene.Update(deltaTime);
```

---

### Step 7: Build and Run

Compile and run the game. You should still see the rectangle reacting to keyboard input, just like before.

But now:

✅ All logic is encapsulated inside a reusable `GameObject`
✅ You’re managing behavior and rendering **through the scene system**
✅ The code is modular, scalable, and follows real engine design

---

### ✅ Summary

You’ve just completed a major architectural step:

* Moved object logic into a custom `GameObject`-derived class
* Integrated the scene system into the game
* Cleanly separated rendering, object logic, and input
* Preserved all existing functionality

This is exactly how real engines like Unreal Engine structure their gameplay objects and scene management. Well done!