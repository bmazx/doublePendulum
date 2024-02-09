# Double Pendulum
A small program that renders and simulates a double pendulum in OpenGL

![screenshot_dp](.github/dp1.png)

# Building
Copy and clone the git url

Create the build directory for cmake and build within the directory
```
mkdir build
cd build
cmake ..
```
Build using ```make``` on linux

On Windows use ```cmake --build .```

# Edit with ImGui
The mass, length, angles, and gravity can be changed using Imgui

Press F1 while the program is running to open the settings window. 
You can then pause and configure the mass or lengths of the pendulums.

![screenshot_dp](.github/dpImgui.png)
