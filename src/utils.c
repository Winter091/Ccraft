#include <utils.h>

#include <stdlib.h>

GLuint opengl_create_vao()
{
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    return VAO;
}

GLuint opengl_create_vbo(const void* vertices, size_t buf_size)
{
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, buf_size, vertices, GL_STATIC_DRAW);
    return VBO;
}

GLuint opengl_create_vbo_cube()
{
    static const float vertices[] = {
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    
    return opengl_create_vbo(vertices, sizeof(vertices));
}

GLuint opengl_create_vbo_quad()
{
    static const float vertices[] = {
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
        
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f
    };

    return opengl_create_vbo(vertices, sizeof(vertices));
}

GLuint opengl_create_fbo()
{
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    return fbo;
}

void opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity,
                           GLsizei length, const GLchar* message, const void* userParam)
{
    char* _source;
    char* _type;
    char* _severity;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API_ARB:
            _source = "API";
            break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
            _source = "WINDOW SYSTEM";
            break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
            _source = "SHADER COMPILER";
            break;
        case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
            _source = "THIRD PARTY";
            break;
        case GL_DEBUG_SOURCE_APPLICATION_ARB:
            _source = "APPLICATION";
            break;
        case GL_DEBUG_SOURCE_OTHER_ARB:
            _source = "UNKNOWN";
            break;
        default:
            _source = "UNKNOWN";
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR_ARB:
            _type = "ERROR";
            break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
            _type = "DEPRECATED BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
            _type = "UDEFINED BEHAVIOR";
            break;
        case GL_DEBUG_TYPE_PORTABILITY_ARB:
            _type = "PORTABILITY";
            break;
        case GL_DEBUG_TYPE_PERFORMANCE_ARB:
            _type = "PERFORMANCE";
            break;
        case GL_DEBUG_TYPE_OTHER_ARB:
            _type = "OTHER";
            break;
        default:
            _type = "UNKNOWN";
    }

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH_ARB:
            _severity = "HIGH";
            break;
        case GL_DEBUG_SEVERITY_MEDIUM_ARB:
            _severity = "MEDIUM";
            break;
        case GL_DEBUG_SEVERITY_LOW_ARB:
            _severity = "LOW";
            break;
        default:
            _severity = "UNKNOWN";
    }

    fprintf(stderr,"%d: %s of %s severity, raised from %s: %s\n",
            id, _type, _severity, _source, message);
}