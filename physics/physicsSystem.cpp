#include "physicsSystem.h"

// For BoxShapeSettings and other shape settings if needed
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h> // For static meshes
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h> // For convex objects
#include <cstdarg>

// You might need to include specific Model headers if you want to extract mesh data
// #include "../model.h"
// #include "../mesh.h" // Assuming Mesh class exists with vertex data access

// Define these parameters based on your scene's needs.
// A good rule of thumb is to allow for some growth beyond your initial scene size.
static const JPH::uint cMaxBodies = 1024;
static const JPH::uint cNumBodyMutexes = 0; // Use default
static const JPH::uint cMaxBodyPairs = 1024;
static const JPH::uint cMaxContactConstraints = 1024;

// The number of physics simulation steps per second.
// A common value is 60 (1/60th second per step).
// This is the *fixed* time step, which should be constant.
static const float cFixedTimestep = 1.0f / 60.0f;

// The maximum number of steps to simulate in a single game frame.
// If your rendering framerate drops significantly, this prevents the physics from "freezing"
// or taking too long in a single frame to catch up.
static const int cMaxPhysicsStepsPerFrame = 5;


// --- Eigen <-> Jolt conversion helpers implementations moved from .h to .cpp ---
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
    rotation.normalize(); // Ensure quaternion is normalized
}

inline Eigen::Matrix4f ComposeMatrix(const Eigen::Vector3f& position, const Eigen::Quaternionf& rotation, const Eigen::Vector3f& scale) {
    Eigen::Matrix4f modelMatrix = Eigen::Matrix4f::Identity();
    modelMatrix.block<3, 3>(0, 0) = rotation.toRotationMatrix() * Eigen::DiagonalMatrix<float, 3>(scale);
    modelMatrix.block<3, 1>(0, 3) = position;
    return modelMatrix;
}
// --- End of conversion helpers implementations ---

// Jolt Trace function:
// Prints to the console by default, but you can redirect it to your logging system.
void PhysicsSystem::TraceImpl(const char* inFMT, ...)
{
    // Use your engine's logging system if available, e.g., LOG_INFO(str);
    // For now, just print to std::cout
    va_list args;
    va_start(args, inFMT);
    vprintf(inFMT, args);
    va_end(args);
    std::cout << std::endl; // Add a newline for easier reading
}

// Jolt Assert function:
// Called when a Jolt assertion fails.
bool PhysicsSystem::AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, JPH::uint inLine)
{
    // Use your engine's assertion system or crash handling
    // For now, print to std::cerr and terminate
    std::cerr << "Jolt Assert Failed: " << inExpression << " at " << inFile << ":" << inLine;
    if (inMessage != nullptr)
        std::cerr << " - " << inMessage;
    std::cerr << std::endl;
    std::abort(); // Or throw an exception, depending on your error handling policy

    // This line won't be reached if std::abort() is called,
    // but it's here to satisfy the function's return type.
    return false; // Return false usually indicates assertion failure, should not continue
}

PhysicsSystem::PhysicsSystem()
    : mPhysicsSystem(nullptr),
      mTempAllocator(nullptr),
      mJobSystem(nullptr),
      // --- IMPORTANT: Initialize interface pointers to nullptr ---
      mBroadPhaseLayerInterface(nullptr),
      mObjectVsBroadPhaseLayerFilter(nullptr),
      mObjectLayerPairFilter(nullptr)
{
    // Nothing else to do in the constructor, actual initialization happens in Init()
}

PhysicsSystem::~PhysicsSystem()
{
    // Cleanup happens in Shutdown(), ensuring proper order
    Shutdown(); // Call Shutdown to ensure proper resource release

    // These pointers should be nullptr after Shutdown(), but for safety:
    delete mObjectLayerPairFilter;
    delete mObjectVsBroadPhaseLayerFilter;
    delete mBroadPhaseLayerInterface;
}

void PhysicsSystem::Init()
{
    std::cout << "PhysicsSystem::Init() - Initializing Jolt Physics..." << std::endl;

    // Register custom allocator and debug functions
    JPH::RegisterDefaultAllocator();

    // Install callbacks
    JPH::Trace = TraceImpl;
    JPH_IF_ENABLE_ASSERTS(JPH::AssertFailed = AssertFailedImpl);

    // Create a factory for serializing / deserializing objects in Jolt.
    JPH::Factory::sInstance = new JPH::Factory();

    // Register all physics types with the factory.
    JPH::RegisterTypes();

    // --- IMPORTANT: Dynamically allocate collision filtering interfaces AFTER Jolt types are registered ---
    // This ensures that any internal Jolt dependencies (like allocators, factory) are ready.
    mBroadPhaseLayerInterface = new Layers::BPLayerInterfaceImpl();
    mObjectVsBroadPhaseLayerFilter = new Layers::ObjectVsBroadPhaseLayerFilterImpl();
    mObjectLayerPairFilter = new Layers::ObjectLayerPairFilterImpl();

    // Create the Jolt Physics system.
    mPhysicsSystem = new JPH::PhysicsSystem();

    // Initialize the physics system with the dynamically allocated interfaces
    mPhysicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
                         *mBroadPhaseLayerInterface,       // Pass by reference
                         *mObjectVsBroadPhaseLayerFilter,  // Pass by reference
                         *mObjectLayerPairFilter);         // Pass by reference

    // Create a temporary allocator for the physics system.
    mTempAllocator = new JPH::TempAllocatorImpl(10 * 1024 * 1024); // 10 MB

    // Create a job system to multi-thread the physics simulation.
    mJobSystem = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, std::thread::hardware_concurrency() - 1); // Use all but one core

    // Set the gravity.
    mPhysicsSystem->SetGravity(JPH::Vec3(0.0f, -9.81f, 0.0f));

    std::cout << "PhysicsSystem::Init() - Jolt Physics initialized." << std::endl;

    // --- Create and add initial scene objects ---
    // Example: Add a static floor body
    {
        JPH::BoxShapeSettings floorShapeSettings(JPH::Vec3(100.0f, 1.0f, 100.0f)); // Size 200x2x200
        floorShapeSettings.SetEmbedded(); // Optimize memory for simple shapes
        JPH::ShapeSettings::ShapeResult floorShapeResult = floorShapeSettings.Create();
        JPH_ASSERT(floorShapeResult.IsValid());
        JPH::ShapeRefC floorShape = floorShapeResult.Get();

        // Position at Y=-1 to make its top surface at Y=0
        JPH::BodyCreationSettings floorSettings(floorShape, JPH::RVec3(0.0f, -1.0f, 0.0f), JPH::Quat::sIdentity(), JPH::EMotionType::Static, Layers::Object::NON_MOVING);

        JPH::BodyID floorID = mPhysicsSystem->GetBodyInterface().CreateAndAddBody(floorSettings, JPH::EActivation::DontActivate);
        std::cout << "Added static floor body (ID: " << floorID.GetIndex() << ")." << std::endl;
    }

    // Example: Add a dynamic sphere (we'll associate this with a placeholder ISceneObject later if needed)
    {
        JPH::SphereShapeSettings sphereShapeSettings(0.5f); // Radius 0.5
        sphereShapeSettings.SetEmbedded();
        JPH::ShapeSettings::ShapeResult sphereShapeResult = sphereShapeSettings.Create();
        JPH_ASSERT(sphereShapeResult.IsValid());
        JPH::ShapeRefC sphereShape = sphereShapeResult.Get();

        // Position at Y=2, so it falls onto the floor
        JPH::BodyCreationSettings sphereSettings(sphereShape, JPH::Vec3(0.0f, 2.0f, 0.0f), JPH::Quat::sIdentity(), JPH::EMotionType::Dynamic, Layers::Object::MOVING);
        sphereSettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateMassAndInertia; // Correct enum value
        sphereSettings.mMassPropertiesOverride.mMass = 1.0f; // Set a mass for the dynamic body

        JPH::BodyID sphereID = mPhysicsSystem->GetBodyInterface().CreateAndAddBody(sphereSettings, JPH::EActivation::Activate);
        mPhysicsSystem->GetBodyInterface().SetLinearVelocity(sphereID, JPH::Vec3(0.0f, -5.0f, 0.0f)); // Give it an initial downward push
        std::cout << "Added dynamic sphere body (ID: " << sphereID.GetIndex() << ")." << std::endl;

        // For demonstration, let's keep track of this sphere's ID.
        // In a real application, you'd associate this with an ISceneObject.
        // We can add a dummy ISceneObject for it if you want to track it visually later.
        // For now, this sphere is just in the physics world.
    }
}

void PhysicsSystem::Update(float inDeltaTime)
{
    // The fixed time step is the ideal simulation step.
    // We can simulate multiple smaller steps if the actual frame time is too large.
    float internalDeltaTime = cFixedTimestep;

    // Clamp the delta time to prevent "spiral of death" and instability
    // if the framerate drops very low.
    float simulationTimeRemaining = inDeltaTime;
    int stepsTaken = 0;

    JPH::BodyInterface& bodyInterface = mPhysicsSystem->GetBodyInterface();

    while (simulationTimeRemaining > 0.0f && stepsTaken < cMaxPhysicsStepsPerFrame)
    {
        // Take a physics step
        mPhysicsSystem->Update(internalDeltaTime, 1, mTempAllocator, mJobSystem);
        simulationTimeRemaining -= internalDeltaTime;
        stepsTaken++;
    }

    // --- Synchronization: Update renderable objects from physics bodies ---
    // Get all active rigid bodies.
    JPH::BodyIDVector activeRigidBodyIDs;
    // Corrected: EBodyType::RigidBody to get active rigid bodies
    mPhysicsSystem->GetActiveBodies(JPH::EBodyType::RigidBody, activeRigidBodyIDs); 

    for (JPH::BodyID bodyID : activeRigidBodyIDs)
    {
        // We might only want to update *dynamic* active bodies, not kinematic or static.
        // Let's filter for dynamic bodies here.
        if (bodyInterface.IsAdded(bodyID) && bodyInterface.GetMotionType(bodyID) == JPH::EMotionType::Dynamic)
        {
            // Check if this BodyID corresponds to one of our managed ISceneObjects
            auto it = mBodyIdToSceneObjectMap.find(bodyID);
            if (it != mBodyIdToSceneObjectMap.end())
            {
                ISceneObject* sceneObject = it->second;

                // Get current position and rotation from physics system
                JPH::RVec3 joltPosition = bodyInterface.GetCenterOfMassPosition(bodyID);
                JPH::Quat joltRotation = bodyInterface.GetRotation(bodyID);

                // Convert to Eigen types
                Eigen::Vector3f eigenPosition = ToEigen(joltPosition);
                Eigen::Quaternionf eigenRotation = ToEigen(joltRotation);

                // Get current scale from the scene object (assuming scale doesn't change in physics)
                // IMPORTANT: Ensure your ISceneObject has a public method getScale() that returns Eigen::Vector3f.
                // If it returns a scalar for uniform scale, you'll need to adapt this.
                Eigen::Vector3f eigenScale = sceneObject->getScale(); 

                // Compose the new model matrix and set it
                Eigen::Matrix4f newModelMatrix = ComposeMatrix(eigenPosition, eigenRotation, eigenScale);
                sceneObject->setModelMatrix(newModelMatrix);

                // Optional: Print position for debugging
                // if (bodyID.GetIndex() == someSphereID.GetIndex()) // If you want to track a specific body
                // {
                //      std::cout << "Body " << bodyID.GetIndex() << " Position: "
                //                       << eigenPosition.x() << ", " << eigenPosition.y() << ", " << eigenPosition.z() << std::endl;
                // }
            }
        }
    }
}

void PhysicsSystem::Shutdown()
{
    std::cout << "PhysicsSystem::Shutdown() - Shutting down Jolt Physics..." << std::endl;

    if (mPhysicsSystem != nullptr)
    {
        JPH::BodyInterface& bodyInterface = mPhysicsSystem->GetBodyInterface();

        // Remove and destroy all bodies managed by this PhysicsSystem
        std::vector<JPH::BodyID> bodiesToDestroy;
        for (const auto& pair : mBodyIdToSceneObjectMap)
        {
            bodiesToDestroy.push_back(pair.first);
        }
        
        for (JPH::BodyID bodyID : bodiesToDestroy)
        {
            // Only remove and destroy if the body still exists in the physics system
            if (bodyInterface.IsAdded(bodyID))
            {
                bodyInterface.RemoveBody(bodyID);
                bodyInterface.DestroyBody(bodyID);
            }
        }
        mBodyIdToSceneObjectMap.clear();
        mSceneObjectToBodyIdMap.clear();

        // Destroy the physics system
        delete mPhysicsSystem;
        mPhysicsSystem = nullptr;
    }
    
    // Destroy allocators (order matters: physics system might use them during its destruction)
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

    // Release collision filtering interfaces
    // Order of deletion should be reverse of creation
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

    // Unregisters all types with the factory and frees the factory itself.
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
    
    // Ensure physics system is initialized before getting BodyInterface
    if (mPhysicsSystem == nullptr)
    {
        std::cerr << "PhysicsSystem::AddSceneObject - Physics system not initialized! Call Init() first." << std::endl;
        return JPH::BodyID();
    }

    JPH::BodyInterface& bodyInterface = mPhysicsSystem->GetBodyInterface();

    Eigen::Vector3f initialPos;
    Eigen::Quaternionf initialRot;
    Eigen::Vector3f initialScale; // Declared here to be used in shape creation

    // Extract position, rotation, and scale from the scene object's model matrix
    DecomposeMatrix(inSceneObject->getModelMatrix(), initialPos, initialRot);
    initialScale = inSceneObject->getScale(); // Assuming getScale() returns Eigen::Vector3f

    // If no shape is provided, try to create a default box shape based on scale
    if (!inShape)
    {
        // Jolt BoxShape expects half-extents.
        // Assuming your ISceneObject's scale directly corresponds to the dimensions.
        // If the base model is 1x1x1, then scale * 0.5f is correct for half-extents.
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
    bodySettings.mUserData = reinterpret_cast<JPH::uint64>(inSceneObject); // Store pointer to your scene object

    if (inMotionType == JPH::EMotionType::Dynamic)
    {
        bodySettings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateMassAndInertia; // Correct enum value
        // You can also set a specific density or mass directly if CalculateMassAndInertia isn't sufficient
        // bodySettings.mMassPropertiesOverride.mMass = 1.0f; // Example: Set a specific mass
    }

    // Corrected: Use IsInvalid() for BodyID check
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
    
    // Ensure physics system is initialized
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

    // Ensure the body is removed and destroyed
    // Check IsValid to prevent double-deletion or operating on an already destroyed body
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