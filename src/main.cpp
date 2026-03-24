#include <iostream>
#include <vector>
#include <random>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include "physicsEngine.h"

#define CHECK_CUDA(call) \
do { \
  cudaError_t err = call; \
  if (err != cudaSuccess) { \
    std::cerr << "CUDA Error at " << __FILE__ << ":" << __LINE__ << " - " << cudaGetErrorString(err) << std::endl; \
    exit(EXIT_FAILURE); \
  } \
} while (0)

#include "Shader.h"
#include "Sphere.h"

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const int NUM_PARTICLES = 100000;
const float SPHERE_RADIUS = 0.05f;

glm::vec3 cameraPos   = glm::vec3(0.0f, 30.0f, 50.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);


float deltaTime = 0.0f;
float lastFrame = 0.0f; 

bool firstMouse = true;
float yaw   = -90.0f;	
float pitch =  0.0f; 
float lastX =  SCR_WIDTH / 2.0;
float lastY =  SCR_HEIGHT / 2.0;


void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; //y axis are inverted in openGL
  lastX = xpos;
  lastY = ypos;

  float sensitivity = 0.1f; 
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw   += xoffset;
  pitch += yoffset;

  if (pitch > 89.0f) pitch = 89.0f;
  if (pitch < -89.0f) pitch = -89.0f;

  glm::vec3 front;
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
  cameraFront = glm::normalize(front);
}

void processInput(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  float cameraSpeed = static_cast<float>(2.5 * deltaTime); //forcing 2.5 meters a 
                                                          //second 

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    cameraPos += cameraSpeed * cameraFront;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    cameraPos -= cameraSpeed * cameraFront;

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}


void generateCube(std::vector<Position3D>& positions, 
                  std::vector<float3>& velocities, int numParticles) {

  positions.clear();
  velocities.clear();
  velocities.resize(numParticles);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(-20.0f, 20.0f);
  std::uniform_real_distribution<float> disMass(10.0f, 10.0f);

  for (int i = 0; i < numParticles; ++i) {
    positions.push_back({dis(gen), dis(gen), dis(gen), disMass(gen)});
    velocities[i] = make_float3(0.0f, 0.0f, 0.0f); 
  }
  //inserting random mass on some for noise and caos in the animation
  positions[40].mass = 4000.0f;
  positions[500].mass = 100000.0f;
}


void generateBlackHole(std::vector<Position3D>& positions, 
                       std::vector<float3>& velocities, int numParticles) {

  positions.clear();
  velocities.clear();
  velocities.resize(numParticles);

  float G = 0.0001f;
  float massBH = 1000000.0f;

  positions.push_back({0.0f, 0.0f, 0.0f, massBH});
  velocities[0] = make_float3(0.0f, 0.0f, 0.0f);

  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> disRadius(10.0f, 60.0f);
  std::uniform_real_distribution<float> disAngle(0.0f, 2.0f * 3.14159265f);
  std::uniform_real_distribution<float> disHeight(-5.0f, 5.0f); 
  std::uniform_real_distribution<float> disMass(1.0f, 5.0f);
  std::uniform_real_distribution<float> noise(-0.3f, 0.3f);       

  for (int i = 1; i < numParticles; ++i) {
    float r = disRadius(gen);
    float theta = disAngle(gen);

    float x = r * cos(theta);
    float z = r * sin(theta);
    float y = disHeight(gen); 

    positions.push_back({x, y, z, disMass(gen)});

    float dist3D = sqrt(x*x + y*y + z*z);
    float speed = sqrt((G * massBH) / dist3D) * 0.7f; 

    float tx = (-z / r) + noise(gen);
    float ty = noise(gen) * 0.5f; 
    float tz = (x / r) +noise(gen);

    velocities[i] = make_float3(tx * speed, ty * speed, tz * speed);
  }
}


int main(int argc, char *argv[]) {
  cudaGraphicsResource_t cudaVBOResource;
  float3* d_velocities = nullptr;

  if (!glfwInit()) return -1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "n-body simulation", 
                                        NULL, NULL);
  if (window == NULL) {
    std::cerr << "Error creating GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

  glEnable(GL_DEPTH_TEST);

  Shader shader("shaders/default.vert", "shaders/default.frag");

  Sphere sphere(SPHERE_RADIUS, 18, 18, NUM_PARTICLES);

  std::vector<Position3D> spherePositions;
  std::vector<float3> host_velocities;

  if(argc > 1 &&  ( atol(argv[1]) || !atol(argv[1]) ) ) {
    if(atol(argv[1])) 
      generateBlackHole(spherePositions, host_velocities, NUM_PARTICLES);
    else
      generateCube(spherePositions, host_velocities, NUM_PARTICLES);
  }else 
    std::cout<<"You need to pass an argument 1 for black hole 0 for Cube \n"; 

  sphere.updateInstances(spherePositions);

  CHECK_CUDA(cudaMalloc((void**)&d_velocities, NUM_PARTICLES * sizeof(float3)));

  CHECK_CUDA(cudaMemcpy(d_velocities, host_velocities.data(), 
                        NUM_PARTICLES * sizeof(float3), cudaMemcpyHostToDevice));


  CHECK_CUDA(cudaGraphicsGLRegisterBuffer(&cudaVBOResource, 
                                          sphere.getInstanceVBO(), 
                                          cudaGraphicsRegisterFlagsNone));

  while (!glfwWindowShouldClose(window)) {

    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window);

    float4* d_positions;
    size_t num_bytes;

    CHECK_CUDA(cudaGraphicsMapResources(1, &cudaVBOResource, 0));

    CHECK_CUDA(cudaGraphicsResourceGetMappedPointer((void**)&d_positions, 
                                                    &num_bytes, cudaVBOResource));

    updatePhysics(d_positions, d_velocities, deltaTime, NUM_PARTICLES);

    CHECK_CUDA(cudaGraphicsUnmapResources(1, &cudaVBOResource, 0));

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    shader.use();

    glm::mat4 projection = glm::perspective(glm::radians(45.0f),  
                                            //FOV field of view
                                            (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                            //aspect ratio
                                            0.1f, 180.0f);
                                            //near and far plane of rendering

    shader.setMat4("projection", glm::value_ptr(projection));


    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    shader.setMat4("view", glm::value_ptr(view));




    sphere.draw(NUM_PARTICLES);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  CHECK_CUDA(cudaGraphicsUnregisterResource(cudaVBOResource));
  CHECK_CUDA(cudaFree(d_velocities));

  shader.destroy();
  glfwTerminate();

  return 0;
}
