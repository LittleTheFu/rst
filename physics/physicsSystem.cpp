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

void PhysicsSystem::DrawDebug() {
    // Check if the debug renderer is set and physics system is initialized
    if (JPH::DebugRenderer::sInstance != nullptr && mPhysicsSystem != nullptr) {
        // Create an instance of drawing settings
        JPH::BodyManager::DrawSettings bodyDrawSettings;
        
        // Configure what you want to draw
        bodyDrawSettings.mDrawShape = true;           // Draw collision shapes (e.g., wireframe)
        bodyDrawSettings.mDrawBoundingBox = true;     // Draw Axis-Aligned Bounding Boxes
        bodyDrawSettings.mDrawWorldTransform = true;  // Draw body's world coordinate axes
        bodyDrawSettings.mDrawCenterOfMassTransform = true; // Draw body's center of mass axes
        bodyDrawSettings.mDrawVelocity = true;        // Draw velocity vectors
        bodyDrawSettings.mDrawMassAndInertia = false; // Usually not needed for visual debug

        // --- Now, correctly call DrawBodies with the settings and your renderer ---
        mPhysicsSystem->DrawBodies(bodyDrawSettings, JPH::DebugRenderer::sInstance); 

        // --- Other debug visualizations (these usually only take the renderer) ---
        // mPhysicsSystem->DrawConstraints(JPH::DebugRenderer::sInstance);
        // mPhysicsSystem->DrawConstraintLimits(JPH::DebugRenderer::sInstance);
        // mPhysicsSystem->DrawConstraintReferenceFrame(JPH::DebugRenderer::sInstance);

        // You can also enable broadphase debugging if needed
        // mPhysicsSystem->DrawBroadPhase(JPH::DebugRenderer::sInstance);
        // And forces, if applicable
        // mPhysicsSystem->DrawForces(JPH::DebugRenderer::sInstance);
    }
}

void PhysicsSystem::TraceImpl(const char *inFMT, ...)
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
}

// --- 常量定义（确保这些常量在 PhysicsSystem.cpp 的顶部或合适的位置定义）---
// 物理模拟的固定时间步长（例如 60Hz，即每秒 60 步）
// const float cFixedTimestep = 1.0f / 60.0f;
// 每帧最多允许进行多少次物理步进，防止渲染帧率过低时物理模拟跟不上，
// 避免“螺旋式死亡”（spiral of death）
// const int cMaxPhysicsStepsPerFrame = 5;

void PhysicsSystem::Update(float inDeltaTime)
{
    // 用于处理可变帧率的固定时间步长逻辑
    // inDeltaTime 是从 Window::render() 传递进来的实际帧时间
    float simulationTimeRemaining = inDeltaTime;
    int stepsTaken = 0;

    // 获取 Jolt 物理系统的 BodyInterface，用于操作物理体
    JPH::BodyInterface& bodyInterface = mPhysicsSystem->GetBodyInterface();

    // 循环进行物理步进，直到用完当前帧的时间或者达到最大步进次数
    while (simulationTimeRemaining > 0.0f && stepsTaken < cMaxPhysicsStepsPerFrame)
    {
        // 核心的 Jolt 物理世界更新调用。
        // cFixedTimestep: 每次步进的时间量。
        // 1: 碰撞子步数 (Collision Steps)。Jolt 文档通常推荐为 1。
        // mTempAllocator: 临时内存分配器。
        // mJobSystem: 作业系统，用于多线程计算。
        mPhysicsSystem->Update(cFixedTimestep, 1, mTempAllocator, mJobSystem);
        
        simulationTimeRemaining -= cFixedTimestep; // 减去已消耗的物理时间
        stepsTaken++;                             // 增加已采取的物理步数
    }

    // --- 物理状态到渲染状态的同步（核心逻辑）---
    // 1. 获取所有当前活跃的物理体。
    //    活跃的物理体是指那些位置或旋转在上一物理步中发生变化的物体（例如，正在移动、碰撞或掉落）。
    JPH::BodyIDVector activeRigidBodyIDs;
    mPhysicsSystem->GetActiveBodies(JPH::EBodyType::RigidBody, activeRigidBodyIDs);

    // 2. 遍历每一个活跃的物理体，并更新其对应的渲染对象（ISceneObject）。
    for (JPH::BodyID bodyID : activeRigidBodyIDs)
    {
        // 检查这个 BodyID 是否仍然有效，并且在我们的映射表中是否存在对应的 ISceneObject。
        // bodyInterface.IsAdded(bodyID) 确保物理体仍在物理世界中。
        if (bodyInterface.IsAdded(bodyID) && mBodyIdToSceneObjectMap.count(bodyID))
        {
            // 通过 BodyID 从映射表中获取对应的 ISceneObject 指针。
            ISceneObject* sceneObject = mBodyIdToSceneObjectMap[bodyID];

            // 从 Jolt 物理体获取其最新的世界空间位置和旋转。
            JPH::RVec3 joltPosition = bodyInterface.GetCenterOfMassPosition(bodyID);
            JPH::Quat joltRotation = bodyInterface.GetRotation(bodyID);

            // 将 Jolt 的数学类型（JPH::RVec3, JPH::Quat）转换为你渲染引擎使用的 Eigen 类型。
            Eigen::Vector3f eigenPosition = ToEigen(joltPosition);
            Eigen::Quaternionf eigenRotation = ToEigen(joltRotation);

            // 获取 ISceneObject 自身的缩放。
            // 缩放通常不随物理模拟而改变，所以我们使用对象当前的缩放值。
            Eigen::Vector3f eigenScale = sceneObject->getScale(); 

            // 使用最新的位置、旋转和原有的缩放来组合出新的模型矩阵。
            Eigen::Matrix4f newModelMatrix = ComposeMatrix(eigenPosition, eigenRotation, eigenScale);

            // **将新计算出的模型矩阵应用到 ISceneObject。**
            // 这是最关键的一步，它将物理模拟的结果同步到渲染对象上，
            // 确保渲染时物体显示在正确的位置和姿态。
            sceneObject->setModelMatrix(newModelMatrix);
        }
    }

    // 注意：你之前代码中关于 mDynamicSphereBodyID 的硬编码部分可以删除，
    // 因为现在所有添加到物理系统的动态物体都会通过上面的循环进行通用处理和同步。
    // if (!mDynamicSphereBodyID.IsInvalid()) { /* ... */ }
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