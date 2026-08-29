# Lesson 6 — Adding Light (Diffuse Lighting)

As you’ve noticed, all our objects — whether it’s a cube or a loaded 3D model like Suzanne — are drawn in 3D space, but they look… flat.

What gives them the illusion of volume? **Lighting.**

Lighting is what creates shadows and highlights, what makes those tiny bumps, dents, and creases visible depending on where the light source is, what color it has, and so on. Without light, everything looks lifeless.

So let’s bring more beauty and depth into our scene — by implementing **light** in our engine.

We’ll start simple but with something that immediately has a strong visual effect: **diffuse lighting**.

---

## Concept of Diffuse Lighting

The core idea of diffuse lighting is very straightforward.

Imagine we have a **point light source** — like a glowing sphere or a lamp. It sits somewhere in 3D space, at a certain position, and it emits light equally in all directions.

That light travels outward and eventually hits the surface of our object. A light ray strikes a tiny point on the surface (a *fragment*).

The brightness of that fragment depends on the **angle** between the incoming light ray and the surface’s **normal vector**.

* A **normal** is simply a vector that’s perpendicular to the surface.
* Since all our objects are made of triangles, each triangle is flat and has its own normal.

So, to compute lighting at a fragment, we only need **two vectors**:

1. The **normal vector** at that fragment.
2. The **light direction vector** (from the fragment toward the light source).

How do we compute the light direction?

```cpp
lightDir = normalize(lightPosition - fragPosition);
```

Now we take the **dot product** (scalar product) of the normal and lightDir:

```cpp
float diff = dot(normal, lightDir);
```

* If the light is directly above the fragment (normal and lightDir aligned), the dot product is `1` → maximum brightness.
* If the light is at 90° to the surface, the dot product is `0` → completely dark.

Simple, right? With this concept in mind, let’s implement it step by step.

---

## Light Component

We’ll start by creating a `LightComponent`.

In `scene/components/`, add two new files:

* `LightComponent.h`
* `LightComponent.cpp`

And of course, register them in `CMakeLists.txt` and includes.

The `LightComponent` will have two main properties:

* **Position** (but that comes from its parent GameObject, so no need to store it separately).
* **Color** (let’s store this one).

```cpp
glm::vec3 m_color = glm::vec3(1.0f); // default: white light
```

---

## Collecting Lights

Unlike the camera, where there’s usually only one active camera, we may have multiple lights in a scene.

So we’ll collect all of them.

To keep things neat, let’s make a new common header `Common.h` inside `engine/source/`. It will store reusable data structs.

Move `CameraData` here, and also add:

```cpp
struct LightData 
{
    glm::vec3 color;
    glm::vec3 position;
};
```

Now in the `Scene` class, add a method:

```cpp
std::vector<LightData> CollectLights();
```

Its implementation will:

1. Iterate through all objects.
2. Recursively check if they have a `LightComponent`.
3. If yes, grab its color and world position.

For recursion, add a private helper:

```cpp
void CollectLightsRecursive(GameObject* obj, std::vector<LightData>& out);
```

Inside it:

```cpp
if (auto* light = obj->GetComponent<LightComponent>()) 
{
    LightData data;
    data.color = light->GetColor();
    data.position = obj->GetWorldPosition(); // global position
    out.push_back(data);
}
for (auto& child : obj->m_children) 
{
    CollectLightsRecursive(child.get(), out);
}
```

Finally:

```cpp
std::vector<LightData> Scene::CollectLights() 
{
    std::vector<LightData> lights;
    for (auto& obj : mObjects)
        CollectLightsRecursive(obj.get(), lights);
    return lights;
}
```

---

## Getting World Position

To compute light positions in world space, we’ll add a helper in `GameObject`:

```cpp
glm::vec3 getWorldPosition() const 
{
    glm::vec4 hom = getWorldTransform() * glm::vec4(0,0,0,1);
    return glm::vec3(hom) / hom.w;
}
```

---

## Passing Light to the Renderer

In `Engine::Update()`:

```cpp
auto lights = currentScene->CollectLights();
```

Pass this vector to `RenderQueue::Draw()`.

Inside `RenderQueue::Draw()`, for now, we’ll just use the **first light**:

```cpp
if (!lights.empty()) 
{
    const auto& light = lights[0];
    // pass it to shader
}
```

---

## Light Uniform in Shader

In the fragment shader, declare a struct:

```glsl
struct Light 
{
    vec3 color;
    vec3 position;
};

uniform Light uLight;
```

Now, from C++, we can set:

```cpp
shader->SetUniform("uLight.color", light.color);
shader->SetUniform("uLight.position", light.position);
```

To support this, add in `ShaderProgram`:

```cpp
void SetUniform(const std::string& name, const glm::vec3& value) 
{
    GLint loc = GetUniformLocation(name);
    glUniform3fv(loc, 1, glm::value_ptr(value));
}
```

---

## Adding Normals

To compute lighting, we need **normals**.

* Add a new attribute index in `VertexLayout.h`:

```cpp
static constexpr int NormalIndex = 3;
```

* When loading a mesh, extend the accessors array to size 4.
* If an attribute is of type `NORMAL`, map it to `NormalIndex`.

```cpp
if (attr.type == cgltf_attribute_type_normal) 
{
    accessors[VertexElement::NormalIndex] = attr.data;
    element.index  = VertexElement::NormalIndex;
    element.type   = GL_FLOAT;
    element.size   = 3;
    element.offset = vertexLayout.stride;
    vertexLayout.stride += 3 * sizeof(float);
    vertexLayout.elements.push_back(element);
}
```

---

## Vertex Shader Changes

We now need to pass:

* Normal (in world space).
* Fragment position (in world space).

```glsl
layout(location = 3) in vec3 aNormal;

out vec3 vNormal;
out vec3 vFragPos;

void main() 
{
    // position in world space
    vFragPos = vec3(uModel * vec4(aPos, 1.0));

    // transform normal into world space
    vNormal = mat3(transpose(inverse(uModel))) * aNormal;

    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}
```

---

## Fragment Shader Changes

```glsl
in vec3 vNormal;
in vec3 vFragPos;

uniform Light uLight;
uniform sampler2D baseColorTexture;

void main() 
{
    // normalize normal
    vec3 norm = normalize(vNormal);

    // light direction
    vec3 lightDir = normalize(uLight.position - vFragPos);

    // diffuse factor
    float diff = max(dot(norm, lightDir), 0.0);

    // diffuse component
    vec3 diffuse = diff * uLight.color;

    // sample texture
    vec4 texColor = texture(baseColorTexture, vUV);

    // final color
    vec3 result = diffuse * texColor.rgb;
    fragColor = vec4(result, 1.0);
}
```

---

## Adding a Light in the Scene

In `game.cpp`:

```cpp
auto& lightObj = scene.createObject("Light");
auto& light = lightObj.addComponent<LightComponent>();
light.setColor(glm::vec3(1.0f, 1.0f, 1.0f)); // white
lightObj.setPosition(glm::vec3(0.0f, 5.0f, 0.0f));
```

Now Suzanne will be lit beautifully.

---

## Normals for the Cube

But wait — our cubes are pitch black. Why? Because we never gave them normals.

Let’s fix that.

We’ll move cube creation into `Mesh::createCube()` and add normals for each face:

* Front: `(0, 0, 1)`
* Back: `(0, 0, -1)`
* Top: `(0, 1, 0)`
* Bottom: `(0, -1, 0)`
* Right: `(1, 0, 0)`
* Left: `(-1, 0, 0)`

Update the vertex layout:

```cpp
vertexLayout.elements.push_back({ VertexElement::NormalIndex, 3, GL_FLOAT, ... });
vertexLayout.stride = 11 * sizeof(float); // pos(3) + color(3) + normal(3) + uv(2)
```

---

## Results

Run the project.

Now Suzanne looks three-dimensional, with proper shading. And our cubes — now with normals — are also beautifully lit.

Congratulations 🎉 We’ve just added **light** to our world. Things are no longer flat; they now have depth, shadows, and highlights.

Welcome to the illuminated 3D world!