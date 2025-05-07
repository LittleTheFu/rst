#ifndef MESH_H
#define MESH_H

#include <vector>
#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Eigen/Dense>
#include "shader.h"

struct Vertex {
    Eigen::Vector3f position;
    Eigen::Vector3f normal;
    Eigen::Vector2f texCoords;
};

class Mesh {
public:
    Mesh(const std::string& filePath);
    ~Mesh();
    void draw(Shader& shader);

private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    GLuint VAO, VBO, EBO;
    void setupMesh();
};

#endif