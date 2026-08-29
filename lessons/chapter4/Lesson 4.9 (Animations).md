# Lesson: Bringing Models to Life — Loading and Playing glTF Animations

Now that we can load models, it’s time to **bring them to life** — i.e., load and play **animations**.
glTF files can store animations for objects, and in this course we’ll use simple object animations. That’s enough to understand the core concept and how it’s applied in real projects.

**What is an animation?**
Conceptually, it’s just a set of **keyframes** over time. Each keyframe stores **position**, **rotation**, or **scale** of an object at a specific time. That’s it — a timeline of transforms.

We’ll start by adding a dedicated component to manage animations.

---

## 1) New Component: `AnimationComponent`

Create two files in `scene/components/`:

* `AnimationComponent.h`
* `AnimationComponent.cpp`

In `AnimationComponent.h`, declare the class `AnimationComponent`.

---

## 2) Data Structures: Keyframes, Tracks, Clips, Bindings

As said, animation = time-stamped transforms. We’ll define compact keyframe structs and then build tracks/clips on top.

### 2.1 Keyframes

* For **position** / **scale** (vector keyframes):

```cpp
struct KeyframeVec3 
{
    float time = 0.0f;
    glm::vec3 value = glm::vec3(0.0f);
};
```

* For **rotation** (quaternion keyframes):

```cpp
struct KeyframeQuat 
{
    float time = 0.0f;
    glm::quat value = glm::quat(1, 0, 0, 0); // w,x,y,z
};
```

### 2.2 Transform Track (per target object)

A track targets **one object** (by name) and may carry three channels:

```cpp
struct TransformTrack 
{
    std::string targetName;                 // name of the animated GameObject
    std::vector<KeyframeVec3> positions;    // translation keys
    std::vector<KeyframeQuat> rotations;    // rotation keys (quats)
    std::vector<KeyframeVec3> scales;       // scale keys
};
```

### 2.3 Animation Clip (can animate multiple objects)

One clip may contain many tracks (e.g., weapon fire: trigger, slide, muzzle flash, etc.):

```cpp
struct AnimationClip 
{
    std::string name;                       // clip name
    float duration = 0.0f;                  // total length (seconds)
    bool looping = true;                    // default loop flag
    std::vector<TransformTrack> tracks;     // all tracks in the clip
};
```

### 2.4 Object Binding (fast mapping from clip tracks to scene objects)

We’ll have **one** `AnimationComponent` on a top-level object, while clips often animate **children**. We need a quick way to map track targets to actual `GameObject*`s and remember which track indices affect each object:

```cpp
struct ObjectBinding 
{
    GameObject* object = nullptr;           // the actual object to animate
    std::vector<size_t> trackIndices;       // indices into AnimationClip::tracks
};
```

---

## 3) `AnimationComponent` fields & basic API

Add to `AnimationComponent`:

```cpp
class AnimationComponent : public Component 
{
    COMPONENT(AnimationComponent)
public:
    void Update(float deltaTime) override;
    void SetClip(AnimationClip* clip);
    void RegisterClip(const std::string& name, const std::shared_ptr<AnimationClip>& clip);
    void Play(const std::string& name, bool loop = true);

private:
    // Current playback state
    AnimationClip* m_clip = nullptr;
    float m_time = 0.0f;
    bool m_looping = true;
    bool m_isPlaying = false;

    // All clips loaded from this object’s glTF
    std::unordered_map<std::string, std::shared_ptr<AnimationClip>> m_clips;

    // Target-object bindings for current clip
    std::unordered_map<GameObject*, std::unique_ptr<ObjectBinding>> m_bindings;

    // Helpers
    void BuildBindings(); // rebuild m_bindings for m_clip

    // Interpolation helpers
    glm::vec3 Interpolate(const std::vector<KeyframeVec3>& keys, float time) const;
    glm::quat Interpolate(const std::vector<KeyframeQuat>& keys, float time) const;
};
```

---

## 4) `Update`: time, looping, and applying transforms

### 4.1 Early outs & time update

```cpp
void AnimationComponent::Update(float deltaTime) 
{
    if (!m_clip) 
    {
        return;
    }
    if (!m_isPlaying) 
    {
        return;
    }

    m_time += deltaTime;

    if (m_time > m_clip->duration) 
    {
        if (mLooping) 
        {
            m_time = std::fmod(m_time, m_clip->duration);
        } 
        else 
        {
            m_time = 0.0f;
            m_isPlaying = false; // stop (do not clear m_clip, we might replay)
            return;
        }
    }
```

### 4.2 Apply to all bound objects

```cpp
    for (auto& binding : m_bindings) 
    {
        GameObject* obj = binding.first;
        const auto& indices = binding.second->trackIndices;

        for (size_t idx : indices) 
        {
            const TransformTrack& track = m_clip->tracks[idx];

            // Evaluate tracks at m_time
            if (!track.positions.empty())
            {
                glm::vec3 pos = Interpolate(track.positions, mTime);
                obj->SetPosition(pos);
            } 
            if (!track.rotations.empty()) 
            {
                glm::quat rot = Interpolate(track.rotations, mTime);
                obj->SetRotation(rot);
            }
            if (!track.scales.empty())
            {
                glm::vec3 scl = Interpolate(track.scales,    mTime);
                obj->SetScale(scl);
            }    
        }
    }
}
```

---

## 5) Interpolation helpers (Vec3 linear, Quat slerp)

We’ll linearly interpolate vectors and **slerp** quaternions.

**Edge cases handled**:

* No keys → return zero/default.
* One key → return that key’s value.
* Time outside key range → clamp to first/last value.
* Find the first key with `key.time >= time` to get the right segment.

```cpp
glm::vec3 AnimationComponent::Interpolate(const std::vector<KeyframeVec3>& keys, float time) const 
{
    if (keys.empty()) 
    {
        return glm::vec3(0.0f);
    }

    if (keys.size() == 1) 
    {
        return keys[0].value;
    }

    if (time <= keys.front().time)
    {
        return keys.front().value;
    }

    if (time >= keys.back().time)
    {
        return keys.back().value;
    }

    size_t i0 = 0;
    size_t i1 = 0;

    for (size_t i = 1; i < keys.size(); ++i)
    {
        if (time <= keys[i].time)
        {
            i1 = i;
            break;
        }
    }

    i0 = i1 > 0 ? i1 - 1 : 0;

    if (time >= keys[i0].time && time <= keys[i1].time)
    {
        float deltaTime = keys[i1].time - keys[i0].time;
        float k = (time - keys[i0].time) / deltaTime;

        return glm::mix(keys[i0].value, keys[i1].value, k);
    }

    return keys.back().value;
}

glm::quat AnimationComponent::Interpolate(const std::vector<KeyframeQuat>& keys, float t) const 
{
    if (keys.empty())
    {
        return glm::vec3(0.0f);
    }

    if (keys.size() == 1)
    {
        return keys[0].value;
    }

    if (time <= keys.front().time)
    {
        return keys.front().value;
    }

    if (time >= keys.back().time)
    {
        return keys.back().value;
    }

    size_t i0 = 0;
    size_t i1 = 0;

    for (size_t i = 1; i < keys.size(); ++i)
    {
        if (time <= keys[i].time)
        {
            i1 = i;
            break;
        }
    }

    i0 = i1 > 0 ? i1 - 1 : 0;

    if (time >= keys[i0].time && time <= keys[i1].time)
    {
        float deltaTime = keys[i1].time - keys[i0].time;
        float k = (time - keys[i0].time) / deltaTime;

        return glm::slerp(keys[i0].value, keys[i1].value, k);
    }

    return keys.back().value;
}
```

---

## 6) `SetClip`, `RegisterClip`, and `Play`

```cpp
void AnimationComponent::SetClip(AnimationClip* clip) 
{
    m_clip = clip;
    m_time = 0.0f;
    BuildBindings();
}

void AnimationComponent::RegisterClip(const std::string& name,
                                      const std::shared_ptr<AnimationClip>& clip) 
{
    mClips[name] = clip;
}

void AnimationComponent::Play(const std::string& name, bool loop) 
{
    // If already on this clip, just restart
    if (m_clip && m_clip->name == name) 
    {
        m_time     = 0.0f;
        m_isPlaying = true;
        m_looping  = loop;
        return;
    }
    // Switch to a registered clip
    if (auto it = m_clips.find(name); it != m_clips.end()) 
    {
        SetClip(it->second);
        m_isPlaying = true;
        m_looping   = loop;
    }
}
```

### 6.1 Building bindings (track target → object)

```cpp
void AnimationComponent::BuildBindings() 
{
    m_bindings.clear();
    if (!m_clip) 
    {
        return;
    }

    for (size_t i = 0; i < m_clip->tracks.size(); ++i) 
    {
        const auto& track = m_clip->tracks[i];

        GameObject* target =
            m_owner->FindChildByName(track.targetName); // recursive
        if (!target) 
        {
            continue;
        }

        auto it = m_bindings.find(target);
        if (it != m_bindings.end()) 
        {
            it->second->trackIndices.push_back(i);
        } 
        else 
        {
            auto binding = std::make_unique<ObjectBinding>();
            binding->object = target;
            binding->trackIndices.push_back(i);
            m_bindings.emplace(target, std::move(binding));
        }
    }
}
```

---

## 7) Loading Animations from glTF in `GameObject::LoadGLTF`

We’ll add helper methods to `GameObject` and then parse glTF `animations`.

### 7.1 `FindChildByName`

Add to `GameObject`:

```cpp
GameObject* FindChildByName(const std::string& name) 
{
    if (m_name == name) 
    {
        return this;
    }

    for (auto& child : m_children) 
    {
        if (auto res = child->FindChildByName(name)) 
        {
            return res;
        }
    }
    return nullptr;
}
```

### 7.2 glTF accessor helpers

We’ll read scalars, vec3, quats, input times, and output values:

```cpp
static float ReadScalar(const cgltf_accessor* acc, cgltf_size index) 
{
    float v = 0.0f;
    cgltf_accessor_read_float(acc, index, &v, 1);
    return v;
}

static glm::vec3 ReadVec3(const cgltf_accessor* acc, cgltf_size index) 
{
    glm::vec3 v(0.0f);
    cgltf_accessor_read_float(acc, index, glm::value_ptr(v), 3);
    return v;
}

static glm::quat ReadQuat(const cgltf_accessor* acc, cgltf_size index) 
{
    float tmp[4] = {0,0,0,1}; // x,y,z,w expected by cgltf; adjust to glm ctor
    cgltf_accessor_read_float(acc, index, tmp, 4);
    // glm::quat(w, x, y, z)
    return glm::quat(tmp[3], tmp[0], tmp[1], tmp[2]);
}

static void ReadTimes(const cgltf_accessor* acc, std::vector<float>& outTimes) 
{
    outTimes.resize(acc->count);
    for (cgltf_size i = 0; i < acc->count; ++i)
    {
        outTimes[i] = ReadScalar(acc, i);
    }
}

static void ReadOutputVec3(const cgltf_accessor* acc, std::vector<glm::vec3>& outValues) 
{
    outValues.resize(acc->count);
    for (cgltf_size i = 0; i < acc->count; ++i)
    {
        outValues[i] = ReadVec3(acc, i);
    }
}

static void ReadOutputQuat(const cgltf_accessor* acc, std::vector<glm::quat>& outValues) 
{
    outValues.resize(acc->count);
    for (cgltf_size i = 0; i < acc->count; ++i)
    {
        outValues[i] = ReadQuat(acc, i);
    }
}
```

### 7.3 Parsing `animations`

Inside `GameObject::LoadGLTF(...)`, after you’ve created the result root object and parsed scene nodes, add:

```cpp
std::vector<std::shared_ptr<AnimationClip>> clips;

for (cgltf_size ai = 0; ai < data->animations_count; ++ai) 
{
    auto& anim = data->animations[ai];

    auto clip = std::make_shared<AnimationClip>();
    clip->name     = anim.name ? anim.name : "noname";
    clip->duration = 0.0f;

    // Map node* -> track index in clip->tracks
    std::unordered_map<cgltf_node*, size_t> trackIndexOf;

    auto GetOrCreateTrack = [&](cgltf_node* node) -> TransformTrack& 
    {
        if (auto it = trackIndexOf.find(node); it != trackIndexOf.end())
        {
            return clip->tracks[it->second];
        }

        TransformTrack track;
        track.targetName = node && node->name ? node->name : std::string{};
        clip->tracks.push_back(track);
        size_t idx = clip->tracks.size() - 1;
        trackIndexOf[node] = idx;
        return clip->tracks[idx];
    };

    for (cgltf_size ci = 0; ci < anim.channels_count; ++ci) 
    {
        auto& channel = anim.channels[ci];
        auto  sampler = channel.sampler;

        if (!channel.target_node || !sampler || !sampler->input || !sampler->output) 
        {
            continue;
        }

        std::vector<float> times;
        ReadTimes(sampler->input, times);
        if (times.empty()) 
        {
            continue;
        }

        auto& track = GetOrCreateTrack(channel.target_node);

        switch (channel.target_path) 
        {
        case cgltf_animation_path_type_translation: 
        {
            std::vector<glm::vec3> values;
            ReadOutputVec3(sampler->output, values);
            track.positions.resize(times.size());
            for (size_t i = 0; i < times.size(); ++i) 
            {
                track.positions[i].time  = times[i];
                track.positions[i].value = values[i];
            }
            break;
        }
        case cgltf_animation_path_type_scale: {
            std::vector<glm::vec3> values;
            ReadOutputVec3(sampler->output, values);
            track.scales.resize(times.size());
            for (size_t i = 0; i < times.size(); ++i) 
            {
                track.scales[i].time  = times[i];
                track.scales[i].value = values[i];
            }
            break;
        }
        case cgltf_animation_path_type_rotation: 
        {
            std::vector<glm::quat> values;
            ReadOutputQuat(sampler->output, values);
            track.rotations.resize(times.size());
            for (size_t i = 0; i < times.size(); ++i) 
            {
                track.rotations[i].time  = times[i];
                track.rotations[i].value = values[i];
            }
            break;
        }
        default:
            // Ignore weights/morph targets for now
            break;
        }

        // Update clip duration
        clip->duration = std::max(clip->duration, times.back());
    }

    clips.push_back(std::move(clip));
}
```

### 7.4 Attaching the component and registering clips

At the end of `LoadGLTF`, before freeing `data`:

```cpp
if (!clips.empty()) 
{
    auto animComp = new AnimationComponent();
    resultObject->AddComponent(animComp);
    for (auto& clip : clips) 
    {
        animComp->RegisterClip(clip->name, clip);
    }
}
```

---

## 8) Object activation toggles (`SetActive`)

We’ll need to hide certain effect objects (e.g., muzzle flash, bullet mesh) that might be present in the original animation.

Add to `GameObject`:

```cpp
private:
    bool m_active = true;

public:
    void SetActive(bool active) 
    { 
        m_active = active; 
    }

    bool IsActive() const 
    { 
        return m_active; 
    }
```

In `GameObject::Update` (top of the method):

```cpp
if (!m_active) 
{
    return;
}
```

---

## 9) Using it in `Game.cpp`

We already loaded the weapon object `gun` that carries the animation component.

```cpp
if (auto anim = gun->GetComponent<AnimationComponent>()) 
{
    // Hide effect sub-objects we don’t need
    if (auto bullet = gun->FindChildByName("bullet_33")) 
    {
        bullet->SetActive(false);
    }

    if (auto fire = gun->FindChildByName("BOOM_35")) 
    {
        fire->SetActive(false);
    }
    // Play the “shoot” animation once (no loop)
    anim->Play("shoot", /*loop=*/false);
}
```

If you set `loop = true`, the clip will play in a loop.

---

## 10) Result

* The animation plays (e.g., **shoot**).
* Extra effect nodes present in the original asset (`bullet_33`, `BOOM_35`) can be disabled for our needs.
* We can control **looping**, **start/stop**, and **bindings** are auto-built based on target names.

**Congratulations!** You now have a working foundation for loading and playing simple **glTF transform animations** in your engine — a big step toward understanding how games are built.
