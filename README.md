# C++/CUDA N-Body Gravity Simulation
N-Body gravity simulation written in C++ and CUDA

## 1. Project Overview
The simulation calculates the gravitational interactions of **100,000 particles** . Everything runs entirely on the GPU using openGL and CUDA. I've tested it and runs smoothly on
my rtx 2060 with T106 architecture until around 130 000 particles.

## 2. Physics
The simulation is grounded in Newton's law of universal gravitation:
$$F = G \frac{m_1 m_2}{r^2}$$

A softening factor is injected into the distance to prevent the slingshot effect when particles collide or get extremely close. In the kernel, an acceleration vector is calculated using:
$$a = G \frac{m_2}{r^2}$$

For the direction of the force, the kernel uses:
$$\hat{r} = \left( \frac{\Delta x}{r}, \frac{\Delta y}{r}, \frac{\Delta z}{r} \right)$$

## 3. Algorith efficiency
The theoretical complexity of an N-Body problem is O(N^2). 
The kernel utilizes a **Tiled Shared Memory** architecture to speed up performance. Every therad load "his part" to the shared memory of the block, once every therad has ended his operations the shared memory is cleaned and loaded again with new data avoiding multiples loads for the same data. 

## 4. Graphics 
The performance comes from bypassing the CPU and the PCI-Express bus completely during the physics update loop.
Using `cudaGraphicsMapResources` and `cudaGraphicsResourceGetMappedPointer`, the OpenGL Vertex Buffer Object (VBO) containing the instance data is directly mapped to a CUDA memory pointer. The CUDA physics kernel writes the new positions directly into the OpenGL buffer residing in the GPU VRAM. Once the kernel finishes, the resource is unmapped, and OpenGL renders the updated state instantly.

## 5. Build and Usage
The project uses CMake (minimum version 3.1 on your system.

**Build Steps:**
```bash
mkdir build
cd build
cmake ..
make
cd ..
```

**Usage:**
The executable takes an optional command-line argument to switch between simulation states.

* **Standard Cube:** Generates a random cubic distribution of particles with slight mass variations (it has 2 way outproportioned particles to add caos to the simulation).
    ```bash/
    ./build/nbody 0
    ```
* **Black Hole :** Generates a rotating disc of particles orbiting a supermassive central body, simulating galactic kinematics.
    ```bash
    ./build/nbody 1
    ```

Controls: Standard `WASD` for camera movement and Mouse for look direction. Press `ESC` to exit.

## 6. Simulation Showcase

Here are two recordings of the simulation running in real-time on the RTX 2060. 

### Standard Cube Mode
[![Standard Cube Simulation](https://img.youtube.com/vi/P3asqAdBewI/maxresdefault.jpg)](https://www.youtube.com/watch?v=P3asqAdBewI)

### Black Hole Galaxy Mode
[![Black Hole Simulation](https://img.youtube.com/vi/C-pzLlpKUTo/maxresdefault.jpg)](https://www.youtube.com/watch?v=C-pzLlpKUTo)
