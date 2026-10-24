/**
 * Hello Triangle OpenGL ES 3.0
 * Example from the Book OpenGL ES 3.0 Second edition by
 * Dan Ginsburg and Budirijanto Purnomo
 */


#include <stdlib.h>
#include "VGEUtils.h"
#include "VGEUtils_win.h"


typedef struct {
    // Handle a program Object
    GLuint programObject;
} User_Data;



/*
    Create a shader object, load the shader from source and
    compile the shader
*/
GLuint LoadShader( GLenum type, const char *shaderSource )
{
    GLuint shader;
    GLint compiled;

    // Create the shader object
    shader = glCreateShader(type);
    if( shader == 0 ) return 0;

    // Load the shader source
    glShaderSource( shader, 1, &shaderSource, NULL);

    // Compile the shader object
    glCompileShader( shader );

    // Check the compile status
    glGetShaderiv( shader, GL_COMPILE_STATUS, &compiled );
    if( compiled == GL_FALSE )
    {

        GLint infoLen = 0;
        glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &infoLen );

        if( infoLen > 1 ) 
        {
            char *log = malloc( sizeof( char ) * infoLen );

            glGetShaderInfoLog( shader, infoLen, NULL, log );
            vgeLogMessage( "Error compiling shader:\n%s\n", log );
            free( log );
        }

        glDeleteShader( shader );
        return 0;
    }

    return shader;
}

/*
    Initialize the shader and the program object
*/

int Init( VGEContext *vgeContext )
{
    User_Data *userData = vgeContext->userData;

    char vShader[] = 
    "#version 300 es                        \n"
    "layout( location = 0) in vec4 vPosition;\n"
    "void main( void)                        \n"
    "{                                      \n"
    "\tgl_Position = vPosition;             \n"
    "}                                      \n";

    char fShader[] = 
    "#version 300 es                        \n"
    "precision mediump float;               \n"
    "out vec4 fragColor;                    \n"
    "void main( void )                      \n"
    "{                                      \n"
    "\tfragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}                                      \n";

    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint linked;

    // Load the vertex and fragment shaders
    vertexShader = LoadShader( GL_VERTEX_SHADER, vShader );
    fragmentShader = LoadShader( GL_FRAGMENT_SHADER, fShader );

    //  Create program
    programObject = glCreateProgram();

    if ( programObject == 0 )
    {
        return 0;
    }

    glAttachShader ( programObject, vertexShader );
    glAttachShader ( programObject, fragmentShader );
    glLinkProgram ( programObject );

    glGetProgramiv ( programObject, GL_LINK_STATUS, &linked);
    if ( linked == GL_FALSE )
    {
        GLint infoLen;
        glGetProgramiv ( programObject, GL_INFO_LOG_LENGTH, &infoLen );

        if ( infoLen > 1 )
        {
            char *log = malloc ( sizeof (char) * infoLen );
            glGetProgramInfoLog ( programObject, infoLen, NULL, log );
            
            vgeLogMessage ( "Error linking program:\n%s", log);
            free ( log );
        }

        glDeleteShader ( vertexShader );
        glDeleteShader ( fragmentShader );
        glDeleteProgram (programObject);

        return GL_FALSE;
    }

    // store the program object
    userData->programObject = programObject;

    glClearColor ( 0.3, 0.3, 0.3, 1.0);
    return GL_TRUE;
}


/*
    Draw a triangle using the shader pair created in Init
*/

void Draw ( VGEContext *vgeContext )
{
    User_Data *userData = vgeContext->userData;

    GLfloat vertices[] = {
        0.0f,   0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f,   -0.5f, 0.0f
    };

    // Set the viewport
    glViewport ( 0, 0, vgeContext->width, vgeContext->height);

    // Clear the color buffer
    glClear ( GL_COLOR_BUFFER_BIT );

    // Use the program object
    glUseProgram ( userData->programObject);

    // Load the vertex data
    glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, vertices );
    glEnableVertexAttribArray ( 0 );

    // Send data throught OpenGL pipeline
    glDrawArrays ( GL_TRIANGLES, 0, 3 );
}

void Shutdown ( VGEContext *vgeContext )
{
    User_Data *userData = vgeContext->userData;
    glDeleteProgram ( userData->programObject);
}

int esMain ( VGEContext *vgeContext )
{
    vgeContext->userData = malloc(sizeof ( User_Data ) );

    vgeCreateWindow ( vgeContext, "Vertex Game Engine - Android - Hello Triangle", 512, 512, VGE_WINDOW_RGB | VGE_WINDOW_DEPTH);
    vgeLogMessage ("Vertex Game Engine - Android - Hello Triangle");
    
    if ( !Init (vgeContext) ) return GL_FALSE;
    
    vgeRegisterShutdownFunc ( vgeContext, Shutdown);
    vgeRegisterDrawFunc ( vgeContext, Draw );

    return GL_TRUE;
}