
// RmlUiOpenGLRenderer.cpp
#include "RmlUiOpenGLRenderer.h"
#include <iostream>
#include <vector>
#include <shaderManager.h>

// 假设你的 stb_image_wrapper 提供以下函数来加载图像数据
// extern unsigned char* stbi_load(char const *filename, int *x, int *y, int *channels_in_file, int desired_channels);
// extern void stbi_image_free(void *retval_from_stbi_load);
// #include "stb_image_wrapper.h" // 假设你有这个文件，用于加载图像

// RmlUi 渲染器所需的着色器代码（非常简单）
// 注意：这些着色器是硬编码的，你可以将它们放在文件中并通过 ShaderManager 加载
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
    // 注意：这里的路径是示例，你需要确保这些着色器文件存在或使用硬编码字符串
    rmlUiShader_ = ShaderManager::getInstance().loadShader(
        "shaders/rmlui_vertex.glsl", // 假设你的着色器文件路径
        "shaders/rmlui_fragment.glsl"
    );

    if (!rmlUiShader_) {
        std::cerr << "Error: Failed to load RmlUi shaders from files. Attempting to compile from hardcoded strings." << std::endl;
        // 如果文件加载失败，尝试从硬编码字符串编译
        try {
            // 创建一个临时的 Shader 对象来编译硬编码的着色器
            // 注意：这种方式不会被 ShaderManager 缓存，每次都会重新编译
            // 更好的做法是让 ShaderManager 支持从字符串加载
            GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertexShader, 1, &RMLUI_VERTEX_SHADER_SOURCE, NULL);
            glCompileShader(vertexShader);
            // 检查编译错误 (你需要 Shader::checkCompileErrors 的逻辑)
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
            // 检查编译错误
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
            // 检查链接错误 (你需要 Shader::checkProgramErrors 的逻辑)
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

            // 创建一个新的 Shader 对象来封装这个程序ID
            // 注意：这可能需要修改 Shader 类的构造函数或添加一个工厂方法
            // 假设 Shader 有一个接受 GLuint ID 的构造函数或者可以设置 ID
            rmlUiShader_ = std::make_shared<Shader>(); // 使用默认构造函数
            rmlUiShader_->ID = programId; // 直接设置 ID，这需要 Shader::ID 为 public
            // RmlUi Shader 不需要 m_id，因为它是内部使用的
            // rmlUiShader_->m_id = "RmlUiInternalShader"; // 只是一个标识
            
            // 检查 Shader::isValid() 是否能正确处理这种情况
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
    // RmlUi 通常直接渲染到屏幕，所以这里不需要改变 OpenGL 视口
    // 视口设置通常在主渲染循环中完成
    // 但是，我们需要更新投影矩阵以匹配新的视口尺寸
    projectionMatrix_ = Eigen::Matrix4f::Identity();
    projectionMatrix_(0, 0) = 2.0f / target_width;
    projectionMatrix_(1, 1) = -2.0f / target_height; // Y 轴翻转，RmlUi 坐标系原点在左上角
    projectionMatrix_(0, 3) = -1.0f;
    projectionMatrix_(1, 3) = 1.0f;
}

void RmlUiOpenGLRenderer::RenderGeometry(Rml::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rml::Core::TextureHandle texture_handle, const Rml::Core::Vector2f& translation) {
    if (!rmlUiShader_ || !rmlUiShader_->isValid()) {
        std::cerr << "Error: RmlUi shader is not valid during RenderGeometry." << std::endl;
        return;
    }

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
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 标准 Alpha 混合
    glDisable(GL_CULL_FACE); // RmlUi 几何体通常不需要背面剔除
    glDisable(GL_DEPTH_TEST); // RmlUi 渲染在 3D 场景之上，不需要深度测试

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
            glActiveTexture(GL_TEXTURE0);
            rmlTexture.texture->bind();
            rmlUiShader_->setInt("uTexture", 0);
        } else {
            // 纹理无效，禁用纹理
            rmlUiShader_->setBool("hasTexture", false);
        }
    }

    // 创建或更新 VBO, IBO, VAO
    // RmlUi 每次 RenderGeometry 都会提供新的顶点/索引数据，
    // 所以我们每次都需要更新缓冲区或创建新的。
    // 为了效率，可以考虑重用缓冲区并使用 glBufferSubData。
    // 这里为了简化，每次都创建新的缓冲区。

    // 顶点数据布局: 位置 (vec2), 颜色 (vec4), 纹理坐标 (vec2)
    // Rml::Core::Vertex 结构体通常是 { Rml::Core::Vector2f position, Rml::Core::ColourB color, Rml::Core::Vector2f tex_coord }
    // 需要将 Rml::Core::ColourB 转换为 float 颜色
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
    vao.unbind(); // 或者 glDisableVertexArrayAttribs(vao.id(), 0, 1, 2) 等

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
    // RmlUi 的 Y 轴原点在左上角，OpenGL 的 Y 轴原点在左下角
    // 需要将 Y 坐标翻转
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glScissor(x, viewport[3] - (y + height), width, height);
}

bool RmlUiOpenGLRenderer::LoadTexture(Rml::Core::TextureHandle& texture_handle, Rml::Core::Vector2i& texture_dimensions, const Rml::Core::String& source) {
    // 尝试从你的 TextureManager 加载纹理
    std::string path = source.CString(); // Rml::Core::String 到 std::string

    std::shared_ptr<Texture2D> texture = TextureManager::getInstance().loadTexture2D(path, false, false, false); // RmlUi 纹理通常不需要翻转Y轴，也不需要sRGB

    if (texture && texture->id() != 0) {
        RmlUiTextureHandle rmlTexture;
        rmlTexture.texture = texture;
        rmlTexture.dimensions = Eigen::Vector2i(texture->getWidth(), texture->getHeight());

        texture_handle = nextTextureHandle_++;
        textureMap_[texture_handle] = rmlTexture;

        texture_dimensions.x = rmlTexture.dimensions.x();
        texture_dimensions.y = rmlTexture.dimensions.y();
        return true;
    }

    std::cerr << "Error: Failed to load RmlUi texture from path: " << path << std::endl;
    texture_handle = 0;
    return false;
}

bool RmlUiOpenGLRenderer::GenerateTexture(Rml::Core::TextureHandle& texture_handle, const Rml::Core::byte* source, const Rml::Core::Vector2i& source_dimensions) {
    // RmlUi 内部生成纹理 (例如字体图集)
    if (!source || source_dimensions.x <= 0 || source_dimensions.y <= 0) {
        texture_handle = 0;
        return false;
    }

    // 创建新的 Texture2D 对象
    // assetId 可以是任意唯一字符串，例如 "RmlUiGeneratedTexture_" + nextTextureHandle_
    std::string assetId = "RmlUiGeneratedTexture_" + std::to_string(nextTextureHandle_);
    std::shared_ptr<Texture2D> texture = std::make_shared<Texture2D>(
        assetId,
        source_dimensions.x,
        source_dimensions.y,
        GL_RGBA8, // RmlUi 通常使用 RGBA 格式
        1 // 单个 mip 级别
    );

    if (!texture || texture->id() == 0) {
        std::cerr << "Error: Failed to create OpenGL texture for RmlUi generated data." << std::endl;
        texture_handle = 0;
        return false;
    }

    // 上传像素数据
    texture->uploadData(source, GL_RGBA, GL_UNSIGNED_BYTE);

    RmlUiTextureHandle rmlTexture;
    rmlTexture.texture = texture;
    rmlTexture.dimensions = Eigen::Vector2i(source_dimensions.x, source_dimensions.y);

    texture_handle = nextTextureHandle_++;
    textureMap_[texture_handle] = rmlTexture;

    return true;
}

void RmlUiOpenGLRenderer::ReleaseTexture(Rml::Core::TextureHandle texture_handle) {
    auto it = textureMap_.find(texture_handle);
    if (it != textureMap_.end()) {
        // shared_ptr 会在引用计数归零时自动释放 OpenGL 资源
        it->second.texture.reset(); // 显式释放 shared_ptr
        textureMap_.erase(it);
    }
}

void RmlUiOpenGLRenderer::ReleaseAllTextures() {
    for (auto const& [handle, rmlTexture] : textureMap_) {
        rmlTexture.texture.reset(); // 释放所有 shared_ptr
    }
    textureMap_.clear();
    nextTextureHandle_ = 1; // 重置句柄计数器
}
