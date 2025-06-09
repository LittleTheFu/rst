#ifndef UNIFORM_BUFFER_H
#define UNIFORM_BUFFER_H

#include "GLResource.h"
#include <glad/glad.h>

class UniformBuffer : public GLResource
{
public:
    UniformBuffer();

    // 使用 DSA 创建和分配存储
    // 初始化设置不再需要 glBindBuffer
    void create(size_t size, GLenum usage); 

    // 将 UBO 绑定到特定的着色器绑定点
    // 这是着色器进行“激活”的关键步骤
    void bindToBindingPoint(GLuint bindingPoint) const; 

    // 使用 DSA 更新数据；在此之前不需要调用 bind()
    void updateData(size_t offset, size_t size, const void* data); 

protected:
    void release() override; 

private:
    size_t size_ = 0; 
};

#endif // UNIFORM_BUFFER_H