#pragma once

#include "GL/glew.h"
#include <QOpenGLWidget>

#include "glm/gtc/constants.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/glm.hpp"
#include "utils/shaderloader.h"

struct PostProcessingShader {
    GLuint shader;
    bool hdr;
    std::function<void(GLuint, float)> bindUniforms;
};

struct PostProcessingConfig {
    bool hdr_enabled = true;
    float hdr_exposure = 1.0;

    bool bloom_enabled = true;
    float bloom_threshold = 2.5;

    std::vector<PostProcessingShader> shaders{};

    GLuint tone_mapping = 0;
    GLuint default_shader = 0;
    GLuint bloom_filter = 0;
    GLuint bloom_composite = 0;
    GLuint gaussian_blur = 0;
};

class Realtime;

class PostProcessor
{
public:
    PostProcessor(/*QOpenGLWidget *parent*/);

    // Must be called once after GL is initialized and a context is current
    void init(GLuint defaultFBO, int w, int h, QOpenGLWidget *myParent);

    void addEffect(PostProcessingShader shader);
    void removeEffect(PostProcessingShader shader);

    void bindInitFBO();
    void applyEffects();

    void applyBloom(GLuint hdr_texture, GLuint write_to);

    void onResize(int w, int h);

    GLuint m_defaultFBO = 3;

    QOpenGLWidget *parent;

private:

    bool m_initialized;

    int m_devicePixelRatio;
    int m_fbo_width = 0;
    int m_fbo_height = 0;

    GLuint m_fullscreen_vbo;
    GLuint m_fullscreen_vao;

    GLuint fbo_a = 0;
    GLuint fbo_a_texture = 0;
    GLuint fbo_a_renderbuffer = 0;

    GLuint fbo_b = 0;
    GLuint fbo_b_texture = 0;
    GLuint fbo_b_renderbuffer = 0;

    GLuint default_texture_shader;

    GLuint bloom_bright_fbo, bloom_bright_tex;
    GLuint bloom_pingpong_fbo[2];
    GLuint bloom_pingpong_tex[2];

    PostProcessingConfig config;

    void makeFBO(GLuint *fbo, GLuint *texture, GLuint *renderbuffer);
    void initFBOs();

    void paintTexture(GLuint texture, GLuint shader, bool prebound = false);
    void paintTexture(GLuint texture, PostProcessingShader shaderStruct, bool prebound);

};
