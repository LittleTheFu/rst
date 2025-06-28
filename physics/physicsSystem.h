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

// Your engine includes
#include "sceneData.h" // Assuming SceneData.h is in the parent directory
#include "sceneObject.h" // For ISceneObject and its derived classes
#include "debug_utils.h" // For GL_CHECK_ERROR and potentially your logging

// For Eigen to Jolt conversions
#include <Eigen/Dense>

#include <iostream> // For Jolt trace/assert
#include <map>      // To map Jolt BodyID to your ISceneObject
#include <vector>   // To store BodyIDs
#include <memory>   // For std::unique_ptr

// Include the collision layers header
#include "PhysicsLayers.h"

// Define a namespace for your engine if you use one, e.g., namespace rst {
// For now, let's keep it in the global namespace as per your context.

// Helper functions for Eigen <-> Jolt conversions
// Jolt uses float for Vec3 by default, but RVec3 can be double depending on JPH_DOUBLE_PRECISION
inline JPH::Vec3 ToJolt(const Eigen::Vector3f& v);

inline Eigen::Vector3f ToEigen(const JPH::Vec3& v);

// Jolt's RVec3 is double if JPH_DOUBLE_PRECISION is defined, otherwise float
// If you enable double precision in Jolt, you'll need to change Eigen::Vector3f to Eigen::Vector3d here.
inline JPH::RVec3 ToJoltRVec(const Eigen::Vector3f& v);


inline JPH::Quat ToJolt(const Eigen::Quaternionf& q);

inline Eigen::Quaternionf ToEigen(const JPH::Quat& q);

// Function to decompose an Eigen::Matrix4f into position and rotation
// This is crucial because Jolt wants position and quaternion separately.
inline void DecomposeMatrix(const Eigen::Matrix4f& matrix, Eigen::Vector3f& position, Eigen::Quaternionf& rotation);

// Function to compose a Model Matrix from position and rotation (and optional scale if needed)
// Assuming uniform scale, or you need to extend ITransformable for non-uniform scale
inline Eigen::Matrix4f ComposeMatrix(const Eigen::Vector3f& position, const Eigen::Quaternionf& rotation, const Eigen::Vector3f& scale);


class PhysicsSystem
{
public:
    PhysicsSystem();
    ~PhysicsSystem();

    // Initializes the Jolt Physics system and adds initial bodies.
    void Init();

    // Updates the physics simulation by a given delta time.
    void Update(float inDeltaTime);

    // Shuts down the Jolt Physics system, removing all bodies.
    void Shutdown();

    // Returns the BodyInterface for interacting with physics bodies.
    // Note: This returns a reference to the internal BodyInterface, ensure mPhysicsSystem is valid.
    JPH::BodyInterface& GetBodyInterface() { 
        JPH_ASSERT(mPhysicsSystem != nullptr); // Assert that physics system is initialized
        return mPhysicsSystem->GetBodyInterface(); 
    }

    // Adds a renderable object to the physics simulation.
    // This will create a corresponding Jolt Body based on the ISceneObject's properties.
    // @param inSceneObject The ISceneObject to add. The PhysicsSystem will manage its corresponding Jolt Body.
    // @param inMotionType The motion type (Static, Dynamic, Kinematic).
    // @param inObjectLayer The object layer for collision filtering.
    // @param inShape The Jolt Shape to use for this object. If nullptr, a default box shape will be attempted.
    // @return The JPH::BodyID of the created body, or JPH::BodyID() if creation failed.
    JPH::BodyID AddSceneObject(ISceneObject* inSceneObject, JPH::EMotionType inMotionType, JPH::ObjectLayer inObjectLayer, JPH::ShapeRefC inShape = nullptr);

    // Removes a scene object from the physics simulation.
    void RemoveSceneObject(ISceneObject* inSceneObject);

private:
    // Internal helper for Jolt Trace function
    static void TraceImpl(const char* inFMT, ...);

    // Internal helper for Jolt AssertFailed function
    static bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine);

    // Physics world
    JPH::PhysicsSystem* mPhysicsSystem;

    // Allocators
    JPH::TempAllocatorImpl* mTempAllocator;
    JPH::JobSystemThreadPool* mJobSystem;

    // Collision filtering - Changed to pointers as they are abstract interfaces
    Layers::BPLayerInterfaceImpl* mBroadPhaseLayerInterface;
    Layers::ObjectVsBroadPhaseLayerFilterImpl* mObjectVsBroadPhaseLayerFilter;
    Layers::ObjectLayerPairFilterImpl* mObjectLayerPairFilter;

    // Store mapping from Jolt BodyID to your ISceneObject pointers
    std::map<JPH::BodyID, ISceneObject*> mBodyIdToSceneObjectMap;
    std::map<ISceneObject*, JPH::BodyID> mSceneObjectToBodyIdMap; // For quick lookup when removing
};

#endif // PHYSICS_PHYSICS_SYSTEM_H