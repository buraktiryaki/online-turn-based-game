# online-turn-based-game
This project includes a client and server written in c++ and OpenGL. This is a simple turn-based strategy game that you can play with two people for now.
It uses tcp protocol for communication. Boost.asio library is used for the tcp protocol.

![Screen Shot](https://github.com/buraktiryaki/online-turn-based-game/blob/master/resources/screenshots/ss1.png)

## Dependencies
- [CMake](https://cmake.org/)
- [GLFW](https://github.com/glfw/glfw)
- [Boost](https://www.boost.org/)
- [Assimp](https://github.com/assimp/assimp)
- [GLM](https://github.com/g-truc/glm)
- [Dear ImGui](https://github.com/ocornut/imgui)

## Build and Run

### On Windows
```
md build
cd build
cmake -DCMAKE_PREFIX_PATH=c:\your_libs_dir
cmake ..
cmake --build . --config --config Release --target ALL_BUILD
```
Finally move `resources\` and `shaders\` into `build\` 

### On Linux
```
mkdir build
cd build
cmake ..
make
```

### Run
```
./server port
./client playerId playerName host port

./server 30000
./client 0 player1 127.0.0.1 30000
./client 1 player2 127.0.0.1 30000
```
