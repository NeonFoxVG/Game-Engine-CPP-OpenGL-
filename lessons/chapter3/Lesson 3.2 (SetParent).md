Now let’s implement the core logic behind **assigning parents** to objects in the scene graph.

We’ll cover various scenarios, such as:

* Moving a `GameObject` to the **scene root**
* Reparenting to another object
* Avoiding **circular dependencies** in the hierarchy

---

### Step-by-Step: Implementing `SetParent()`

We'll begin with:

```cpp
bool result = false;
auto currentParent = obj->GetParent();
```

This gives us the current parent of the object we want to reassign.

---

### **Case 1: Assigning to Root (i.e., `parent == nullptr`)**

```cpp
if (parent == nullptr) {
```

We want to move the object to the root of the scene.

#### Subcase 1.1: Object already has a parent

```cpp
if (currentParent != nullptr) {
```

We need to:

1. Remove it from the current parent's child list.
2. Add it to the root list (`m_objects`).

First, locate the object inside its current parent:

```cpp
auto it = std::find_if(
    currentParent->m_children.begin(),
    currentParent->m_children.end(),
    [obj](const std::unique_ptr<GameObject>& el) {
        return el.get() == obj;
    });
```

If found:

```cpp
if (it != currentParent->m_children.end()) 
{
    m_objects.push_back(std::move(*it));
    obj->m_marent = nullptr;
    currentParent->m_children.erase(it);
    result = true;
}
```

#### Subcase 1.2: Object has no parent, but might not be in the scene yet

If the object is not found in the root list, it means it's **new**. We add it:

```cpp
auto it = std::find_if(
    m_objects.begin(), m_objects.end(),
    [obj](const std::unique_ptr<GameObject>& el) 
    {
        return el.get() == obj;
    });

if (it == m_objects.end()) 
{
    std::unique_ptr<GameObject> objHolder(obj);
    m_objects.push_back(std::move(objHolder));
    result = true;
}
```

---

### **Case 2: Assigning to a Valid Parent (i.e., `parent != nullptr`)**

```cpp
else {
```

We are trying to add the object as a child to another `GameObject`.

---

#### Subcase 2.1: Object already has a parent (reparenting)

```cpp
if (currentParent != nullptr) {
```

First, locate the object in its current parent's children:

```cpp
auto it = std::find_if(
    currentParent->m_children.begin(),
    currentParent->m_children.end(),
    [obj](const std::unique_ptr<GameObject>& el) {
        return el.get() == obj;
    });

if (it != currentParent->m_children.end()) {
```

Now, **ensure there’s no circular parenting**:
We must not allow assigning a child to one of its own descendants.

```cpp
bool found = false;
auto currentElement = parent;
while (currentElement) 
{
    if (currentElement == obj) 
    {
        found = true;
        break;
    }
    currentElement = currentElement->GetParent();
}
```

If everything is valid:

```cpp
if (!found) 
{
    parent->m_children.push_back(std::move(*it));
    obj->m_parent = parent;
    currentParent->m_children.erase(it);
    result = true;
}
```

---

#### Subcase 2.2: Object is currently in the root list or has been just created

If `currentParent == nullptr`, then:

```cpp
auto it = std::find_if(
    m_objects.begin(), m_objects.end(),
    [obj](const std::unique_ptr<GameObject>& el) {
        return el.get() == obj;
    });

```

Object has been just created

```cpp
if (it == m_objects.end()) 
{
    std::unique_ptr<GameObject> objHolder(obj);
    parent->m_children.push_back(std::move(objHolder));
    obj->m_parent = parent;
    result = true;
}
else {
```

Then, check again for **circular parenting**:

```cpp
bool found = false;
auto currentElement = parent;

while (currentElement) 
{
    if (currentElement == obj) 
    {
        found = true;
        break;
    }
    currentElement = currentElement->GetParent();
}
```

If the new parent is valid:

```cpp
if (!found) 
{
    parent->m_children.push_back(std::move(*it));
    obj->m_parent = parent;
    m_objects.erase(it);
    result = true;
}
```

---

### ✅ Final Step: Return the Result

```cpp
return result;
```

---

### Summary

This `SetParent()` logic now safely supports:

* Moving objects into or out of scene root
* Reassigning between parents
* Detecting and preventing circular parenting (which would break the tree structure)