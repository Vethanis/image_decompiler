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
