#ifndef PHYSICS_LAYERS_H
#define PHYSICS_LAYERS_H

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>

/// All physics objects in Jolt have a BroadPhaseLayer and an ObjectLayer
namespace Layers
{
    // Define your object layers
    // You can have up to 256 different object layers
    namespace Object
    {
        static constexpr JPH::ObjectLayer NON_MOVING = 0; // Static objects like ground, walls
        static constexpr JPH::ObjectLayer MOVING = 1;     // Dynamic objects like boxes, characters
        static constexpr JPH::ObjectLayer NUM_LAYERS = 2; // Total number of layers
    }

    // Define how layers interact
    // BroadPhaseLayer: A broadphase collision filter that is used to quickly rule out pairs of objects that should not collide.
    // ObjectLayer: A more fine-grained filter, determining which object layers collide.
    namespace BroadPhase
    {
        static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
        static constexpr JPH::BroadPhaseLayer MOVING(1);
        static constexpr JPH::BroadPhaseLayer NUM_LAYERS(2);
    }

    // Class that helps map JPH::ObjectLayer to JPH::BroadPhaseLayer
    // This is used by the broad phase to quickly rule out pairs that don't collide
    class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
    {
    public:
        BPLayerInterfaceImpl()
        {
            // Create a mapping from object layers to broad phase layers
            // Non-moving objects are only in the non-moving broad phase layer
            mObjectToBroadPhase[Object::NON_MOVING] = BroadPhase::NON_MOVING;
            // Moving objects are in the moving broad phase layer
            mObjectToBroadPhase[Object::MOVING] = BroadPhase::MOVING;
        }

        virtual JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inObjectLayer) const override
        {
            JPH_ASSERT(inObjectLayer < Object::NUM_LAYERS);
            return mObjectToBroadPhase[inObjectLayer];
        }

        virtual JPH::uint GetNumBroadPhaseLayers() const override
        {
            // --- CORRECTED LINE HERE ---
            return (JPH::uint)BroadPhase::NUM_LAYERS;
            // --- END CORRECTED LINE ---
        }

    #if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_ENABLE_DEBUG_RENDERER)
        virtual const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inBroadPhaseLayer) const override
        {
            switch ((JPH::uint)inBroadPhaseLayer)
            {
            case (JPH::uint)BroadPhase::NON_MOVING: return "NON_MOVING";
            case (JPH::uint)BroadPhase::MOVING:     return "MOVING";
            default:                                 JPH_ASSERT(false); return "UNKNOWN";
            }
        }
    #endif // JPH_EXTERNAL_PROFILE || JPH_ENABLE_DEBUG_RENDERER

    private:
        JPH::BroadPhaseLayer mObjectToBroadPhase[Object::NUM_LAYERS];
    };

    /// Class that determines if an object layer can collide with a broadphase layer
    class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
    {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
        {
            switch (inLayer1)
            {
            case Object::NON_MOVING:
                return inLayer2 == BroadPhase::MOVING; // Non-moving only collides with moving
            case Object::MOVING:
                return true; // Moving collides with everything
            default:
                JPH_ASSERT(false);
                return false;
            }
        }
    };

    /// Class that determines if two object layers can collide
    class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
    {
    public:
        virtual bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
        {
            switch (inObject1)
            {
            case Object::NON_MOVING:
                return inObject2 == Object::MOVING; // Non-moving only collides with moving
            case Object::MOVING:
                return true; // Moving collides with everything
            default:
                JPH_ASSERT(false);
                return false;
            }
        }
    };
} // namespace Layers

#endif // PHYSICS_LAYERS_H