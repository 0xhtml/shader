#include "shader.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Shader {
    char* filename;
    GLuint compiled;
    struct Shader* next;
} *cache;

struct Shader* getCachedShader(const char* filename) {
    if (filename == NULL) return NULL;

    struct Shader* cached = cache;
    while (cached != NULL) {
        if (strcmp(cached->filename, filename) == 0) return cached;
        cached = cached->next;
    }

    cached = malloc(sizeof(struct Shader));
    if (cached == NULL) {
        printf("getCachedShader(%s): error allocating cache entry\n", filename);
        return NULL;
    }

    cached->filename = strdup(filename);
    cached->compiled = 0;
    cached->next = cache;

    cache = cached;

    return cached;
}

bool compileAndAttatchShader(struct Shader* shader, GLuint shaderProgram, GLenum type) {
    if (shader == NULL) return true;

    if (shader->compiled == 0) {
        FILE* fp = fopen(shader->filename, "r");
        if (fp == NULL) {
            printf("loadShader(%s): error opening file\n", shader->filename);
            return false;
        }

        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        rewind(fp);

        char* source = malloc(size + 1);
        if (source == NULL) {
            printf("compileAndAttatchShader(%s): error allocating buffer\n", shader->filename);
            fclose(fp);
            return false;
        }

        if (fread(source, 1, size, fp) != size) {
            printf("compileAndAttatchShader(%s): error reading data\n", shader->filename);
            fclose(fp);
            return false;
        }
        fclose(fp);

        source[size] = 0;

        shader->compiled = glCreateShader(type);
        glShaderSource(shader->compiled, 1, (const char**)&source, NULL);
        free(source);

        glCompileShader(shader->compiled);

        GLint success;
        glGetShaderiv(shader->compiled, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE) {

            GLint length;
            glGetShaderiv(shader->compiled, GL_INFO_LOG_LENGTH, &length);

            char infoLog[length];
            glGetShaderInfoLog(shader->compiled, length, NULL, infoLog);
            printf("error compiling shader %s:\n%s\n", shader->filename, infoLog);

            return false;
        }
    }

    glAttachShader(shaderProgram, shader->compiled);

    return true;
}

void detatchShader(struct Shader* shader, GLuint shaderProgram) {
    if (shader == NULL) return;
    assert(shader->compiled != 0);
    glDetachShader(shaderProgram, shader->compiled);
}

GLuint loadShaderProgram(const char* vertex, const char* fragment, const char* compute) {
    struct Shader* vertexShader = getCachedShader(vertex);
    struct Shader* fragmentShader = getCachedShader(fragment);
    struct Shader* computeShader = getCachedShader(compute);

    GLuint shaderProgram = glCreateProgram();

    if (!compileAndAttatchShader(vertexShader, shaderProgram, GL_VERTEX_SHADER)) return 0;
    if (!compileAndAttatchShader(fragmentShader, shaderProgram, GL_FRAGMENT_SHADER)) return 0;
    if (!compileAndAttatchShader(computeShader, shaderProgram, GL_COMPUTE_SHADER)) return 0;

    glLinkProgram(shaderProgram);

    detatchShader(vertexShader, shaderProgram);
    detatchShader(fragmentShader, shaderProgram);
    detatchShader(computeShader, shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (success == GL_FALSE) {
        GLint length;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &length);

        char infoLog[length];
        glGetProgramInfoLog(shaderProgram, length, NULL, infoLog);
        printf("error linking shader program:\n%s\n", infoLog);

        return 0;
    }

    return shaderProgram;
}

void freeShaderCache() {
    while (cache != NULL) {
        free(cache->filename);
        if (cache->compiled != 0) glDeleteShader(cache->compiled);
        struct Shader* next = cache->next;
        free(cache);
        cache = next;
    }
}
