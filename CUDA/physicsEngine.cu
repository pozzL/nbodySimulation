#include "physicsEngine.h"
#include <cuda_runtime.h>

#define BLOCK_SIZE 256   

__global__ 
void nbody_kernel_tiled(float4* positions, float3* velocities, float deltaTime, int numParticles) {

  int i = blockIdx.x * blockDim.x + threadIdx.x;

  int tx = threadIdx.x;

  float3 acceleration = make_float3(0.0f, 0.0f, 0.0f);

  __shared__ float4 sharedPos[BLOCK_SIZE]; 

  float4 myPos;
  if (i < numParticles) {
    myPos = positions[i]; 
  } else {
    myPos = make_float4(0.0f, 0.0f, 0.0f, 0.0f);
  }

  float G = 0.0001f; 
  float softening = 15.0f;

  int numTiles = (numParticles + BLOCK_SIZE - 1) / BLOCK_SIZE; //formula per fare cealing senza cast

  for (int tile = 0; tile < numTiles; tile++) {

    int targetIdx = tile * BLOCK_SIZE + tx;
    if (targetIdx < numParticles) {
      sharedPos[tx] = positions[targetIdx];
    } else {
      sharedPos[tx] = make_float4(0.0f, 0.0f, 0.0f, 0.0f); 
    }

    __syncthreads();

    if (i < numParticles) {

      #pragma unroll 32 
      for (int j = 0; j < BLOCK_SIZE; j++) {
        float4 otherPos = sharedPos[j];

        float dx = otherPos.x - myPos.x;
        float dy = otherPos.y - myPos.y;
        float dz = otherPos.z - myPos.z;

        float distSqr = dx*dx + dy*dy + dz*dz + softening; //r^2

        float invDist = rsqrtf(distSqr); //1 / r
        float invDistCube = invDist * invDist * invDist;

        float accel = G * otherPos.w * invDistCube; // m2 * G * 1 / r^3

        acceleration.x += dx * accel;
        acceleration.y += dy * accel;
        acceleration.z += dz * accel;
      }
    }

    __syncthreads();
  }

  if (i < numParticles) {
    float3 myVel = velocities[i];

    myVel.x += acceleration.x * deltaTime;
    myVel.y += acceleration.y * deltaTime;
    myVel.z += acceleration.z * deltaTime;

    myPos.x += myVel.x * deltaTime;
    myPos.y += myVel.y * deltaTime;
    myPos.z += myVel.z * deltaTime;

    velocities[i] = myVel;
    positions[i] = myPos;
  }
}

void updatePhysics(void* d_positions, void* d_velocities, float deltaTime, int numParticles) {
  int blocksPerGrid = (numParticles + BLOCK_SIZE - 1) / BLOCK_SIZE;

  nbody_kernel_tiled<<<blocksPerGrid, BLOCK_SIZE>>>(
    (float4*)d_positions, 
    (float3*)d_velocities, 
    deltaTime, 
    numParticles
  );

  cudaDeviceSynchronize();
}
