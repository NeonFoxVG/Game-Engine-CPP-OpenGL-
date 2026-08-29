# Anchors + Pivots + `RectTransformComponent` — full UI layout pass

We’ve already got interactive UI (buttons, text). Now we’ll add a **core layout concept** used in real engines: **anchors**. In short:

* **Pivot** — the local origin *inside the element’s own rectangle* (for drawing/positioning that element).
* **Anchor** — the origin for **children** of a parent UI element (where a child’s positioning starts from within the parent).

So if `A` is the parent and `B` is a child:

* `A` has a **pivot** (used to place `A` on screen).
* `A` has an **anchor** (used by descendants).
* `B` has its own **pivot** (used to place `B`) **relative to** `A`’s **anchor**.

This gives us responsive behavior when parent size/pos changes.

---

## 1) Add `RectTransformComponent`

**Files:**
`scene/components/ui/RectTransformComponent.h`
`scene/components/ui/RectTransformComponent.cpp`

**Private fields:**

```cpp
glm::vec2 m_size   {0.0f, 0.0f};   // width, height of this UI box
glm::vec2 m_anchor {0.0f, 0.0f}; // normalized [0..1] point inside parent (child origin)
glm::vec2 m_pivot  {0.0f, 0.0f}; // normalized [0..1] origin inside self
```

**Public API:**

```cpp
// getters / setters
const glm::vec2& GetSize()   const;
const glm::vec2& GetAnchor() const;
const glm::vec2& GetPivot()  const;

void SetSize  (const glm::vec2& s);
void SetAnchor(const glm::vec2& a);
void SetPivot (const glm::vec2& p);

// JSON loader
void LoadProperties(const nlohmann::json& jsonObject) override
{
    if (jsonObject.contains("size"))
    {
        auto& sizeObj = jsonObject["size"];
        SetSize(glm::vec2(
            sizeObj.value("x", 1.0f),
            sizeObj.value("y", 1.0f)
        ));
    }
    if (jsonObject.contains("anchor"))
    {
        auto& anchorObj = jsonObject["anchor"];
        SetAnchor(glm::vec2(
            anchorObj.value("x", 0.0f),
            anchorObj.value("y", 0.0f)
        ));
    }
    if (jsonObject.contains("pivot"))
    {
        auto& pivotObj = jsonObject["pivot"];
        SetPivot(glm::vec2(
            pivotObj.value("x", 0.0f),
            pivotObj.value("y", 0.0f)
        ));
    }
}
// Reads: "size": { "x":..., "y":... }, "anchor": { "x":..., "y":... }, "pivot": { "x":..., "y":... }

// Screen-space resolver (key method)
glm::vec2 GetScreenPosition(); // returns the anchor-aligned *screen* position for this object
```

**Implementation notes:**
```cpp
glm::vec2 RectTransformComponent::GetScreenPosition() 
{
    auto* parent = GetOwner()->GetParent();
    // If there's no parent, or parent has no RectTransform,
    // we consider we've exited the UI tree (e.g., hit Canvas boundary)
    if (!parent || !parent->GetComponent<RectTransformComponent>()) 
    {
        return GetOwner()->GetPosition2D();
    }

    auto rect = parent->GetComponent<RectTransformComponent>();
    // Parent anchor position in screen space:
    //    parentScreen + (parentAnchor - parentPivot) * parentSize
    glm::vec2 parentAnchorPos =
        rect->GetScreenPosition()
      + (rect->GetAnchor() - rect->GetPivot()) * rect->GetSize();

    // Final = local 2D pos + resolved parent anchor position
    return GetOwner()->GetPosition2D() + parentAnchorPos;
}
```

**Build integration:**

* Add to **CMakeLists** (sources + include).
* Add to **Scene::RegisterTypes**: `RectTransformComponent::Register();`
* Include headers where needed.

---

## 2) Move pivot/anchor out of old classes

We **remove all pivot/anchor fields** from:

* `UIElementComponent` (keep only virtual interaction methods).
* `ButtonComponent` (the size/rect will live in RectTransform now).
* `TextComponent` (pivot-aware placement will query RectTransform).

> The RectTransform is now the **single source of truth** for UI sizing/layout.

---

## 3) Update `ButtonComponent`

### 3.1 Hit testing

Old code used owner position + local `m_rect` + `m_pivot`. Replace with RectTransform:

```cpp
bool ButtonComponent::HitTest(const glm::vec2& pos) 
{
    auto rt = GetOwner()->GetComponent<RectTransformComponent>();
    if (!rt) 
    {
        return false;
    }

    glm::vec2 ownerPos = rt->GetScreenPosition();
    // Lower-left corner:
    glm::vec2 p1 = ownerPos - rt->GetSize() * rt->GetPivot();
    // Upper-right corner:
    glm::vec2 p2 = p1 + rt->GetSize();

    return (p1.x <= pos.x && p2.x >= pos.x &&
            p1.y <= pos.y && p2.y >= pos.y);
}
```

### 3.2 Rendering

We no longer use `m_rect`; we use RectTransform:

```cpp
void ButtonComponent::Render(CanvasComponent* canvas) 
{
    if (!canvas) 
    {
        return;
    }

    auto rt = GetOwner()->GetComponent<RectTransformComponent>();
    if (!rt) 
    {
        return;
    }

    glm::vec2 ownerPos = rt->GetScreenPosition();
    ownerPos -= rt->GetSize() * rt->GetPivot();
    glm::vec2 p1 = ownerPos;
    glm::vec2 p2 = ownerPos + rt->GetSize();

    // m_currentColor is a pointer to current state color (normal/hover/pressed)
    canvas->drawRect(p1, p2, *m_currentColor);
}
```

### 3.3 Load properties cleanup

Remove any previous `rect{ x, y }` parsing from `ButtonComponent::LoadProperties` — size is now read by `RectTransformComponent`.
And also remove `SetRect()/GetRect()` methods.

---

## 4) Update `TextComponent`

We had a helper that computed the starting pen position based on pivot; replace internal pivot with RectTransform:

```cpp
glm::vec2 TextComponent::GetPivotPos() 
{
    auto rt = GetOwner()->GetComponent<RectTransformComponent>();
    glm::vec2 pos = rt ? rt->GetScreenPosition() : GetOwner()->GetWorldPosition2D();

    // measure bounds of rendered text (same as before)
    glm::vec2 rect(0.0f); // rect.x = total advance width, rect.y = max glyph height
    for (char c : m_text) 
    {
        const auto& desc = m_font->GetGlyphDescription(c);
        rect.x += static_cast<float>(desc.advance);
        rect.y = std::max(rect.y, static_cast<float>(desc.height));
    }

    if (rt) 
    {
        pos -= rect * rt->GetPivot(); // shift by pivot inside the text box
    }

    // Align to integer pixels (avoid blurry text)
    pos.x = std::round(pos.x);
    pos.y = std::round(pos.y);
    return pos;
}
```

## 5) Update `CanvasComponent` size every frame

Before starting recursive UI render, make sure Canvas knows the screen size:

```cpp
void CanvasComponent::Update(float deltaTime)
{
    if (!m_active) 
    {
        return;
    }

    if (auto rt = GetOwner()->GetComponent<RectTransformComponent>()) 
    {
        auto& graphics = Engine::GetInstance().GetGraphicsAPI();
        const auto& viewport = graphics.GetViewport();
        rt->SetSize(glm::vec2(static_cast<float>(viewport.width),
                               static_cast<float>(viewport.height) ));
    }

    // ... then BeginRendering(); Render(); Flush(); (as you already do)
}
```

Canvas is the “root UI layer”, so setting its RectTransform size to the **current viewport** each frame keeps everything responsive. (It’s simple, not the most optimal, but perfect to demonstrate the concept.)

---

## 6) Scene wiring (JSON)

Open `scene.sc` and set up RectTransforms properly.

### 6.1 Main Canvas

```json
{
  "name": "MainCanvas",
  "components": [
    { "type": "CanvasComponent",
      "active": true
    },
    { "type": "RectTransformComponent",
      "anchor": { "x": 0.5, "y": 0.5 },      // Canvas anchor for its children is center
      "pivot":  { "x": 0.0, "y": 0.0 }       // Canvas’s own pivot (LL corner)
    }
  ],
  "children": [ /* buttons below */ ]
}
```

> `size` is ignored at load time (Canvas sets it every frame from viewport).

### 6.2 Play Button (centered, shifted up)

```json
{
  "name": "PlayButton",
  "components": [
    { "type": "ButtonComponent" },
    { "type": "RectTransformComponent",
      "size":   { "x": 150, "y": 50 },
      "anchor": { "x": 0.5, "y": 0.5 },
      "pivot":  { "x": 0.5, "y": 0.5 }
    }
  ],
  "position": { "x": 0, "y": 50, "z": 0 },   // shift up relative to center
  "children": [
    {
      "name": "Text",
      "components": [
        { "type": "TextComponent",
          "text": "Play",
          "font": { "path": "fonts/arial.ttf", "size": 24 }
        },
        { "type": "RectTransformComponent",
          "anchor": { "x": 0.5, "y": 0.5 },
          "pivot":  { "x": 0.5, "y": 0.5 }
        }
      ]
    }
  ]
}
```

### 6.3 Quit Button (centered, shifted down)

```json
{
  "name": "QuitButton",
  "components": [
    { "type": "ButtonComponent" },
    { "type": "RectTransformComponent",
      "size":   { "x": 150, "y": 50 },
      "anchor": { "x": 0.5, "y": 0.5 },
      "pivot":  { "x": 0.5, "y": 0.5 }
    }
  ],
  "position": { "x": 0, "y": -50, "z": 0 },  // shift down relative to center
  "children": [
    {
      "name": "Text",
      "components": [
        { "type": "TextComponent",
          "text": "Quit",
          "font": { "path": "fonts/arial.ttf", "size": 24 }
        },
        { "type": "RectTransformComponent",
          "anchor": { "x": 0.5, "y": 0.5 },
          "pivot":  { "x": 0.5, "y": 0.5 }
        }
      ]
    }
  ]
}
```

> Remove any old `rect` fields inside `ButtonComponent` JSON — size now belongs to `RectTransformComponent`.

---

## 7) Result & behavior

* Both **Play** and **Quit** remain **centered** with a vertical offset, regardless of window size.
* Resizing the window updates Canvas size → parent anchors/pivots propagate → children keep relative placement.
* Hit tests use RectTransform box; rendering uses RectTransform too.
* You now have the **Anchor + Pivot** workflow that mirrors major engines.

---

**Congrats!** You’ve just implemented a real UI layout backbone: **RectTransform with Anchors & Pivots**, hierarchical resolution, and responsive behavior on resize. This is the “fundamental rulebook” behind proper UI construction in game engines.