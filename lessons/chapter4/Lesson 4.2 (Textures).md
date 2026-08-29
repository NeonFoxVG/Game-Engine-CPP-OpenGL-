## Textures

Now it’s time to figure out what a *texture* really is. This is one of the most common terms you’ll hear in the world of games.

So, what is a texture? In simple words: it’s just an image that you apply to an object. You can imagine it as a candy wrapper, a sticker you place on something, clothes you put on, or packaging around an item. A texture is simply an image stretched across the surface of an object — that’s all.

### UV Coordinates

To apply textures to a mesh, we need texture coordinates — also known as **UVs**.

UV coordinates form a 2D system that ranges from (0,0) in the bottom-left corner to (1,1) in the top-right. Each vertex of your mesh gets its own UV coordinates.


Since a texture is essentially an image, everything starts with image manipulation. Let’s create a `Texture` class.

Go to the `Graphics` folder and create two files: `Texture.h` and `Texture.cpp`. Add them to `CMakeLists` and include them as usual.

### The Texture class

A texture in OpenGL is stored on the GPU and has its own unique identifier. So first, declare a field:

```cpp
GLuint m_textureID = 0;
```

Textures also have key attributes: width, height, and the number of color channels (e.g. 3 for RGB). Declare them as:

```cpp
int m_width = 0;
int m_height = 0; 
int m_numChannels = 0;
```

Next, create a constructor:

```cpp
Texture(int width, int height, int numChannels, unsigned char* data);
```

Also add a destructor and a simple getter:

```cpp
GLuint GetID() const;
```

### Implementation

In the constructor, initialize `m_width`, `m_height`, and `m_numChannels`. Then, create the texture object in GPU memory:

```cpp
glGenTextures(1, &m_textureID);
glBindTexture(GL_TEXTURE_2D, m_textureID);
```

Now we need to upload the raw image data to the GPU. Since we already know how to load images, we’ll use the provided `data` pointer:

```cpp
glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGB,
    width, height, 0,
    GL_RGB, GL_UNSIGNED_BYTE, data
);
```

After that, generate **mipmaps**:

```cpp
glGenerateMipmap(GL_TEXTURE_2D);
```

Mipmaps are progressively smaller versions of the same texture (halved in size each time, down to 1×1). They are used for filtering and smooth rendering when objects move closer or farther away in the scene.

### Wrapping and Filtering

When we upload a texture, we also need to tell OpenGL how it should behave if our UV coordinates go outside the standard \[0,1] range. This is called **texture wrapping**. There are a few common modes:

* **GL\_REPEAT** — the default. The texture simply tiles infinitely, like floor tiles. If your UV is `1.2`, it will just wrap back to `0.2`. Perfect for patterns (bricks, grass, etc.).
* **GL\_MIRRORED\_REPEAT** — similar to repeat, but every second tile is mirrored. This avoids hard seams when tiling. Imagine wallpaper strips flipping back and forth.
* **GL\_CLAMP\_TO\_EDGE** — instead of tiling, the edge pixels are stretched outward. This is useful if you want a texture to stop at its borders and not repeat at all (e.g. a UI element).

Let’s set wrapping for our texture:

```cpp
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
```

Here `S` and `T` are the horizontal and vertical axes in UV space.

Next, let’s configure **filtering**. Filtering determines how the texture is scaled when it’s drawn larger or smaller than its actual resolution:

```cpp
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
```

* **MIN\_FILTER** controls how textures are displayed when they’re *minified* (shrunk).
* **MAG\_FILTER** controls how they’re displayed when they’re *magnified* (enlarged).

Here we’re using linear filtering with mipmaps for minification, and simple linear filtering for magnification — this gives smooth results.

Finally, in the destructor, free the texture:

```cpp
if (mTextureId > 0) 
{
    glDeleteTextures(1, &m_textureID);
}
```

Great — we now have a `Texture` object.

---

### Connecting Textures to Materials

Go to `ShaderProgram` and add a method:

```cpp
void SetTexture(const std::string& name, Texture* texture);
```

Then in `Material`, add a container for textures:

```cpp
std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
```

Implement `SetParam(name, texture)` so it stores textures in that map. In the `Bind()` method, loop through all textures and pass them into the shader via `SetTexture()`.

---

### UV Coordinates

Update your vertex data in `Game.cpp` to include UVs. Now we need declare each face separately.

```
std::vector<float> vertices =
{
   // Front face
   0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
   -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
   -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
   0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

   // Top face
   0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
   -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
   -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
   0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

   // Right face
   0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
   0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
   0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
   0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

   // Left face
   -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
   -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
   -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
   -0.5f, -0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

   // Bottom face
   0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
   -0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
   -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
   0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

   // Back face
   -0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
   0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
   0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
   -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f
};

std::vector<unsigned int> indices =
{
    // front face
    0, 1, 2,
    0, 2, 3,
    // top face
    4, 5, 6,
    4, 6, 7,
    // right face
    8, 9, 10,
    8, 10, 11,
    // left face
    12, 13, 14,
    12, 14, 15,
    // bottom face
    16, 17, 18,
    16, 18, 19,
    // back face
    20, 21, 22,
    20, 22, 23
};
```

Update the vertex layout accordingly:

* Position → 3 floats
* Color → 3 floats
* UV → 2 floats

That’s 8 floats per vertex (instead of 6).

---

### Updating Shaders

In the **vertex shader**, add:

```glsl
layout (location = 2) in vec2 uv;
out vec2 vUV;
...
vUV = uv;
```

In the **fragment shader**, receive it:

```glsl
in vec2 vUV;
uniform sampler2D brickTexture;
```

Then sample the texture (get the color of the texture pixel using vUV coordinates):

```glsl
vec4 texColor = texture(brickTexture, vUV);
fragColor = texColor * vec4(vColor, 1.0);
```

This multiplies the base vertex color with the sampled texture color.

---

### Texture Units

Now let’s talk about something very important: **texture units**.

Your GPU can work with many textures at once, but shaders don’t directly know about texture IDs. Instead, they reference textures through *samplers* (`sampler2D` that we used in a fragment shader), which are linked to something called a *texture unit*.

Think of texture units as numbered slots or cells inside the GPU — like a row of lockers. Each locker can hold one texture at a time. When a shader wants to read a texture, it doesn’t say “give me texture #5 with this ID”; instead, it says “sample from unit 0” or “sample from unit 1.” Then it’s your job, as the engine developer, to bind the correct texture to that slot.

Here’s how it works step by step:

1. **Activate a texture unit**

   ```cpp
   glActiveTexture(GL_TEXTURE0 + unitIndex);
   ```

   This tells OpenGL: “From now on, I’m working with texture unit number `unitIndex`.”

2. **Bind a texture to the active unit**

   ```cpp
   glBindTexture(GL_TEXTURE_2D, texture->GetID());
   ```

   Now that slot holds your texture.

3. **Tell the shader which unit to use**
   Remember, in your shader you declared a `uniform sampler2D brickTexture;`. That sampler doesn’t hold a texture ID — it just expects an integer that says *which texture unit to read from*.

   ```cpp
   glUniform1i(location, unitIndex);
   ```

So the full sequence looks like this:

```cpp
glActiveTexture(GL_TEXTURE0 + unitIndex);
glBindTexture(GL_TEXTURE_2D, texture->GetID());
glUniform1i(location, unitIndex);
```
Here, `unitIndex` is the index of the texture slot, and `location` is the uniform location of the sampler.

That’s it — the shader now samples the correct texture.

One important note: since multiple textures can be used in one material, we need to carefully assign each texture its own free unit. A simple approach (and what we’ll use for now) is to keep a counter inside the `ShaderProgram`. Every time we bind a new texture, we increase the counter and assign the next available unit. When the shader is bound, we reset the counter back to zero. This keeps everything predictable.


So create a counter `m_currentTextureUnit` that starts from 0 each time you bind the shader. Every new texture gets bound to the next available unit and `m_currentTextureUnit` is incremented.

```cpp
glActiveTexture(GL_TEXTURE0 + m_currentTextureUnit);
glBindTexture(GL_TEXTURE_2D, texture->GetID());
glUniform1i(location, m_currentTextureUnit);
++m_currentTextureUnit;
```

---

### Putting It All Together

Now, in `Game.cpp`, you can do:

Create a texture
```cpp
auto texture = std::make_shared<eng::Texture>(width, height, channels, data);
```

Set it to material
```cpp
material->SetParam("brickTexture", texture);
```

Don't forget to clear the data after usage
```cpp
stbi_image_free(data);
```

When binding the material, the shader gets the texture, the mesh provides UVs, and the pipeline renders it correctly.

Run the project, and you should see your cube (or quad) textured with **BrickTexture**, tinted by the vertex color.

Congratulations — you’ve just implemented textures!