Let’s recall how we previously talked about a scene as a kind of space. In our case, we imagined it as a house or a large room with various objects placed inside. You understand that these objects are positioned within that room — they are brought in and placed at a specific point. Essentially, they are *born* at a specific location — the zero point, at the center of the room or the center of the house, the center of the world.

But if we place all the objects at this single origin point, they’ll overlap each other, and the result won’t make much sense visually. What we want instead is to be able to move them from one place to another, rotate them in space, and even change their size. That’s exactly why we use **transformations**.

So, what is a **transform**?

A transform is simply a combination of three main properties:

1. **Position** – This defines where the object is located in the space. Using it, we can move the object to a different location.

2. **Rotation** – This defines how the object is oriented in space, i.e., how it's rotated relative to the X, Y, and Z axes.

3. **Scale** – This defines the size of the object. It tells us whether the object is scaled up, scaled down, or remains at its original size.

But here’s the interesting part — or rather, the challenge:
A computer doesn’t understand words like *move*, *rotate*, or *scale*. It needs simple numbers, and not just numbers — those numbers have to be structured in a very specific way.

And that’s where **matrices** come in.

### What is a Matrix?

A matrix is just a table of numbers. For example, this is a 3×3 identity matrix:

```
1 0 0  
0 1 0  
0 0 1
```

If you look diagonally from the top-left to the bottom-right, you’ll see that all those diagonal elements are equal to 1 — that’s why it’s called the *identity* matrix. It doesn’t change the object at all when applied.

But once we start changing the values inside the matrix, it begins to affect the object in different ways — moving it, rotating it, or scaling it.

For now, we won’t go into the deep math behind it. What’s important to understand is this: a matrix allows us to *combine* and *express* our transformations — position, rotation, and scale — in a way the computer can understand.

In 3D (three-dimensional) environments, we use **4×4 matrices** for these transformations. This size is necessary because it lets us combine all three operations (translation, rotation, scaling) into a single matrix — and apply them all at once.

### How Does It Work?

Let’s say we have an object. We want to apply position, rotation, and scaling to it.

We start by creating an identity matrix — a “neutral” matrix that doesn’t do anything.
Then, step-by-step, we add transformations to it:

1. First, we apply **translation** using a function like:

   ```cpp
   model = glm::translate(model, position);
   ```

2. Next, we add **rotation**, which allows us to rotate the object around a specific axis by a certain angle.

3. Lastly, we add **scaling** to resize the object along the X, Y, and Z axes.

By combining these three transformations in order, we get what’s called the **model matrix** (also sometimes called the **world matrix**). This matrix describes how to convert an object from its **local space** (its own origin) into **world space** (the scene as a whole).

It’s like holding a cube in your hand — first you rotate it, then you move it to the left. That sequence of actions is the same as applying multiple transforms. And the matrix is just a way of recording those steps using numbers.

### Using Transformations in Our Project

Let’s try applying this concept in our current game project.

To make working with matrices easier, we’ll use the standard **GLM library** (OpenGL Mathematics). You can find the code and setup instructions in the resources attached to this video.

GLM allows us to perform all the essential 3D transformations we’re discussing now and will continue using in the future. It’s also well-integrated with our rendering API — remember, we’re using **OpenGL** to send all rendering data to the GPU.

### Implementation Time

To integrate transformations into our game, we’ll go into the `GameObject` class.
It’s time to start using transforms.