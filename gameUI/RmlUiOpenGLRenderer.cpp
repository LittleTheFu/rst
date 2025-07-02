#include "RmlUiOpenGLRenderer.h"
#include <iostream>
#include <vector>
#include "shaderManager.h"
#include "openGLStateSaver.h"

// #include "stb_image_wrapper.h" // 假设你有这个文件，用于加载图像

// RmlUi 渲染器所需的着色器代码（非常简单）
const char* RMLUI_VERTEX_SHADER_SOURCE = R"(
#version 460 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec4 vColor;
out vec2 vTexCoord;

uniform mat4 projection;
uniform vec2 translation; // RmlUi 提供的平移

void main()
{
    gl_Position = projection * vec4(aPos + translation, 0.0, 1.0);
    vColor = aColor;
    vTexCoord = aTexCoord;
}
)";

const char* RMLUI_FRAGMENT_SHADER_SOURCE = R"(
#version 460 core
out vec4 FragColor;

in vec4 vColor;
in vec2 vTexCoord;

uniform sampler2D uTexture;
uniform bool hasTexture;

void main()
{
    if (hasTexture) {
        FragColor = vColor * texture(uTexture, vTexCoord);
    } else {
        FragColor = vColor;
    }
}
)";

RmlUiOpenGLRenderer::RmlUiOpenGLRenderer() : rmlUiShader_(nullptr) {
    // 构造函数中不执行 OpenGL 调用，因为上下文可能尚未准备好
}

RmlUiOpenGLRenderer::~RmlUiOpenGLRenderer() {
    // 析构函数中也不执行 OpenGL 调用，因为上下文可能已被销毁
    // 纹理和着色器由 shared_ptr 自动管理
}

bool RmlUiOpenGLRenderer::Initialize() {
    // 在这里加载并编译 RmlUi 的着色器
    rmlUiShader_ = ShaderManager::getInstance().loadShader(
        "shader/rmlui.vert", // 假设你的着色器文件路径
        "shader/rmlui.frag"
    );

    if (!rmlUiShader_) {
        std::cerr << "Error: Failed to load RmlUi shaders from files. Attempting to compile from hardcoded strings." << std::endl;
        try {
            GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertexShader, 1, &RMLUI_VERTEX_SHADER_SOURCE, NULL);
            glCompileShader(vertexShader);
            GLint success;
            glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
            if (!success) {
                char infoLog[512];
                glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
                std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
                glDeleteShader(vertexShader);
                return false;
            }

            GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragmentShader, 1, &RMLUI_FRAGMENT_SHADER_SOURCE, NULL);
            glCompileShader(fragmentShader);
            glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
            if (!success) {
                char infoLog[512];
                glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
                std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
                glDeleteShader(vertexShader);
                glDeleteShader(fragmentShader);
                return false;
            }

            GLuint programId = glCreateProgram();
            glAttachShader(programId, vertexShader);
            glAttachShader(programId, fragmentShader);
            glLinkProgram(programId);
            glGetProgramiv(programId, GL_LINK_STATUS, &success);
            if (!success) {
                char infoLog[512];
                glGetProgramInfoLog(programId, 512, NULL, infoLog);
                std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
                glDeleteProgram(programId);
                glDeleteShader(vertexShader);
                glDeleteShader(fragmentShader);
                return false;
            }

            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);

            rmlUiShader_ = std::make_shared<Shader>(); 
            rmlUiShader_->ID = programId; 
            
            if (!rmlUiShader_->isValid()) {
                 std::cerr << "Error: RmlUi shader compiled from string is invalid." << std::endl;
                 rmlUiShader_ = nullptr;
                 return false;
            }

        } catch (const std::exception& e) {
            std::cerr << "Exception during RmlUi shader compilation from string: " << e.what() << std::endl;
            rmlUiShader_ = nullptr;
            return false;
        }
    }

    if (!rmlUiShader_ || !rmlUiShader_->isValid()) {
        std::cerr << "Error: RmlUi shader is not valid after initialization." << std::endl;
        return false;
    }

    return true;
}

void RmlUiOpenGLRenderer::SetViewport(int target_width, int target_height) {
    projectionMatrix_ = Eigen::Matrix4f::Identity();
    projectionMatrix_(0, 0) = 2.0f / target_width;
    projectionMatrix_(1, 1) = -2.0f / target_height; // Y 轴翻转，RmlUi 坐标系原点在左上角
    projectionMatrix_(0, 3) = -1.0f;
    projectionMatrix_(1, 3) = 1.0f;
}

// <--- 修正 Rml:: 到 Rml::
void RmlUiOpenGLRenderer::RenderGeometry(Rml::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::TextureHandle texture_handle, const Rml::Vector2f& translation) {
    if (!rmlUiShader_ || !rmlUiShader_->isValid()) {
        std::cerr << "Error: RmlUi shader is not valid during RenderGeometry." << std::endl;
        return;
    }

    // OpenGLStateSaver stateSaver;
    // 保存当前的 OpenGL 状态
    glGetIntegerv(GL_CURRENT_PROGRAM, &lastProgramId_);
    glGetBooleanv(GL_BLEND, &lastBlendEnabled_);
    glGetBooleanv(GL_SCISSOR_TEST, &lastScissorEnabled_);
    glGetIntegerv(GL_SCISSOR_BOX, lastScissorBox_);
    glGetBooleanv(GL_CULL_FACE, &lastCullFaceEnabled_);
    glGetBooleanv(GL_DEPTH_TEST, &lastDepthTestEnabled_);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &lastActiveTexture_);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &lastTextureBinding2D_);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &lastVertexArrayBinding_);
    glGetIntegerv(GL_VIEWPORT, lastViewport_);

    // 设置 OpenGL 状态以进行 RmlUi 渲染
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // <--- 修正：预乘 Alpha 混合模式
    glDisable(GL_CULL_FACE); 
    glDisable(GL_DEPTH_TEST); 

    // 绑定 RmlUi 着色器
    rmlUiShader_->use();
    rmlUiShader_->setMat4("projection", projectionMatrix_);
    rmlUiShader_->setVec2("translation", Eigen::Vector2f(translation.x, translation.y));

    // 绑定纹理
    bool hasTexture = (texture_handle != 0 && textureMap_.count(texture_handle));
    rmlUiShader_->setBool("hasTexture", hasTexture);
    if (hasTexture) {
        RmlUiTextureHandle rmlTexture = textureMap_[texture_handle];
        if (rmlTexture.texture && rmlTexture.texture->id() != 0) {
            // glActiveTexture(GL_TEXTURE0);
            rmlTexture.texture->activate(0);
            rmlUiShader_->setInt("uTexture", 0);
        } else {
            // 纹理无效，禁用纹理
            rmlUiShader_->setBool("hasTexture", false);
        }
    }

    // 顶点数据布局: 位置 (vec2), 颜色 (vec4), 纹理坐标 (vec2)
    // Rml::Vertex 结构体通常是 { Rml::Vector2f position, Rml::ColourB color, Rml::Vector2f tex_coord }
    // 需要将 Rml::ColourB 转换为 float 颜色
    std::vector<float> interleaved_data;
    interleaved_data.reserve(num_vertices * (2 + 4 + 2)); // 2 pos, 4 color, 2 texcoord

    for (int i = 0; i < num_vertices; ++i) {
        interleaved_data.push_back(vertices[i].position.x);
        interleaved_data.push_back(vertices[i].position.y);
        interleaved_data.push_back(vertices[i].colour.red / 255.0f);
        interleaved_data.push_back(vertices[i].colour.green / 255.0f);
        interleaved_data.push_back(vertices[i].colour.blue / 255.0f);
        interleaved_data.push_back(vertices[i].colour.alpha / 255.0f);
        interleaved_data.push_back(vertices[i].tex_coord.x);
        interleaved_data.push_back(vertices[i].tex_coord.y);
    }

    // 使用你的 VertexBuffer 和 IndexBuffer
    VertexBuffer vbo(interleaved_data.data(), interleaved_data.size() * sizeof(float), GL_STREAM_DRAW);
    IndexBuffer ibo(indices, num_indices, GL_STREAM_DRAW);

    VertexArray vao;
    vao.bind();

    // 设置顶点属性
    // 位置 (aPos): layout (location = 0)
    vao.setAttribute(0, vbo, 2, GL_FLOAT, GL_FALSE, 0, 8 * sizeof(float), 0);
    // 颜色 (aColor): layout (location = 1)
    vao.setAttribute(1, vbo, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 8 * sizeof(float), 0);
    // 纹理坐标 (aTexCoord): layout (location = 2)
    vao.setAttribute(2, vbo, 2, GL_FLOAT, GL_FALSE, (2 + 4) * sizeof(float), 8 * sizeof(float), 0);

    vao.enableAttribute(0);
    vao.enableAttribute(1);
    vao.enableAttribute(2);

    vao.setIndexBuffer(ibo);

    // 绘制
    glDrawElements(GL_TRIANGLES, num_indices, ibo.getType(), 0);

    // 解绑 VAO
    vao.unbind(); 

    // 恢复之前的 OpenGL 状态
    if (lastProgramId_ != 0) glUseProgram(lastProgramId_);
    if (lastBlendEnabled_) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (lastScissorEnabled_) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
    glScissor(lastScissorBox_[0], lastScissorBox_[1], lastScissorBox_[2], lastScissorBox_[3]);
    if (lastCullFaceEnabled_) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
    if (lastDepthTestEnabled_) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    glActiveTexture(lastActiveTexture_);
    glBindTexture(GL_TEXTURE_2D, lastTextureBinding2D_);
    glBindVertexArray(lastVertexArrayBinding_);
    glViewport(lastViewport_[0], lastViewport_[1], lastViewport_[2], lastViewport_[3]);
}

void RmlUiOpenGLRenderer::EnableScissorRegion(bool enable) {
    if (enable) {
        glEnable(GL_SCISSOR_TEST);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
}

void RmlUiOpenGLRenderer::SetScissorRegion(int x, int y, int width, int height) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glScissor(x, viewport[3] - (y + height), width, height);
}

// <--- 修正 Rml:: 到 Rml::
bool RmlUiOpenGLRenderer::LoadTexture(Rml::TextureHandle& texture_handle, Rml::Vector2i& texture_dimensions, const Rml::String& source) {
    std::string path = source; // 直接使用 Rml::String 类型的 source

    std::shared_ptr<Texture2D> texture = TextureManager::getInstance().loadTexture2D(path, false, false, false); 

    if (texture && texture->id() != 0) {
        RmlUiTextureHandle rmlTexture;
        rmlTexture.texture = texture;
        rmlTexture.dimensions = Rml::Vector2i(texture->width(), texture->height()); // <--- 修正 Rml:: 到 Rml::

        texture_handle = nextTextureHandle_++;
        textureMap_[texture_handle] = rmlTexture;

        texture_dimensions.x = rmlTexture.dimensions.x;
        texture_dimensions.y = rmlTexture.dimensions.y;
        return true;
    }

    std::cerr << "Error: Failed to load RmlUi texture from path: " << path << std::endl;
    texture_handle = 0;
    return false;
}

// <--- 修正 Rml:: 到 Rml::
bool RmlUiOpenGLRenderer::GenerateTexture(Rml::TextureHandle& texture_handle, const Rml::byte* source, const Rml::Vector2i& source_dimensions) {
    if (!source || source_dimensions.x <= 0 || source_dimensions.y <= 0) {
        texture_handle = 0;
        return false;
    }

    std::string assetId = "RmlUiGeneratedTexture_" + std::to_string(nextTextureHandle_);
    std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>(
        assetId,
        source_dimensions.x,
        source_dimensions.y,
        GL_RGBA8, 
        1 
    );

    if (!texture || texture->id() == 0) {
        std::cerr << "Error: Failed to create OpenGL texture for RmlUi generated data." << std::endl;
        texture_handle = 0;
        return false;
    }

    texture->uploadData(source, GL_RGBA, GL_UNSIGNED_BYTE);

    RmlUiTextureHandle rmlTexture;
    rmlTexture.texture = texture;
    rmlTexture.dimensions = Rml::Vector2i(source_dimensions.x, source_dimensions.y); // <--- 修正 Rml:: 到 Rml::

    texture_handle = nextTextureHandle_++;
    textureMap_[texture_handle] = rmlTexture;

    return true;
}

// <--- 修正 Rml:: 到 Rml::
void RmlUiOpenGLRenderer::ReleaseTexture(Rml::TextureHandle texture_handle) {
    auto it = textureMap_.find(texture_handle);
    if (it != textureMap_.end()) {
        it->second.texture.reset(); 
        textureMap_.erase(it);
    }
}

void RmlUiOpenGLRenderer::ReleaseAllTextures() {
    for (auto const& [handle, rmlTexture] : textureMap_) {
        rmlTexture.texture->release(); 
    }
    textureMap_.clear();
    nextTextureHandle_ = 1; 
}
