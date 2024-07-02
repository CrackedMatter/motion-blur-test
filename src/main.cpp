#include <algorithm>
#include <dlfcn.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

const char* vertexShaderSource = R"(
attribute vec4 aPosition;
attribute vec2 aTexCoord;
varying vec2 vTexCoord;
void main() {
    gl_Position = aPosition;
    vTexCoord = aTexCoord;
}
)";

const char* fragmentShaderSource = R"(
precision mediump float;
varying vec2 vTexCoord;
uniform sampler2D uCurrentFrame;
uniform sampler2D uPreviousFrame;
uniform float uBlendFactor;
void main() {
    vec4 currentFrameColor = texture2D(uCurrentFrame, vTexCoord);
    vec4 previousFrameColor = texture2D(uPreviousFrame, vTexCoord);
    gl_FragColor = mix(currentFrameColor, previousFrameColor, uBlendFactor);
}
)";

GLuint framebuffer           = 0;
GLuint renderTexture         = 0;
GLuint previousFrameTexture  = 0;
GLuint shaderProgram         = 0;
GLuint vertexBuffer          = 0;
GLuint indexBuffer           = 0;
GLint  positionLocation      = -1;
GLint  texCoordLocation      = -1;
GLint  currentFrameLocation  = -1;
GLint  previousFrameLocation = -1;
GLint  blendFactorLocation   = -1;

void initializeMotionBlurResources(GLint width, GLint height) {
    // create and compile shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    // link shaders into program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // get attribute and uniform locations
    positionLocation      = glGetAttribLocation(shaderProgram, "aPosition");
    texCoordLocation      = glGetAttribLocation(shaderProgram, "aTexCoord");
    currentFrameLocation  = glGetUniformLocation(shaderProgram, "uCurrentFrame");
    previousFrameLocation = glGetUniformLocation(shaderProgram, "uPreviousFrame");
    blendFactorLocation   = glGetUniformLocation(shaderProgram, "uBlendFactor");

    // create framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // create textures
    glGenTextures(1, &renderTexture);
    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);

    glGenTextures(1, &previousFrameTexture);
    glBindTexture(GL_TEXTURE_2D, previousFrameTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // unbind framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // create vertex buffer
    GLfloat vertices[] = {
            // positions  // texture coords
            -1.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f
    };

    GLushort indices[] = {
            0, 1, 2,
            0, 2, 3
    };

    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &indexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

extern "C" __attribute__ ((visibility ("default"))) void mod_preinit() {
    auto h = dlopen("libmcpelauncher_mod.so", 0);

    auto mcpelauncher_preinithook = (void (*)(const char*, void*, void**)) dlsym(h, "mcpelauncher_preinithook");

    mcpelauncher_preinithook("eglSwapBuffers", (void*) +[](EGLDisplay dpy, EGLSurface surface) {
        EGLint width, height;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        glViewport(0, 0, width, height);

        if (!framebuffer)
            initializeMotionBlurResources(width, height);

        // copy current framebuffer to render texture
        glBindTexture(GL_TEXTURE_2D, renderTexture);
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, width, height, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, renderTexture);
        glUniform1i(currentFrameLocation, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, previousFrameTexture);
        glUniform1i(previousFrameLocation, 1);

        glUniform1f(blendFactorLocation, 0.7f);

        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

        glEnableVertexAttribArray(positionLocation);
        glVertexAttribPointer(positionLocation, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);

        glEnableVertexAttribArray(texCoordLocation);
        glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*) (2 * sizeof(GLfloat)));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

        std::swap(renderTexture, previousFrameTexture);

        return eglSwapBuffers(dpy, surface);
    }, nullptr);
}

extern "C" __attribute__ ((visibility ("default"))) void mod_init() {}
