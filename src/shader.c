#include "stdio.h"
#include "stdlib.h"

#include "shader.h"

static char* get_file_data(const char* path)
{
    FILE* f = fopen(path, "rb");

    if (!f)
    {
        fprintf(stderr, "Unable to open file:\n%s\n", path);
        return NULL;
    }

    // get file size
    fseek(f, 0, SEEK_END);
    int data_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* file_content = malloc(data_size + 1);
    fread(file_content, 1, data_size, f);
    file_content[data_size - 1] = '\0';

    fclose(f);
    return file_content;
}

static GLuint compile_shader(const char* path, GLenum shader_type)
{
    char* shader_src = get_file_data(path);

    if (!shader_src)
        return 0;

    GLuint shader_id = glCreateShader(shader_type);
    glShaderSource(shader_id, 1, (const GLchar * const*)&shader_src, NULL);
    glCompileShader(shader_id);

    free(shader_src);

    GLint success;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char info[512];
        glGetShaderInfoLog(shader_id, 512, NULL, info);
        fprintf(stderr, "Failed to compile shader\n%s\n", path);
        fprintf(stderr, "%s\n", info);
        return 0;
    }

    return shader_id;
}

GLuint create_shader_program(const char* vs_path, const char* fs_path)
{
    GLuint vs_id = compile_shader(vs_path, GL_VERTEX_SHADER);
    GLuint fs_id = compile_shader(fs_path, GL_FRAGMENT_SHADER);

    if (!vs_id || !fs_id)
        return 0;
    
    GLuint shader_prog = glCreateProgram();
    glAttachShader(shader_prog, vs_id);
    glAttachShader(shader_prog, fs_id);
    glLinkProgram(shader_prog);

    GLint success;
    glGetProgramiv(shader_prog, GL_LINK_STATUS, &success);

    if (!success) 
    {
        char info[512];
        glGetProgramInfoLog(shader_prog, 512, NULL, info);
        fprintf(stderr, "Failed to link shader program\n");
        fprintf(stderr, "%s\n", info);
        return 0;
    }

    glDeleteShader(vs_id);
    glDeleteShader(fs_id);

    return shader_prog;
}