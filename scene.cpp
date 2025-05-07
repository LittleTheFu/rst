#include "scene.h"
#include <iostream>
#include <glad/glad.h>

void Scene::init()
{
    // 1. 初始化场景数据
    sceneData_.screenWidth = 800;
    sceneData_.screenHeight = 600;

    // 创建一些示例对象
    std::unique_ptr<Renderable> triangle1 = createTriangleMesh();
    Transform transform;
    transform1.setPosition(Eigen::Vector3f(0.0f, 0.0f, 0.0f));
    ObjectData object1;
    object1.renderable = std::move(triangle1);
    object1.transform = transform1;
    sceneData_.objects.push_back(std::move(object1));

    std::unique_ptr<Renderable> triangle2 = createTriangleMesh();
    Transform transform2;
    transform2.setPosition(Eigen::Vector3f(1.0f, 0.5f, 0.0f));
    ObjectData object2;
    object2.renderable = std::move(triangle2);
    object2.transform = transform2;
    sceneData_.objects.push_back(std::move(object2));

    // 2. 初始化 G-Buffer Pass
    gBufferPass_ = std::make_unique<GBufferPass>();
    gBufferPass_->Initialize(sceneData_.screenWidth, sceneData_.screenHeight);

    // 3. 初始化相机
    camera_.Position = Eigen::Vector3f(0.0f, 0.0f, 5.0f);
    camera_.updateCameraVectors();

    // 4. 设置一些简单的三角形顶点数据
    vertices_[0].position = Eigen::Vector3f(-0.5f, -0.5f, 0.0f);
    vertices_[1].position = Eigen::Vector3f(0.5f, -0.5f, 0.0f);
    vertices_[2].position = Eigen::Vector3f(0.0f, 0.5f, 0.0f);

    // ... (加载和编译着色器的代码)
    // vertex_shader_ = ...;
    // fragment_shader_ = ...;
    // shaderProgram_ = createShaderProgram(vertex_shader_, fragment_shader_);

    // ... (创建 VAO 和 VBO 的代码)
    // glGenVertexArrays(1, &VAO_);
    // glGenBuffers(1, &VBO_);
    // ...
}

void Scene::run()
{
    // 渲染循环
    while (!/* 窗口关闭条件 */)
    {
        // ... (处理输入、更新游戏逻辑等)

        // 1. 渲染 G-Buffer
        gBufferPass_->Render(sceneData_, camera_);

        // ... (执行其他渲染Pass，例如光照计算、后处理等)

        // ... (交换缓冲区、处理事件等)
    }
}

// 示例：创建一个简单的三角形网格
std::unique_ptr<Renderable> Scene::createTriangleMesh()
{
    // 创建一个简单的三角形网格
    class TriangleMesh : public Renderable
    {
    public:
        TriangleMesh()
        {
            // 设置三角形的顶点数据
            float vertices[] = {
                -0.5f, -0.5f, 0.0f,
                0.5f, -0.5f, 0.0f,
                0.0f, 0.5f, 0.0f};

            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
            glEnableVertexAttribArray(0);

            glBindVertexArray(0);
        }
        ~TriangleMesh()
        {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
        }

        void render(Shader &shader) override
        {
            // 设置一些默认材质
            shader.setVec3("material.ambient", 1.0f, 0.5f, 0.31f);
            shader.setVec3("material.diffuse", 1.0f, 0.5f, 0.31f);
            shader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
            shader.setFloat("material.shininess", 32.0f);

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
        }

        Eigen::Matrix4f getModelMatrix() const override {
            return modelMatrix;
        }
        void setModelMatrix(const Eigen::Matrix4f &model) override {
            modelMatrix = model;
        }

    private:
        GLuint VAO, VBO;
        Eigen::Matrix4f modelMatrix = Eigen::Matrix4f::Identity();
    };
    return std::make_unique<TriangleMesh>();
}
