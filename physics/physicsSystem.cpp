#include "physicsSystem.h"

#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <cstdarg>
#include <algorithm> // For std::find

static const JPH::uint cMaxBodies = 1024;
static const JPH::uint cNumBodyMutexes = 0;
static const JPH::uint cMaxBodyPairs = 1024;
static const JPH::uint cMaxContactConstraints = 1024;

static const float cFixedTimestep = 1.0f / 60.0f;
static const int cMaxPhysicsStepsPerFrame = 5;

inline JPH::Vec3 ToJolt(const Eigen::Vector3f& v) {
    return JPH::Vec3(v.x(), v.y(), v.z());
}

inline Eigen::Vector3f ToEigen(const JPH::Vec3& v) {
    return Eigen::Vector3f(v.GetX(), v.GetY(), v.GetZ());
}

inline JPH::RVec3 ToJoltRVec(const Eigen::Vector3f& v) {
    return JPH::RVec3(static_cast<JPH::Real>(v.x()), static_cast<JPH::Real>(v.y()), static_cast<JPH::Real>(v.z()));
}

inline JPH::Quat ToJolt(const Eigen::Quaternionf& q) {
    return JPH::Quat(q.x(), q.y(), q.z(), q.w());
}

inline Eigen::Quaternionf ToEigen(const JPH::Quat& q) {
    return Eigen::Quaternionf(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
}

inline void DecomposeMatrix(const Eigen::Matrix4f& matrix, Eigen::Vector3f& position, Eigen::Quaternionf& rotation) {
    position = matrix.block<3, 1>(0, 3);
    Eigen::Matrix3f rotationMatrix = matrix.block<3, 3>(0, 0);
    rotation = Eigen::Quaternionf(rotationMatrix);
    rotation.normalize();
}

inline Eigen::Matrix4f ComposeMatrix(const Eigen::Vector3f& position, const Eigen::Quaternionf& rotation, const Eigen::Vector3f& scale) {
    Eigen::Matrix4f modelMatrix = Eigen::Matrix4f::Identity();
    modelMatrix.block<3, 3>(0, 0) = rotation.toRotationMatrix() * Eigen::DiagonalMatrix<float, 3>(scale);
    modelMatrix.block<3, 1>(0, 3) = position;
    return modelMatrix;
}

void PhysicsSystem::TraceImpl(const char* inFMT, ...)
{
    va_list args;
    va_start(args, inFMT);
    vprintf(inFMT, args);
    va_end(args);
    std::cout << std::endl;
}

bool PhysicsSystem::AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine)
{
    std::cerr << "Jolt Assert Failed: " << inExpression << " at " << inFile << ":" << inLine;
    if (inMessage != nullptr)
        std::cerr << " - " << inMessage;
    std::cerr << std::endl;
    std::abort();
    return false;
}

PhysicsSystem::PhysicsSystem()
    : mPhysicsSystem(nullptr),
      mTempAllocator(nullptr),
      mJobSystem(nullptr),
      mBroadPhaseLayerInterface(nullptr),
      mObjectVsBroadPhaseLayerFilter(nullptr),
      mObjectLayerPairFilter(nullptr),
      mDynamicSphereBodyID(JPH::BodyID())
{
}

PhysicsSystem::~PhysicsSystem()
{
    Shutdown();
    delete mObjectLayerPairFilter;
    delete mObjectVsBroadPhaseLayerFilter;
    delete mBroadPhaseLayerInterface;
}

void PhysicsSystem::Init()
{
    std::cout << "PhysicsSystem::Init() - Initializing Jolt Physics..." << std::endl;

    JPH::RegisterDefaultAllocator();
    JPH::Trace = TraceImpl;
    JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl);

    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    mBroadPhaseLayerInterface = new Layers::BPLayerInterfaceImpl();
    mObjectVsBroadPhaseLayerFilter = new Layers::ObjectVsBroadPhaseLayerFilterImpl();
    mObjectLayerPairFilter = new Layers::ObjectLayerPairFilterImpl();

    mPhysicsSystem = new JPH::PhysicsSystem();
    mPhysicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
                         *mBroadPhaseLayerInterface,
                         *mObjectVsBroadPhaseLayerFilter,
                         *mObjectLayerPairFilter);

    mTempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024);
    mJobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1);

    mPhysicsSystem->SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));
    mPhysicsSystem->SetContactListener(this);

    std::cout << "PhysicsSystem::Init() - Jolt Physics initialized." << std::endl;

    // Static floor
    {
        JPH::BoxShapeSettings floorShapeSettings(JPH::Vec3(100.0f, 1.0f, 100.0f));
        floorShapeSettings.SetEmbedded();
        JPH::ShapeSettings::ShapeResult floorShapeResult = floorShapeSettings.Create();
        JPH_ASSERT(floorShapeResult.IsValid());
        JPH::ShapeRefC floorShape = floorShapeResult.Get();
        JPH::BodyCreationSettings floorSettings(floorShape, JPH::RVec3(0.0f, -1.0f, 0.0f), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::Object::NON_MOVING);
        JPH::BodyID floorID = mPhysicsSystem->GetBodyInterface().CreateAndAddBody(floorSettings, JPH::EActivation::DontActivate);
        std::cout << "Added static floor body (ID: " << floorID.GetIndex() << ")." << std::endl;
    }

    // Dynamic sphere
    {
        JPH::SphereShapeSettings sphereShapeSettings(0.5f);
        sphereShapeSettings.SetEmbedded();
        JPH::ShapeSettings::ShapeResult sphereShapeResult = sphereShapeSettings.Create();
        JPH_ASSERT(sphereShapeResult.IsValid());
        JPH::ShapeRefC sphereShape = sphereShapeResult.Get();
        JPH::BodyCreationSettings sphereSettings(sphereShape, JPH::Vec3(0.0f, 2.0f, 0.0f), JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::Object::MOVING);
        sphereSettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateMassAndInertia;
        sphereSettings.mMassPropertiesOverride.mMass = 1.0f;

        mDynamicSphereBodyID = mPhysicsSystem->GetBodyInterface().CreateAndAddBody(sphereSettings, JPH::EActivation::Activate);
        mPhysicsSystem->GetBodyInterface().SetLinearVelocity(mDynamicSphereBodyID, JPH::Vec3(0.0f, -5.0f, 0.0f));
        std::cout << "Added dynamic sphere body (ID: " << mDynamicSphereBodyID.GetIndex() << ")." << std::endl;
    }
}

void PhysicsSystem::Update(float inDeltaTime)
{
    float internalDeltaTime = cFixedTimestep;
    float simulationTimeRemaining = inDeltaTime;
    int stepsTaken = 0;

    JPH::BodyInterface& bodyInterface = mPhysicsSystem->GetBodyInterface();

    while (simulationTimeRemaining > 0.0f && stepsTaken < cMaxPhysicsStepsPerFrame)
    {
        mPhysicsSystem->Update(internalDeltaTime, 1, mTempAllocator, mJobSystem);
        simulationTimeRemaining -= internalDeltaTime;
        stepsTaken++;
    }

    if (!mDynamicSphereBodyID.IsInvalid())
    {
        if (bodyInterface.IsActive(mDynamicSphereBodyID))
        {
            JPH::RVec3 joltPosition = bodyInterface.GetCenterOfMassPosition(mDynamicSphereBodyID);
            Eigen::Vector3f eigenPosition = ToEigen(joltPosition);
            // --- 修正点: 获取物理时间 ---
            // std::cout << "Dynamic Sphere Position: ("
            //           << eigenPosition.x() << ", "
            //           << eigenPosition.y() << ", "
            //           << eigenPosition.z() << ") Time: " << mPhysicsSystem->GetTime() << std::endl;
        }
        else
        {
            static bool printed_sleep = false;
            if (!printed_sleep)
            {
                std::cout << "Dynamic Sphere has gone to sleep." << std::endl;
                printed_sleep = true;
            }
        }
    }
}

JPH::ValidateResult PhysicsSystem::OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult)
{
    // --- 修正点: 使用正确的 ValidateResult 枚举值 ---
    return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
}

void PhysicsSystem::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
{
    std::cout << "Contact added between body " << inBody1.GetID().GetIndex() << " and " << inBody2.GetID().GetIndex()
              << " at position: (" << inManifold.mBaseOffset.GetX() << ", " << inManifold.mBaseOffset.GetY() << ", " << inManifold.mBaseOffset.GetZ() << ")" << std::endl;
}

void PhysicsSystem::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
{
}

void PhysicsSystem::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
{
}


void PhysicsSystem::Shutdown()
{
    std::cout << "PhysicsSystem::Shutdown() - Shutting down Jolt Physics..." << std::endl;

    if (mPhysicsSystem != nullptr)
    {
        JPH::BodyInterface& bodyInterface = mPhysicsSystem->GetBodyInterface();
        std::vector<JPH::BodyID> bodiesToDestroy;
        for (const auto& pair : mBodyIdToSceneObjectMap)
        {
            bodiesToDestroy.push_back(pair.first);
        }
        if (!mDynamicSphereBodyID.IsInvalid() && std::find(bodiesToDestroy.begin(), bodiesToDestroy.end(), mDynamicSphereBodyID) == bodiesToDestroy.end())
        {
            bodiesToDestroy.push_back(mDynamicSphereBodyID);
        }
        
        for (JPH::BodyID bodyID : bodiesToDestroy)
        {
            if (bodyInterface.IsAdded(bodyID))
            {
                bodyInterface.RemoveBody(bodyID);
            }
            bodyInterface.DestroyBody(bodyID);
        }
        mBodyIdToSceneObjectMap.clear();
        mSceneObjectToBodyIdMap.clear();

        delete mPhysicsSystem;
        mPhysicsSystem = nullptr;
    }
    
    if (mJobSystem != nullptr)
    {
        delete mJobSystem;
        mJobSystem = nullptr;
    }
    if (mTempAllocator != nullptr)
    {
        delete mTempAllocator;
        mTempAllocator = nullptr;
    }

    if (mObjectLayerPairFilter != nullptr)
    {
        delete mObjectLayerPairFilter;
        mObjectLayerPairFilter = nullptr;
    }
    if (mObjectVsBroadPhaseLayerFilter != nullptr)
    {
        delete mObjectVsBroadPhaseLayerFilter;
        mObjectVsBroadPhaseLayerFilter = nullptr;
    }
    if (mBroadPhaseLayerInterface != nullptr)
    {
        delete mBroadPhaseLayerInterface;
        mBroadPhaseLayerInterface = nullptr;
    }

    if (JPH::Factory::sInstance != nullptr)
    {
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }

    std::cout << "PhysicsSystem::Shutdown() - Jolt Physics shut down." << std::endl;
}

JPH::BodyID PhysicsSystem::AddSceneObject(ISceneObject* inSceneObject, JPH::EMotionType inMotionType, JPH::ObjectLayer inObjectLayer, JPH::ShapeRefC inShape)
{
    if (inSceneObject == nullptr)
    {
        std::cerr << "PhysicsSystem::AddSceneObject - Cannot add null ISceneObject!" << std::endl;
        return JPH::BodyID();
    }
    if (mSceneObjectToBodyIdMap.count(inSceneObject))
    {
        std::cerr << "PhysicsSystem::AddSceneObject - ISceneObject '" << inSceneObject->getName() << "' already has a physics body!" << std::endl;
        return mSceneObjectToBodyIdMap[inSceneObject];
    }
    
    if (mPhysicsSystem == nullptr)
    {
        std::cerr << "PhysicsSystem::AddSceneObject - Physics system not initialized! Call Init() first." << std::endl;
        return JPH::BodyID();
    }

    JPH::BodyInterface& bodyInterface = mPhysicsSystem->GetBodyInterface();

    Eigen::Vector3f initialPos;
    Eigen::Quaternionf initialRot;
    Eigen::Vector3f initialScale;

    DecomposeMatrix(inSceneObject->getModelMatrix(), initialPos, initialRot);
    initialScale = inSceneObject->getScale();

    if (!inShape)
    {
        JPH::Vec3 halfExtents = ToJolt(initialScale * 0.5f);
        JPH::BoxShapeSettings boxShapeSettings(halfExtents);
        boxShapeSettings.SetEmbedded();
        JPH::ShapeSettings::ShapeResult boxShapeResult = boxShapeSettings.Create();
        if (!boxShapeResult.IsValid())
        {
            std::cerr << "PhysicsSystem::AddSceneObject - Failed to create default box shape for '" << inSceneObject->getName() << "': " << boxShapeResult.GetError().c_str() << std::endl;
            return JPH::BodyID();
        }
        inShape = boxShapeResult.Get();
        std::cout << "PhysicsSystem::AddSceneObject - Auto-generated box shape for '" << inSceneObject->getName() << "' with half extents: "
                  << halfExtents.GetX() << ", " << halfExtents.GetY() << ", " << halfExtents.GetZ() << std::endl;
    }

    JPH::BodyCreationSettings bodySettings(inShape, ToJoltRVec(initialPos), ToJolt(initialRot), inMotionType, inObjectLayer);
    bodySettings.mUserData = reinterpret_cast<JPH::uint64>(inSceneObject);

    if (inMotionType == JPH::EMotionType::Dynamic)
    {
        bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateMassAndInertia;
    }

    JPH::BodyID newBodyID = bodyInterface.CreateAndAddBody(bodySettings, JPH::EActivation::Activate);
    if (newBodyID.IsInvalid())
    {
        std::cerr << "PhysicsSystem::AddSceneObject - Failed to create Jolt body for '" << inSceneObject->getName() << "'!" << std::endl;
        return JPH::BodyID();
    }

    mBodyIdToSceneObjectMap[newBodyID] = inSceneObject;
    mSceneObjectToBodyIdMap[inSceneObject] = newBodyID;

    std::cout << "PhysicsSystem::AddSceneObject - Added '" << inSceneObject->getName() << "' with BodyID: " << newBodyID.GetIndex() << std::endl;

    return newBodyID;
}

void PhysicsSystem::RemoveSceneObject(ISceneObject* inSceneObject)
{
    if (inSceneObject == nullptr)
    {
        std::cerr << "PhysicsSystem::RemoveSceneObject - Cannot remove null ISceneObject!" << std::endl;
        return;
    }
    
    if (mPhysicsSystem == nullptr)
    {
        std::cerr << "PhysicsSystem::RemoveSceneObject - Physics system not initialized!" << std::endl;
        return;
    }

    auto it = mSceneObjectToBodyIdMap.find(inSceneObject);
    if (it == mSceneObjectToBodyIdMap.end())
    {
        std::cerr << "PhysicsSystem::RemoveSceneObject - ISceneObject '" << inSceneObject->getName() << "' does not have a physics body to remove!" << std::endl;
        return;
    }

    JPH::BodyID bodyID = it->second;
    JPH::BodyInterface& bodyInterface = mPhysicsSystem->GetBodyInterface();

    if (bodyInterface.IsAdded(bodyID))
    {
        bodyInterface.RemoveBody(bodyID);
        bodyInterface.DestroyBody(bodyID);
    }
    else
    {
        std::cerr << "PhysicsSystem::RemoveSceneObject - Attempted to remove invalid/already removed BodyID " << bodyID.GetIndex() << " for '" << inSceneObject->getName() << "'." << std::endl;
    }

    mBodyIdToSceneObjectMap.erase(bodyID);
    mSceneObjectToBodyIdMap.erase(inSceneObject);

    std::cout << "PhysicsSystem::RemoveSceneObject - Removed '" << inSceneObject->getName() << "' (BodyID: " << bodyID.GetIndex() << ")." << std::endl;
}