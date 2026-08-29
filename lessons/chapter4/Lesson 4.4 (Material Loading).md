# Loading Materials

Now let’s learn how to load **materials**.

To load them, we first need to decide how we’ll represent them as assets. The simplest and most convenient option is to store materials as **JSON text files**.

---

## Adding JSON Support

To parse JSON, we’ll use an external library called **nlohmann/json**. It’s a header-only library, just like stb\_image. Add it to your engine with:

```cmake
include_directories(third_party/json)
```

That’s it — no extra setup required.

---

## Creating a Material Asset

Inside the `assets` folder, create a new folder `materials`. Then create our first test material file, e.g. `brick.mat`.

Since we agreed to use JSON, the structure will look like this:

```json
{
  "shader": {
    "vertex": "shaders/vertex.glsl",
    "fragment": "shaders/fragment.glsl"
  },
  "params": {
    "float": [
      { "name": "roughness", "value": 0.5 }
    ],
    "float2": [
      { "name": "uvOffset", "value0": 0.0, "value1": 0.0 }
    ],
    "textures": [
      { "name": "brickTexture", "path": "textures/brick.png" }
    ]
  }
}
```

* The `shader` object contains references to the vertex and fragment shader files.
* The `params` object contains three subsections: `float`, `float2`, and `textures`. Each stores an array of parameter objects.

  * Floats: `{ name, value }`
  * Float2: `{ name, value0, value1 }`
  * Textures: `{ name, path }`

Let’s also create a folder `assets/textures` and place `brick.png` there.

---

## Loading a Material

Just like with textures, let’s give the `Material` class a static loading function:

```cpp
static std::shared_ptr<Material> Load(const std::string& path);
```

### Implementation

1. **Read file contents**

```cpp
auto contents = Engine::GetInstance()
                   .GetFileSystem()
                   .LoadAssetFileText(path);

if (contents.empty()) return nullptr;
```

2. **Parse JSON**

```cpp
nlohmann::json json = nlohmann::json::parse(contents);
std::shared_ptr<Material> result;
```

3. **Load shader program**

```cpp
if (json.contains("shader")) 
{
    auto shaderObj = json["shader"];
    std::string vertexPath   = shaderObj.value("vertex", "");
    std::string fragmentPath = shaderObj.value("fragment", "");

    auto& fs = Engine::GetInstance().GetFileSystem();
    auto vertexSrc   = fs.LoadAssetFileText(vertexPath);
    auto fragmentSrc = fs.LoadAssetFileText(fragmentPath);

    auto& graphics = Engine::GetInstance().GetGraphicsAPI();
    auto shaderProgram = graphics.CreateShaderProgram(vertexSrc, fragmentSrc);
    if (!shaderProgram) return nullptr;

    result = std::make_shared<Material>();
    result->SetShaderProgram(shaderProgram);
}
```

4. **Load parameters**

```cpp
if (json.contains("params")) 
{
    auto paramsObj = json["params"];

    // Floats
    if (paramsObj.contains("float")) 
    {
        for (auto& p : paramsObj["float"]) 
        {
            std::string name  = p.value("name", "");
            float value = p.value("value", 0.0f);
            result->SetParam(name, value);
        }
    }

    // Float2
    if (paramsObj.contains("float2")) 
    {
        for (auto& p : paramsObj["float2"]) 
        {
            std::string name = p.value("name", "");
            float v0 = p.value("value0", 0.0f);
            float v1 = p.value("value1", 0.0f);
            result->SetParam(name, v0, v1);
        }
    }

    // Textures
    if (paramsObj.contains("textures")) 
    {
        for (auto& p : paramsObj["textures"]) 
        {
            std::string name = p.value("name", "");
            std::string texPath = p.value("path", "");
            auto texture = Texture::Load(texPath);
            if (texture) 
            {
                result->setParam(name, texture);
            }
        }
    }
}
```

5. **Return result**

```cpp
return result;
```

---

## Using the Material

Now in `Game.cpp`, we can simplify our code. Remove the manual shader loading and shader program creation. Instead, just do:

```cpp
auto material = eng::Material::Load("materials/brick.mat");
```

That’s it! Run the project — everything should look the same, but now the material is fully defined in an external JSON file and loaded automatically.

---

## Conclusion

We’ve added a new kind of resource: **materials**.

* Textures are resources.
* Shaders are resources.
* And now materials are resources too.

Each of them lives in the `assets/` folder, each loaded through our file system.

Congratulations! You’ve just taken another big step toward unifying resource management in your engine — and making your life much easier as a developer.
