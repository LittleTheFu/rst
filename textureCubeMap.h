// TextureCubeMap.h
#include "Texture.h"

class TextureCubeMap : public Texture {
private:
    int mipLevels_;

public:
    TextureCubeMap(int resolution, GLenum internalFormat, int mipLevels = 1);

    void allocateStorage(int mipLevels) override;
    void setParameters() override;

    // 特定方法：上传特定面的数据
    void uploadFaceData(GLenum faceTarget, const void* data, GLenum format, GLenum type, int level = 0);

    // 获取分辨率 (宽高等于分辨率)
    int getResolution() const { return width_; } // resolution_ 在基类中可以用 width_ 表示
};

