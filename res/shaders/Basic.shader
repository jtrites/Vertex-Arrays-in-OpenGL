#shader vertex
#version 330 core

//in vec4 aPos;
in vec4 position;
//layout(location = 0) in vec3 aPos;
//layout(location = 0) in vec3 position;
 
void main()
{
    //gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    gl_Position = position;
}

#shader fragment
#version 330 core

//layout(location = 0) out vec4 FragColor;
out vec4 FragColor;

// Vid#11 (2:40) new uniform fragment shader code
//  u_ is a uniform variable

uniform vec4 u_Color;

void main()
{
    // Vid#11 (3:00) replace FragColor = vec4(0.0f, 0.0f, 1.0f, 1.0f); and set it in C++
    //FragColor = vec4(0.0f, 0.0f, 1.0f, 1.0f);
    FragColor = u_Color;
}