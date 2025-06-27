#ifndef CAMERA_H
#define CAMERA_H

#include <Eigen/Dense>
// #include <corecrt_math_defines.h> // 这个头文件通常包含 M_PI，但建议在 CMake 中定义或者使用 Eigen/Core 的 M_PI_2 等

// 确保 M_PI 被定义。在C++20中，<numbers>头文件提供了std::numbers::pi。
// 对于旧标准，通常可以通过这样定义：
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 定义相机移动方向的枚举
enum CameraMovement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,   // 沿着相机局部上方向移动
    DOWN  // 沿着相机局部下方向移动
};

class Camera
{
public:
    // 相机参数
    float movementSpeed = 2.5f * 1.0f;
    float mouseSensitivity = 0.0f;
    float zoomSensitivity = 0.5f; // 用于滚轮缩放FOV
    float fov = 45.0f;           // 视野
    float aspectRatio = 800.0f / 600.0f; // 屏幕宽高比
    float nearClip = 0.1f;       // 近裁剪面距离
    float farClip = 50.0f;       // 远裁剪面距离

    // 相机位置和方向向量
    Eigen::Vector3f Position;  // 相机在世界中的位置
    Eigen::Vector3f WorldUp;   // 世界坐标系的上方向 (通常是Y轴正方向，(0, 1, 0))

    Eigen::Vector3f Front;     // 相机朝向的前方向量
    Eigen::Vector3f Right;     // 相机朝向的右方向量 (根据Front和WorldUp计算)
    Eigen::Vector3f Up;        // 相机朝向的上方向量 (根据Right和Front计算，是局部上方向)

    // 欧拉角
    float Yaw;                 // 偏航角 (绕Y轴旋转)
    float Pitch;               // 俯仰角 (绕X轴旋转)

    // 构造函数
    Camera(const Eigen::Vector3f &position = Eigen::Vector3f(0.0f, 0.0f, 3.0f),
           const Eigen::Vector3f &worldUp = Eigen::Vector3f(0.0f, 1.0f, 0.0f),
           float yaw = -90.0f,   // 初始偏航角，默认-90度使相机看向X轴正方向 (OpenGL默认看向Z轴负方向)
           float pitch = 0.0f);  // 初始俯仰角

    // 获取视图矩阵
    Eigen::Matrix4f GetViewMatrix() const;
    // 获取投影矩阵
    Eigen::Matrix4f GetProjectionMatrix() const;

    // 设置视野
    void setFOV(float fov);
    // 设置宽高比 (只接收一个比例值，而非宽和高)
    void setAspectRatio(float ratio);

    // 处理键盘输入
    void ProcessKeyboard(CameraMovement direction, float deltaTime);
    // 处理鼠标移动 (用于视角旋转)
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    // 处理鼠标滚轮 (用于调整FOV)
    void ProcessMouseScroll(float yoffset);

    // 获取相机当前位置
    Eigen::Vector3f getPosition() const;
    // 获取相机当前前方向量
    Eigen::Vector3f getFront() const;

    // 根据当前的Yaw和Pitch更新Front, Right, Up向量
    void updateCameraVectors();
    
    // 如果你还需要一个让相机看向某个目标的方法，这个也保留
    void lookAt(const Eigen::Vector3f &target);
};

#endif