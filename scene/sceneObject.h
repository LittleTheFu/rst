#ifndef ISCENEOBJECT_H
#define ISCENEOBJECT_H

#include "renderable.h"
#include "transformable.h"
#include "nameable.h"

class ISceneObject : public IRenderable, public ITransformable, public INamable {
public:
   
    virtual ~ISceneObject() = default;
};

#endif // ISCENEOBJECT_H