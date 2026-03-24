#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>

class Shader {
public:
  unsigned int ID;

  Shader(const char* vertexPath, const char* fragmentPath);

  void use();

  void setFloat(const std::string &name, float value) const;

  void setVec3(const std::string &name, float x, float y, float z) const;

  void setMat4(const std::string &name, const float* matValue) const;

  void destroy();

private:
  void checkCompileErrors(unsigned int shader, std::string type);
};

#endif
