## Lesson: What a Font Is, and How We’ll Use It to Render Text

So, what **is** a font? It’s the foundation of text rendering and the way we’re going to use text in our engine.

You already know fonts live in special files (e.g. **.ttf**, **.otf**, etc.). You’ve seen them before. In fact, a font file is basically a set of **mathematical functions**: the file describes **glyphs** (individual characters from various alphabets). Each glyph has a **code** (unified), so from any character table we can find the same glyph by the same code and get its shape in a consistent style.

The font file stores a **table of glyphs**: for each glyph it stores a mathematical description of its shape. This means we can **scale** a font arbitrarily — as small or as large as we like — with no loss in quality, because the shape is defined analytically.

But our graphics engine doesn’t draw analytic curves; it draws **rasters** (bitmaps). So before we can use a font’s math, we must **rasterize** it to a specific size. After rasterization, we can treat glyph images like regular **textures** (e.g., draw them on a sprite). That’s the core concept behind font rendering in a raster pipeline.

I’ve prepared a font and put it into the `assets/fonts/` folder. I’ve also prepared a third-party library for rasterization: **FreeType**. It’s already added in the appropriate folder and wired up in CMake in the usual way.

Now let’s implement the basics.

---

## 1) `Font` class

Create a new folder: `source/font/` (or `engine/source/font/`, as fits your structure).
Inside, create `Font.h` and `Font.cpp`.

We’ll declare a `Font` class. We’ll be able to request rendering at a **given pixel size**.

* Fields:

```cpp
// Each glyph has a description (placement in atlas + metrics)
struct GlyphDescription 
{
    int x0, y0;  // lower-left corner in the atlas
    int x1, y1;  // upper-right corner in the atlas

    int width;   // rasterized glyph width
    int height;  // rasterized glyph height

    int advance; // horizontal advance (distance to next glyph start)
};

class Font 
{
public:
    int GetSize() const { return m_size; }

    const GlyphDescription& GetGlyphDescription(char asciiCode) const 
    {
        return m_descriptions[static_cast<unsigned char>(asciiCode)];
    }

    const std::shared_ptr<Texture>& GetTexture() const { return m_texture; }

private:
    // The chosen pixel size for this font instance
    int m_size = 0;

    // We’ll keep ASCII 0..127 (128 glyphs) for our learning purposes
    GlyphDescription m_descriptions[128];

    // The texture atlas containing all glyph bitmaps
    std::shared_ptr<Texture> m_texture;

    // FontManager needs to fill private fields:
    friend class FontManager;
};
```

Why **128**? Because we’ll use the standard **ASCII** table for this course: English letters, digits, punctuation, etc. That’s more than enough for our learning tasks.

Don’t forget to `#include "graphics/Texture.h"` (or your actual include path) for the `Texture` type.

---

## 2) `FontManager` (font buffer / manager)

We’ll also need a **manager** to load, store, and serve fonts of different families and sizes. Create `FontManager.h` and `FontManager.cpp` in the same `font` folder.

We’ll integrate **FreeType**:

* Forward declarations as required by FreeType:

```cpp
// Forward decls as FreeType expects:
typedef struct FT_LibraryRec_* FT_Library;
```

* Class outline:

```cpp
class FontManager 
{
public:
    ~FontManager();
    void Init();
    std::shared_ptr<Font> GetFont(const std::string& path, int size);

private:
    FT_Library m_fontLibrary = nullptr;

    // For each font path we store a map: size -> Font instance
    using FontFamily = std::unordered_map<int, std::shared_ptr<Font>>;
    std::unordered_map<std::string, FontFamily> m_fonts;
};
```

### Engine integration

Add a `FontManager` field to `Engine`:

```cpp
FontManager m_fontManager;
FontManager& GetFontManager() { return m_fontManager; }
```

In `Engine::Init()` (near the bottom, before `Application::Init()`), call:

```cpp
m_fontManager.Init();
```

---

## 3) FreeType initialization & shutdown

In `FontManager.cpp`:

```cpp
#include <ft2build.h>
#include FT_FREETYPE_H

void FontManager::Init() 
{
    FT_Error error = FT_Init_FreeType(&m_fontLibrary);
    if (error != FT_Err_Ok) 
    {
        // Initialization failed; handle (log) as needed
        m_fontLibrary = nullptr;
        return;
    }
}

FontManager::~FontManager() 
{
    if (m_fontLibrary) 
    {
        FT_Done_FreeType(m_fontLibrary);
        m_fontLibrary = nullptr;
    }
}
```

---

## 4) Loading or retrieving a font: `GetFont(path, size)`

This function will:

1. Try to find the requested font family and size in the cache,
2. If not found, **load** the font from memory,
3. Set **pixel size**,
4. **Rasterize** ASCII glyphs into a single RGBA atlas,
5. Create a `Texture` from the atlas,
6. Fill `Font`’s glyph descriptions,
7. Store in the cache and return it.

Pseudocode (with all the important steps):

```cpp
std::shared_ptr<Font> FontManager::GetFont(const std::string& path, int size) 
{
    // 1) Check cache
    if (auto famIt = m_fonts.find(path); famIt != m_fonts.end()) 
    {
        auto& fam = famIt->second;
        if (auto it = fam.find(size); it != fam.end())
        {
            return it->second;
        }
    }

    // 2) Load font file into memory
    auto buffer = Engine::GetInstance()->GetFileSystem()->LoadAssetFile(path);
    if (buffer.empty()) 
    {
        return nullptr;
    }

    // 3) Create FreeType face from memory
    FT_Face face = nullptr;
    FT_Error res = FT_New_Memory_Face(
        m_fontLibrary,
        reinterpret_cast<const FT_Byte*>(buffer.data()), buffer.size(),
        0, &face
    );
    if (res != FT_Err_Ok) 
    {
        return nullptr;
    }

    // 4) Set pixel size
    FT_Set_Pixel_Sizes(face, 0, size);

    // 5) Determine atlas size (power-of-two large enough for all ASCII glyphs)
    //    We’ll use a simple heuristic based on font metrics.
    //    height in 26.6 fixed-point → shift right by 6 to get pixels.
    const int lineHeight = (face->size->metrics.height >> 6);
    // Estimate a square that can fit 128 glyphs (rough heuristic)
    int maxDimension = static_cast<int>(std::sqrtf(128.0f) * (lineHeight + 1));
    int textureWidth = 1;
    while (textureWidth < maxDimension) textureWidth <<= 1;
    int textureHeight = textureWidth;

    // 6) Create RGBA buffer and fill with zeros (transparent)
    const size_t stride = textureWidth * 4;
    const size_t totalBytes = static_cast<size_t>(textureWidth) * textureHeight * 4;
    auto* atlas = new unsigned char[totalBytes];
    std::memset(atlas, 0, totalBytes);

    // 7) Rasterize ASCII 0..127 into the atlas
    int penX = 0;
    int penY = 0;

    auto font = std::make_shared<Font>();

    for (int c = 0; c < 128; ++c) 
    {
        // Load & render glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER) != FT_Err_Ok) 
        {
            // Failed glyph; skip but keep metrics sane
            font->m_descriptions[c] = {0,0,0,0, 0,0, 0};
            continue;
        }

        FT_Bitmap& bmp = face->glyph->bitmap;

        // New line if overflow
        if (penX + static_cast<int>(bmp.width) >= textureWidth) 
        {
            penX = 0;
            penY += lineHeight + 1;
        }

        // Copy glyph bitmap (grayscale) into RGBA atlas
        for (uint32_t row = 0; row < bmp.rows; ++row) 
        {
            for (uint32_t col = 0; col < bmp.width; ++col) 
            {
                int x = penX + static_cast<int>(col);
                int y = penY + static_cast<int>(row);
                if (x < 0 || x >= textureWidth || y < 0 || y >= textureHeight) continue;

                const unsigned char value = bmp.buffer[row * bmp.pitch + col];
                const size_t idx = static_cast<size_t>(y) * stride + x * 4;

                // Write grayscale into all RGB channels, and alpha too
                atlas[idx + 0] = value;
                atlas[idx + 1] = value;
                atlas[idx + 2] = value;
                atlas[idx + 3] = value;
            }
        }

        // Store glyph rect & metrics
        auto& gd = font->m_descriptions[c];
        gd.x0 = penX;
        gd.y0 = penY;
        gd.x1 = penX + static_cast<int>(bmp.width);
        gd.y1 = penY + static_cast<int>(bmp.rows);
        gd.width  = static_cast<int>(bmp.width);
        gd.height = static_cast<int>(bmp.rows);
        gd.advance = (face->glyph->advance.x >> 6); // 26.6 fixed to pixels

        // Advance cursor with 1px spacing
        penX += static_cast<int>(bmp.width) + 1;
    }

    // 8) Create texture from atlas
    font->m_texture = std::make_shared<Texture>(textureWidth, textureHeight, 4, atlas);
    font->m_size = size;

    // 9) Cache it
    m_fonts[path][size] = font;

    // 10) Cleanup
    delete[] atlas;
    FT_Done_Face(face);

    return font;
}
```

That’s the core of our font loading pipeline: we produce a **texture atlas** and store **per-glyph descriptors**.

---

## 5) Hooking fonts into `TextComponent`

Let’s extend `TextComponent` so it knows which font to use, its color, etc.

Add to `TextComponent`:

```cpp
private:
    std::string m_text;
    glm::vec4   m_color = glm::vec4(1.0f);
    std::shared_ptr<Font> m_font;

public:
    void SetText(const std::string& t) { m_text = t; }
    const std::string& GetText() const { return m_text; }

    void SetColor(const glm::vec4& c) { m_color = c; }
    const glm::vec4& GetColor() const { return m_color; }

    void SetFont(const std::shared_ptr<Font>& font) { m_font = font; }
    void SetFont(const std::string& path, int size) 
    {
        m_font = Engine::GetInstance().GetFontManager().GetFont(path, size);
    }
    const std::shared_ptr<Font>& GetFont() const { return m_font; }
```

And add `LoadProperties(const nlohmann::json& j)`:

```cpp
void TextComponent::LoadProperties(const nlohmann::json& j) 
{
    const std::string text = j.value("text", "");
    SetText(text);

    if (j.contains("font")) 
    {
        const auto& fj = j["font"];
        std::string fontPath = fj.value("path", "");
        int fontSize = fj.value("size", 12);
        if (!fontPath.empty()) 
        {
            SetFont(fontPath, fontSize);
        }
    }

    if (j.contains("color")) 
    {
        const auto& cj = j["color"];
        glm::vec4 col(
            cj.value("r", 1.0f),
            cj.value("g", 1.0f),
            cj.value("b", 1.0f),
            cj.value("a", 1.0f)
        );
        SetColor(col);
    }
}
```

In `Render(CanvasComponent* canvas)` we’ll soon draw the text. For now, we prepare preconditions:

```cpp
void TextComponent::render(CanvasComponent* canvas) 
{
    if (m_text.empty() || !m_font || !canvas) 
    {
        return;
    }

    // We’ll draw in the next lesson.
}
```

---

## 6) Pivot-aware placement helper

We’ll compute where to start drawing the text by accounting for the **pivot** and the **final rectangle** of the rendered string.

Add a small helper:

```cpp
glm::vec2 GetPivotPos() const 
{
    auto pos = m_owner->GetWorldPosition2D(); // our 2D helpers from earlier

    glm::vec2 rect(0.0f); // width/height of full text
    for (const char c : m_text) 
    {
        const auto& d = m_font->GetGlyphDescription(c);
        rect.x += static_cast<float>(d.advance);
        rect.y = std::max(rect.y, static_cast<float>(d.height));
    }

    // Shift position by pivot (0..1 in both axes)
    // Remember: text rendering starts from the LOWER-LEFT corner
    pos.x -= std::round(rect.x * m_pivot.x);
    pos.y -= std::round(rect.y * m_pivot.y);
    return pos;
}
```

This computes the **starting point** so that the full rendered string is aligned according to the pivot (e.g., 0.5,0.5 centers it).

---

## Where we are now

* We explained what a **font** is and why rasterization is needed.
* We created a `Font` class with **glyph descriptors** and a **texture atlas**.
* We built a `FontManager` that **initializes FreeType**, **loads** fonts from memory, **rasterizes ASCII** to a power-of-two RGBA atlas, and **caches** the result by `(path, size)`.
* We integrated `FontManager` into the `Engine`.
* We extended `TextComponent` with **font**, **color**, and **text**, added JSON loading, and prepared a **pivot-aware positioning** helper.

What remains is the most fun part — **actually drawing** the text using our 2D pipeline (sprite-like quads per glyph, sampling from the atlas, advancing by each glyph’s `advance`). We’ll do that in the **next lesson**.