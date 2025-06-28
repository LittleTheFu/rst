#ifndef PHYSICS_PHYSICS_SYSTEM_H
#define PHYSICS_PHYSICS_SYSTEM_H

// Jolt Physics includes
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollector.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/AABoxCast.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/ContactListener.h>

// Your engine includes
#include "sceneData.h" // Assuming SceneData.h is in the parent directory
#include "sceneObject.h" // For ISceneObject and its derived classes
#include "debug_utils.h" // For GL_CHECK_ERROR and potentially your logging

// For Eigen to Jolt conversions
#include <Eigen/Dense>

#include <iostream> // For Jolt trace/assert
#include <map>      // To map Jolt BodyID to your ISceneObject
#include <vector>   // To store BodyIDs
#include <memory>   // For std::unique_ptr

// Include the collision layers header
#include "PhysicsLayers.h"

inline JPH::Vec3 ToJolt(const Eigen::Vector3f& v);
inline Eigen::Vector3f ToEigen(const JPH::Vec3& v);
inline JPH::RVec3 ToJoltRVec(const Eigen::Vector3f& v);
inline JPH::Quat ToJolt(const Eigen::Quaternionf& q);
inline Eigen::Quaternionf ToEigen(const JPH::Quat& q);
inline void DecomposeMatrix(const Eigen::Matrix4f& matrix, Eigen::Vector3f& position, Eigen::Quaternionf& rotation);
inline Eigen::Matrix4f ComposeMatrix(const Eigen::Vector3f& position, const Eigen::Quaternionf& rotation, const Eigen::Vector3f& scale);


class PhysicsSystem : public JPH::ContactListener
{
public:
    PhysicsSystem();
    ~PhysicsSystem();

    void Init();
    void Update(float inDeltaTime);
    void Shutdown();

    JPH::BodyInterface& GetBodyInterface() { 
        JPH_ASSERT(mPhysicsSystem != nullptr);
        return mPhysicsSystem->GetBodyInterface(); 
    }

    JPH::BodyID AddSceneObject(ISceneObject* inSceneObject, JPH::EMotionType inMotionType, JPH::ObjectLayer inObjectLayer, JPH::ShapeRefC inShape = nullptr);
    void RemoveSceneObject(ISceneObject* inSceneObject);

    // ContactListener overrides
    virtual JPH::ValidateResult OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override;
    virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;
    virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;
    virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;

    // Getter for the dynamic sphere's BodyID for main.cpp to query its position
    JPH::BodyID GetDynamicSphereBodyID() const { return mDynamicSphereBodyID; }

private:
    static void TraceImpl(const char* inFMT, ...);
    static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine);

    JPH::PhysicsSystem* mPhysicsSystem;
    JPH::TempAllocatorImpl* mTempAllocator;
    JPH::JobSystemThreadPool* mJobSystem;

    Layers::BPLayerInterfaceImpl* mBroadPhaseLayerInterface;
    Layers::ObjectVsBroadPhaseLayerFilterImpl* mObjectVsBroadPhaseLayerFilter;
    Layers::ObjectLayerPairFilterImpl* mObjectLayerPairFilter;

    std::map<JPH::BodyID, ISceneObject*> mBodyIdToSceneObjectMap;
    std::map<ISceneObject*, JPH::BodyID> mSceneObjectToBodyIdMap;

    JPH::BodyID mDynamicSphereBodyID;
};

#endif // PHYSICS_PHYSICS_SYSTEM_H