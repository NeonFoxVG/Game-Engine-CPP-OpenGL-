**Alright!**
Now let’s finally step into a true 3D space and start drawing actual three-dimensional objects instead of the 2D ones we’ve been rendering so far. Let’s make our rectangle grow and evolve into a cube. This way, our flat, two-dimensional shape will become fully three-dimensional.

**How do we do that?**
As you remember, our rectangle was defined by four vertices. We can take that plane (which was sitting at Z = 0), move it slightly backward by half a unit, then duplicate it and move the copy forward by half a unit.
This gives us the *front* and *back* faces of a cube. Combined, they form a solid shape with eight vertices.

Since we already have a place to store these vertices — the `vertices` array in `TestObject` — we can just update it.
Here’s the plan:

* Copy the existing 4 vertices.
* For the first 4 vertices (front face), set the Z coordinate to `+0.5`.
* For the duplicated 4 vertices (back face), set the Z coordinate to `-0.5`.

**Next, the indices.**
Indices define the order in which vertices are connected to form triangles. This part is a bit trickier, so follow me carefully.

We replace the `indices` array as follows:

* **Front face:** `0, 1, 2` and `0, 2, 3`
* **Top face:** `4, 5, 1` and `4, 1, 0`
* **Right face:** `4, 0, 3` and `4, 3, 7`
* **Left face:** `1, 5, 6` and `1, 6, 2`
* **Bottom face:** `3, 2, 6` and `3, 6, 7`
* **Back face:** `4, 7, 6` and `4, 6, 5`

That’s it — now our coordinates and indices define a cube.
The vertex layout hasn’t changed, so our mesh component will automatically calculate vertex and index counts.

---

### Before testing — a word about the Depth Buffer

Now that we’re moving to 3D, we need to talk about something important: the **depth buffer**, also known as the **Z-Buffer**.

What is it?
Think of it as an invisible grayscale image, exactly the same size as your screen. For every pixel you draw, the depth buffer stores its **Z-coordinate** (how far it is from the camera).
When the next object is drawn, the GPU checks its depth against what’s already stored in the buffer:

* If it’s closer to the camera, it replaces the pixel.
* If it’s farther away, it’s hidden behind what’s already there.

**Why do we need it?**
Imagine you first draw a plane that’s closer to the camera, then a sphere behind it. Without a depth buffer, the sphere might incorrectly draw over the plane just because it’s rendered later. With the depth buffer enabled, the GPU can correctly determine that the sphere is behind the plane and hide it accordingly.

---

### Enabling Depth Testing

By default in OpenGL, depth testing is **disabled**, so we need to turn it on.

In `GraphicsAPI.h` (or `.cpp`), add a method:

```cpp
bool Init() 
{
    glEnable(GL_DEPTH_TEST);
    return true;
}
```
And in `Engine::Init()` insert it before `m_application->Init()`

Also, when clearing the screen, we must clear the depth buffer **in addition** to the color buffer, otherwise old depth values will mess things up.
In `ClearBuffers`, change:

```cpp
glClear(GL_COLOR_BUFFER_BIT);
```

to:

```cpp
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
```

Now we’re ready to draw 3D objects properly.

---

### Testing the Cube

Run the project and move the camera — you should now see a **3D cube** instead of a flat rectangle.

---

### Drawing Multiple Objects

Let’s take it one step further: render **several objects** in the scene.

The easiest (but not the most efficient) way is to just create multiple `TestObject` instances. But here’s the catch — every `TestObject` constructor creates a new mesh and a new material, which means we’re duplicating GPU resources unnecessarily.

**Better approach:**

* Create a single shared `Mesh` and `Material` in `Game::Init()`.
* Pass these to all the objects you create.

Example:

```cpp
auto mesh = std::make_shared<Mesh>(...);
auto material = std::make_shared<Material>(...);

auto objectA = m_scene->CreateObject();
objectA->AddComponent(new MeshComponent(material, mesh));
objectA->SetPosition({0, 0, 0});

auto objectB = m_scene->CreateObject();
objectB->AddComponent(new MeshComponent(material, mesh));
objectB->SetPosition({2, 0, 0});
objectB->SetRotation({1, 0, 0});

auto objectC = m_scene->CreateObject();
objectC->AddComponent(new MeshComponent(material, mesh));
objectC->SetPosition({-2, 0, 0});
objectC->SetScale({1.5f, 1.5f, 1.5f});
```

Run it — you’ll see multiple cubes in the scene, all drawn efficiently using the same mesh and material.