# Improving the Scene: Adding Bullets and Better Scene Management

Now let’s make our scene a bit more **playable** and also improve some of the systems we already have.

We’ll start with **rendering** and then move on to **scene management** and finally add **bullets** that we can shoot.

---

## 1. Rendering Improvements

Let’s go into the `Mesh` class and add a new function:

```cpp
void Mesh::Unbind() 
{
    glBindVertexArray(0);
}
```

Why do we need this?
After we finish rendering a mesh, we don’t need to keep it active anymore. Unbinding is more correct from a logical point of view.

Next, go into `GraphicsAPI` and add a helper:

```cpp
void GraphicsAPI::UnbindMesh(Mesh* mesh) 
{
    if (mesh) 
    {
        mesh->Unbind();
    }
}
```

Finally, open `RenderQueue`, find the `DrawAll` function. After calling:

```cpp
m_graphicsAPI.DrawMesh(command.mesh);
```

we should immediately unbind it:

```cpp
m_graphicsAPI.UnbindMesh(command.mesh);
```

Great. Rendering is now cleaner.

---

## 2. Scene Improvements

You remember that objects are removed using **MarkForDestroy**. Let’s make this safer.

1. In the `Scene` class, add:

```cpp
bool m_isUpdating = false;
```

2. Inside `Scene::Update`:

   * At the start, set `m_isUpdating = true;`.
   * At the end, set `m_isUpdating = false;`.

3. Before running the main update, remove all dead objects:

```cpp
m_objects.erase(
    std::remove_if(m_objects.begin(), m_objects.end(),
        [](auto& obj) { return !obj->IsAlive(); }),
    m_objects.end()
);
```

Now we’re safe from objects lingering after destruction.

---

### Handling new objects during update

What if we create new objects **while updating the scene**?
For that, let’s add a temporary container:

```cpp
std::vector<std::pair<GameObject*, GameObject*>> m_objectsToAdd;
```

This will store **candidates** (the new object + its parent).

In every `CreateObject` function, instead of directly calling `SetParent`, do this:

```cpp
if (m_isUpdating) 
{
    m_objectsToAdd.push_back({ obj, parent });
} 
else 
{
    SetParent(obj, parent);
}
```

And back in `Scene::Update`, after removing dead objects, we process new ones:

```cpp
for (auto& obj : m_objectsToAdd) 
{
    SetParent(obj.first, obj.second);
}
m_objectsToAdd.clear();
```

Perfect. Now we can safely add objects during scene update.

---

## 3. Adding Bullets

Let’s make our player actually **shoot projectiles**.

### Step 1: Expose Rigidbody

In `PhysicsComponent`, add:

```cpp
const std::shared_ptr<RigidBody>& GetRigidBody() const { return m_rigidBody; }
```

---

### Step 2: Create Bullet Class

In gameplay code, add:

```cpp
class Bullet : public eng::GameObject 
{
    GAMEOBJECT(Bullet) // our macro
public:
    void Update(float dt) override 
    {
        eng::GameObject::Update(dt);
        m_lifetime -= dt;
        if (m_lifetime <= 0.0f)
        {
            MarkForDestroy();
        }
    }
private:
    float m_lifetime = 2.0f; // bullet lives for 2 seconds
};
```

So bullets automatically disappear after 2 seconds.

---

### Step 3: Player Shooting Code

In `Player::Update`, where we already play the **shoot animation** and **shoot sound**, add:

```cpp
auto bullet = m_scene->CreateObject<Bullet>("Bullet");
```

Next, give the bullet a **material**:

```cpp
auto material = eng::Material::Load("materials/suzanne.mat");
```

And a **mesh**. For bullets, let’s use a **sphere**.
So in `Mesh`, add:

```cpp
static std::shared_ptr<Mesh> CreateSphere(float radius, int sectors, int stacks);
```

This function generates vertices, indices, normals, and UVs for a sphere and returns a mesh.

Then in `Player`:

```cpp
auto mesh = eng::Mesh::CreateSphere(0.2f, 32, 32);
bullet->AddComponent(new eng::MeshComponent(material, mesh));
```

---

### Step 4: Spawn Position

We need the bullet to spawn from the weapon’s muzzle.
I’ve picked a point manually — just repeat it:

```cpp
auto pos = FindChildByName("BOOM_35")->GetWorldPosition();
bullet->SetPosition(pos + m_rotation * glm::vec3(-0.2f, 0.2f, -1.75f));
```

---

### Step 5: Physics for the Bullet

Add a collider and rigidbody:

```cpp
auto collider = std::make_shared<eng::SphereCollider>(0.2f);
auto rigidbody = std::make_shared<eng::RigidBody>(
    eng::BodyType::Dynamic, collider, 10.0f, 0.1f
);

bullet->AddComponent(new eng::PhysicsComponent(rigidbody));
```

---

### Step 6: Make it Fly

In `RigidBody`, add:

```cpp
void ApplyImpulse(const glm::vec3& impulse) 
{
    m_body->applyCentralImpulse(btVector3(impulse.x, impulse.y, impulse.z));
}
```

Now in `Player::Update`:

```cpp
glm::vec3 front = m_rotation * glm::vec3(0, 0, -1);
rigidbody->ApplyImpulse(front * 500.0f);
```

---

## Result

Now when you shoot:

* A sphere (bullet) spawns at the weapon’s muzzle,
* It gets a **mesh**, a **physics body**, and a **collider**,
* It flies forward using **impulse**,
* After 2 seconds, it disappears.

Congratulations!
You now have a working shooting system with physical bullets.