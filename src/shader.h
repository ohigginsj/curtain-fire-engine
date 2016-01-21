#pragma once

#include "game.h"
#include "util.h"

enum shader_id
{
    ShaderId_Sprite,
    ShaderId_Text,
    ShaderId_Rectangle,
    ShaderId_Circle,
    ShaderId_Polygon4,
    ShaderId_PostProcessor,

    ShaderId_Count,
};

struct shader
{
    GLuint GL_Id;
};

void
ShaderUse(shader* Shader)
{
    glUseProgram(Shader->GL_Id);
}

void
ShaderCompile(shader* Shader,
              const GLchar* VertexSource,
              const GLchar* GeometrySource,
              const GLchar* FragmentSource)
{
    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    Assert("Could not create Vertex Shader.", VertexShader);
    glShaderSource(VertexShader, 1, &VertexSource, 0);
    glCompileShader(VertexShader);
    GLint VertexSuccess = 0;
    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &VertexSuccess);
    if (!VertexSuccess)
    {
        GLchar VertexInfoLog[1024];
        glGetShaderInfoLog(Shader->GL_Id, 1024, 0, VertexInfoLog);
        printf("| ERROR::SHADER: Compile-time error: Type: VERTEX\n%s", VertexInfoLog);
    }

    GLuint GeometryShader = 0;
    if (GeometrySource)
    {
        GeometryShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(GeometryShader, 1, &GeometrySource, 0);
        glCompileShader(GeometryShader);
        GLint GeometrySuccess = 0;
        glGetShaderiv(GeometryShader, GL_COMPILE_STATUS, &GeometrySuccess);
        if (!GeometrySuccess)
        {
            GLchar GeometryInfoLog[1024];
            glGetShaderInfoLog(Shader->GL_Id, 1024, 0, GeometryInfoLog);
            printf("| ERROR::SHADER: Compile-time error: Type: FRAGMENT\n%s", GeometryInfoLog);
        }
    }

    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &FragmentSource, 0);
    glCompileShader(FragmentShader);
    GLint FragmentSuccess = 0;
    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &FragmentSuccess);
    if (!FragmentSuccess)
    {
        GLchar FragmentInfoLog[1024];
        glGetShaderInfoLog(Shader->GL_Id, 1024, 0, FragmentInfoLog);
        printf("| ERROR::SHADER: Compile-time error: Type: FRAGMENT\n%s", FragmentInfoLog);
    }

    Shader->GL_Id = glCreateProgram();
    glAttachShader(Shader->GL_Id, VertexShader);
    if (GeometrySource)
    {
        glAttachShader(Shader->GL_Id, GeometryShader);
    }
    glAttachShader(Shader->GL_Id, FragmentShader);
    glLinkProgram(Shader->GL_Id);

    GLint LinkSuccess = 0;
    glGetProgramiv(Shader->GL_Id, GL_LINK_STATUS, &LinkSuccess);
    if (!LinkSuccess)
    {
        GLchar LinkInfoLog[1024];
        glGetProgramInfoLog(Shader->GL_Id, 1024, 0, LinkInfoLog);
        printf("| ERROR::SHADER: Link-time error: Type: PROGRAM\n%s", LinkInfoLog);
    }

    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);
}

void
ShaderSetFloat(shader* Shader,
               const GLchar* Name,
               GLfloat Value)
{
    glUniform1f(glGetUniformLocation(Shader->GL_Id, Name), Value);
}

void
ShaderSetInteger(shader* Shader,
                 const GLchar* Name,
                 GLint Value)
{
    glUniform1i(glGetUniformLocation(Shader->GL_Id, Name), Value);
}

void
ShaderSetVector2f(shader* Shader,
                  const GLchar* Name,
                  glm::vec2* Value)
{
    glUniform2f(glGetUniformLocation(Shader->GL_Id, Name), Value->x, Value->y);
}

void
ShaderSetVector3f(shader* Shader,
                  const GLchar* Name,
                  glm::vec3* Value)
{
    glUniform3f(glGetUniformLocation(Shader->GL_Id, Name), Value->x, Value->y, Value->z);
}

void
ShaderSetVector4f(shader* Shader,
                  const GLchar* Name,
                  color* Color)
{
    glUniform4f(glGetUniformLocation(Shader->GL_Id, Name), Color->R, Color->G, Color->B, Color->A);
}

void
ShaderSetMatrix4f(shader* Shader,
                  const GLchar* Name,
                  glm::mat4* Value)
{
    glUniformMatrix4fv(glGetUniformLocation(Shader->GL_Id, Name), 1, GL_FALSE, glm::value_ptr(*Value));
}

void
LoadShaderFromFile(game_state* GameState,
                   const char* VertexFile,
                   const char* GeometryFile,
                   const char* FragmentFile,
                   shader_id ShaderId)
{
	char* VertexShaderCode = 0;
    if (VertexFile)
    {
	    LoadFileAsString(&GameState->RenderMemory, VertexFile, &VertexShaderCode);
    }

    char* GeometryShaderCode = 0;
    if (GeometryFile)
    {
	    LoadFileAsString(&GameState->RenderMemory, GeometryFile, &GeometryShaderCode);
    }

    char* FragmentShaderCode = 0;
    if (FragmentFile)
    {
	    LoadFileAsString(&GameState->RenderMemory, FragmentFile, &FragmentShaderCode);
    }

    shader* Shader = &GameState->Shaders[ShaderId];
    ShaderCompile(Shader, VertexShaderCode, GeometryShaderCode, FragmentShaderCode);
}
