#ifndef RENDERABLE_H
#define RENDERABLE_H

#include "Shader.h"

class IRenderable
{
public:
    virtual ~IRenderable() = default;

    virtual void render(Shader &shader) const = 0;
};

#endif