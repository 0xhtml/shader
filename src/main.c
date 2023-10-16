#define _GNU_SOURCE

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <assert.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdint.h>

void GLAPIENTRY msg(GLenum src, GLenum type, GLenum id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    printf("%s\n", message);
}

unsigned int loadShader(const char* filename, int type) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        printf("file not found\n");
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char* buffer = malloc(fsize + 1);
    if (!buffer) {
        printf("file too big\n");
        return 0;
    }

    fread(buffer, fsize, 1, fp);
    buffer[fsize] = 0;
    fclose(fp);

    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, (const char**)&buffer, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[512];
        glGetShaderInfoLog(shader, 512, NULL, info);
        printf("%s\n", info);
        return 0;
    }

    return shader;
}

unsigned int linkShaderProgram(unsigned int vertex, unsigned int fragment, unsigned int compute) {
    unsigned int shaderProgram = glCreateProgram();
    if (vertex) glAttachShader(shaderProgram, vertex);
    if (fragment) glAttachShader(shaderProgram, fragment);
    if (compute) glAttachShader(shaderProgram, compute);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char info[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, info);
        printf("%s\n", info);
        return 0;
    }

    return shaderProgram;
}

unsigned int linkFragmentProgram(unsigned int vertex, const char* fragment) {
    unsigned int fragmentShader = loadShader(fragment, GL_FRAGMENT_SHADER);
    if (!fragmentShader) return 0;
    unsigned int program = linkShaderProgram(vertex, fragmentShader, 0);
    glDeleteShader(fragmentShader);
    return program;
}

unsigned int loadTexture(const char* filename, int type) {
    png_image image = { .version = PNG_IMAGE_VERSION };

    if (!png_image_begin_read_from_file(&image, filename)) {
        printf("%s\n", image.message);
        return 0;
    }

    image.format = PNG_FORMAT_RGBA;

    png_bytep buffer = malloc(PNG_IMAGE_SIZE(image));
    if (buffer == NULL) {
        printf("image too big\n");
        return 0;
    }

    if (!png_image_finish_read(&image, NULL, buffer, 0, NULL)) {
        printf("%s\n", image.message);
        free(buffer);
        return 0;
    }
    png_image_free(&image);

    uint32_t *buffer32 = (uint32_t *) buffer;
    for (int y = 0; y < image.height / 2; ++y) {
        for (int x = 0; x < image.width; ++x) {
            uint32_t h = buffer32[y * image.width + x];
            uint32_t t = buffer32[(image.height - 1 - y) * image.width + x];
            buffer32[y * image.width + x] = t;
            buffer32[(image.height - 1 - y) * image.width + x] = h;
        }
    }

    unsigned int texture;
    glGenTextures(1, &texture);

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, type, image.width, image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    free(buffer);

    return texture;
}

int main() {
    printf("Hello World!\n");

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow* window = glfwCreateWindow(500, 500, "shader", NULL, NULL);
    glfwMakeContextCurrent(window);

    glfwSwapInterval(0);

    GLenum error = glewInit();
    if (error != GLEW_OK) {
        printf("error: %s\n", glewGetErrorString(error));
        return 1;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(msg, NULL);

    unsigned int vertexShader = loadShader("src/vertex.glsl", GL_VERTEX_SHADER);
    if (!vertexShader) return 1;

    unsigned int lumaProgram = linkFragmentProgram(vertexShader, "src/luma_fragment.glsl");
    if (!lumaProgram) return 1;

    unsigned int sortingProgram = linkFragmentProgram(vertexShader, "src/sorting_fragment.glsl");
    if (!sortingProgram) return 1;

    unsigned int displayProgram = linkFragmentProgram(vertexShader, "src/display_fragment.glsl");
    if (!displayProgram) return 1;

    glDeleteShader(vertexShader);

    unsigned int computeShader = loadShader("src/compute.glsl", GL_COMPUTE_SHADER);
    if (!computeShader) return 1;
    unsigned int computeProgram = linkShaderProgram(0, 0, computeShader);
    if (!computeProgram) return 1;
    glDeleteShader(computeShader);

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glActiveTexture(GL_TEXTURE0);
    unsigned int texture = loadTexture("img/texture.png", GL_RGB);
    if (!texture) return 1;

    glActiveTexture(GL_TEXTURE1);
    unsigned int depth = loadTexture("img/depth.png", GL_RED);
    if (!depth) return 1;

    unsigned int fboTextures[2];
    glGenTextures(2, fboTextures);

    unsigned int FBOs[2];
    glGenFramebuffers(2, FBOs);

    for (int i = 0; i < 2; ++i) {
        glActiveTexture(GL_TEXTURE2 + i);
        glBindTexture(GL_TEXTURE_2D, fboTextures[i]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 500, 500, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glBindFramebuffer(GL_FRAMEBUFFER, FBOs[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTextures[i], 0);
    }

    glBindImageTexture(0, fboTextures[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8);

    unsigned int SSBO;
    glGenBuffers(1, &SSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 1, NULL, GL_DYNAMIC_READ);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        glBindFramebuffer(GL_FRAMEBUFFER, FBOs[0]);
        glUseProgram(lumaProgram);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        /* glUseProgram(computeProgram); */
        /* glDispatchCompute(500, 1, 1); */
        /* glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT); */

        /* int max = *(int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY); */
        /* printf("max: %d\n", max); */
        /* glUnmapBuffer(GL_SHADER_STORAGE_BUFFER); */
        int max = 500;

        glUseProgram(sortingProgram);
        int k;
        for (k = 0; k < max; ++k) {
            glBindFramebuffer(GL_FRAMEBUFFER, FBOs[1 - (k % 2)]);
            glUniform1i(0, 2 + (k % 2));
            glUniform1i(1, 1);
            glUniform1i(2, 1 + k);
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            glUniform1f(3, x / 500.0f);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        glUseProgram(displayProgram);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUniform1i(0, 2 + (k % 2));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
