Alright, now I’d like to talk about making camera rotations more convenient.
As you remember, up to this point we’ve been using three rotation angles — one for each axis X, Y, and Z — to set the orientation of our camera (and any object in general).

At first glance, this seems simple: with these three angles we can describe any orientation in space.
But here’s the catch — the order matters.

If you look inside the `GameObject` class, you’ll see that the rotation matrix is built by applying three rotations one after another, each around its corresponding axis.
And here’s the question — what’s the *correct* order to use?
Because if we change the order, the object’s final orientation changes as well.

For example:

* Rotate around X first, then Y → one orientation.
* Rotate around Y first, then X → a completely different orientation.

In some cases, we might need one order, in others — another. This can create conflicting situations when we want to achieve a specific rotation.

So… how do we solve this?

I have a great answer: instead of using three separate rotation angles, we can use a single entity called a **quaternion**.

---

### What’s a quaternion?

A quaternion consists of four components: `x`, `y`, `z`, and `w`.
In essence, a quaternion is a mathematical object that stores an orientation in 3D space.

It defines an **axis** around which the object is rotated, and an **angle** describing how much rotation occurs.
You can think of `x`, `y`, and `z` as defining the axis of rotation, while `w` represents the amount of rotation (the angle).

So instead of three independent angles (one per axis), we have one quaternion that stores the orientation cleanly and avoids the rotation-order problem entirely.

---

### Converting GameObject to use quaternions

We’ll go to the `GameObject` class and change the `m_rotation` member from `vec3` to `quat`.
By default, it will be initialized to:

```cpp
glm::quat(1.0f, 0.0f, 0.0f, 0.0f)
```

This is the **identity quaternion**, meaning “no rotation.”

We also update our `GetRotation` and `SetRotation` methods accordingly.

Next, in `GetLocalTransform`, we remove the old code that applied three separate rotations in sequence.
Instead, we just write:

```cpp
mat = mat * glm::mat4_cast(m_rotation);
```

Here, `glm::mat4_cast` simply converts our quaternion into a rotation matrix.
This single step replaces all the earlier multi-axis rotation logic.

---

### Updating CameraComponent

Now let’s update `CameraComponent`’s `GetViewMatrix` method.

We start with an identity matrix:

```cpp
glm::mat4 mat = glm::mat4(1);
```

For the camera, the transformation order is slightly different from a normal object.
First, we apply the rotation:

```cpp
mat = glm::mat4_cast(m_owner->GetRotation());
```

Then, we apply the translation:

```cpp
mat = glm::translate(mat, m_owner->GetPosition());
```

We don’t apply scale, because camera scaling doesn’t make sense for view transformations.

If the camera has a parent, we multiply by the parent’s world transform:

```cpp
if (m_owner->GetParent())
    mat = m_owner->GetParent()->GetWorldTransform() * mat;
```

Finally, we invert the matrix:

```cpp
return glm::inverse(mat);
```

---

### Updating PlayerController for quaternion rotations

In `PlayerControllerComponent::Update`, we change how the camera rotation reacts to mouse movement.

Before writing code, let’s review how the camera should rotate:

A camera has three local basis vectors:

* **Front** — the direction the camera is looking.
* **Right** — the direction to the right from the camera’s perspective.
* **Up** — the upward direction relative to the camera.

When rotating:

* Moving the mouse horizontally (ΔX) should rotate around the **global** Y-axis (world up).
* Moving the mouse vertically (ΔY) should rotate around the **local** right axis of the camera.

---

### Horizontal rotation (around global Y)

```cpp
float yAngle = -deltaX * m_sensitivity * deltaTime;
glm::quat yRot = glm::angleAxis(yAngle, glm::vec3(0.0f, 1.0f, 0.0f));
```

---

### Vertical rotation (around local right axis)

```cpp
float xAngle = -deltaY * m_sensitivity * deltaTime;
glm::vec3 right = rotation * glm::vec3(1.0, 0.0, 0.0);
glm::quat xRot = glm::angleAxis(xAngle, right);
```

---

### Combining the rotations

We multiply them:

```cpp
glm::quat deltaRot = yRot * xRot;
rotation = glm::normalize(deltaRot * rotation);
m_owner->SetRotation(rotation);
```

---

### Movement stays mostly the same

We still get the updated rotation, then derive the front and right vectors:

```cpp
glm::vec3 front = rotation * glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 right = rotation * glm::vec3(1.0f, 0.0f, 0.0f);
```

These are then used for movement input as before.

---

### Testing

When we run the project now, the camera moves and rotates exactly like a proper FPS (first-person shooter) camera.
We’ve eliminated the rotation-order problem entirely, and rotations feel much smoother and more predictable.

Congratulations — we’ve now covered the basics of working with scenes, positioning objects, controlling the camera, and moving through the world!