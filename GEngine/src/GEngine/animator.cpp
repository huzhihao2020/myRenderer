#include "GEngine/animator.h"
#include "common.h"
#include "log.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>
#include <string>

#define MAX_TOTAL_BONE 200

/* Bone */
GEngine::Bone::Bone(const std::string &name, int id, const aiNodeAnim *channel)
    : name_(name), id_(id), local_transform_(glm::mat4(1.0f)) {
  // load positions
  num_positions_ = channel->mNumPositionKeys;
  for (int i = 0; i < num_positions_; ++i) {
    aiVector3D aiPosition = channel->mPositionKeys[i].mValue;
    float time_stamp_ = channel->mPositionKeys[i].mTime;
    SKeyPosition key_pos = {GEngine::Utils::GetGLMVec(aiPosition), time_stamp_};
    positions_.push_back(key_pos);
  }
  // load rotations
  num_rotations_ = channel->mNumRotationKeys;
  for (int i = 0; i < num_rotations_; ++i) {
    aiQuaternion aiOrientation = channel->mRotationKeys[i].mValue;
    float time_stamp_ = channel->mRotationKeys[i].mTime;
    // glm quaternion layout [w, x, y, z]
    SKeyRotation key_rotation = {GEngine::Utils::GetGLMQuat(aiOrientation), time_stamp_};
    rotations_.push_back(key_rotation);
  }
  // load scales
  num_scalings_ = channel->mNumScalingKeys;
  for (int i = 0; i < num_scalings_; ++i) {
    aiVector3D aiSacle = channel->mScalingKeys[i].mValue;
    float time_stamp_ = channel->mScalingKeys[i].mTime;
    SKeyScale key_scale = {GEngine::Utils::GetGLMVec(aiSacle), time_stamp_};
    scales_.push_back(key_scale);
  }
}

// interpolate the matrix between last keyframe and next keyframe
void GEngine::Bone::Update(float animationTime) {
  glm::mat4 translation = InterpolatePosition(animationTime);
  glm::mat4 rotation = InterpolateRotation(animationTime);
  glm::mat4 scale = InterpolateScaling(animationTime);
  local_transform_ = translation * rotation * scale;
}

/* |lastTimeStamp|============|animationTime|-----|nextTimeStamp|
 *               |<--midway-->|
 *               |<------------framesDiff-------->|
*/

float GEngine::Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp,
                                    float animationTime) {
  float scale_factor = 0.0f;
  float midWayLength = animationTime - lastTimeStamp;
  float framesDiff = nextTimeStamp - lastTimeStamp;
  scale_factor = midWayLength / framesDiff;
  assert(scale_factor >= 0.0f && scale_factor <= 1.0f);
  return scale_factor;
}

int GEngine::Bone::GetPositionIndex(float animationTime) {
  for (int index = 0; index < num_positions_ - 1; ++index) {
    if (animationTime < positions_[index + 1].time_stamp_)
      return index;
  }
  assert(0);
}

int GEngine::Bone::GetRotationIndex(float animationTime) {
  for (int index = 0; index < num_rotations_ - 1; ++index) {
    if (animationTime < rotations_[index + 1].time_stamp_)
      return index;
  }
  assert(0);
}

int GEngine::Bone::GetScaleIndex(float animationTime) {
  for (int index = 0; index < num_scalings_ - 1; ++index) {
    if (animationTime < scales_[index + 1].time_stamp_)
      return index;
  }
  assert(0);
}

glm::mat4 GEngine::Bone::InterpolatePosition(float animationTime) {
  if (num_positions_ == 1)
    return glm::translate(glm::mat4(1.0f), positions_[0].position_);

  int p0Index = GetPositionIndex(animationTime);
  int p1Index = p0Index + 1;
  float scaleFactor =
      GetScaleFactor(positions_[p0Index].time_stamp_,
                     positions_[p1Index].time_stamp_, animationTime);
  glm::vec3 finalPosition =
      glm::mix(positions_[p0Index].position_, positions_[p1Index].position_,
               scaleFactor);
  return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 GEngine::Bone::InterpolateRotation(float animationTime) {
  if (num_rotations_ == 1) {
    auto rotation = glm::normalize(rotations_[0].orientation_);
    return glm::toMat4(rotation);
  }

  int p0Index = GetRotationIndex(animationTime);
  int p1Index = p0Index + 1;
  float scaleFactor =
      GetScaleFactor(rotations_[p0Index].time_stamp_,
                     rotations_[p1Index].time_stamp_, animationTime);
  glm::quat finalRotation =
      glm::slerp(rotations_[p0Index].orientation_,
                 rotations_[p1Index].orientation_, scaleFactor);
  finalRotation = glm::normalize(finalRotation);
  return glm::toMat4(finalRotation);
}

glm::mat4 GEngine::Bone::InterpolateScaling(float animationTime) {
  if (num_scalings_ == 1)
    return glm::scale(glm::mat4(1.0f), scales_[0].scale_);

  int p0Index = GetScaleIndex(animationTime);
  int p1Index = p0Index + 1;
  float scaleFactor =
      GetScaleFactor(scales_[p0Index].time_stamp_, scales_[p1Index].time_stamp_,
                     animationTime);
  glm::vec3 finalScale =
      glm::mix(scales_[p0Index].scale_, scales_[p1Index].scale_, scaleFactor);
  return glm::scale(glm::mat4(1.0f), finalScale);
}

/* Animation */

/**
 * @brief Construct a new CAnimation object
 *
 * @param animationPath path to the animation file
 * @param mesh mesh for this animation
 */
GEngine::CAnimation::CAnimation(const std::string &animationPath,
                                std::shared_ptr<CMesh> &mesh, int animationID) {
  Assimp::Importer importer;
  auto flags = aiProcess_Triangulate
              | aiProcess_JoinIdenticalVertices;
  const aiScene *scene =
      importer.ReadFile(animationPath, flags);
  assert(scene && scene->mRootNode);
  // for(int i=0; i<scene->mNumAnimations; i++) {
  auto animation = scene->mAnimations[animationID];
  duration_ = animation->mDuration;
  ticks_per_second_ = animation->mTicksPerSecond;
  ReadHeirarchyData(root_node_, scene->mRootNode);
  ReadMissingBones(animation, mesh);
  // }
  
  if(bones_.size() > MAX_TOTAL_BONE) {
    GE_ERROR("Number of bones in model exceeds 200");
  }
}

/**
 * @brief some times bones may be missing in model file and the missing bones are in the animation file
 * 
 * @param animation 
 * @param mesh 
 */
void GEngine::CAnimation::ReadMissingBones(const aiAnimation *animation,
                                           std::shared_ptr<CMesh>& mesh) {
  int size = animation->mNumChannels;

  // getting bone info from Mesh class
  auto& boneInfoMap = mesh->GetBoneInfoMap(); 
  int boneCount = mesh->GetBoneCount();

  // reading channels(bones engaged in an animation and their keyframes)
  for (int i = 0; i < size; i++) {
    auto channel = animation->mChannels[i];
    std::string boneName = channel->mNodeName.data;

    if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
      auto new_bone_info = std::make_shared<SBoneInfo>();
      new_bone_info->id = boneCount;
      new_bone_info->inverse_bind_transform = glm::mat4(1.0f);
      boneInfoMap[boneName] = new_bone_info;
      boneCount++;
    }
    auto new_bone = std::make_shared<Bone>(
        channel->mNodeName.data, boneInfoMap[channel->mNodeName.data]->id,
        channel);
    bones_.push_back(new_bone);
  }

  bone_info_ = boneInfoMap;
}

void GEngine::CAnimation::ReadHeirarchyData(std::shared_ptr<SAssimpNodeData>& dest, const aiNode* src) {
  assert(src);
  
  dest->name_ = src->mName.data;
  // assimp aiMat4x4 stores row vector, but glm::mat4 stores col
  dest->transformation_ = GEngine::Utils::ConvertMatrixToGLMFormat(src->mTransformation);
  dest->children_count_ = src->mNumChildren;

  for (int i = 0; i < src->mNumChildren; i++) {
    auto newData = std::make_shared<SAssimpNodeData>();
    ReadHeirarchyData(newData, src->mChildren[i]);
    dest->children_.push_back(newData);
  }
}

GEngine::CAnimator::CAnimator(std::shared_ptr<CAnimation> animation) {

  current_time_ = 0.0f;
  current_animation_ = animation;
  final_bone_matrices_.reserve(MAX_TOTAL_BONE);

  for (int i = 0; i < MAX_TOTAL_BONE; i++)
    final_bone_matrices_.push_back(glm::mat4(1.0f));
}

void GEngine::CAnimator::UpdateAnimation(float dt) {
  delta_time_ = dt;
  if (current_animation_) {
    current_time_ += current_animation_->GetTicksPerSecond() * dt;
    current_time_ = fmod(current_time_, current_animation_->GetDuration());
    CalculateBoneTransform(current_animation_->GetRootNode(), glm::mat4(1.0f));
  }
}

void GEngine::CAnimator::PlayAnimation(std::shared_ptr<CAnimation> animation) {
  current_animation_ = animation;
  current_time_ = 0.0f;
}

void GEngine::CAnimator::CalculateBoneTransform(std::shared_ptr<SAssimpNodeData> node,
                                      glm::mat4 parentTransform) {
  std::string nodeName = node->name_;
  glm::mat4 nodeTransform = node->transformation_;

  auto bone_ptr = current_animation_->FindBone(nodeName);

  if (bone_ptr) {
    bone_ptr->Update(current_time_);
    nodeTransform = bone_ptr->GetLocalTransform();
  }

  glm::mat4 globalTransformation = parentTransform * nodeTransform;

  auto boneInfoMap = current_animation_->GetBoneIDMap();
  if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
    int index = boneInfoMap[nodeName]->id;
    glm::mat4 inverse_bind_transform = boneInfoMap[nodeName]->inverse_bind_transform;
    final_bone_matrices_[index] = globalTransformation * inverse_bind_transform;
  }

  for (int i = 0; i < node->children_count_; i++)
    CalculateBoneTransform(node->children_[i], globalTransformation);
}
