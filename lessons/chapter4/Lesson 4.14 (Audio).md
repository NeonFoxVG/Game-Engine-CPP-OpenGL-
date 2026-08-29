# Lesson: Adding Sound to the Scene

Now it’s time to add **sound** to our scene.
For audio, we’ll use a lightweight header-only library called **miniaudio**.

All you need to do is:

* Add an `include_directories` entry in your **CMake** file,
* Add platform-specific dependencies for audio drivers.

That’s it — we can start.

---

## 1) AudioManager

We’ll start simple, just like with other systems: create an **AudioManager**.

1. Go to `engine/source/`.
   Create a new folder: `audio/`.
   Inside, add two files:

   * `AudioManager.h`
   * `AudioManager.cpp`
   * Declare a class `AudioManager`

2. In `Engine`, add:

   * `#include "audio/AudioManager.h"`
   * A private field:

     ```cpp
     AudioManager m_audioManager;
     ```
   * A getter:

     ```cpp
     AudioManager& GetAudioManager() { return m_audioManager; }
     ```

This lets us access the audio system from anywhere.

---

### Implementation

**AudioManager.h**

```cpp
#pragma once
#include "Common.h"
#include <memory>

struct ma_engine; // forward declaration

class AudioManager 
{
public:
    AudioManager();
    ~AudioManager();

    bool Init();
    ma_engine* GetEngine();

    void SetListenerPosition(const glm::vec3& pos);

private:
    std::unique_ptr<ma_engine> m_engine;
};
```

**AudioManager.cpp**

```cpp
#include "audio/AudioManager.h"
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

AudioManager::AudioManager() 
{
    m_engine = std::make_unique<ma_engine>();
}

AudioManager::~AudioManager() 
{
    if (m_engine)
        ma_engine_uninit(m_engine.get());
}

bool AudioManager::Init() 
{
    auto result = ma_engine_init(nullptr, m_engine.get());
    return result == MA_SUCCESS;
}

ma_engine* AudioManager::GetEngine() 
{
    return m_engine.get();
}

void AudioManager::SetListenerPosition(const glm::vec3& pos) 
{
    if (m_engine) 
    {
        ma_engine_listener_set_position(m_engine.get(), 0, pos.x, pos.y, pos.z);
    }
}
```

And in `Engine::Init()`, call:

```cpp
mAudioManager.Init();
```

Now the audio system is ready.

---

## 2) Audio resource

Just like we have **textures**, **meshes**, and **models**, we’ll also have **Audio** as a resource.

Create two files:

* `Audio.h`
* `Audio.cpp`

**Audio.h**

```cpp
#pragma once
#include "Common.h"
#include <memory>
#include <string>
#include <vector>

struct ma_sound;
struct ma_decoder;

class Audio 
{
public:
    ~Audio();

    void SetPosition(const glm::vec3& pos);
    void Play(bool loop = false);
    void Stop();
    bool IsPlaying() const;

    void SetVolume(float volume);
    float GetVolume() const;

    static std::shared_ptr<Audio> Load(const std::string& path);

private:
    std::unique_ptr<ma_sound> m_sound;
    std::unique_ptr<ma_decoder> m_decoder;
    std::vector<char> m_buffer;
};
```

**Audio.cpp (core logic)**

In the destructor, we need to destroy things in the correct order:

* First the sound,
* Then the decoder.
```cpp
Audio::~Audio()
{
    if (m_sound)
    {
        ma_sound_uninit(m_sound.get());
    }
    if (m_decoder)
    {
        ma_decoder_uninit(m_decoder.get());
    }
}
```

In the `Load` function:
1. We load raw binary data from the filesystem,

   ```cpp
   auto buffer = Engine::GetInstance().GetFileSystem().LoadAssetFile(path);
   ```
2. We get a pointer to the audio engine:

   ```cpp
   auto engine = Engine::GetInstance().GetAudioManager().GetEngine();
   ```
3. We create an `Audio` object, store the buffer in it, and set up a decoder.
   
   ```cpp
   auto audio = std::make_shared<Audio>();
   audio->m_sound = std::make_unique<ma_sound>();
   audio->m_buffer = buffer;
   audio->m_decoder = std::make_unique<ma_decoder>();
   ```
4. Initialize the decoder with `ma_decoder_init_memory(...)`.
   If it fails, return `nullptr`.
   
   ```cpp
   auto result = ma_decoder_init_memory(audio->m_buffer.data(), audio->m_buffer.size(), nullptr, audio->m_decoder.get());
   if (result != MA_SUCCESS)
   {
       return nullptr;
   }
   ```
5. Initialize the sound with `ma_sound_init_from_data_source(...)`.
   If it fails, return `nullptr`.

   ```cpp
   result = ma_sound_init_from_data_source(
       engine,
       audio->m_decoder.get(),
       0,
       NULL,
       audio->m_sound.get()
   );
   if (result != MA_SUCCESS) 
   {
       return nullptr;
   }
   ```
6. Enable spatialization:

   ```cpp
   ma_sound_set_spatialization_enabled(audio->m_sound.get(), MA_TRUE);
   ```
7. Return the audio object.

Other methods:

* `SetPosition`: update sound’s position in 3D.
  ```cpp
  void Audio::SetPosition(const glm::vec3& position)
  {
      if (m_sound)
      {
          ma_sound_set_position(m_sound.get(), position.x, position.y, position.z);
      }
  }
  ```
* `Play`: start playing, optionally looped.
  ```cpp
  void Audio::Play(bool loop)
  {
      if (m_sound)
      {
          ma_sound_start(m_sound.get());
          auto looping = loop ? MA_TRUE : MA_FALSE;
          ma_sound_set_looping(m_sound.get(), looping);
      }
  }
  ```
* `Stop`: stop and rewind to frame 0.
  ```cpp
  void Audio::Stop()
  {
      if (m_sound)
      {
          ma_sound_stop(m_sound.get());
          ma_sound_seek_to_pcm_frame(m_sound.get(), 0);
      }
  }
  ```
* `IsPlaying`: return true/false.
  ```cpp
  bool Audio::IsPlaying() const
  {
      if (m_sound)
      {
          return ma_sound_is_playing(m_sound.get());
      }
      return false;
  }
  ```
* `SetVolume`: clamp volume between 0 and 1.
  ```cpp
  void Audio::SetVolume(float volume)
  {
      if (m_sound)
      {
          ma_sound_set_volume(m_sound.get(), std::clamp(volume, 0.0f, 1.0f));
      }
  }
  ```
* `GetVolume`: return current volume.
  ```cpp
  float Audio::GetVolume() const
  {
      if (m_sound)
      {
          return ma_sound_get_volume(m_sound.get());
      }
      return 0.0f;
  }
  ```

Now audio can be used as a resource in our engine.

---

## 3) AudioComponent

As you probably guessed, we will control sound via a **Component**.

Go to `scene/components` and add two new files:

* `AudioComponent.h`
* `AudioComponent.cpp`

**AudioComponent.h**

```cpp
#pragma once
#include "scene/Component.h"
#include "audio/Audio.h"
#include <unordered_map>

class AudioComponent : public Component 
{
    COMPONENT(AudioComponent)
public:
    void LoadProperties(const nlohmann::json& j) override;
    void Update(float dt) override;

    void RegisterAudio(const std::string& name, const std::shared_ptr<Audio>& clip);
    void Play(const std::string& name, bool loop = false);
    void Stop(const std::string& name);
    bool IsPlaying(const std::string& name) const;

private:
    std::unordered_map<std::string, std::shared_ptr<Audio>> m_clips;
};
```

**LoadProperties flow**:

* Expect a JSON field `"audio"`.
* It contains an array of clips.
* Each clip has:

  * `name` (identifier),
  * `path` (file path),
  * `volume` (optional, default = 1.0).
* Load each clip via `Audio::Load()`, set volume, and register it.
  ```cpp
  void AudioComponent::LoadProperties(const nlohmann::json& jsonObject)
  {
      if (jsonObject.contains("audio"))
      {
          auto& clipsObject = jsonObject["audio"];
          for (auto& clip : clipsObject)
          {
              std::string name = clip.value("name", "noname");
              std::string path = clip.value("path", "");  
              auto audio = Audio::Load(path);
              if (audio)
              {
                  float volume = clip.value("volume", 1.0f);
                  audio->SetVolume(volume);
                  RegisterAudio(name, audio);
              }
          }
      }
  }
  ```

**Update**:

* Each frame, iterate clips.
* If a clip is playing, update its position = owner’s world position.

```cpp
void AudioComponent::Update(float deltaTime)
{
    auto pos = m_owner->GetWorldPosition();
    for (auto& clip : m_clips)
    {
        if (clip.second->IsPlaying())
        {
            clip.second->SetPosition(pos);
        }
    }
}
```

* **RegisterAudio**: store clip in the map.

```cpp
void AudioComponent::RegisterAudio(const std::string& name, const std::shared_ptr<Audio>& clip)
{
   m_clips[name] = clip;
}
```

* **Play**: find clip by name and play it.

```cpp
void AudioComponent::Play(const std::string& name, bool loop)
{
    auto it = m_clips.find(name);
    if (it != m_clips.end())
    {
        if (it->second)
        {
            it->second->Play(loop);
        }
    }
}
```

* **Stop**: stop clip.

```cpp
void AudioComponent::Stop(const std::string& name)
{
    auto it = m_clips.find(name);
    if (it != m_clips.end())
    {
        if (it->second)
        {
            it->second->Stop();
        }
    }
}
```

* **IsPlaying**: return true if clip is currently active.

```cpp
bool AudioComponent::IsPlaying(const std::string& name) const
{
    auto it = m_clips.find(name);
    if (it != m_clips.end())
    {
        if (it->second)
        {
            return it->second->IsPlaying();
        }
    }
    return false;
}
```

---

## 4) AudioListenerComponent

We also need a **listener** (the “ears”).
This will be a very simple component.

**AudioListenerComponent.h**

```cpp
class AudioListenerComponent : public Component 
{
    COMPONENT(AudioListenerComponent)
public:
    void Update(float dt) override 
    {
        auto pos = m_owner->GetWorldPosition();
        Engine::GetInstance()->GetAudioManager().SetListenerPosition(pos);
    }
};
```

There will be only one listener — the **Player**.

---

## 5) Updating scene JSON

Open `scene.sc`, find `MainPlayer`, and add:

```json
"components": [
  { "type": "CameraComponent" },
  { "type": "PlayerControllerComponent" },
  { "type": "AudioListenerComponent" },
  {
    "type": "AudioComponent",
    "audio": [
      { "name": "shoot", "path": "audio/shoot.wav" },
      { "name": "step",  "path": "audio/step.wav" },
      { "name": "jump",  "path": "audio/jump.wav" }
    ]
  }
]
```

Now our Player has both a listener and an audio component with three sounds.

---

## 6) Register components

In `Scene::RegisterTypes()`, add:

```cpp
AudioComponent::Register();
AudioListenerComponent::Register();
```

---

## 7) Integrating with Player

In `Player.h`, add fields:

```cpp
AudioComponent* m_audioComponent = nullptr;
PlayerControllerComponent* m_playerControllerComponent = nullptr;
```

Assign them in `Init()`.
```cpp
m_audioComponent = GetComponent<eng::AudioComponent>();
m_playerControllerComponent = GetComponent<eng::PlayerControllerComponent>();
```

---

## 8) Playing sounds

* **Shooting**:

```cpp
if (input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) 
{
    if (m_animationComponent && !m_animationComponent->IsPlaying()) 
    {
        m_animationComponent->Play("shoot", false);

        if (m_audioComponent) 
        {
            if (m_audioComponent->IsPlaying("shoot"))
            {
                m_audioComponent->Stop("shoot");
            }
            m_audioComponent->Play("shoot");
        }
    }
}
```

* **Jumping**:

```cpp
if (input.IsKeyPressed(GLFW_KEY_SPACE)) 
{
    if (m_audioComponent && !m_audioComponent->IsPlaying("jump")) 
    {
        m_audioComponent->Play("jump");
    }
}
```

* **Walking loop**:

```cpp
bool walking =
    input.IsKeyPressed(GLFW_KEY_W) ||
    input.IsKeyPressed(GLFW_KEY_A) ||
    input.IsKeyPressed(GLFW_KEY_S) ||
    input.IsKeyPressed(GLFW_KEY_D);

if (walking && m_playerControllerComponent && m_playerControllerComponent->OnGround()) 
{
    if (m_audioComponent && !m_audioComponent->IsPlaying("step"))
    {
        m_audioComponent->Play("step", true); // loop
    }
} 
else 
{
    if (m_audioComponent && m_audioComponent->IsPlaying("step"))
    {
        m_audioComponent->Stop("step");
    }
}
```

---

## 9) OnGround in PlayerControllerComponent

Add:

```cpp
bool PlayerControllerComponent::OnGround() const 
{
    if (m_kinematicController)
    {
        return m_kinematicController->OnGround();
    }
    return false;
}
```

---

## Result

If we did everything correctly and didn’t make mistakes, now we can run the game and actually **hear sounds**.

* Shooting plays the shoot sound,
* Jumping plays the jump sound,
* Walking plays step sounds in a loop when moving on the ground.

Congratulations! 
This is another very important milestone.
We have successfully integrated **sound** into our engine.
Now our game not only has graphics and physics, but also **audio feedback**.
Awesome work