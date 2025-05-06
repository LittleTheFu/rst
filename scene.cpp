#include "scene.h"
#include <glad/glad.h>

void Scene::init()
{
    Vertex v0{Eigen::Vector3f{-0.5, -0.5, 0}};
    Vertex v1{Eigen::Vector3f{0.5, -0.5, 0}};
    Vertex v2{Eigen::Vector3f{0, 0.5, 0}};

    vertices_[0] = v0;
    vertices_[1] = v1;
    vertices_[2] = v2;

    vertex_shader_ = "#version 330 core\n"
                     "layout (location = 0) in vec3 aPos;\n"
                     "void main()\n"
                     "{\n"
                     "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                     "}\0";

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    const char *vertex_shader_source = vertex_shader_.c_str();

    glShaderSource(vertexShader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertexShader);

    fragment_shader_ = "#version 330 core\n"
                       "out vec4 FragColor;\n"
                       "void main()\n"
                       "{\n"
                       "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                       "}\n\0";

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    const char *fragment_shader_source = fragment_shader_.c_str();

    glShaderSource(fragmentShader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragmentShader);

    shaderProgram_ = glCreateProgram();

    glAttachShader(shaderProgram_, vertexShader);
    glAttachShader(shaderProgram_, fragmentShader);
    glLinkProgram(shaderProgram_);

    glUseProgram(shaderProgram_);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);  

    glGenVertexArrays(1, &VAO_);
    glBindVertexArray(VAO_);

    glGenBuffers(1, &VBO_);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_), vertices_.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0); // 解绑 VAO，防止误操作


}

void Scene::run()
{
    glBindVertexArray(VAO_);
    glUseProgram(shaderProgram_);

    // glBindVertexArray(VBO_);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}