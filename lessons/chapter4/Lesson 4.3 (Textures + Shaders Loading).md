# More About Resources

Great, let’s continue our journey into the world of resources. We already know how to load and use textures. But what are textures? That’s right — textures are nothing more than **resources**.

So, let’s make our work with them more unified and universal. Instead of loading textures manually in `Game.cpp`, let’s move that responsibility into the `Texture` class itself.

---

## Refactoring the Texture Class

Go into the `Texture` class and add a new static method that takes just a file path:

```cpp
static std::shared_ptr<Texture> Load(const std::string& path);
```

Also add a helper method:

```cpp
void Init(int width, int height, int numChannels, unsigned char* data);
```
Put all the code from constructor there and replace the constructor code with this method call

### Implementation

* In the new `Texture::Load(const std::string& path)` method:

  1. Declare variables `int width, height, numChannels;`
  2. Build the full path to the asset:

     ```cpp
     auto fullPath = eng::Engine::GetInstance()
                         .GetFileSystem()
                         .GetAssetsFolder() / path;
     ```
  3. Check if the file exists:

     ```cpp
     if (!std::filesystem::exists(fullPath)) 
     {
        return nullptr;
     }
     ```
  4. Create a return object:

     ```cpp
     std::shared_ptr<Texture> result;
     ```
  5. Use **stb\_image** to load it:

     ```cpp
     unsigned char* data = stbi_load(
         fullPath.string().c_str(),
         &width, &height, &numChannels, 0
     );
     ```
  6. If `data` is valid:

     * call `std::make_shared<Texture>(...)`
     * free memory with `stbi_image_free(data)`
  7. Return the result:

     ```cpp
     return result;
     ```

And that’s it! We’ve moved all image-loading logic into the `Texture::Create` function.

Now in `Game.cpp`, we can delete all the manual stb\_image loading code and simply do:

```cpp
auto texture = eng::Texture::Load("brick.jpg");
```

Notice that the path is relative to our `assets` folder. Clean and simple.

---

## Thinking About Shaders as Resources

Why stop at textures? Shaders are also resources. They’re just text files containing code that the GPU executes. Let’s treat them as first-class resources too.

1. Inside the `assets` folder, create a new folder `shaders`.
2. Add two files there:

   * `vertex.glsl` (vertex shader)
   * `fragment.glsl` (fragment shader)

The `.glsl` extension is just a convention since we’re using OpenGL and its shading language GLSL. But technically, they’re just text files.

Copy the shader code you previously hard-coded in `Game.cpp` into these files.

---

## Extending the File System

We already have a file system that can resolve paths relative to `assets`. Now let’s teach it to **load files**.

Add a generic function:

```cpp
std::vector<char> LoadFile(const std::filesystem::path& path);
```

### Implementation of `LoadFile`

1. Open the file in binary mode and seek to the end:

   ```cpp
   std::ifstream file(path, std::ios::binary | std::ios::ate);
   if (!file.is_open()) 
   {
    return {};
   }
   auto size = file.tellg();
   file.seekg(0);
   ```
2. Allocate a buffer of that size:

   ```cpp
   std::vector<char> buffer(size);
   ```
3. Read the entire file into the buffer. If reading fails — return empty. Otherwise — return the buffer.
   ```cpp
   if (!file.read(buffer.data(), size))
   {
    return {};
   }

   return buffer;
   ```
---

### Asset-Specific Helpers

Now, let’s make things easier:

```cpp
std::vector<char> LoadAssetFile(const std::string& relativePath) 
{
    return LoadFile(getAssetsFolder() / relativePath);
}
```

And since shader code is plain text, we’ll also add:

```cpp
std::string LoadAssetFileText(const std::string& relativePath) 
{
    auto buffer = LoadAssetFile(relativePath);
    return std::string(buffer.begin(), buffer.end());
}
```

This way, we can directly get the shader source code as a string.

---

## Using the New Resource System

Now in `Game.cpp`, we can replace the old shader-loading code with:

```cpp
auto& fs = eng::Engine::GetInstance().GetFileSystem();

auto vertexShaderSource   = fs.LoadAssetFileText("shaders/vertex.glsl");
auto fragmentShaderSource = fs.LoadAssetFileText("shaders/fragment.glsl");
```

And that’s it. Everything works just like before, but now shaders are proper resources, loaded by our file system, just like textures.

---

## Conclusion

We’ve unified our approach:

* **Textures** are resources.
* **Shaders** are resources.

Both are stored in `assets/`, and both are loaded through the file system.

This is a big step forward. From now on, we won’t clutter game logic with low-level file loading — every resource type will know how to initialize itself, and the file system will handle the paths.

Congratulations — you’re now one step closer to understanding how a real game engine organizes its resources!