# image decompiler

Arranges triangles randomly in order to minimize the difference between the framebuffer and the source image.

## __Dependencies:__

* OpenGL 4.3
* glew
* glfw3
* glm
* c++11 compiler
* cmake
  
## __Building:__

* mkdir build
* cd build
* cmake .. -G "*your platform*"
* cd ..
* cmake --build build --config Release

## __Running:__

* cd bin
* ./main imageName numTriangles

# How it works

The goal of the image decompiler is to produce:

1. A list of colored semi-transparent triangles approximating a supplied image
2. The image produced from drawing the triangle list as the program optimizes the list

The process of creating the list of triangles is as follows.

0. Read in parameters from user, such as maximum triangle count and image selection
1. Create N * 2 canvases 
2. Create N empty triangle lists
3. Add a small triangle with random color and random position to the current most optimal list
4. Create N - 1 variations of the current most optimal list. These are each a random 'distance' and random 'direction' away parameter-wise, and biased towards changing the most recently added triangle
5. For the N main canvases: draw the triangle lists into the canvases
6. For N secondary canvases: draw the absolute value of the difference between the source image and the primary canvas into the secondary canvas
7. For N secondary canvases: determine the average value of the difference (top mipmap value)
8. Choose the canvas with the lowest average difference and set it as the current most optimal list
9. Save a screenshot to file once every M minutes
10. Handle user input
11. If the triangle list is full goto 4
12. goto 3
