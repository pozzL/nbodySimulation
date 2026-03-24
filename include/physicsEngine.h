#pragma once
#include <cuda_runtime.h> 

extern "C" {

void updatePhysics(void* d_positions, void* d_velocities, float deltaTime, int numParticles);

}
