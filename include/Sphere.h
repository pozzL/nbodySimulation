#ifndef SPHERE_H
#define SPHERE_H

#include <vector>

struct Position3D {
  float x;
  float y;
  float z;
  float mass;
};

class Sphere {
public:
  Sphere(float radius, int sectors, int stacks, int maxInstances = 100000);    

  void updateInstances(const std::vector<Position3D>& positions);

  void draw(int instanceCount);

  unsigned int getInstanceVBO() const { return instanceVBO; }

  ~Sphere();

private:
  unsigned int VAO, VBO, EBO, instanceVBO;
  int indexCount;
  void setupMesh(float radius, int sectors, int stacks, int maxInstances);
};

#endif 
