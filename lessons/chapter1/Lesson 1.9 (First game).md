### Introduction
I created this game for you to demonstrate that you're already capable of applying your newly acquired knowledge in practice.
Let’s dive into the code for our game — the classic Snake! You probably know the rules: the snake moves across the field using the arrow keys (left, right, up, down), collecting fruits (or berries, or dots), and growing longer with each one. If the snake crashes into a wall or itself — the game ends. Every few fruits, the game speeds up a little. Whoever scores more points wins!

Everything is here — movement, score tracking, fruit collection, collision detection, and game restart. Let’s walk through exactly how all of this is implemented in the code.

---

### Header Files

At the top, we include standard libraries:

* OpenGL headers: `<GL/glew.h>`, `<GLFW/glfw3.h>`
* Standard C++ utilities like `<vector>`, `<random>`, `<chrono>`, `<map>`, `<string>`, `<iostream>`

---

### Data Structures

We define three custom structs:

* `Vec2` — stores a 2D position using floats. Useful for screen-space positions sent to shaders.
* `Vec2i` — integer grid coordinates. The playing field is a grid, and this struct represents each cell.
* `Vec3` — holds RGB color values (red, green, blue).

---

### Game Constants

We define:

* The number of grid cells horizontally and vertically.
* Snake movement speed (`UPDATE_INTERVAL`).
* Cell dimensions for converting grid to screen space.

---

### Game State Variables

These track everything about the game:

* Snake body (`std::vector<Vec2i>`), direction (`enum class Direction`), score, fruit position, and flags for whether the game has started or ended.
* Also a time accumulator to control movement pacing.

---

### Shader Code

We use two shaders:

* **Vertex shader:** receives a square’s coordinates and applies `uOffset` and `uScale` to position them on the screen correctly.
* **Fragment shader:** fills the square with a solid color, passed as the `uColor` uniform.

These shaders are extremely simple, making them ideal for a grid-based game.

---

### OpenGL Variables

We prepare OpenGL objects:

* VAO and VBO for drawing quads.
* Locations of our shader uniforms.
* Shader program ID.

---

### Bitmap Font System

Text is displayed by drawing letters as 5x5 pixel grids. Each character is defined in a `std::map<char, std::vector<int>>`, where 1 means a filled cell and 0 means transparent.

* This allows us to draw letters and numbers using the same grid logic as the rest of the game.
* The drawing functions loop through this data and render each character pixel-by-pixel.

---

### Game Initialization

* `InitGame()` calls `ResetGame()` and `SpawnFruit()`.
* `ResetGame()` sets the initial snake position and resets score, flags, and timers.
* `SpawnFruit()` picks a random cell that isn’t occupied by the snake.

This ensures a clean start every time.

---

### Main Game Loop

Found in `main()`:

1. We calculate `deltaTime` between frames.
2. We process input with `glfwPollEvents()`.
3. We update game logic by calling `UpdateGame(deltaTime)`.
4. We draw everything with `RenderGame(window)`.

The loop continues until the window is closed.

---

### Game Update Logic

Inside `UpdateGame()`:

* If the game has started and isn’t over, we accumulate time.
* Once enough time has passed, we move the snake.
* We compute a new head position based on direction.
* If the snake hits the wall or its own body, the game ends.
* If it eats a fruit:

  * We increase the score by 10.
  * Add a new fruit.
  * Every 5 fruits (50 points), we make the snake move faster.
* If no fruit was eaten, we remove the last tail segment.

---

### Rendering Process

`RenderGame()` handles all drawing:

1. Clear the screen with a dark background.
2. Use our shader program.
3. Bind VAO.
4. Call `DrawBorder()` to draw the frame and background grid.

Then:

* If game hasn’t started: `DrawStartScreen()` (title + instructions).
* If game is over: `DrawGameOver()` (overlay, score, restart hint).
* Otherwise: draw the snake and the score.

---

### Drawing Functions

* `DrawCell(position, color)` — draws a single grid cell.
* `DrawSnake()` — draws each segment, head is bright green, body is darker with gradient.
* `DrawScore()` — renders the score text at the top.
* `DrawBorder()` — draws outer walls and checkerboard background.
* `DrawStartScreen()` — shows title and instructions.
* `DrawGameOver()` — shows semi-transparent overlay and restart prompt.

---

### Keyboard Input

Handled by `KeyCallback()`:

* Before game starts: any key starts the game and moves right.
* If game is over and `R` is pressed: reset game and spawn fruit.
* During the game: arrow keys change direction (no direct reversal).

This function is registered with GLFW and is triggered automatically on key events.

---

### Cleanup

When the player closes the window:

* We delete VAO, VBO, and the shader program.
* Then we terminate GLFW.

This is good practice to free GPU and system memory.

---

### Summary

We've just covered the core components of a game — from rendering and input handling to the game loop and state updates. Now you can implement all of this on your own using the knowledge you've gained. Congratulations on building your first game from scratch! There's much more ahead.