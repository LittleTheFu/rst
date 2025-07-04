#include "RenderSystem.h"
#include <glad/glad.h> // 包含 OpenGL 头文件
#include <Eigen/Geometry> // For Eigen::AngleAxisd if needed for rotations

RenderSystem::RenderSystem(Shader& shader, Eigen::Matrix4f& viewMatrix, Eigen::Matrix4f& projectionMatrix)
    : m_shader(shader),
      m_viewMatrix(viewMatrix),
      m_projectionMatrix(projectionMatrix),
      m_modelManager(ModelManager::getInstance()),
      m_meshManager(MeshManager::getInstance()),
      m_materialManager(MaterialManager::getInstance())
{
    std::cout << "RenderSystem: Initialized." << std::endl;
}

void RenderSystem::setGlobalUniforms(const Eigen::Vector3f& viewPos, const Eigen::Vector3f& lightDir, const Eigen::Vector3f& lightColor) {
    m_shader.use();
    m_shader.setVec3("viewPos", viewPos);
    m_shader.setVec3("light.direction", lightDir);
    m_shader.setVec3("light.color", lightColor);
    // 传递视图和投影矩阵给着色器（这些通常在每帧更新，但可以作为全局 uniform）
    m_shader.setMat4("view", m_viewMatrix);
    m_shader.setMat4("projection", m_projectionMatrix);
}

// 核心渲染循环
void RenderSystem::render(const std::vector<std::pair<unsigned int, std::pair<TransformComponent*, ModelRenderComponent*>>>& renderEntities) {
    m_shader.use(); // 激活着色器一次

    for (const auto& entityPair : renderEntities) {
        unsigned int entityId = entityPair.first;
        TransformComponent* transformComp = entityPair.second.first;
        ModelRenderComponent* modelRenderComp = entityPair.second.second;

        if (!transformComp || !modelRenderComp) {
            std::cerr << "RenderSystem Warning: Entity " << entityId << " has missing TransformComponent or ModelRenderComponent." << std::endl;
            continue;
        }

        // 1. 获取 ModelResource
        std::shared_ptr<ModelResource> modelResource = m_modelManager.getModel(modelRenderComp->modelName);
        if (!modelResource) {
            std::cerr << "RenderSystem Warning: Could not find ModelResource for name '" << modelRenderComp->modelName << "' on entity " << entityId << std::endl;
            continue;
        }

        // 2. 计算实体的世界模型矩阵
        // 假设 TransformComponent 存储了位置、旋转和缩放
        Eigen::Matrix4f modelMatrix = Eigen::Matrix4f::Identity();
        // 缩放
        modelMatrix = Eigen::Transform<float, 3, Eigen::Affine>::Identity().scale(transformComp->scale).matrix() * modelMatrix;
        // 旋转
        // 将欧拉角转换为四元数，然后转换为旋转矩阵
        Eigen::Quaternionf rotation_quat = 
            Eigen::AngleAxisf(transformComp->rotation.z(), Eigen::Vector3f::UnitZ()) *
            Eigen::AngleAxisf(transformComp->rotation.y(), Eigen::Vector3f::UnitY()) *
            Eigen::AngleAxisf(transformComp->rotation.x(), Eigen::Vector3f::UnitX());
        modelMatrix = Eigen::Transform<float, 3, Eigen::Affine>::Identity().rotate(rotation_quat).matrix() * modelMatrix;
        // 平移
        modelMatrix = Eigen::Transform<float, 3, Eigen::Affine>::Identity().translate(transformComp->position).matrix() * modelMatrix;

        // 3. 遍历 ModelResource 中的每个子网格并绘制
        for (const auto& meshInfo : modelResource->meshes) {
            // 获取 MeshGLData
            std::shared_ptr<MeshGLData> meshGLData = m_meshManager.getMeshGLData(meshInfo.meshId);
            if (!meshGLData) {
                std::cerr << "RenderSystem Warning: Could not find MeshGLData for ID " << meshInfo.meshId << " in model '" << modelRenderComp->modelName << "'" << std::endl;
                continue;
            }

            // 获取 Material
            std::shared_ptr<Material> material = m_materialManager.getMaterial(meshInfo.materialName);
            if (!material) {
                std::cerr << "RenderSystem Warning: Could not find Material for name '" << meshInfo.materialName << "' in model '" << modelRenderComp->modelName << "'" << std::endl;
                // 可以使用一个默认材质
                material = m_materialManager.getMaterial("default_material"); // 确保你有一个默认材质
                if (!material) {
                     std::cerr << "RenderSystem Error: No default material found!" << std::endl;
                     continue; // 严重错误，跳过绘制
                }
            }

            // 计算最终的模型矩阵 (实体世界矩阵 * 模型内部局部变换)
            Eigen::Matrix4f finalModelMatrix = modelMatrix * meshInfo.localTransform;
            m_shader.setMat4("model", finalModelMatrix);

            // 设置材质 Uniforms 和绑定纹理
            material->bindTextures(m_shader);
            material->setUniforms(m_shader);

            // 绑定 VAO 并绘制
            meshGLData->Bind();
            glDrawElements(GL_TRIANGLES, meshGLData->getNumIndices(), GL_UNSIGNED_INT, 0);
            meshGLData->Unbind(); // 绘制完成后解绑
            
            // 清理解绑纹理单元（可选，但推荐在 PBR 渲染中确保没有残留绑定）
            // 例如： for(int i=0; i<NUM_TEXTURE_UNITS; ++i) glBindTextureUnit(i, 0);
        }
    }
}