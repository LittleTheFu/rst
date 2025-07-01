#include "Animator.h"
#include <iostream>

// Assimp 辅助函数，将 aiVector3D 转换为 Eigen::Vector3f
// (已在 AssimpUtils.h/cpp 中定义)
// Eigen::Vector3f ConvertAssimpVec3ToEigen(const aiVector3D& vec) {
//     return Eigen::Vector3f(vec.x, vec.y, vec.z);
// }

// Assimp 辅助函数，将 aiQuaternion 转换为 Eigen::Quaternionf
// (已在 AssimpUtils.h/cpp 中定义)
// Eigen::Quaternionf ConvertAssimpQuatToEigen(const aiQuaternion& quat) {
//     return Eigen::Quaternionf(quat.w, quat.x, quat.y, quat.z);
// }


Animator::Animator() {
    // 初始化 finalBoneMatrices_ 数组
    finalBoneMatrices_.resize(MAX_BONES);
    for (int i = 0; i < MAX_BONES; ++i) {
        finalBoneMatrices_[i] = Eigen::Matrix4f::Identity();
    }
}

Animator::~Animator() {
    // 智能指针会自动管理 animations_ 中的 Animation 对象
    // bones_ 是一个vector，存储的是实际的Bone对象，会随Animator析构而析构
    // rootBone_ 和 children 都是指向 bones_ 内部元素的指针，无需手动删除
}

void Animator::Init(const aiScene* scene, std::map<std::string, int>& boneInfoMap) {
    if (!scene || !scene->mRootNode) {
        std::cerr << "ERROR::ANIMATOR::Init: Invalid Assimp scene or root node." << std::endl;
        return;
    }

    this->boneInfoMap_ = boneInfoMap; // 接收从Model中收集的骨骼信息

    // 清空旧数据，准备重新填充
    bones_.clear();
    rootBone_ = nullptr;
    animations_.clear();
    
    // Assimp骨骼名称到我们Bone对象的映射 (构建层次时临时使用)
    std::map<std::string, Bone*> boneNameToBonePtr;

    // --- 修改点：计算最大骨骼ID来正确地resize bones_ 向量 ---
    int maxBoneId = -1;
    if (!boneInfoMap_.empty()) {
        for (const auto& pair : boneInfoMap_) {
            if (pair.second > maxBoneId) {
                maxBoneId = pair.second;
            }
        }
        // 根据最大 ID + 1 来resize，保证所有ID都能被索引
        bones_.resize(maxBoneId + 1); 
    } else {
        // 如果没有骨骼信息，不需要resize，bones_ 保持为空
        bones_.clear();
    }
    // --- 结束修改点 ---

    // 填充 bones_ 向量并建立 boneNameToBonePtr 映射
    for (const auto& pair : boneInfoMap_) {
        // pair.first 是骨骼名称, pair.second 是骨骼ID
        // 默认的 inverseBindPoseTransform 可以是单位矩阵，之后在 BuildBoneHierarchy 中从 aiBone 填充
        bones_[pair.second] = Bone(pair.first, pair.second, Eigen::Matrix4f::Identity());
        boneNameToBonePtr[pair.first] = &bones_[pair.second];
    }
    
    // 寻找实际的根骨骼节点
    // 这里的逻辑可能需要根据实际FBX文件结构调整
    aiNode* assimpRootBoneNode = scene->mRootNode;
    // 这是一个简化逻辑，假设骨架根节点是场景根节点的子节点之一
    // 实际项目中可能需要更复杂的遍历来找到真正的骨架根
    // 如果 Assimp 场景根节点不是骨骼，则尝试查找其子节点
    if (boneInfoMap.find(assimpRootBoneNode->mName.C_Str()) == boneInfoMap.end() && assimpRootBoneNode->mNumChildren > 0) {
        // 简单地假设第一个子节点可能是骨架的根 (这可能需要更智能的检测)
        assimpRootBoneNode = assimpRootBoneNode->mChildren[0];
        // 实际游戏中，会遍历查找mRootNode的子节点，直到找到与某个aiBoneName匹配的aiNode
    }


    // 递归构建骨骼层次 (父子关系)，并填充 inverseBindPoseTransform
    // 初始调用时，parentBone 为 nullptr
    BuildBoneHierarchy(assimpRootBoneNode, nullptr, scene); 

    // 确保 rootBone_ 被正确设置 (通常就是 boneNameToBonePtr 中最顶层的骨骼)
    // 如果 Assimp 场景根节点是骨骼，那么它就是 rootBone_
    // 否则，需要找到第一个被识别为骨骼的节点作为 rootBone_
    if (!boneNameToBonePtr.empty()) {
        // 再次遍历 Assimp 场景节点，找到真正的根骨骼
        // 这里的逻辑可能需要根据实际FBX文件结构调整
        std::function<void(aiNode*, Bone*)> findRoot = 
            [&](aiNode* node, Bone* currentBone) {
            if (currentBone) {
                if (!currentBone->parent) { // 找到了没有父骨骼的Bone，这就是根
                    rootBone_ = currentBone;
                    return;
                }
            }
            for (unsigned int i = 0; i < node->mNumChildren; ++i) {
                std::string childName = node->mChildren[i]->mName.C_Str();
                auto it = boneNameToBonePtr.find(childName);
                if (it != boneNameToBonePtr.end()) {
                    findRoot(node->mChildren[i], it->second);
                } else {
                    findRoot(node->mChildren[i], nullptr); // 继续遍历非骨骼子节点
                }
            }
        };
        findRoot(scene->mRootNode, nullptr);
    } else {
        rootBone_ = nullptr; // 如果没有骨骼，根骨骼为nullptr
    }

    // 2. 加载动画数据
    LoadAnimations(scene);

    // 初始化 finalBoneMatrices_
    // 这一部分保持不变，因为 MAX_BONES 是固定的最大值
    for (int i = 0; i < MAX_BONES; ++i) {
        finalBoneMatrices_[i] = Eigen::Matrix4f::Identity();
    }
}

void Animator::BuildBoneHierarchy(aiNode* assimpNode, Bone* parentBone, const aiScene* scene) {
    std::string nodeName = assimpNode->mName.C_Str();
    Bone* currentBone = nullptr;

    // 检查这个Assimp节点是否是我们的骨骼 (在 boneInfoMap 中存在)
    auto it = boneInfoMap_.find(nodeName);
    if (it != boneInfoMap_.end()) {
        int boneId = it->second;
        currentBone = &bones_[boneId]; // 获取到我们自己的Bone对象

        // 设置父子关系
        if (parentBone) {
            currentBone->parent = parentBone;
            parentBone->children.push_back(currentBone);
        }

        // 填充 inverseBindPoseTransform (从aiMesh::mBones 获取)
        // 注意：aiBone 附加在 aiMesh 上，所以我们需要遍历所有 Mesh 才能找到
        // 对应的 aiBone::mOffsetMatrix。这个逻辑通常在 Model::processMesh 中完成
        // 并将 BoneID -> OffsetMatrix 的映射传递给 Animator
        // 这里为了简化，我们假设 currentBone->inverseBindPoseTransform 已经在外部（Model::processMesh）被设置了
        // 或者我们在这里查找所有mesh的aiBone来填充
        
        // 为了确保 inverseBindPoseTransform 被正确设置，
        // 最好在 Model::processMesh 中，当发现一个 aiBone 时，
        // 就将其 mOffsetMatrix 存储到 boneInfoMap 的某个结构中，
        // 或者直接在这里遍历 scene->mMeshes 来查找
        
        // 临时查找 aiBone::mOffsetMatrix 的逻辑 (非最佳实践，但演示用)
        for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[i];
            if (mesh->HasBones()) {
                for (unsigned int j = 0; j < mesh->mNumBones; ++j) {
                    aiBone* bone = mesh->mBones[j];
                    if (std::string(bone->mName.C_Str()) == nodeName) {
                        currentBone->inverseBindPoseTransform = AssimpUtils::ConvertAssimpMat4ToEigen(bone->mOffsetMatrix);
                        break; // 找到就跳出
                    }
                }
            }
        }

        // 设置根骨骼 (如果尚未设置，且当前骨骼没有父骨骼)
        if (!rootBone_ && !parentBone) {
            rootBone_ = currentBone;
        }
    }

    // 递归处理子节点
    for (unsigned int i = 0; i < assimpNode->mNumChildren; ++i) {
        // 如果当前 Assimp 节点是骨骼，则它的子节点会以它为父骨骼。
        // 否则，子节点继续以父骨骼的父骨骼为父骨骼（即跳过非骨骼节点）
        BuildBoneHierarchy(assimpNode->mChildren[i], currentBone ? currentBone : parentBone, scene);
    }
}


void Animator::LoadAnimations(const aiScene* scene) {
    animations_.clear();
    for (unsigned int i = 0; i < scene->mNumAnimations; ++i) {
        aiAnimation* aiAnim = scene->mAnimations[i];
        std::string animName = aiAnim->mName.C_Str();
        if (animName.empty()) {
            animName = "Animation_" + std::to_string(i); // 如果没有名字，给一个默认名
        }

        auto animation = std::make_unique<Animation>(animName, aiAnim->mDuration, aiAnim->mTicksPerSecond);

        for (unsigned int j = 0; j < aiAnim->mNumChannels; ++j) {
            aiNodeAnim* aiNodeAnim = aiAnim->mChannels[j];
            std::string boneName = aiNodeAnim->mNodeName.C_Str();

            BoneNodeAnim boneAnim;
            boneAnim.boneName = boneName;

            // 位置关键帧
            for (unsigned int k = 0; k < aiNodeAnim->mNumPositionKeys; ++k) {
                PositionKey key;
                key.position = AssimpUtils::ConvertAssimpVec3ToEigen(aiNodeAnim->mPositionKeys[k].mValue);
                key.time = aiNodeAnim->mPositionKeys[k].mTime;
                boneAnim.positionKeys.push_back(key);
            }

            // 旋转关键帧
            for (unsigned int k = 0; k < aiNodeAnim->mNumRotationKeys; ++k) {
                RotationKey key;
                key.rotation = AssimpUtils::ConvertAssimpQuatToEigen(aiNodeAnim->mRotationKeys[k].mValue);
                key.time = aiNodeAnim->mRotationKeys[k].mTime;
                boneAnim.rotationKeys.push_back(key);
            }

            // 缩放关键帧
            for (unsigned int k = 0; k < aiNodeAnim->mNumScalingKeys; ++k) {
                ScalingKey key;
                key.scale = AssimpUtils::ConvertAssimpVec3ToEigen(aiNodeAnim->mScalingKeys[k].mValue);
                key.time = aiNodeAnim->mScalingKeys[k].mTime;
                boneAnim.scalingKeys.push_back(key);
            }
            animation->boneAnimations[boneName] = boneAnim; // 存储到map中
        }
        animations_[animName] = std::move(animation); // 将unique_ptr存储到map中
    }

    // 默认播放第一个动画 (如果有的话)
    if (!animations_.empty()) {
        currentAnimation_ = animations_.begin()->second.get();
        currentTime_ = 0.0f;
    }
}

void Animator::UpdateAnimation(float deltaTime) {
    if (currentAnimation_) {
        // 计算当前动画时间（以 ticks 为单位）
        float tickPerSecond = (float)currentAnimation_->ticksPerSecond;
        if (tickPerSecond == 0) tickPerSecond = 25.0f; // 默认值，防止除以零
        
        currentTime_ += deltaTime * animationPlaySpeed_ * tickPerSecond;
        currentTime_ = fmod(currentTime_, currentAnimation_->duration); // 循环播放

        // 更新骨骼的局部变换和全局变换
        // 遍历所有骨骼，根据当前动画时间计算其局部变换
        for (Bone& bone : bones_) {
            BoneNodeAnim* boneAnim = currentAnimation_->FindBoneNodeAnim(bone.name);
            if (boneAnim) {
                // 计算当前骨骼在动画时间下的插值变换
                CalculateBoneTransform(boneAnim, currentTime_, &bone);
            } else {
                // 如果当前骨骼没有动画数据，则保持其局部变换为单位矩阵 (或绑定姿态的局部变换)
                // Assimp的aiNode::mTransformation是绑定姿态的局部变换
                // 在BuildBoneHierarchy时，我们需要确保非动画节点的localTransform正确初始化
                // 暂时简单设置为单位矩阵，这意味着这些骨骼不会动。
                bone.localTransform = Eigen::Matrix4f::Identity(); 
            }
        }
        
        // 从根骨骼开始递归更新所有骨骼的全局变换
        if (rootBone_) {
            rootBone_->UpdateGlobalTransform();
        }

        // 计算最终的蒙皮矩阵
        for (int i = 0; i < bones_.size(); ++i) {
            // 最终矩阵 = 骨骼的全局变换 * 骨骼的逆绑定姿态变换
            finalBoneMatrices_[bones_[i].id] = bones_[i].globalTransform * bones_[i].inverseBindPoseTransform;
        }
    }
}

void Animator::PlayAnimation(const std::string& animationName) {
    auto it = animations_.find(animationName);
    if (it != animations_.end()) {
        currentAnimation_ = it->second.get();
        currentTime_ = 0.0f; // 从动画开始播放
        std::cout << "Playing animation: " << animationName << std::endl;
    } else {
        std::cerr << "WARNING::ANIMATOR::PlayAnimation: Animation '" << animationName << "' not found." << std::endl;
    }
}

void Animator::StopAnimation() {
    currentAnimation_ = nullptr;
    currentTime_ = 0.0f;
    // 重置所有骨骼到绑定姿态（可选，但通常需要）
    for (int i = 0; i < MAX_BONES; ++i) {
        finalBoneMatrices_[i] = Eigen::Matrix4f::Identity(); // 或者恢复到绑定姿态矩阵
    }
    std::cout << "Animation stopped." << std::endl;
}

void Animator::CalculateBoneTransform(BoneNodeAnim* boneAnim, float animationTime, Bone* bone) {
    // 1. 位置插值
    Eigen::Vector3f animatedPosition = InterpolatePosition(boneAnim, animationTime);
    Eigen::Affine3f translation = Eigen::Affine3f(Eigen::Translation3f(animatedPosition));
    Eigen::Matrix4f posMat = translation.matrix();


    // 2. 旋转插值
    Eigen::Quaternionf animatedRotation = InterpolateRotation(boneAnim, animationTime);
    Eigen::Matrix3f rotation3x3 = animatedRotation.toRotationMatrix(); // 先接收为3x3矩阵
    Eigen::Matrix4f rotMat = Eigen::Matrix4f::Identity(); // 创建一个4x4单位矩阵
    rotMat.block<3, 3>(0, 0) = rotation3x3; // 将3x3旋转矩阵赋值给4x4矩阵的左上角3x3块

    // 3. 缩放插值
    Eigen::Vector3f animatedScale = InterpolateScaling(boneAnim, animationTime);
    Eigen::Matrix4f scaleMat = Eigen::Affine3f(Eigen::Scaling(animatedScale)).matrix();

    // 组合局部变换矩阵： 缩放 -> 旋转 -> 位移
    bone->localTransform = posMat * rotMat * scaleMat;
}


Eigen::Vector3f Animator::InterpolatePosition(const BoneNodeAnim* nodeAnim, float animationTime) {
    if (nodeAnim->positionKeys.size() == 1) {
        return nodeAnim->positionKeys[0].position;
    }

    size_t p0Index = nodeAnim->GetPositionIndex(animationTime);
    size_t p1Index = p0Index + 1;

    float t0 = static_cast<float>(nodeAnim->positionKeys[p0Index].time); // 确保类型一致
    float t1 = static_cast<float>(nodeAnim->positionKeys[p1Index].time); // 确保类型一致
    float factor = (animationTime - t0) / (t1 - t0); // 插值因子

    // 替换为手动实现的线性插值
    // result = (1 - factor) * P0 + factor * P1
    return (1.0f - factor) * nodeAnim->positionKeys[p0Index].position + factor * nodeAnim->positionKeys[p1Index].position;
}

Eigen::Quaternionf Animator::InterpolateRotation(const BoneNodeAnim* nodeAnim, float animationTime) {
    if (nodeAnim->rotationKeys.size() == 1) {
        return nodeAnim->rotationKeys[0].rotation;
    }

    size_t r0Index = nodeAnim->GetRotationIndex(animationTime);
    size_t r1Index = r0Index + 1;

    float t0 = nodeAnim->rotationKeys[r0Index].time;
    float t1 = nodeAnim->rotationKeys[r1Index].time;
    float factor = (animationTime - t0) / (t1 - t0); // 插值因子

    Eigen::Quaternionf finalRot = nodeAnim->rotationKeys[r0Index].rotation.slerp(factor, nodeAnim->rotationKeys[r1Index].rotation);
    finalRot.normalize(); // 确保归一化
    return finalRot;
}

Eigen::Vector3f Animator::InterpolateScaling(const BoneNodeAnim* nodeAnim, float animationTime) {
    if (nodeAnim->scalingKeys.size() == 1) {
        return nodeAnim->scalingKeys[0].scale;
    }

    size_t s0Index = nodeAnim->GetScalingIndex(animationTime);
    size_t s1Index = s0Index + 1;

    float t0 = static_cast<float>(nodeAnim->scalingKeys[s0Index].time); // 确保类型一致
    float t1 = static_cast<float>(nodeAnim->scalingKeys[s1Index].time); // 确保类型一致
    float factor = (animationTime - t0) / (t1 - t0); // 插值因子

    // 替换为手动实现的线性插值
    // result = (1 - factor) * S0 + factor * S1
    return (1.0f - factor) * nodeAnim->scalingKeys[s0Index].scale + factor * nodeAnim->scalingKeys[s1Index].scale;
}