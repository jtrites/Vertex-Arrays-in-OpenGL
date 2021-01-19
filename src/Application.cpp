#include <GL/glew.h>  /* must be the 1st #include BEFORE ANY other #includes */
#include <GLFW/glfw3.h>
#include <iostream>

/* Vid#8 add includes to read, parse C++ external file: Basic.shader, and add to each Shader buffer */
#include <fstream> 
#include <string>
#include <sstream>

/* Vid#10: (14:30) add ASSERT(x) macro to validate a condition and call a breakpoint if true 
    using the MSVC function __debugbreak() */

#define ASSERT(x) if (!(x)) __debugbreak();

/* Vid#10: (16:20) GLCall(x) macro where (x) is the call function to Clear OpenGL Error(s)
    that calls the GLClearErrors() function */
/* Vid#10 (18:45) use macros to find out which line of code this errored function occurred.
    In GLLogCall(x) - changed to a string (#x) for printing the file name (__FILE__),
    and printing the line number (__LINE__) */

#define GLCall(x) GLClearErrors();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

/* Vid10: add new GLClearError() static function that returns void */

static void GLClearErrors()
{
    /* loop while there are errors and until GL_NO_ERROR is returned */
    while (glGetError() != GL_NO_ERROR);
}

/* Vid10: add new GLCheckErrors() static function that returns unsigned enum (int) in order */
/* (14:00) change GLCheckErrors() --> GLLogCall() */
/* (17:45) modify GLLogCall to accept parameters that allow the console 
    to printout the C++ source file, the line of code, and  the function name that errored */

static bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL Error] (" << error << ") " << function 
            << " " << file << ": " << line << std::endl;
        return false;
    }

    return true;
}

/*** Create a struct that allows returning multiple items ***/

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

/*** Vid#8 Add new function ParseShader to parse external Basic.shader file 
    returns - struct ShaderProgramSource above which contains two strings (variables)
    note: C++ functions are normally capable of only returning one variable ***/

static ShaderProgramSource ParseShader(const std::string& filepath)
{

    /* create enum class for each Shader type */
    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    /* open file */
    std::ifstream stream(filepath);

    /* define buffers for 2 Shaders:  vertexShader and fragmentShader */
    std::stringstream ss[2];

    /* set initial ShaderType = NONE */
    ShaderType type = ShaderType::NONE;

    /* parse file line by line */
    std::string line;
    while (getline(stream, line))
    {
        /* find "#shader" keyword */
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
                /* set mode to vertex */
                type = ShaderType::VERTEX;

            else if (line.find("fragment") != std::string::npos)
                /* set mode to fragment */
                type = ShaderType::FRAGMENT; 
        }
        else if (type != ShaderType::NONE)
        /* add each line to the corresponding buffer after detecting the ShaderType */
        {
            /* type is an index to push data into the selected array buffer, casted to a Shader int type,
                to add each new line plus newline char */

            ss[(int)type] << line << '\n';
        }
        else
        {
            /* Got non-introductory line out of sequence! Don't know what type to use! Consider asserting,
                or throwing an exception, or something, depending on how defensive you
                want to be with respect to the input file format. */
        }
    }

    /* returns a struct comprised of two ss strings */
    return { ss[0].str(), ss[1].str() };
}

/*** Vid#7: create static int CreateShader function with parameters:
    unsigned int type (used raw C++ type instead of OpenGL GLuint type to allow other non-OpenGL GPU driver implementations), 
    const std::string& source
    returns a static unsigned int, takes in a type and a string ptr reference to a source ***/

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    /* change GL_VERTEX_SHADER to type */
    unsigned int id = glCreateShader(type);

    /* returns a char ptr* src to a raw string (the beginning of our data) 
        assigned to source which needs to exist before this code is executed 
        pointer to  beginning of our data */
    const char* src = source.c_str();   

    /* specify glShaderSource(Shader ID, source code count, ptr* to memory address of ptr*, length) 
        as the source of our Shader */
    glShaderSource(id, 1, &src, nullptr);

    /* specify glCompileShader(Shader ID), then return the Shader ID */
    glCompileShader(id);

    /*error handling - query void glGetShaderiv(GLuint shader, GLenum pname, GLint *params); 
        i - specifies an integer
        v - specifies a vector (array) */
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE)
    {
        /* query message - length and contents 
           void glGetShaderiv(GLuint shader, GLenum pname, GLint *params); */
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);

        /* construct char message[length] array allocated on the stack */
        char* message = (char*)alloca(length * sizeof(char));

        /* glGetShaderInfoLog — Returns the information log for a shader object 
            void glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog); */
        glGetShaderInfoLog(id, length, &length, message);

        /* print the message to the console using std::cout */
        std::cout << "Failed to Compile " 
            << (type == GL_VERTEX_SHADER ? "vertex shader" : "fragment shader")
            << std::endl; 
        std::cout << message << std::endl;

        /* delete Shader using id and return error code = 0 */
        glDeleteShader(id);
        return 0;
    }
    return id;
}

/*** Vid#7: create static int CreateShader function with parameters:
    const string pointer vertexShader(actual source code),
    const string pointer fragmentShader (actual source code)
    returns a static int, takes in the actual source code of these two Shader strings ***/

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    /* glCreateProgram() return an unsigned int program */
    unsigned int program = glCreateProgram();

    /* create vertexShader object */
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);

    /* create fragmentShader object */
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    /* attach vs and fs Shader files, link and validate them to our program ID 
       void glAttachShader(GLuint program, GLuint shader); */
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    /* void glLinkProgram(GLuint program); */
    glLinkProgram(program);

    /* void glValidateProgram(	GLuint program); */
    glValidateProgram(program);

    /* finally, delete the intermediary *.obj files (objects vs and fs) of program ID
        and return an unsigned int program 
        void glDeleteShader(GLuint shader); */
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Vid#12 (8:10) OpenGL GLFW  Version 3.3 create an Open Context and Window with the Core Profile 
        GLFW_OPENGL_CORE_PROFILE 
        Note:  ONLY (GLFW_CONTEXT_VERSION_MAJOR, 2) and (GLFW_CONTEXT_VERSION_MINOR, 1) WORKS!!!
        All other combinations of ints (e.g. 2, 3) of later major/minor versions Fails 
        with the following console output msg:

        C:\Dev\Cherno\OpenGL\bin\Win32\Debug\OpenGL.exe (process 4936) exited with code -1.
        To automatically close the console when debugging stops, 
        enable Tools->Options->Debugging->Automatically close the console when debugging stops.
        Press any key to close this window . . .
     */

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_ANY_PROFILE, GLFW_OPENGL_CORE_PROFILE);


/****** Create a windowed mode window and its OpenGL context 
    glfwCreateWindow MUST BE PERFORMED BEFORE ANY glfwWindowHint(s) ******/

    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    //if (!window)
    if (window==NULL)
    {
        //glfwTerminate();
        return -1;
    }

    /*** Make the window's context current - this MUST BE PEFORMED BEFORE glewInit() !!! ***/
    glfwMakeContextCurrent(window);

    /* Vid#11 (9:00) - should sync our Swap with the monitor's refresh rate
        and produce a smooth color change transition */
    GLCall(glfwSwapInterval(1));

    /*** Vid#3: JT Added Modern OpenGL code here - MUST FOLLOW glfwMakeContextCurrent(window) ***/
    if (glewInit() != GLEW_OK)
    {
        std::cout << "glewInit() Error!" << std::endl;
    }

    /*** Vid#3: JT Added Print Modern OpenGL Version code here ***/
    std::cout << glGetString(GL_VERSION) << std::endl;

    /* Vid#9A: add 2nd set of (3) x, y, z vertex positions for 2nd inverted triangle added 
            to original right triangle forming a new Rectangle */
    /* Vid#8: modified to (3) x, y, and z vertex positions per LearnOpenGL */
    /* Vid#4: JT Define Vertex Buffer code based on Vid#2 example commmented out below */
    /* create float array of [3] verticies - (3) x, y, z vertex position pairs by Alt+Shift Legacy vertices --> Ctrl+c */
    
    /* Vid#9B: Vertex Buffer - remove 2 duplicate vertices of the 6 vertices in position[] to implement
            an Index Buffer */

    float positions[] = {
        -0.5f, -0.5f, 0.0f, // vertex 0
         0.5f, -0.5f, 0.0f, // vertex 1
         0.5f,  0.5f, 0.0f, // vertex 2
        -0.5f,  0.5f, 0.0f, // vertex 3
    };
    
    /* Vid9B: create Index Buffer using new indices[] array 
        note: must be unsigned but can use char, short, int, etc. */

    unsigned int indices[] = {
        0, 1, 2,        // 1st right triangle drawn CCW
        2, 3, 0         // 2nd inverted right triangle drawn CCW
    };

/* Vid#12: (10:00) Create Vertex Array Object (vao) BEFORE Creating Vertex Buffer (buffer) 
    Create unsigned int Vertex Array Object ID:  vao
    GLCall(glGenVertexArrays(numVAs, stored vao IDrefptr)) 
    GLCall(glBindVertexArray(VAO ID)) */

    unsigned int vao;
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));

/* Vid#5: Created Vertex Buffer (buffer) original glGenBuffers(), glBindBuffer(), and glBufferData() calls */
    /* glGenBuffer(int bufferID, pointer to memory address of unsigned int buffer) creates buffer and provides and ID */

/* Vid#10: (20:30) wrap GLCall() around these (3) gl calls 
    create unsigned int buffer
    GLCall(glGenBuffers(numBuf, &buffer));
    Bind or Select Buffer which is the target (type = GL_ARRAY_BUFFER, ID = buffer) 
    Specify the layout (type, size of data, offset) of the Vertex Buffer to be placed into the buffer */
       
    unsigned int buffer;
    GLCall(glGenBuffers(1, &buffer));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));
    GLCall(glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW));

 /*** Vid#5 - OpenGL Vertex Attributes - use attribute pointers to GPU Memory Layout
    for each primitive type that to be drawn on the screen ***/
    /* Enable or disable a generic vertex attribute array for index = 0 */

    GLCall(glEnableVertexAttribArray(0));

    /* When we specify the GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0)); ,
        index 0 (1st param) of this Vertex Array is going to be bound to the currently bound 
        glBindBuffer(GL_ARRAY_BUFFER, buffer)
     define an array of generic vertex attribute data
        index = 0 1st param,
        size = 3 2nd param for a (3) component vector that represents each Vertex position,
        symbolic constant = GL_FLOAT 3rd param,
        normalized = converted directly as fixed-point values (GL_FALSE) 4th param,
        stride = the amount of bytes between each Vertex based on
            2nd param vec2 (x, y, z) component position vector of 3 floats = 12 bytes,
        pointer = position has an offset pointer = 0 */

    GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0));


/* Vid9: new Index Buffer calls */
    /* glGenBuffer(int bufferID, pointer to memory address of unsigned int buffer) creates buffer and provides and ID */

/* Vid#10: (20:30) wrap GLCall() around these (3) gl calls 
    create unsigned int Index Buffer Object
    Bind or Select Buffer which is the target (type = GL_ELEMENT_ARRAY_BUFFER, ID = ibo)
    Specify the type, size of data to be placed into the buffer */

    unsigned int ibo;
    GLCall(glGenBuffers(1, &ibo));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

    /* Vid#8: (15:30) - replace with new Basic.shader test code */

    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    std::cout << "VERTEX" << std::endl;
    std::cout << source.VertexSource << std::endl;
    std::cout << "FRAGMENT" << std::endl;
    std::cout << source.FragmentSource << std::endl;

    /*** Vid#8 (15:10) - temporarily commented out for 1st test new 
        Basic.shader file and supporting Application.cpp code ***/

    /* Call to create vertexShader and fragmentShader above */
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);

    /* Vid#11 (3:40) Bind our Shader - now wrapped in GLCall() to check if shader was found */
    GLCall(glUseProgram(shader));


/* Vid#11 (3:45) glUniform — Specify the value of a uniform (vec4 = 4 floats) variable for the current program object */
    /* void glUniform4f(GLint location,
        GLfloat v0,
        GLfloat v1,
        GLfloat v2,
        GLfloat v3);
    
        each uniform gets assigned a unique ID name for referencing
        GLCall(int location = glGetUniformLocation(shader, "u_Color"));  // Retrieve value of "u_Color" from Shader
        ASSERT(location != -1); // means we could NOT find our uniform, WAS Found but NOT used, Or Errored when called.
        GLCall(glUniform4f(location, 0.2f, 0.3f, 0.0f, 1.0f)); is used to pass in the Blue Triangle "u_Color" 
            into the Fragment Shader in Basic.shader */

/* Vid#11: (7:00) */
    /* Retrieve the int location of the variable location */
    GLCall(int location = glGetUniformLocation(shader, "u_Color"));
    
    /* Check if location DID NOT return -1 */
    GLCall(ASSERT(location != -1));

    /* Determine the type of Data to send to the GPU's Shader - (4) floats 
        1st param - int location of these (4) floats */
    GLCall(glUniform4f(location, 0.8f, 0.3f, 0.8f, 1.0f));

/* Vid#12: (4:00) Unbind the Shader (shader), the vertex buffer (buffer), and the index buffer (ibo)
        by setting each = 0 and re-bind all (3) inside the Rendering while loop before the glDraw cmd */
    /* Vid#12 (11:10) add clear glBindVertexArray(0) binding */

    GLCall(glBindVertexArray(0));
    GLCall(glUseProgram(0));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));


/* Vid#11 (8:00) - Animate Loop: 1st define 4 float variables: r, g, b, and i */
    float r = 0.0f;             // red color float var initially set to zero
    float increment = 0.05f;    // color animation float increment var initially set to 0.05

    /* Games Render Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        GLCall(glClear(GL_COLOR_BUFFER_BIT));
     
        /* Vid#11 (8:15) copy & paste glUniform4f() GLCall here and wrap GLCall around 
            glUniform4f() and glDrawElements() Calls 
         Note: glUniform's CANNOT be changed between drawing ANY elements contained within a glDraw call 
            which means you can't draw one triangle in one color and the other triangle in another color */

        /* Vid#12: (4:45) Bind Shader (shader), Uniform (location), Vertex Buffer (buffer)
            and Index Buffer (ibo) BEFORE calling glDrawElements... */

        GLCall(glUseProgram(shader));                           // bind our shader
        GLCall(glUniform4f(location, r, 0.3f, 0.8f, 1.0f));     // setup uniforms

        /* Vid#12: (11:15) new method - add binding GLCall(glBindVertexArray(vao));
            and then bind Index Array Buffer (ibo) 
            Run & Test (F5) - this works b/c we are linking our Vertex Buffer to our Vertex Array Object */
        GLCall(glBindVertexArray(vao));                         // bind our Vertex Array Object
        GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));     // bind our Index Buffer (ibo)

    /* Vid#12: (11:00) comment out GLCalls to glBindBuffer(GL_ARRAY_BUFFER, buffer), 
        glEnableVertexAttribArray(0), 
        and glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0) */
        //GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer));          // bind our Vertex Array Buffer (buffer)
        //GLCall(glEnableVertexAttribArray(0));                   // enable vertex attributes of index 0
        //GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0));  // set vertex attributes


        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));  // Draw Elements call

        /* Vid#11 (8:30) check if r value > 1.0f --> set increment = -0.05f 
            else if r value < 0.0f --> set increment = 0.05f */
        
        if (r > 1.0f)
            increment = -0.05f;
        else if (r < 0.0f)
            increment = 0.05f;

        r += increment;

        /* Swap front and back buffers */
        GLCall(glfwSwapBuffers(window));

        /* Poll for and process events */
        GLCall(glfwPollEvents());
    }

    /* delete Shader */
    GLCall(glDeleteProgram(shader));

    glfwTerminate();
    return 0;
}