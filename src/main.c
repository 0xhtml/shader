#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#include "shader.h"
#include "texture.h"

void GLAPIENTRY msg(GLenum src, GLenum type, GLenum id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    printf("%s\n", message);
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

    int version = gladLoadGL(glfwGetProcAddress);
    printf("OpenGL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(msg, NULL);

    GLuint lumaProgram = loadShaderProgram("glsl/vertex.glsl", "glsl/luma_fragment.glsl", NULL);
    if (lumaProgram == 0) {
        freeShaderCache();
        return 1;
    }

    GLuint sortingProgram = loadShaderProgram("glsl/vertex.glsl", "glsl/sorting_fragment.glsl", NULL);
    if (sortingProgram == 0) {
        freeShaderCache();
        return 1;
    }

    GLuint computeProgram = loadShaderProgram(NULL, NULL, "glsl/compute.glsl");
    freeShaderCache();
    if (computeProgram == 0) return 1;

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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 500, 500, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

        glBindFramebuffer(GL_FRAMEBUFFER, FBOs[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTextures[i], 0);
    }

    unsigned int SSBO;
    glGenBuffers(1, &SSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 4, NULL, GL_DYNAMIC_READ);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        double x, y;
        glfwGetCursorPos(window, &x, &y);
        float threshold = x / 500.0f;

        glBindFramebuffer(GL_FRAMEBUFFER, FBOs[0]);
        glUseProgram(lumaProgram);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glUseProgram(computeProgram);
        glUniform1f(0, threshold);
        glDispatchCompute(500, 1, 1);
        glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

        int *map = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
        int max = *map;
        *map = 0;
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        glUseProgram(sortingProgram);
        for (int k = 0; k < max; ++k) {
            if (k == max - 1) glBindFramebuffer(GL_FRAMEBUFFER, 0);
            else glBindFramebuffer(GL_FRAMEBUFFER, FBOs[1 - (k % 2)]);

            glUniform1i(0, 2 + (k % 2));
            glUniform1i(1, 1 + k);
            glUniform1f(2, threshold);

            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
