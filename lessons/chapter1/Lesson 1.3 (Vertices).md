All objects rendered using modern graphics APIs exist in three-dimensional space. Therefore, each object has its own position in that space, defined by three coordinate axes: X, Y, and Z.

There are two different coordinate systems: right-handed and left-handed. What’s the difference? It’s in the name. In a **right-handed** coordinate system, if you point your right-hand thumb in the direction of the X-axis, your index finger in the direction of the Y-axis, then your middle finger (perpendicular to both) will point along the Z-axis. In this case, the Z-axis points **towards you**, out of the screen.

In a **left-handed** system, using your left hand in the same way, the Z-axis will point **away from you**, into the screen.

All models in a scene consist of primitive objects that form a mesh. These primitives are polygons, and most commonly, triangles. So any 3D model can be represented as a collection of triangles arranged in space.

To understand how rendering works, we’ll start with a simple triangle — the "Hello World" of computer graphics.

We should also mention the concept of **Normalized Device Coordinates (NDC)**. The screen has its own coordinate system where the X, Y, and Z values range from -1 to 1.

Each polygon is defined by its vertices. A triangle has three. So to define a triangle, we specify its three vertex coordinates. In our example:

* First vertex: (0, 0.5)
* Second vertex: (-0.5, -0.5)
* Third vertex: (0.5, -0.5)

We’re omitting the Z-coordinate here, assuming all points lie at Z = 0 — the screen plane.

It’s important to note that the **winding order** of vertices matters. Graphics APIs like OpenGL use **counterclockwise (CCW)** winding by default. This means that when connecting the vertices in the given order, they should form a triangle that is "facing forward".

Thus, the correct vertex order is:

1. (0.0f, 0.5f, 0.0f)
2. (-0.5f, -0.5f, 0.0f)
3. (0.5f, -0.5f, 0.0f)

Now, let’s create the data structure representing our triangle. We’ll use a `std::vector<float>` to store the vertex coordinates:

```cpp
#include <vector>

std::vector<float> triangleVertices = 
{
    0.0f,  0.5f, 0.0f,
   -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f
};
```

Congratulations! We've described our first triangle and prepared the data for rendering.

