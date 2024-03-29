#include <shader.h>

#include <stdio.h>
#include <stdlib.h>

#include <texture.h>

GLuint shader_block;
GLuint shader_line;
GLuint shader_skybox;
GLuint shader_sun;
GLuint shader_deferred1;
GLuint shader_deferred2;
GLuint shader_shadow;
GLuint shader_pip;
GLuint shader_handitem;

static char* get_file_data(const char* path)
{
    FILE* f = fopen(path, "rb");
    if (!f)
    {
        fprintf(stderr, "Unable to open file:\n%s\n", path);
        return NULL;
    }

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
    glShaderSource(shader_id, 1, (const GLchar* const*)&shader_src, NULL);
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
        exit(-1);
    }

    return shader_id;
}

static GLuint create_shader_program(const char* vs_path, const char* fs_path)
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

void shaders_init()
{
    shader_block = create_shader_program(
        "shaders/block_vertex.glsl",
        "shaders/block_fragment.glsl"
    );

    shader_line = create_shader_program(
        "shaders/line_vertex.glsl",
        "shaders/line_fragment.glsl"
    );

    shader_skybox = create_shader_program(
        "shaders/skybox_vertex.glsl",
        "shaders/skybox_fragment.glsl"
    );

    shader_sun = create_shader_program(
        "shaders/sun_vertex.glsl",
        "shaders/sun_fragment.glsl"
    );

    shader_deferred1 = create_shader_program(
        "shaders/deferred1_vertex.glsl",
        "shaders/deferred1_fragment.glsl"
    );

    shader_deferred2 = create_shader_program(
        "shaders/deferred2_vertex.glsl",
        "shaders/deferred2_fragment.glsl"
    );

    shader_shadow = create_shader_program(
        "shaders/shadow_vertex.glsl",
        "shaders/shadow_fragment.glsl"
    );

    shader_pip = create_shader_program(
        "shaders/pip_vertex.glsl",
        "shaders/pip_fragment.glsl"
    );

    shader_handitem = create_shader_program(
        "shaders/handitem_vertex.glsl",
        "shaders/handitem_fragment.glsl"
    );
}

static GLint get_attrib_location(GLuint shader, char* name)
{
    GLint location = glGetUniformLocation(shader, name);
    if (location == -1)
       fprintf(stderr, "Shader attrib location is -1: %s\n", name);
    return location;
}

void shader_set_int1(GLuint shader, char* name, int value)
{
    glUniform1i(get_attrib_location(shader, name), value);
}

void shader_set_float1(GLuint shader, char* name, float value)
{
    glUniform1f(get_attrib_location(shader, name), value);
}

void shader_set_float3(GLuint shader, char* name, vec3 vec)
{
    glUniform3f(get_attrib_location(shader, name), vec[0], vec[1], vec[2]);
}

void shader_set_mat4(GLuint shader, char* name, mat4 matrix)
{
    glUniformMatrix4fv(get_attrib_location(shader, name), 1, GL_FALSE, matrix[0]);
}

void shader_set_texture_2d(GLuint shader, char* name, GLuint texture, int slot)
{
    shader_set_int1(shader, name, slot);
    texture_2d_bind(texture, slot);
}

void shader_set_texture_array(GLuint shader, char* name, GLuint texture, int slot)
{
    shader_set_int1(shader, name, slot);
    texture_array_bind(texture, slot);
}

void shader_set_texture_skybox(GLuint shader, char* name, GLuint texture, int slot)
{
    shader_set_int1(shader, name, slot);
    texture_skybox_bind(texture, slot);
}

void shader_use(GLuint shader)
{
    glUseProgram(shader);
}

static void shader_free(GLuint* shader)
{
    if (*shader)
    {
        glDeleteProgram(*shader);
        *shader = 0;
    }
}

void shaders_free()
{
    shader_free(&shader_block);
    shader_free(&shader_line);
    shader_free(&shader_skybox);
    shader_free(&shader_sun);
    shader_free(&shader_deferred1);
    shader_free(&shader_deferred2);
    shader_free(&shader_shadow);
    shader_free(&shader_pip);
    shader_free(&shader_handitem);
}