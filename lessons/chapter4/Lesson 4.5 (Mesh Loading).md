# Loading 3D Models (Meshes)

Alright, it’s time to step up our game and learn how to load something even cooler — **3D models**, or meshes.

As you probably know, 3D models can come in all kinds of formats. There are so many of them that you can easily get lost trying to pick one. For our purposes though, let’s not overcomplicate things. We’ll just agree to use one format: **GLTF**.

GLTF is a relatively new format developed by the Khronos Group. And honestly, it’s perfect for us: modern, compact, and widely supported.

So just like with images, our task here is to be able to:

1. Read a GLTF file into memory.
2. Parse it.
3. Extract vertex and index data into buffers.

To make life easier, I’ve prepared a small header-only library for you called **cgltf**. It works the same way as stb\_image or nlohmann/json: just drop the header into your `thirdparty` folder, and add it in CMake:

```cmake
include_directories(third_party/cgltf)
```

That’s it. No linking, no configs.

---

## Preparing the Assets

Inside the `assets` folder, let’s create a new folder called `models`.
Into it, we’ll put:

* `Suzanne.gltf` — this is basically just a JSON file that describes the geometry.
* `Suzanne.bin` — a big binary buffer with all the raw vertex/index data.
* `Suzanne_BaseColor.png` — a texture.
* `Suzanne_MetallicRoughness.png` — another texture.

For now, we’ll only worry about the `.gltf` and `.bin` files.

---

## Mesh Loader Skeleton

In the `Mesh` class, let’s add a static function:

```cpp
static std::shared_ptr<Mesh> Load(const std::string& path);
```

And inside, the first thing we do is read the JSON description:

```cpp
auto contents = Engine::getInstance()
                   .GetFileSystem()
                   .LoadAssetFileText(path);

if (contents.empty()) return nullptr;
```

If there’s no content — well, nothing to load.

---

## Accessors and Helper Functions

cgltf exposes vertex/index data through *accessors*. They basically describe “how to interpret” a piece of the binary buffer (type, stride, offset, etc).

Let’s write two helper lambdas:

```cpp
auto readFloats = [](const cgltf_accessor* acc, cgltf_size i, float* out, int n) 
{
    std::fill(out, out + n, 0.0f); // zero-init
    return cgltf_accessor_read_float(acc, i, out, n) == 1;
};

auto readIndex = [](const cgltf_accessor* acc, cgltf_size i) 
{
    cgltf_uint out = 0;
    cgltf_bool ok = cgltf_accessor_read_uint(acc, i, &out, 1);
    return ok ? static_cast<uint32_t>(out) : 0;
};
```

Now we can pull out floats (positions, UVs, etc.) and unsigned ints (indices).

---

## Parsing with cgltf

Now we let cgltf do the heavy lifting.

```cpp
cgltf_options options = {};
cgltf_data* data = nullptr;

cgltf_result res = cgltf_parse(&options, contents.data(), contents.size(), &data);
if (res != cgltf_result_success) return nullptr;

auto fullPath = Engine::GetInstance().GetFileSystem().GetAssetsFolder() / path;

// Now load the .bin buffer(s)
res = cgltf_load_buffers(&options, data, fullPath.remove_filename().string().c_str());
if (res != cgltf_result_success) {
    cgltf_free(data);
    return nullptr;
}
```

If both steps succeed, `data` now holds everything: meshes, accessors, and buffer views.

---

## Iterating Meshes and Primitives

GLTF can contain multiple meshes, and each mesh can contain multiple primitives. A primitive is basically a chunk of geometry: a set of attributes (positions, normals, UVs) plus optional indices.

We’ll just grab the **first primitive that uses triangles** — that’s enough for now.

```cpp
std::shared_ptr<Mesh> result = nullptr;

for (cgltf_size mi = 0; mi < data->meshes_count && !result; ++mi) 
{
    const cgltf_mesh& mesh = data->meshes[mi];
    for (cgltf_size pi = 0; pi < mesh.primitives_count && !result; ++pi) 
    {
        const cgltf_primitive& prim = mesh.primitives[pi];
        if (prim.type != cgltf_primitive_type_triangles) continue;

        // process this primitive
    }
}
```

---

## Vertex Layout

Remember in our engine we have a `VertexLayout` that tells the GPU how to interpret vertex data. Let’s fix some attribute indices to avoid chaos:

```cpp
struct VertexElement {
    static constexpr int PositionIndex = 0;
    static constexpr int ColorIndex    = 1;
    static constexpr int UVIndex       = 2;
};
```

So:

* Positions → always at index 0
* Colors → index 1
* UVs → index 2

This will help keep vertex layouts consistent with our shaders.

---

```cpp
VertexLayout vertexLayout;
const cgltf_accessor* accessors[3] = { nullptr, nullptr, nullptr };
```

---

## Reading Attributes

GLTF primitives have a list of attributes. For each one, we check its type:

* If it’s **POSITION**, map it to index 0.
* If it’s **COLOR\_0**, map it to index 1.
* If it’s **TEXCOORD\_0**, map it to index 2.

```cpp
for (cgltf_size ai = 0; ai < prim.attributes_count; ++ai) 
{
    const cgltf_attribute& attr = prim.attributes[ai];
    const cgltf_accessor*  acc  = attr.data;
    if (!acc) continue;

    VertexElement element{};
    element.type = GL_FLOAT;

    switch (attr.type) 
    {
        case cgltf_attribute_type_position:
            accessors[VertexElement::PositionIndex] = acc;
            element.index  = VertexElement::PositionIndex;
            element.size = 3;
            break;
        case cgltf_attribute_type_color:
            if (attr.index != 0) continue;
            accessors[VertexElement::ColorIndex] = acc;
            element.index  = VertexElement::ColorIndex;
            element.size = 3;
            break;
        case cgltf_attribute_type_texcoord:
            if (attr.index != 0) continue;
            accessors[VertexElement::UVIndex] = acc;
            element.index  = VertexElement::UVIndex;
            element.size = 2;
            break;
        default:
            continue;
    }

    // Insert vertex element
}
```

For each attribute we fill out a `VertexElement` and push it into the `VertexLayout`. Don’t forget to update the stride:

```cpp
if (element.size > 0)
{
    element.offset = vertexLayout.stride;
    vertexLayout.stride += element.size * sizeof(float);
    vertexLayout.elements.push_back(element);
}
```

---

## Filling Vertex Data

Once we’ve got our layout, we can finally allocate the buffer.

1. Get the number of vertices from the position accessor. If there are no positions — nothing to draw.

```cpp
if (!accessors[VertexElement::PositionIndex]) 
{
    continue;
}
auto vertexCount = accessors[VertexElement::PositionIndex]->count;
```

2. Resize the vertex buffer:

```cpp

vertices.resize((vertexLayout.stride / sizeof(float)) * vertexCount);
```

```cpp
for (cgltf_size vi = 0; vi < vtxCount; ++vi) 
{
    // vertex processing
}
```

3. For each vertex, loop through all elements in the layout, and:
   
```cpp
for (auto& el : vertexLayout.elements) 
{
}
```

   * Check if an accessor exists for this element.
```cpp
if (!accessors[el.index]) continue;
```

   * If yes, call `readFloats` into the correct spot in the buffer.
```cpp
auto index = (vi * vertexLayout.stride + el.offset) / sizeof(float);
float* outData = &vertices[index];
readFloats(accessors[index], i, outData, el.size);
```

---

## Indices

If the primitive has an index accessor:

```cpp
if (prim.indices)
{
    auto indexCount = prim.indices->count;
    std::vector<uint32_t> indices(indexCount);
    for (cgltf_size i = 0; i < indexCount; ++i) 
    {
        indices[i] = readIndex(prim.indices, i);
    }
    result = std::make_shared<Mesh>(vertexLayout, vertices, indices);
}

```

If not:

```cpp
else
{
    result = std::make_shared<Mesh>(vertexLayout, vertices);
}
```

If mesh was created then break (remember for now we are targeting only one mesh):

```cpp
if (result)
{
    break;
}
```

And break the higher loop too:

```cpp
if (result)
{
    break;
}
```

Once done, free the cgltf data:

```cpp
cgltf_free(data);
```

And return the result:

```cpp
return result;
```

---

## Materials for Suzanne

Let’s also create a material for our new mesh.

* Copy `brick.mat` → `suzanne.mat`.
* In `suzanne.mat`, change the texture path to:

  ```json
  "path": "models/Suzanne_BaseColor.png"
  ```
* Rename the texture uniform from `brickTexture` to `baseColorTexture` in **both materials** (`brick.mat` and `suzanne.mat`).
* Update the fragment shader (`fragment.glsl`) to use `baseColorTexture`.
* For a cleaner look, remove vertex color (`vColor`) from both vertex and fragment shaders.

---

## Using the Mesh

Now let’s try it in code. Go to `Game.cpp` and write:

```cpp
auto suzanneMesh = eng::Mesh::load("models/suzanne.gltf");
auto suzanneMat  = eng::Material::load("materials/suzanne.mat");

auto suzanneObj = m_scene->CreateObject("Suzanne");
suzanneObj->AddComponent(new MeshComponent(suzanneMat, suzanneMesh));
suzanneObj->SetPosition(glm::vec3(0.0f, 0.0f, -5.0f));

// Move the old cube so they don’t overlap
objectA->SetPosition(glm::vec3(1.0f, 0.0f, -5.0f));
```

Run the project — and if everything is correct, you’ll see Suzanne’s monkey head with textures applied. Yep, the classic Suzanne with yellow eyes.

---

## Conclusion

And that’s it: our engine can now load **meshes** from GLTF files.

* Textures → resources
* Shaders → resources
* Materials → resources
* Meshes → resources

Step by step, we’re building a proper resource pipeline. Suzanne is alive, textured, and ready.

Bravo! The journey continues...