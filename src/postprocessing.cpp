#include "postprocessing.h"
#include <cerrno>
#include <stdio.h>
#include <iostream>
#include "realtime.h"

PostProcessor::PostProcessor() {
}

void PostProcessor::init(GLuint defaultFBO, int w, int h, QOpenGLWidget *myParent) {

    parent = myParent;
    parent->makeCurrent();
    m_fbo_width = w;
    m_fbo_height = h;

    //Create shaders once valid GL context
    config.tone_mapping = ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert", ":/resources/shaders/tone_mapping.frag");
    config.default_shader = ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert",":/resources/shaders/texture.frag");
    config.bloom_filter = ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert", ":/resources/shaders/bloom_filter.frag");
    config.gaussian_blur = ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert",":/resources/shaders/gaussian_blur.frag");
    config.bloom_composite = ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert", ":/resources/shaders/bloom_composite.frag");

    // config.shaders.push_back(
    //     {
    //         ShaderLoader::createShaderProgram(":/resources/shaders/texture.vert", ":/resources/shaders/invert.frag"),
    //         false,
    //         [](GLuint program, float time) {}
    //     }
    //     );

    // config.shaders.push_back(
    //     {
    //         ShaderLoader::createShaderProgram(":/resources/shaders/shake.vert", ":/resources/shaders/shake.frag"),
    //         false,
    //         [](GLuint program, float time) {
    //             GLint loc = glGetUniformLocation(program, "time");
    //             glUniform1f(loc, time);
    //         }
    //     }
    // );

    //Add UV coordinates
    std::vector<GLfloat> fullscreen_quad_data =
        { //     POSITIONS    //    UVS
            -1.f,  1.f, 0.f, 0.f, 1.f,
            -1.f, -1.f, 0.f, 0.f, 0.f,
            1.f, -1.f, 0.f, 1.f, 0.f,
            1.f,  1.f, 0.f, 1.f, 1.f,
            -1.f,  1.f, 0.f, 0.f, 1.f,
            1.f, -1.f, 0.f, 1.f, 0.f
        };

    // Generate and bind a VBO and a VAO for a fullscreen quad
    glGenBuffers(1, &m_fullscreen_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_fullscreen_vbo);
    glBufferData(GL_ARRAY_BUFFER, fullscreen_quad_data.size()*sizeof(GLfloat), fullscreen_quad_data.data(), GL_STATIC_DRAW);
    glGenVertexArrays(1, &m_fullscreen_vao);
    glBindVertexArray(m_fullscreen_vao);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void *>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));

    // Unbind the fullscreen quad's VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    initFBOs();
    parent->doneCurrent();
}

void PostProcessor::bindInitFBO() {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_a);
    glViewport(0, 0, m_fbo_width, m_fbo_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//Called after the main scene has been drawn to fbo_a's texture
void PostProcessor::applyEffects() {

    glUseProgram(0);

    glEnable(GL_DEPTH_TEST);

    // true: current result is in fbo_a's texture
    // false: current result is in fbo_b's texture
    bool resultOnA = true;

    auto applyShader = [&](PostProcessingShader &shader) {
        GLuint dstFBO = resultOnA ? fbo_b : fbo_a;
        GLuint srcTexture = resultOnA ? fbo_a_texture : fbo_b_texture;

        glBindFramebuffer(GL_FRAMEBUFFER, dstFBO);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        paintTexture(srcTexture, shader, false);

        resultOnA = !resultOnA;
    };

    std::vector<PostProcessingShader> non_hdr_shaders{};

    glDisable(GL_DEPTH_TEST);

    //Then loop over each shader, alternately drawing them to A/B's texture
    for (int i = 0; i < config.shaders.size(); i++) {
        PostProcessingShader s = config.shaders[i];
        if (s.hdr) { //check if shader is hdr-friendly
            applyShader(s);
        } else { //if not, save it for after tone mapping
            non_hdr_shaders.push_back(s);
        }
    }

    //Apply bloom if enabled
    if (config.bloom_enabled) {
        applyBloom(resultOnA ? fbo_a_texture : fbo_b_texture, resultOnA ? fbo_b : fbo_a);
        resultOnA = !resultOnA;
    }

    //Tone mapping shader maps from unbound hdr colors to 0.0-1.0 range ldr colors
    if (config.hdr_enabled) {
        glUseProgram(config.tone_mapping);
        GLint loc = glGetUniformLocation(config.tone_mapping, "exposure");
        glUniform1f(loc, config.hdr_exposure);

        GLuint dstFBO = resultOnA ? fbo_b : fbo_a;
        GLuint srcTexture = resultOnA ? fbo_a_texture : fbo_b_texture;

        glBindFramebuffer(GL_FRAMEBUFFER, dstFBO);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        paintTexture(srcTexture, config.tone_mapping);

        resultOnA = !resultOnA;
    }

    //Loop over each shader that should be applied to an ldr scene
    for (int i = 0; i < non_hdr_shaders.size(); i++) {
        applyShader(non_hdr_shaders[i]);
    }

    //Bind the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, parent->defaultFramebufferObject());
    glViewport(0, 0, m_fbo_width, m_fbo_height);

    //Clear the color and depth buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //Call paintTexture to draw the final color attachment texture
    GLuint finalTexture = resultOnA ? fbo_a_texture : fbo_b_texture;
    paintTexture(finalTexture, config.default_shader);

    glEnable(GL_DEPTH_TEST);
}

void PostProcessor::applyBloom(GLuint hdr_texture, GLuint write_to) {

    glBindFramebuffer(GL_FRAMEBUFFER, bloom_bright_fbo);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(config.bloom_filter);
    GLint t_loc = glGetUniformLocation(config.bloom_filter, "threshold");
    glUniform1f(t_loc, config.bloom_threshold);
    paintTexture(hdr_texture, config.bloom_filter);

    bool horizontal = true, first_iteration = true;
    int amount = 10;
    glUseProgram(config.gaussian_blur);
    GLint h_loc = glGetUniformLocation(config.gaussian_blur, "horizontal");
    for (unsigned int i = 0; i < amount; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, bloom_pingpong_fbo[horizontal]);

        glUseProgram(config.gaussian_blur);
        glUniform1i(h_loc, horizontal);
        glActiveTexture(GL_TEXTURE0);

        //if first iteration, use bloom_bright_fbo's texture which was initially written to
        paintTexture(first_iteration ? bloom_bright_tex : bloom_pingpong_tex[!horizontal], config.gaussian_blur);
        horizontal = !horizontal;
        first_iteration = false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, write_to);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(config.bloom_composite);

    //bind the current hdr texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdr_texture);
    GLint loc_scene = glGetUniformLocation(config.bloom_composite, "curr_texture");
    glUniform1i(loc_scene, 0);

    //bind the bloom texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, bloom_pingpong_tex[!horizontal]);
    GLint loc_bloom = glGetUniformLocation(config.bloom_composite, "u_texture");
    glUniform1i(loc_bloom, 1);

    paintTexture(bloom_pingpong_tex[!horizontal], config.bloom_composite, true);
}

void PostProcessor::paintTexture(GLuint texture, PostProcessingShader shaderStruct, bool prebound) {
    GLuint shader = shaderStruct.shader;
    glUseProgram(shader);

    if (!prebound) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shader, "u_texture"), 0);
    }

    Realtime* r = static_cast<Realtime*>(parent);
    // shaderStruct.bindUniforms(shader, r->timer.elapsed() / 1000.0);

    glBindVertexArray(m_fullscreen_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void PostProcessor::paintTexture(GLuint texture, GLuint shader, bool prebound) {
    glUseProgram(shader);

    if (!prebound) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shader, "u_texture"), 0);
    }

    glBindVertexArray(m_fullscreen_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

void PostProcessor::makeFBO(GLuint *fbo, GLuint *texture, GLuint *renderbuffer) {
    std::cout << "making" << std::endl;
    //Generate and bind an empty texture, set its min/mag filter interpolation, then unbind
    glGenTextures(1, texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexImage2D(GL_TEXTURE_2D, 0, config.hdr_enabled ? GL_RGBA16F : GL_RGBA, m_fbo_width, m_fbo_height, 0,
                 GL_RGBA, config.hdr_enabled ? GL_FLOAT : GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    //Generate and bind a renderbuffer of the right size, set its format, then unbind
    if (renderbuffer != nullptr) {
        glGenRenderbuffers(1, renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, *renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_fbo_width, m_fbo_height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

    //Generate and bind an FBO
    glGenFramebuffers(1, fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo);

    //Add our texture as a color attachment, and our renderbuffer as a depth+stencil attachment, to our FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0);
    if (renderbuffer != nullptr) glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, *renderbuffer);

    GLenum drawBuf = GL_COLOR_ATTACHMENT0;
    glDrawBuffers(1, &drawBuf);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "FBO incomplete! status = 0x"
                  << std::hex << status << std::dec << std::endl;
    }

    //Unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, parent->defaultFramebufferObject());
}

void PostProcessor::onResize(int w, int h) {
    glDeleteTextures(1, &fbo_a_texture);
    glDeleteRenderbuffers(1, &fbo_a_renderbuffer);
    glDeleteFramebuffers(1, &fbo_a);

    glDeleteTextures(1, &fbo_b_texture);
    glDeleteRenderbuffers(1, &fbo_b_renderbuffer);
    glDeleteFramebuffers(1, &fbo_b);

    glDeleteTextures(1, &bloom_bright_tex);
    glDeleteFramebuffers(1, &bloom_bright_fbo);

    for (int i = 0; i < 2; i++) {
        glDeleteTextures(1, &bloom_pingpong_tex[i]);
        glDeleteFramebuffers(1, &bloom_pingpong_fbo[i]);
    }

    m_fbo_width = w;
    m_fbo_height = h;

    initFBOs();
}

void PostProcessor::initFBOs() {
    std::cout << "in init" << std::endl;
    makeFBO(&fbo_a, &fbo_a_texture, &fbo_a_renderbuffer);
    makeFBO(&fbo_b, &fbo_b_texture, &fbo_b_renderbuffer);
    makeFBO(&bloom_bright_fbo, &bloom_bright_tex, nullptr);
    for (int i = 0; i < 2; i++) makeFBO(&bloom_pingpong_fbo[i], &bloom_pingpong_tex[i], nullptr);
}

void PostProcessor::addEffect(PostProcessingShader shader) {
    config.shaders.push_back(shader);
}

void PostProcessor::removeEffect(PostProcessingShader shader) {
    auto &v = config.shaders;
    v.erase(std::remove_if(v.begin(), v.end(),
                           [&](const PostProcessingShader &p){ return p.shader == shader.shader; }),
            v.end());
}
