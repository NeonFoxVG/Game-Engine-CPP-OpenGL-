# Lesson: Loading Full glTF Scenes into the Engine

As you remember, we already know how to load **meshes** by parsing a glTF file.
But, as you probably understand, what we’ve done so far is a very basic and partial solution.

A glTF file actually describes a *lot* more information, and right now we only extract a very tiny fraction — basically one mesh.
That’s not very reasonable.

So, I suggest that from now on, instead of only loading a mesh, we go deeper and start loading **everything** that comes with it.

We’ll do all of this using our Suzanne file as an example. That way, you’ll be able to clearly see (or not see) how things change.

---

## Step 1. Adding `LoadGLTF` to `GameObject`

Let’s add a new static method to the `GameObject` class, very similar to how we already load meshes:

```cpp
static GameObject* LoadGLTF(const std::string& path);
```

Inside this method we’ll follow the same basic procedure we used in `Mesh::Load`.
If you remember, in `Mesh::Load` we had includes like:

```cpp
#define CGLTF_IMPLEMENTATION
#include <cgltf.h>
```

We’ll need those here as well, so let’s move them.

---

## Step 2. Reading File Contents

Back in `LoadGLTF`, first we read the file contents, exactly like we did for meshes:

```cpp
auto contents = Engine::GetInstance().GetFileSystem().LoadAsFileText(path);
```

This gives us the raw text of the glTF file. After that, we parse it with cgltf.
Once parsing succeeds, we can continue.

We also need to store the relative folder path (using `std::filesystem::path`) so we can later resolve texture URIs, buffers, etc.

```cpp
if (res != cgltf_result_success)
{
    return nullptr;
}
auto fullPath = Engine::GetInstance().GetFileSystem().GetAssetsFolder() / path;
auto fullFolderPath = fullPath.remove_filename();
auto relativeFolderPath = std::filesystem::path(path).remove_filename();

res = cgltf_load_buffers(&options, data, fullFolderPath.string().c_str());
//...

```

---

## Step 3. Thinking About Scene Structure

A glTF file stores data in **nodes**.
Each node may contain:

* a transformation (position, rotation, scale, or even a full matrix),
* zero or one mesh,
* children (other nodes).

So, let’s agree:

* **Each glTF node will map to one `GameObject`.**

Each glTF mesh can contain multiple **primitives**, and each primitive can have:

* geometry,
* material.

That matches perfectly with our engine:

* **One glTF primitive → one `MeshComponent`.**

---

## Step 4. Creating the Root Object

Because a glTF file may contain an entire scene with many nodes, we need a root `GameObject` to hold everything:

```cpp
auto resultObject = Engine::GetInstance().GetScene().CreateObject("Result");
```

Now, a glTF file may contain multiple scenes.
For simplicity, let’s just take the **first scene** (scene index 0).

```cpp
auto scene = data.scenes[0];
```

Then we iterate through its nodes:

```cpp
for (cgltf_size i = 0; i < scene->nodes_count; ++i) 
{
    auto node = scene->nodes[i];
    ParseGLTFNode(node, resultObject, folder);
}
```

---

## Step 5. Implementing `ParseGLTFNode`

We’ll create a helper function inside `GameObject.cpp`:

```cpp
void ParseGLTFNode(cgltf_node* node,
                   GameObject* parent,
                   const std::filesystem::path& folder);
```

This function will:

1. Create a `GameObject` for the node.

   ```cpp
   auto object = parent->GetScene()->CreateObject(node->name, parent);
   ```
2. Apply transformations (matrix or separate translation/rotation/scale).
3. If the node has a mesh → load all its primitives.
4. If primitives have materials → create or load them.
5. Recursively parse children.

---

### Step 5.1. Handling Transformations

Nodes can store transformations in two ways:

* **As a matrix**
  In that case, we use `glm::make_mat4(node->matrix)` and then decompose it:

  ```cpp
  glm::vec3 translation, scale, skew;
  glm::vec4 perspective;
  glm::quat orientation;
  glm::mat4 mat = glm::make_mat4(node->matrix);
  glm::decompose(mat, scale, orientation, translation, skew, perspective);

  object->SetPosition(translation);
  object->SetRotation(orientation);
  object->SetScale(scale);
  ```

* **As separate components**
  If the node doesn’t have a matrix, it may specify translation, rotation, and scale separately:

  ```cpp
  if (node->has_translation)
      object->SetPosition(glm::vec3(node->translation[0],
                                    node->translation[1],
                                    node->translation[2]));
  if (node->has_rotation)
      object->SetRotation(glm::quat(node->rotation[3],
                                    node->rotation[0],
                                    node->rotation[1],
                                    node->rotation[2]));
  if (node->has_scale)
      object->SetScale(glm::vec3(node->scale[0],
                                 node->scale[1],
                                 node->scale[2]));
  ```

---

### Step 5.2. Handling Meshes and Primitives

If the node has a mesh:

```cpp
if (node->mesh) 
{
    for (cgltf_size pi = 0; pi < node->mesh->primitives_count; ++pi) 
    {
        auto primitive = node->mesh->primitives[pi];
        // Read vertex attributes, indices, etc.
    }
}
```

* Read vertex positions, normals, UVs.
* Read indices.
* Create a new `Mesh`.
* Create a `MeshComponent`.
* Attach it to the object.

---

### Step 5.3. Handling Materials

glTF materials are **PBR-based**. Two workflows exist:

* Metallic-Roughness
* Specular-Glossiness

We need a default material in case the primitive doesn’t have one.
So, in `GraphicsAPI`, let’s add:

```cpp
const std::shared_ptr<ShaderProgram>& GetDefaultShaderProgram();
```

This function will lazily create and cache a default shader (by loading vertex.glsl and fragment.glsl, for example).

When parsing a primitive:

* Create a `Material`: `auto mat = std::make_shared<Material>();`
* Assign it the default shader: `mat->SetShaderProgram(Engine::GetInstance().GetGraphicsAPI().GetDefaultShaderProgram());`
* If `primitive.material` exists, check:

  * `has_pbr_metallic_roughness` → use `baseColorTexture`.
  ```cpp
    if (gltfMaterial->has_pbr_metallic_roughness)
    {
        auto pbr = gltfMaterial->pbr_metallic_roughness;
        auto texture = pbr.base_color_texture.texture;
        if (texture && texture->image)
        {
            if (texture->image->uri)
            {
                auto path = folder / std::string(texture->image->uri);
                auto texture = Texture::Load(path.string());
                mat->SetParam("baseColorTexture", texture);
            }
        }
    }
  ```
  * `has_pbr_specular_glossiness` → use `diffuseTexture`.
  ```cpp
    else if (gltfMaterial->has_pbr_specular_glossiness)
    {
        auto pbr = gltfMaterial->pbr_specular_glossiness;
        auto texture = pbr.diffuse_texture.texture;
        if (texture && texture->image)
        {
            if (texture->image->uri)
            {
                auto path = folder / std::string(texture->image->uri);
                auto texture = Texture::Load(path.string());
                mat->SetParam("baseColorTexture", texture);
            }
        }
    }
  ```

Load the texture (if URI present) relative to the glTF file folder.
Set the texture in the material:

```cpp
material->SetParam("baseColorTexture", texture);
```

Finally, create mesh component with material and mesh.

---

### Step 5.4. Recursively Processing Children

After processing the current node:

```cpp
for (cgltf_size ci = 0; ci < node->children_count; ++ci) 
{
    ParseGLTFNode(node->children[ci], object, folder);
}
```

That way, we handle the entire hierarchy.

---

## Step 6. Cleaning Up

After everything is loaded, free the cgltf data:

```cpp
cgltf_free(data);
```

Return the `resultObject`.

---

## Step 7. Scene Management Improvements

To manage objects better, let’s store a reference to the scene inside each `GameObject`:

```cpp
Scene* m_scene = nullptr;
```

In `Scene::CreateObject`, set this pointer.
Also, add:

* `bool SetParent(GameObject* parent);`
* `Scene* GetScene();`

This gives us clean scene and parent management.

---

## Step 8. Testing in `Game.cpp`

Now we can load Suzanne as a full scene:

```cpp
auto suzanneObject = GameObject::LoadGLTF("models/suzanne.gltf");
```

Remove the old single-mesh loading code.
Also remove dummy objects blocking Suzanne.

Finally go back to `Mesh` class and remove `Load` method and `cgltf` includes.

When you run it, Suzanne will look the same as before.
But the difference is huge:
You’re now loading the **entire glTF scene**, not just a single mesh.

This gives us a much more powerful and future-proof engine.

---

That’s it — we now support **full glTF scene loading** with nodes, transformations, meshes, materials, and recursion.