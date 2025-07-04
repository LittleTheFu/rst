// components/ModelRenderComponent.h
#ifndef MODEL_RENDER_COMPONENT_H
#define MODEL_RENDER_COMPONENT_H

#include <string>

struct ModelRenderComponent {
    std::string modelName; // 引用 ModelManager 中加载的 ModelResource 的名称

    ModelRenderComponent(const std::string& name = "") : modelName(name) {}
};

#endif // MODEL_RENDER_COMPONENT_H