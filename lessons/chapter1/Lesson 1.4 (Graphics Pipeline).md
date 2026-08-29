So, we have the geometry data. Now we need to understand how to render it. The path that vertex data takes goes through the **graphics pipeline**. In other words, the journey from geometry to the final image on the screen goes through this pipeline.

What is the graphics pipeline?
It is a sequence of stages that are linearly connected, with some stages being programmable. These programmable stages allow you to influence how the image is formed, which gives a great deal of flexibility and variety in the final output.

Let’s go step by step:

1. **Vertex Shader.**
   The data is first sent to the **vertex shader**.
   A vertex shader is a program that runs on the GPU. It receives a set of points (vertices) and positions them in screen space, forming the base shape of an object or mesh. Then it passes the data to the next stage.

2. **Primitive Assembly.**
   In this phase, the vertices processed by the vertex shader are assembled into complete primitives (like triangles). We can't directly affect this stage yet.

3. **Rasterization.**
   The rasterizer takes these primitives and fills their area with pixels (called fragments). The output of this stage is a collection of fragments that cover the area of the shape.

4. **Fragment Shader.**
   This is another GPU program, but now it works with individual fragments. The fragment shader determines the color of each pixel. This allows us to apply different colors or effects to different parts of the object.

5. **Testing and Blending.**
   After fragment shading, the pixels go through various tests (like depth testing) and blending operations. This ensures that the new pixels are correctly combined with what’s already in the frame buffer.

6. **Final Output.**
   Once all processing is complete, the resulting image is written to the screen buffer and displayed.

As you can see, there are two main stages marked in green — the **vertex shader** and the **fragment (pixel) shader**. These are the programmable stages where we have the most control.

Let’s now take a closer and deeper look at these.