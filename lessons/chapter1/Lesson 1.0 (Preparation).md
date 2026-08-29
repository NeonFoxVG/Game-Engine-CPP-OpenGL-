Welcome to the “zero” lesson. Here we’ll prepare the workspace, wire up third-party libraries, and get a first build running. By the end you’ll fully understand what happens in the provided `CMakeLists.txt` and why.

---

## 1) Folder layout

Create the following structure (names matter):

```
<repo-root>/
├─ CMakeLists.txt
├─ source/
│  └─ main.cpp
└─ thirdparty/
   ├─ glfw-3.4/
   │  └─ ... (GLFW sources)
   └─ glew/
      ├─ include/
      └─ build/
         └─ cmake/
             └─ ... (GLEW CMake files)
```

* `source/` — your app code (we’ll add a minimal `main.cpp` below).
* `thirdparty/glfw-3.4` — GLFW sources (3.4).
* `thirdparty/glew` — GLEW sources (must contain `include/` and `build/cmake/`).

---

## 2) Prerequisites

* **CMake ≥ 3.10**
* A C++17-capable compiler

  * Windows: MSVC (Visual Studio), or clang/MinGW
  * macOS: Apple Clang (Xcode command line tools)
  * Linux: GCC or Clang
* Platform SDK/Dev tools:

  * Windows: install “Desktop development with C++” (for MSVC)
  * macOS: `xcode-select --install`
  * Linux: install `build-essential` (Debian/Ubuntu) or equivalents

---

## 3) What this CMakeLists.txt does (line by line)

```cmake
cmake_minimum_required(VERSION 3.10)
```

Sets the minimum CMake version. We rely on features available since 3.10.

```cmake
project (GameDevelopmentProject)
```

Defines the project name. CMake also creates variables like `PROJECT_NAME`.

```cmake
set(PROJECT_SOURCE_FILES
  source/main.cpp
)
```

Lists your app sources. Start small with just `main.cpp`. As the engine grows, expand this list or switch to `target_sources()`.

```cmake
include_directories(source)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_FILES})
```

* Adds `source/` to the header search path (so `#include "..."` finds your headers).
* Builds an executable target named `GameDevelopmentProject`.

```cmake
# Add GLFW library
add_subdirectory(thirdparty/glfw-3.4 "${CMAKE_CURRENT_BINARY_DIR}/glfw_build")
include_directories(thirdparty/glfw-3.4/include)
```

Tells CMake to configure & build GLFW from sources, placing its build artifacts in `glfw_build/`. We also expose its `include/` so we can `#include <GLFW/glfw3.h>`.

```cmake
# Add Glew library
set(BUILD_UTILS OFF CACHE BOOL "utilities" FORCE)
add_subdirectory(thirdparty/glew/build/cmake "${CMAKE_CURRENT_BINARY_DIR}/glew_build")
include_directories(thirdparty/glew/include)
set_property(TARGET glew_s PROPERTY FOLDER GLEW)
set_property(TARGET glew   PROPERTY FOLDER GLEW)
```

* Disables GLEW utilities we don’t need.
* Builds GLEW via its CMake scripts into `glew_build/`.
* Adds GLEW headers to the search path.
* The `set_property(... FOLDER ...)` lines only affect how IDEs (e.g., Visual Studio) group targets — optional but nice for organization.

```cmake
# Link all thirdparty libraries
target_link_libraries(${PROJECT_NAME} 
    glfw 
    glew_s
)
```

Links our app with GLFW and **static** GLEW (`glew_s`). That’s all we need to open a window and load OpenGL entry points.

---

## 4) Minimal `main.cpp` to verify the setup

Put this into `source/main.cpp`. It's a basic template for any application:

```cpp
int main() 
{
    return 0;
}
```

---

## 5) What we achieved

* A clean, minimal CMake project that vendors GLFW & GLEW from source.
* A `main` function as the entry point to the application.

That’s it — your environment is ready. In the next lesson we’ll start the journey itself.
