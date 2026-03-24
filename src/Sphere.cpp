#include "Sphere.h"
#include <glad/glad.h>
#include <cmath>

Sphere::Sphere(float radius, int sectors, int stacks, int maxInstances) {
  setupMesh(radius, sectors, stacks, maxInstances);
}

void Sphere::setupMesh(float radius, int sectors, int stacks, int maxInstances) {
  std::vector<float> vertices;
  std::vector<unsigned int> indices;

  const float PI = 3.14159265359f;

  for (int i = 0; i <= stacks; ++i) {
    float stackAngle = PI / 2 - i * PI / stacks;        
    float xy = radius * cosf(stackAngle);             
    float z = radius * sinf(stackAngle);             

    for (int j = 0; j <= sectors; ++j) {
      float sectorAngle = j * 2.0f * PI / sectors;           

      float x = xy * cosf(sectorAngle);             
      float y = xy * sinf(sectorAngle);             

      vertices.push_back(x);
      vertices.push_back(y);
      vertices.push_back(z);
    }
  }

  for (int i = 0; i < stacks; ++i) {
    float k1 = i * (sectors + 1);      
    float k2 = k1 + sectors + 1;      

    for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
      if (i != 0) {
        indices.push_back(k1);
        indices.push_back(k2);
        indices.push_back(k1 + 1);
      }
      if (i != (stacks - 1)) {
        indices.push_back(k1 + 1);
        indices.push_back(k2);
        indices.push_back(k2 + 1);
      }
    }
  }

  indexCount = indices.size();

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  glGenBuffers(1, &instanceVBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), 
              GL_STATIC_DRAW); 

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), 
              indices.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);  

  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
  glBufferData(GL_ARRAY_BUFFER, maxInstances * sizeof(Position3D), nullptr, 
               GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Position3D), (void*)0);

  glVertexAttribDivisor(1, 1);

  glBindBuffer(GL_ARRAY_BUFFER, 0); 
  glBindVertexArray(0);
}

void Sphere::updateInstances(const std::vector<Position3D>& positions) {
  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
  glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(Position3D), 
                  positions.data());
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Sphere::draw(int instanceCount) {
  glBindVertexArray(VAO);
  glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, instanceCount);
  glBindVertexArray(0);
}

Sphere::~Sphere() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  glDeleteBuffers(1, &instanceVBO);
}
