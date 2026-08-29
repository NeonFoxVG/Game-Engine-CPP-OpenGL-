Before we begin, be sure to include the necessary standard library header:

```cpp
#include <string>
```

We'll use `std::string` to store the source code of our shaders.

---

## Step 1: Vertex Shader — Writing and Understanding the Code

First, let’s define the source code of our **vertex shader**:

```cpp
std::string vertexShaderSource = R"(
  #version 330 core
  layout (location = 0) in vec3 position;

  void main() 
  {
      gl_Position = vec4(position.x, position.y, position.z, 1.0);
  }
)";
```

### Line-by-line explanation:

* `#version 330 core`
  Informs the GLSL compiler to use OpenGL 3.3 (core profile).

* `layout (location = 0) in vec3 position;`
  Declares a vertex input at location 0. It’s a 3D vector for vertex position.

* `void main()`
  Entry point of the shader program.

* `gl_Position = vec4(position.x, position.y, position.z, 1.0);`
  Converts the 3D position into a 4D clip-space vector. The `w` component is set to 1.0 (default for homogeneous coordinates).

---

## Step 2: Compiling the Vertex Shader (with Error Handling)

```cpp
GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
const char* vertexSourceCStr = vertexShaderSource.c_str();
glShaderSource(vertexShader, 1, &vertexSourceCStr, nullptr);
glCompileShader(vertexShader);

// Compilation error check
GLint success;
glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
if (!success) 
{
    char infoLog[512];
    glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
    std::cerr << "ERROR::VERTEX_SHADER_COMPILATION_FAILED\n" << infoLog << std::endl;
}
```

---

## Step 3: Fragment Shader — Writing and Understanding the Code

Now let’s write the **fragment shader**, which defines the color of each pixel:

```cpp
std::string fragmentShaderSource = R"(
  #version 330 core
  out vec4 FragColor;

  void main() 
  {
      FragColor = vec4(1.0, 0.0, 0.0, 1.0);
  }
)";
```

### Line-by-line explanation:

* `#version 330 core`
  Again, we specify OpenGL 3.3 core profile.

* `out vec4 FragColor;`
  This declares a 4-component output color: Red, Green, Blue, Alpha.

* `FragColor = vec4(1.0, 0.0, 0.0, 1.0);`
  Fully opaque red. R = 1.0, G = 0.0, B = 0.0, A = 1.0.

---

## Step 4: Compiling the Fragment Shader (with Error Handling)

```cpp
GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
const char* fragmentSourceCStr = fragmentShaderSource.c_str();
glShaderSource(fragmentShader, 1, &fragmentSourceCStr, nullptr);
glCompileShader(fragmentShader);

// Compilation error check
glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
if (!success) 
{
    char infoLog[512];
    glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
    std::cerr << "ERROR::FRAGMENT_SHADER_COMPILATION_FAILED\n" << infoLog << std::endl;
}
```

---

## Step 5: Creating and Linking the Shader Program

Now we combine the vertex and fragment shaders into a **single shader program**:

```cpp
GLuint shaderProgram = glCreateProgram();
glAttachShader(shaderProgram, vertexShader);
glAttachShader(shaderProgram, fragmentShader);
glLinkProgram(shaderProgram);

// Link error check
glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
if (!success) 
{
    char infoLog[512];
    glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
    std::cerr << "ERROR::SHADER_PROGRAM_LINKING_FAILED\n" << infoLog << std::endl;
}
```

---

## Step 6: Cleaning Up

Once the program has been successfully linked, we **no longer need the individual shader objects**, so we can delete them to free GPU memory:

```cpp
glDeleteShader(vertexShader);
glDeleteShader(fragmentShader);
```

This is good practice — OpenGL keeps the compiled code internally once the program is linked.
