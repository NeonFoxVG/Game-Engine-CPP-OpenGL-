Great! Now let’s wrap our core rendering objects into **scene-level objects** and introduce the concept of a **Scene** and its **GameObjects**.

---

### **Scene and GameObject Overview**

As mentioned in an earlier lesson, all 3D objects are positioned and rendered within a **scene**. This concept is widely used in game development.

A **scene** is a container for all objects in a particular game context. We will implement it as a **hierarchical tree** structure.

---

#### Analogy: A House

Imagine a **house** as the root object. Inside it, you have furniture like a **table** and **chair** — these are its children.

* Moving the table doesn't affect the house.
* But moving the house also moves the table and chair, since they are children of the house.

This hierarchy allows organized manipulation and transformation of grouped objects.

---

### Step 1: Scene Folder and Files

Create a new folder inside your engine’s `source` directory called `scene`.

Inside it, create the following files:

* `GameObject.h`
* `GameObject.cpp`
* `Scene.h`
* `Scene.cpp`

Update `CMakeLists.txt` and add the headers to `eng.h`.

---

### Step 2: Implement `GameObject` Class

The core unit of a scene is the `GameObject`. It should have:

#### Fields:

```cpp
std::string m_name;
GameObject* m_parent = nullptr;
std::vector<std::unique_ptr<GameObject>> m_children;
bool m_isAlive = true;
```

Make the `Scene` class a friend of `GameObject`.

#### Methods:

```cpp
virtual ~GameObject() = default;
virtual void Update(float deltaTime);
const std::string& GetName() const;
void SetName(const std::string& name);
GameObject* GetParent();
bool IsAlive() const;
void MarkForDestroy();
```

Make the **default constructor protected** to prevent direct instantiation (forces use through factory methods).

---

### Step 3: Implement `GameObject` Methods

In `GameObject.cpp`, define the methods:

#### `Update()`:

```cpp
void GameObject::Update(float deltaTime) 
{
    for (auto it = m_children.begin(); it != m_children.end(); ) 
    {
        if ((*it)->IsAlive()) 
        {
            (*it)->Update(deltaTime);
            ++it;
        } 
        else 
        {
            it = m_children.erase(it);
        }
    }
}
```

Other simple methods:

```cpp
const std::string& GameObject::GetName() const 
{ 
    return m_name; 
}

void GameObject::SetName(const std::string& name) 
{ 
    m_name = name; 
}

GameObject* GameObject::GetParent() 
{ 
    return m_parent; 
}

bool GameObject::IsAlive() const 
{ 
    return m_isAlive; 
}

void GameObject::MarkForDestroy() 
{ 
    m_isAlive = false; 
}
```

---

### Step 4: Implement `Scene` Class

In `Scene.h`, define the class `Scene`:

#### Fields:

```cpp
std::vector<std::unique_ptr<GameObject>> m_objects;
```

#### Methods:

```cpp
void Update(float deltaTime);
void Clear();

GameObject* CreateObject(const std::string& name, GameObject* parent = nullptr);

template <typename T,
          typename = std::enable_if_t<std::is_base_of<GameObject, T>::value>::type>
T* CreateObject(const std::string& name, GameObject* parent = nullptr);
```

---

### Step 5: Implement Scene Methods

#### `Update()`:

```cpp
void Scene::Update(float deltaTime) 
{
    for (auto it = m_objects.begin(); it != m_objects.end(); ) 
    {
        if ((*it)->IsAlive()) 
        {
            (*it)->Update(deltaTime);
            ++it;
        } 
        else 
        {
            it = m_objects.erase(it);
        }
    }
}
```

#### `Clear()`:

```cpp
void Scene::Clear() 
{
    m_objects.clear();
}
```

#### `CreateObject()`:

```cpp
GameObject* Scene::CreateObject(const std::string& name, GameObject* parent) 
{
    auto obj = new GameObject();
    obj->SetName(name);
    SetParent(obj, parent);
    return obj;
}
```

#### Template version for derived types:

```cpp
template <typename T,
          typename = std::enable_if_t<std::is_base_of<GameObject, T>::value>::type>
T* Scene::createObject(const std::string& name, GameObject* parent) 
{
    auto obj = new T();
    obj->SetName(name);
    SetParent(obj, parent);
    return obj;
}
```

---

### Next Step: `SetParent()` Implementation

I've mentioned mentioned `SetParent()` method — we’ll implement that next. It will allow dynamic reparenting of game objects and should handle removing the object from the old parent's children vector and adding it to the new one.
