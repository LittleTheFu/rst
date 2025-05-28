#ifndef SHADOW_UTILS_H
#define SHADOW_UTILS_H

#include <Eigen/Dense>
#include <vector>
#include <cmath> // For std::tan and M_PI

class ShadowUtils {
public:
    /**
     * @brief Computes a generic LookAt view matrix.
     * This function does not depend on any camera instance state; it's a pure mathematical tool.
     * @param eye The position of the camera/observer.
     * @param center The target position the observer is looking at.
     * @param up The up direction vector in world space.
     * @return The calculated view matrix.
     */
    static Eigen::Matrix4f CalculateLookAtMatrix(const Eigen::Vector3f& eye, const Eigen::Vector3f& center, const Eigen::Vector3f& up);

    /**
     * @brief Generates the six light space transformation matrices for a point light shadow map.
     * These matrices are used to transform vertices from world space to the light's clip space
     * for rendering to each face of a cubemap shadow map.
     * @param lightPos The position of the light source in world space.
     * @param nearPlane The near clip plane distance for the shadow map projection.
     * @param farPlane The far clip plane distance for the shadow map projection.
     * @param shadowMapWidth The width of the shadow map (used for aspect ratio calculation).
     * @param shadowMapHeight The height of the shadow map (used for aspect ratio calculation).
     * @return A vector containing the six light space transformation matrices.
     */
    static std::vector<Eigen::Matrix4f> CalculatePointLightSpaceMatrices(
        const Eigen::Vector3f& lightPos,
        float nearPlane,
        float farPlane,
        int shadowMapWidth,
        int shadowMapHeight);

    // TODO: Add other shadow-related utility functions here if needed, e.g.:
    // static Eigen::Matrix4f CalculateDirectionalLightSpaceMatrix(...);
    // static void SetupShadowSamplerParameters(...);

private:
    // Private helper function for creating perspective projection matrix
    static Eigen::Matrix4f CreatePerspectiveProjectionMatrix(float fovRadians, float aspectRatio, float nearPlane, float farPlane);
};

#endif // SHADOW_UTILS_H