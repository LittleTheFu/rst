#ifndef ISCENEOBJECT_H
#define ISCENEOBJECT_H

#include "renderable.h"
#include "transformable.h"
#include "nameable.h"
#include <Eigen/Dense>

class ISceneObject : public IRenderable, public ITransformable, public INamable {
public:
   
    virtual ~ISceneObject() = default;

    //quick and dirty, it should be a pure function.I will come back here later...
    virtual  Eigen::Vector3f getLocalBoundingBoxHalfExtents() const
    {
        return Eigen::Vector3f(0.0f, 0.0f, 0.0f);
    }
};

#endif // ISCENEOBJECT_H