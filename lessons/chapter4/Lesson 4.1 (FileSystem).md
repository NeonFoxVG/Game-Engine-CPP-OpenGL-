Now it’s time to get familiar with the concept of *resources* and, more broadly, with how file loading works. As you already know, we’re building a cross-platform engine. That means our engine must run seamlessly across different platforms. And every platform has its own unique filesystem layout and its own way of handling file access — something we need to take into account.

So let’s do the following: we need to organize a unified and consistent way to access files. To achieve that, let’s agree on one simple rule — all resources (or, as they’re often called, *game assets*) will live in a single fixed folder. Let’s create this folder, call it `assets`, and place it in the root of our project. Great.

Let's create a CMake variable for holding path to that folder:

```
set(ASSETS_DIR "${CMAKE_SOURCE_DIR}/assets")
```

Next, we need to define the structure we want to follow. I suggest this approach: during development and during actual gameplay we should be able to reference files in the same way. Therefore, we’ll support two modes: *development mode* and *runtime mode*. The principle will be as follows: first, we’ll check the development folder; if we don’t find the asset there, we’ll fall back to the folder that sits next to the executable file.

To implement this, we’ll need to tweak some project files. First of all, we’re going to use the functionality provided by `std::filesystem`, which was introduced in the C++17 standard. So the first step is to update our CMake configuration. Open `CMakeLists.txt` for the engine and add two lines at the very top:

```
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
```

Do the same for your project’s `CMakeLists.txt`. That way, we enforce at least the C++17 standard.

The next step is to create a configuration file. Go to `src/engine` and create a new file called `config.h.in`. Then, back in the engine’s `CMakeLists.txt`, after setting up the project, add:

```
configure_file(
  ${PROJECT_SOURCE_DIR}/config.h.in
  ${PROJECT_BINARY_DIR}/config.h
)
```

This generates a configuration header that can hold various constants. For example, inside `config.h.in` we can write:

```cpp
#pragma once
#define ASSETS_ROOT "@ASSETS_DIR@"
```

Here, `ASSETS_DIR` is a CMake variable that we’ll define, and it will be substituted into the header during build time. That gives us a global macro storing the path to our assets directory (handy for development mode).

Now we need to tell our project to use this config file. At the bottom of `CMakeLists.txt`, add:

```
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_BINARY_DIR})
```

One more useful step: let’s make sure that during the build process, the entire `assets` folder is copied next to our executable. That way, when we launch the built game, the resources are already in the right place. Add this to your CMake:

```
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${ASSETS_DIR}
    ${CMAKE_BINARY_DIR}/assets
)
```

Perfect. Now let’s move on to the cross-platform file access mechanism itself. We’ll create a small wrapper called `FileSystem`. Inside `engine/source`, create a folder `io`, and in it two files: `FileSystem.h` and `FileSystem.cpp`. Add them to the engine’s CMake and includes as usual.

In `FileSystem.h`, declare:

```cpp
namespace eng 
{
class FileSystem 
{
public:
    std::filesystem::path GetExecutableFolder() const;
    std::filesystem::path GetAssetsFolder() const;
};
}
```

In `FileSystem.cpp`, implement `GetExecutableFolder`. Since this depends on the platform, we’ll use OS-specific calls (Windows uses `GetModuleFileName`, macOS uses `_NSGetExecutablePath`, Linux reads from `/proc/self/exe`). We strip the executable name and return the folder path.

```cpp

#if defined _WIN32
#include <windows.h>
#elif defined (__APPLE__)
#include <mach-o/dyld.h>
#elif defined (__linux__)
#include <unistd.h>
#include <limits.h>
#endif

//...

std::filesystem::path FileSystem::GetExecutableFolder() const
{
#if defined _WIN32
    wchar_t buf[MAX_PATH];
    GetModuleFileNameW(NULL, buf, MAX_PATH);
    return std::filesystem::path(buf).remove_filename();
#elif defined(APPLE)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string tmp(size, '\0');
    _NSGetExecutablePath(tmp.data(), &size);
    return std::filesystem::weakly_canonical(std::filesystem::path(tmp)).remove_filename();
#elif defined(linux)
    return std::filesystem::weakly_canonical(std::filesystem::read_symlink("/proc/self/exe")).remove_filename();
#else
    return std::filesystem::current_path();
#endif
}
```

Next, implement `GetAssetsFolder`. The logic will be:

1. First, try to use the development path from our `ASSET_ROOT` macro (declared in `config.h`). If it exists, return it.
2. Otherwise, fall back to `<executable_folder>/assets`, which is where CMake copies resources during the build.

This way, during development we load assets directly from the source tree, but in a packaged build we use the bundled assets.

```cpp
std::filesystem::path FileSystem::GetAssetsFolder() const
{
#if defined (ASSETS_ROOT)
    auto path = std::filesystem::path(std::string(ASSETS_ROOT));
    if (std::filesystem::exists(path))
    {
        return path;
    }
#endif
    return std::filesystem::weakly_canonical(GetExecutableFolder() / "assets");
}
```

Also add a `FileSystem` member to Engine. Go to `Engine.h` and add this member with getter.

Now that we have a working file system, let’s actually try loading something. Since our next topic is *textures*, we’ll start with images. I’ve prepared a lightweight, header-only image loading library called **stb\_image**. You’ll find it under `thirdparty/stb_image`. Because it’s header-only, you don’t need any extra CMake setup, just add:

```
include_directories(thirdparty/stb_image)
```

Do this both in the engine and in the client project so both can use it.

I’ve also included a sample texture, `brick.jpg`. Place it inside the `assets` folder.

Now in `Game.cpp`, include stb\_image:

```cpp
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
```

Then let’s load the texture using our file system:

```cpp
auto& fs = eng::Engine::GetInstance().GetFileSystem();
auto path = fs.GetAssetsFolder() / "brick.jpg";

int width, height, channels;
unsigned char* data = stbi_load(path.string().c_str(), &width, &height, &channels, 0);

if (data) 
{
    std::cout << "Loaded image!" << std::endl;
    // don’t forget to free the image later with stbi_image_free(data);
}
```

If you see `"Loaded image!"` in the console, congratulations — your file system works, and so does the image loader.

From here, we’re ready to move on to the next big topic: **textures**.