#include "ShadowUtils.h"
#include <iostream> // For potential debug output

// Ensure M_PI is defined, if not already by <cmath> with certain flags
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Private static helper function implementation
Eigen::Matrix4f ShadowUtils::CreatePerspectiveProjectionMatrix(float fovRadians, float aspectRatio, float nearPlane, float farPlane) {
    Eigen::Matrix4f proj = Eigen::Matrix4f::Zero(); // Initialize to zero matrix

    float tanHalfFovy = std::tan(fovRadians / 2.0f);

    // Set elements of the perspective projection matrix (column-major form)
    proj(0, 0) = 1.0f / (aspectRatio * tanHalfFovy);
    proj(1, 1) = 1.0f / tanHalfFovy;
    proj(2, 2) = -(farPlane + nearPlane) / (farPlane - nearPlane);
    proj(2, 3) = -2.0f * farPlane * nearPlane / (farPlane - nearPlane);
    proj(3, 2) = -1.0f; // Observer space Z to clip space W

    return proj;
}

// Public static member function implementations
Eigen::Matrix4f ShadowUtils::CalculateLookAtMatrix(const Eigen::Vector3f& eye, const Eigen::Vector3f& center, const Eigen::Vector3f& up) {
    Eigen::Vector3f f = (center - eye).normalized(); // Forward vector
    Eigen::Vector3f u = up.normalized();            // Normalized Up vector

    // Check for collinearity to prevent issues with cross product returning a zero vector
    // A small epsilon check for robustness
    if (f.cross(u).norm() < 1e-6) {
        // Handle the case where f and u are collinear (e.g., light source is directly above/below target)
        // This might require a different 'up' vector or a slight perturbation
        std::cerr << "WARNING: ShadowUtils::CalculateLookAtMatrix - Forward and Up vectors are collinear!" << std::endl;
        // For a cubemap, this is usually handled by the specific predefined 'up' vectors
        // but for a generic LookAt, it's a valid concern.
    }
    
    Eigen::Vector3f s = f.cross(u).normalized();    // Right vector (cross product of f and u)
    u = s.cross(f);                                 // Corrected Up vector (cross product of s and f)

    Eigen::Matrix4f result = Eigen::Matrix4f::Identity(); // Initialize as identity matrix

    // Set the rotation part
    result(0,0) = s.x(); result(0,1) = s.y(); result(0,2) = s.z();
    result(1,0) = u.x(); result(1,1) = u.y(); result(1,2) = u.z();
    result(2,0) = -f.x(); result(2,1) = -f.y(); result(2,2) = -f.z(); // View matrix looks down negative Z-axis

    // Set the translation part
    result(0, 3) = -s.dot(eye);
    result(1, 3) = -u.dot(eye);
    result(2, 3) = f.dot(eye); // This is f.dot(eye) because the view matrix looks down negative Z

    return result;
}

std::vector<Eigen::Matrix4f> ShadowUtils::CalculatePointLightSpaceMatrices(
    const Eigen::Vector3f& lightPos,
    float nearPlane,
    float farPlane,
    int shadowMapWidth,
    int shadowMapHeight)
{
    // Projection matrix: For cubemap shadows, FOV is fixed at 90 degrees (pi/2 radians)
    float fovRadians = M_PI / 2.0f; // 90 degrees FOV
    float aspect = (float)shadowMapWidth / (float)shadowMapHeight;
    
    Eigen::Matrix4f shadowProj = CreatePerspectiveProjectionMatrix(fovRadians, aspect, nearPlane, farPlane);

    // View matrix targets and up vectors for the six directions
    std::vector<Eigen::Vector3f> lookTargets = {
        lightPos + Eigen::Vector3f(1.0f, 0.0f, 0.0f),  // +X
        lightPos + Eigen::Vector3f(-1.0f, 0.0f, 0.0f), // -X
        lightPos + Eigen::Vector3f(0.0f, 1.0f, 0.0f),  // +Y
        lightPos + Eigen::Vector3f(0.0f, -1.0f, 0.0f), // -Y
        lightPos + Eigen::Vector3f(0.0f, 0.0f, 1.0f),  // +Z
        lightPos + Eigen::Vector3f(0.0f, 0.0f, -1.0f)  // -Z
    };

    std::vector<Eigen::Vector3f> lookUps = {
        Eigen::Vector3f(0.0f, 1.0f, 0.0f),   // +X face, Y is up
        Eigen::Vector3f(0.0f, 1.0f, 0.0f),   // -X face, Y is up
        Eigen::Vector3f(0.0f, 0.0f, 1.0f),   // +Y face, looking up Y, Z is up (ensure orthogonal right-hand system)
        Eigen::Vector3f(0.0f, 0.0f, -1.0f),  // -Y face, looking down -Y, -Z is up
        Eigen::Vector3f(0.0f, 1.0f, 0.0f),   // +Z face, Y is up
        Eigen::Vector3f(0.0f, 1.0f, 0.0f)    // -Z face, Y is up
    };

    std::vector<Eigen::Matrix4f> lightSpaceMatrices;
    lightSpaceMatrices.reserve(6); // Reserve space to avoid reallocations

    for (int i = 0; i < 6; ++i) {
        Eigen::Matrix4f lightView = CalculateLookAtMatrix(lightPos, lookTargets[i], lookUps[i]);
        lightSpaceMatrices.push_back(shadowProj * lightView);
    }

    return lightSpaceMatrices;
}