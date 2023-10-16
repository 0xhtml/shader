# OpenGL pixel-sorting shader experimentation

![The Mona Lisa with pixel sorting based on luminescence applied based on a depth map](/img/example.png)

This is a little experiment to implement a pixel-sorting shader based on the following video [I Tried Sorting Pixels](https://youtu.be/HMmmBDRy-jE) by Acerola.

My implementation makes use of a fragment shader to sort the pixels into the correct order instead of doing the sorting in a compute shader.

The example picture is based on the [Mona Lisa](https://en.wikipedia.org/wiki/File:Mona_Lisa%2C_by_Leonardo_da_Vinci%2C_from_C2RMF_retouched.jpg) with this [depth map](https://www.reddit.com/r/StableDiffusion/comments/11qhmn1/sdbattle_week_4_controlnet_mona_lisa_depth_map).
