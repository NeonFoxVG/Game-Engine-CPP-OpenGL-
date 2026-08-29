# Lesson: Building a More Interactive Scene (Toward an FPS Prototype)

Now let’s create a more interesting and interactive scene — something that brings us closer to a real game experience, or at least a prototype for one.

Since I personally like shooters (especially first-person ones), I’ll use that as our basis for further exploration.

We already know how to load full glTF files (models, objects, entire scenes).
So I’ve prepared another file for you: a **weapon model**. I downloaded it from Sketchfab (the author is credited in the pinned comment; you can also find it in the resources folder for free).

---

## Step 1. Organizing Assets

In your `assets` folder:

* Create a subfolder called `models` (if it doesn’t already exist).
* Move *Suzanne* into its own subfolder:

  ```
  assets/models/suzanne/
  ```
* Place the weapon model and textures into another subfolder inside `models`:

  ```
  assets/models/weapon/
  ```

Now in `Game.cpp`, update the path to Suzanne:

```cpp
models/suzanne/suzanne.gltf
```

---

## Step 2. Loading the Weapon and Parenting It to the Camera

Let’s load the weapon just like we did with Suzanne.

Then, here comes the trick:
we already have a `GameObject` with a camera and a player controller.
We’ll **attach the weapon as a child of the camera object**.

This way, the weapon will move and rotate together with the camera — giving us a very simple FPS-like effect.

```cpp
gun->SetParent(camera);
```

Now set up the transform. I picked the values experimentally, so just follow along:

```cpp
gun->SetPosition(glm::vec3(0.75f, -0.5f, -0.75f));
gun->SetScale(glm::vec3(-1.0f, 1.0f, 1.0f));
```

👉 Scaling by `-1` along the X-axis mirrors the weapon, so it appears in the right hand.

Run the game:

* The weapon loads correctly.
* It rotates with the mouse.
* It moves with the player.
  This works because it’s a child object of the camera (which itself is driven by the player controller).

---

## Step 3. Fixing Texture Artifacts (RGB vs RGBA)

You’ll notice the weapon looks strange — rainbow-like colors instead of the expected textures. Why?

The answer is in `Texture::Load`.
Currently, we always call:

```cpp
glTexImage2D(..., GL_RGB, ..., GL_RGB, ...);
```

That assumes **all textures are RGB (3 channels)**.

But one of the weapon’s textures actually has **4 channels (RGBA)**.
We still try to upload it as RGB, which causes artifacts (the rainbow effect).

**Solution:**
Add logic to handle channel count:

```cpp
GLint internalFormat = GL_RGB;
GLenum format = GL_RGB;

if (numChannels == 4) 
{
    internalFormat = GL_RGBA;
    format = GL_RGBA;
}

glTexImage2D(..., internalFormat, ..., format, ...);
```

Now textures with alpha channels are handled properly. Run again — and everything renders correctly.

---

## Step 4. Avoiding Duplicate Texture Loads (Caching)

Another issue:
The weapon model is split into many small objects, each with its own mesh and material.
But in practice, they often reuse the *same texture*.

Right now, in `GameObject::ParseGLTF`, we call `Texture::Load` for every material — meaning the same texture file may get loaded multiple times into GPU memory. Wasteful!

### Solution: Add a `TextureManager`

In `Texture.h`, after the `Texture` class, add:

```cpp
class TextureManager 
{
public:
    std::shared_ptr<Texture> GetOrLoadTexture(const std::string& path);

private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
};
```

Implementation:

```cpp
std::shared_ptr<Texture> TextureManager::GetOrLoadTexture(const std::string& path) 
{
    auto it = m_textures.find(path);
    if (it != m_textures.end())
        return it->second;

    auto texture = Texture::Load(path);
    m_textures[path] = texture;
    return texture;
}
```

Now integrate this into the engine:

* In `Engine`, add a `TextureManager m_textureManager;` field.
* Add a getter:

  ```cpp
  TextureManager& GetTextureManager();
  ```

Finally, in `GameObject.cpp`, inside `ParseGLTF`:
replace direct calls to `Texture::Load(path)` with:

```cpp
Engine::GetInstance().GetTextureManager().GetOrLoadTexture(path);
```

This ensures textures are loaded once, cached, and reused.

---

## Step 5. Adding Specular Highlights (Phong Lighting)

So far, our lighting is extremely basic — just Lambertian diffuse (dot product of light dir and normal).
Let’s add **specular highlights** for a more realistic effect.

### Step 5.1. Why Specular?

The diffuse model alone makes surfaces look matte. Real objects, however, often have shiny reflections when light hits them at certain angles.

This is the **specular component** of the Phong reflection model.
It depends on:

* The light direction.
* The surface normal.
* The viewer direction (camera).

The idea:

* Light hits a surface → it reflects at an angle equal to the angle of incidence.
* If the camera is aligned close to that reflection angle, the viewer sees a bright highlight.
* The closer the alignment, the sharper the highlight.

### Step 5.2. Passing Camera Position

To calculate this, we need the **camera position** in the shader.

* In `Common.h`, add to `CameraData`:

  ```cpp
  glm::vec3 position;
  ```
* In `Engine.cpp`, inside `Update/Run`:

  ```cpp
  cameraData.position = cameraObject->GetWorldPosition();
  ```
* In the render queue, pass it to the shader:

  ```cpp
  shaderProgram.SetUniform("uCameraPos", cameraData.position);
  ```

---

### Step 5.3. Updating the Fragment Shader

In your default fragment shader:

```glsl
uniform vec3 uCameraPos;
```

Then compute specular:

```glsl
// View direction: from fragment to camera
vec3 viewDir = normalize(uCameraPos - vFragPos);

// Reflection direction of the light
vec3 reflectDir = reflect(-lightDir, norm);

// Compute specular term
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

// Strength controls how intense the highlight is
float specularStrength = 0.5;

// Final specular component
vec3 specular = specularStrength * spec * uLight.color;
```

Notes:

* `dot(viewDir, reflectDir)` → measures alignment of camera and reflection.
* `max(..., 0.0)` → ignore negative values (light doesn’t reflect “through” the surface).
* `pow(..., shininess)` → sharpens the highlight. Higher shininess → tighter, glossier highlight (e.g. 128 for plastic, 8 for dull wood).

Finally, add it to the result:

```glsl
vec3 result = diffuse + specular;
vec4 texColor = texture(baseColorTexture, vUV);

FragColor = texColor * vec4(result, 1.0);
```

---

## Step 6. Testing the Result

Run the project:

* As you move around Suzanne, you now see shiny highlights.
* They change depending on your viewing angle.
* Materials look less flat and more realistic.

---

## Conclusion

Congratulations!

We’ve:

* Organized assets for multiple models.
* Attached a weapon to the camera, simulating a first-person shooter view.
* Fixed texture format issues (RGB vs RGBA).
* Added texture caching to save GPU memory.
* Enhanced lighting with **diffuse + specular highlights**.

Step by step, we’re not just loading models — we’re building the foundations of a very simple **FPS engine prototype**.
