mkdir build
cd build
cmake ..

vcpkg install opengl-registry glfw3 glm

open sln
make lua-experiments startup project
Properties -> Linker -> Input, select <inherit from parent or project defaults>