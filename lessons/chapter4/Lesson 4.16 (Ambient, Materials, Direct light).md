# Lesson: Ambient Light, Materials, and Directional Sunlight

Let’s continue adding interesting features to our scene.
This time, we’ll improve **lighting, materials, and rendering** to make the scene more playable and visually clear.

---

## 1. Ambient Light (AMBIENT)

If you look at the current lighting, you’ll notice:

* We already have **diffuse lighting**,
* We already have **specular lighting**,
* But surfaces facing away from the light source are completely black, as if in total darkness.

In reality, there is always **ambient light** in the environment — some global brightness. Let’s add it.

Open `shaders/fragment.glsl`.
Inside the `main` function, add:

```glsl
const float ambientStrength = 0.4; 
vec3 ambient = ambientStrength * uLight.color;
```

This creates our ambient light.

Now, update the final result:

```glsl
vec3 result = (ambient + diffuse + specular) * textureColor.rgb;
```

Do the same in the **GraphicsAPI** setup for the default shader, so ambient is always applied.

---

## 2. Updating Mesh::CreateBox for UVs

Let’s improve `Mesh::CreateBox` so it generates **correct UV coordinates**.

We’ll assume the default texture is designed for a **1×1×1 cube**.
That means we must map UVs in a way that works nicely with textures (they’ll repeat automatically across scaled boxes).

Update the UVs inside `CreateBox` to assign proper texture coordinates:

```cpp
std::shared_ptr<Mesh> Mesh::CreateBox(const glm::vec3& extents)
{
    const auto half = extents * 0.5f;
    std::vector<float> vertices =
    {
        // Front face
        half.x, half.y, half.z, 1.0f, 0.0f, 0.0f, extents.x, extents.y, 0.0f, 0.0f, 1.0f,
        -half.x, half.y, half.z, 0.0f, 1.0f, 0.0f, 0.0f, extents.y, 0.0f, 0.0f, 1.0f,
        -half.x, -half.y, half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        half.x, -half.y, half.z, 1.0f, 1.0f, 0.0f, extents.x, 0.0f, 0.0f, 0.0f, 1.0f,

        // Top face 
        half.x, half.y, -half.z, 1.0f, 0.0f, 0.0f, extents.x, extents.z, 0.0f, 1.0f, 0.0f,
        -half.x, half.y, -half.z, 0.0f, 1.0f, 0.0f, 0.0f, extents.z, 0.0f, 1.0f, 0.0f,
        -half.x, half.y, half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        half.x, half.y, half.z, 1.0f, 1.0f, 0.0f, extents.x, 0.0f, 0.0f, 1.0f, 0.0f,

        // Right face
        half.x, half.y, -half.z, 1.0f, 0.0f, 0.0f, extents.z, extents.y, 1.0f, 0.0f, 0.0f,
        half.x, half.y, half.z, 0.0f, 1.0f, 0.0f, 0.0f, extents.y, 1.0f, 0.0f, 0.0f,
        half.x, -half.y, half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        half.x, -half.y, -half.z, 1.0f, 1.0f, 0.0f, extents.z, 0.0f, 1.0f, 0.0f, 0.0f,

        // Left face
        -half.x, half.y, half.z, 1.0f, 0.0f, 0.0f, extents.z, extents.y, -1.0f, 0.0f, 0.0f,
        -half.x, half.y, -half.z, 0.0f, 1.0f, 0.0f, 0.0f, extents.y, -1.0f, 0.0f, 0.0f,
        -half.x, -half.y, -half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        -half.x, -half.y, half.z, 1.0f, 1.0f, 0.0f, extents.z, 0.0f, -1.0f, 0.0f, 0.0f,

        // Bottom face
        half.x, -half.y, half.z, 1.0f, 0.0f, 0.0f, extents.x, extents.z, 0.0f, -1.0f, 0.0f,
        -half.x, -half.y, half.z, 0.0f, 1.0f, 0.0f, 0.0f, extents.z, 0.0f, -1.0f, 0.0f,
        -half.x, -half.y, -half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        half.x, -half.y, -half.z, 1.0f, 1.0f, 0.0f, extents.x, 0.0f, 0.0f, -1.0f, 0.0f,

        // Back face
        -half.x, half.y, -half.z, 1.0f, 0.0f, 0.0f, extents.x, extents.y, 0.0f, 0.0f, -1.0f,
        half.x, half.y, -half.z, 0.0f, 1.0f, 0.0f, 0.0f, extents.y, 0.0f, 0.0f, -1.0f,
        half.x, -half.y, -half.z, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        -half.x, -half.y, -half.z, 1.0f, 1.0f, 0.0f, extents.x, 0.0f, 0.0f, 0.0f, -1.0f
    };
    // ...
}
```

---

## 3. Checker Texture

I’ve prepared a simple **checker texture**, which is very common in engines for testing.

It’s called `checker.png` and is placed in `assets/textures/`.

Now, let’s create a new material:

* Copy `brick.mat`,
* Replace `baseColorTexture` with `textures/checker.png`.

That’s our **checker material**.

---

## 4. Building a Scene with Checker Boxes

Let’s build a **bigger and more interesting scene** using our new material.

Delete everything in `scene.cs` and instead create a new scene that contains **multiple boxes** with the checker material:

* Some boxes scattered around,
* A few stacked on top of each other (so they can fall when you shoot them),
* Four walls surrounding the area,
* A simple platform.

When you play, you’ll see boxes tumbling and falling apart when hit by bullets.
However, all objects still share the same checker texture, which sometimes makes them hard to distinguish.

---

## 5. Adding Color Parameters to Materials

Let’s add **color overrides** so we can tint objects differently.

### Step 1: Material storage

In `Material` class, add:

```cpp
std::unordered_map<std::string, glm::vec3> m_float3Params;
```

This stores `vec3` parameters like colors.

Add:

```cpp
void SetParam(const std::string& name, const glm::vec3& param) 
{
    m_float3Params[name] = param;
}
```

### Step 2: Binding

In `Material::Bind()`, after binding textures, iterate parameters:

```cpp
for (auto& param : m_float3Params) 
{
    m_shaderProgram->SetUniform(param.first, param.second);
}
```

Now materials can pass custom values into shaders.

### Step 3: Loading from JSON

In `Material::Load()`, check for `"float3"` parameters:

```cpp
if (paramsObj.contains("float3")) 
{
    for (auto& p : paramsObj["float3"]) 
    {
        std::string name = p.value("name", "");
        float v0 = p.value("value0", 0.0f);
        float v1 = p.value("value1", 0.0f);
        float v2 = p.value("value2", 0.0f);
        result->SetParam(name, glm::vec3(v0, v1, v2));
    }
}
```

### Step 4: Overriding in MeshComponent

In `MeshComponent::LoadProperties()`, instead of only reading `"material"` as a string, support an object:

```json
"material": {
  "path": "materials/checker.mat",
  "params": {
    "float3": [
      { "name": "color", "value0": 1, "value1": 0, "value2": 0 }
    ]
  }
}
```

In `fragment.glsl` add a uniform `uniform vec3 color;` and use in a final color calculation:

```glsl
vec3 result = (ambient + diffuse + specular) * texColor.xyz * color;
```

This way we can load a base material and then override its parameters (e.g. tint color).

Now we can make different boxes different colors, even if they use the same checker texture.

---

## 6. Directional Light (Sunlight)

Right now our light is a **point light**. Let’s switch to a **directional light**, like the sun.

### Shader changes

In `fragment.glsl`, in the `Light` struct, replace position-based logic with a direction:

```glsl
vec3 lightDir = normalize(-uLight.direction);
```

So instead of computing light direction from position to fragment, we directly use a fixed direction.

### Render Queue

In the `RenderQueue` setup for default shader, replace:

```cpp
shader->SetUniform("uLight.direction", glm::normalize(-light.position));
```

Now the light acts like parallel rays from the sun.

---

## Result

Launch the game:

* Objects are now brighter thanks to **ambient light**,
* Checker texture is applied everywhere,
* Boxes can be tinted with **custom colors**,
* Lighting comes from a **directional sun source**.

Your scene is more colorful, customizable, and realistic