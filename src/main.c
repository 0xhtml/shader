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

    GLFWwindow* window = glfwCreateWindow(512, 512, "shader", NULL, NULL);
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, NULL);

        glBindFramebuffer(GL_FRAMEBUFFER, FBOs[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTextures[i], 0);
    }

    glBindImageTexture(0, fboTextures[0], 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);

        double x, y;
        glfwGetCursorPos(window, &x, &y);
        float threshold = x / 512.0f;

        glBindFramebuffer(GL_FRAMEBUFFER, FBOs[0]);
        glUseProgram(lumaProgram);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glUseProgram(computeProgram);
        glUniform1f(0, threshold);
        glDispatchCompute(512, 1, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glUseProgram(sortingProgram);
        int alternating = 0;
        for (int k = 2; k <= 512; k *= 2) {
            for (int j = k / 2; j > 0; j /= 2) {
                if (k == 512 && j == 1) glBindFramebuffer(GL_FRAMEBUFFER, 0);
                else glBindFramebuffer(GL_FRAMEBUFFER, FBOs[1 - alternating]);

                glUniform1i(0, 2 + alternating);
                glUniform1f(1, threshold);
                glUniform1i(2, k);
                glUniform1i(3, j);

                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

                alternating ^= 1;
            }
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
