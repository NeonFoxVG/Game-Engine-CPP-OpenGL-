Alright, now let's begin implementing what we've discussed.

### 1. Updating `GameObject`

First, open the `GameObject` class.
Include the following GLM headers:

```cpp
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
```

Next, declare three private member variables:

```cpp
glm::vec3 m_position = glm::vec3(0.0f);
glm::vec3 m_rotation = glm::vec3(0.0f);
glm::vec3 m_scale    = glm::vec3(1.0f);
```

These represent the **position**, **rotation**, and **scale** components of the object.

Then, add getter and setter methods for each component:

```cpp
glm::vec3 GetPosition() const;
void SetPosition(const glm::vec3& pos);

glm::vec3 GetRotation() const;
void SetRotation(const glm::vec3& rot);

glm::vec3 GetScale() const;
void SetScale(const glm::vec3& scale);
```

Also, add two methods to compute the transformation matrices:

```cpp
glm::mat4 GetLocalTransform() const;
glm::mat4 GetWorldTransform() const;
```

---

### 2. Implementing Local Transform

The `GetLocalTransform` method combines position, rotation, and scale into a single matrix.

Here’s how it looks conceptually:

```cpp
glm::mat4 mat = glm::mat4(1.0f); // identity

mat = glm::translate(mat, m_position);

// Apply rotation around X, Y, and Z axes
mat = glm::rotate(mat, m_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
mat = glm::rotate(mat, m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
mat = glm::rotate(mat, m_rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

mat = glm::scale(mat, m_scale);

return mat;
```

This gives us the final **local transform** matrix.

---

### 3. Implementing World Transform

All objects exist in a hierarchy. So to compute a **world transform**, we must also consider the parent’s transformation:

```cpp
glm::mat4 GetWorldTransform() const 
{
    if (m_parent) 
    {
        return m_parent->GetWorldTransform() * GetLocalTransform();
    } 
    else 
    {
        return GetLocalTransform();
    }
}
```

This way, an object’s position is correctly computed relative to the global scene, not just its local space.

---

### 4. Updating the Shader

Previously, in the vertex shader, we used a `vec2` uniform for manual offset. Now, we’ll replace that with our **Model Matrix**.

Change the uniform declaration to:

```glsl
uniform mat4 uModel;
```

Then update vertex position transformation like this:

```glsl
gl_Position = uModel * vec4(position, 1.0);
```

This lets the GPU apply the full transform matrix instead of simple 2D offset.

---

### 5. Sending the Matrix to the Shader

To pass this matrix from C++ to the shader, go to your `ShaderProgram` class and add:

```cpp
void setUniform(const std::string& name, const glm::mat4& mat);
```

Implementation:

```cpp
GLint location = GetUniformLocation(name);
glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
```

This uploads the matrix to the GPU.

---

### 6. Updating `RenderCommand`

Now update the `RenderCommand` structure to include the model matrix:

```cpp
glm::mat4 modelMatrix;
```

This will store the world transform for each object.

---

### 7. Setting the Matrix Before Rendering

In your `RenderQueue::DrawAll()` method, when processing each render command, do the following:

* Get the shader from the material
* Use `SetUniform` to pass the model matrix

If your material doesn’t expose the shader program yet, add a method:

```cpp
ShaderProgram* GetShaderProgram();
```

Then call:

```cpp
command.material->GetShaderProgram()->SetUniform("uModel", command.modelMatrix);
```

---

### 8. Applying in the Game Logic

Now go to your test object or game update logic.
In the `Update()` method, when creating the `RenderCommand`, set:

```cpp
command.modelMatrix = GetWorldTransform();
```

This ensures every frame uses the latest transformation data.

Since we're now using `position` instead of manual offsets (`offsetX`, `offsetY`), you can remove those old variables.

Instead:

```cpp
auto position = GetPosition();

// Update position based on input
if (keyPressed) {
    position.x += 1.0f; // for example
}

SetPosition(position);
```

Then `GetWorldTransform()` will compute the final position automatically.

---

### 9. Testing

Now run your program. Visually, nothing changes at first — the object looks the same.

But if you press keys to move it, the object should now react correctly to input using the new transformation system.

---

### Summary

Now you have:

* A working `Transform` system with position, rotation, and scale.
* A matrix-based approach for rendering in 3D.
* Integration with shaders and rendering logic.
* Replaced manual offset logic with robust transformation math.

You’re one step closer to building a flexible 3D engine!