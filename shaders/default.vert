#version 330 core

// attrib 0: contains the vertex for a sphere centered in 0, offset will be 
            //added to all of the vetex
layout (location = 0) in vec3 aPos;

//Attrib 1: contains an array of offsets to applay to attrib 1  
layout (location = 1) in vec3 aOffset;

uniform mat4 view; 
                  
uniform mat4 projection; 

void main()
{
   vec4 worldPosition = vec4(aPos + aOffset, 1.0);

   gl_Position = projection * view * worldPosition;
}
