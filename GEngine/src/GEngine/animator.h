#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <vector>
#include <memory>

#include "GEngine/mesh.h"
#include "glm/fwd.hpp"

namespace GEngine {

struct SKeyPosition {
  glm::vec3 position_;
  float time_stamp_;
};

struct SKeyRotation {
  glm::quat orientation_;
  float time_stamp_;
};

struct SKeyScale {
  glm::vec3 scale_;
  float time_stamp_;
};

class Bone {
  public:
  Bone() = default;
  ~Bone() = default;

  /* reads keyframes from aiNodeAnim */
  Bone(const std::string& name, int id, const aiNodeAnim *channel);

  /* interpolates  b/w positions,rotations & scaling keys based on the current
    time of the animation and prepares the local transformation matrix by
    combining all keys tranformations */
  void Update(float animationTime);

  glm::mat4 GetLocalTransform() { return local_transform_; }
  std::string GetBoneName() const { return name_; }
  int GetBoneID() { return id_; }

  /* Gets the current index to interpolate to based on the current animation
   * time */
  int GetScaleIndex(float animationTime);
  int GetPositionIndex(float animationTime);
  int GetRotationIndex(float animationTime);

  /* figures out which position keys to interpolate b/w and performs the interpolation and returns the translation matrix */
  glm::vec3 InterpolatePosition(float animationTime);
  glm::quat InterpolateRotation(float animationTime);
  glm::vec3 InterpolateScaling(float animationTime);

private:
  std::vector<SKeyPosition> positions_;
  std::vector<SKeyRotation> rotations_;
  std::vector<SKeyScale> scales_;
  int num_positions_;
  int num_rotations_;
  int num_scalings_;

  glm::mat4 local_transform_; // 相对于父关节的变换
  std::string name_;
  int id_;

  /* Gets normalized value for Lerp & Slerp */
  float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);

};

struct SAssimpNodeData
{
    glm::mat4 transformation_;
    std::string name_;
    int children_count_;
    std::vector<std::shared_ptr<SAssimpNodeData>> children_;
};

class CAnimation {
  public:
    CAnimation() = default;
    ~CAnimation() = default;

    CAnimation(const std::string& animationPath, std::shared_ptr<CMesh>& mesh, int animationID);

    inline int GetTicksPerSecond() { return ticks_per_second_; }
    inline float GetDuration() { return duration_; }
    inline const std::shared_ptr<SAssimpNodeData>& GetRootNode() { return root_node_; }
    inline const std::map<std::string, std::shared_ptr<SBoneInfo>>& GetBoneIDMap() { return bone_info_; }

    std::shared_ptr<Bone> FindBone(const std::string &name);

  private:
    void ReadMissingBones(const aiAnimation *animation, std::shared_ptr<CMesh> &mesh);
    void ReadHeirarchyData(std::shared_ptr<SAssimpNodeData>& dest, const aiNode *src);

    float duration_;
    int ticks_per_second_;
    std::vector<std::shared_ptr<Bone>> bones_;
    std::shared_ptr<SAssimpNodeData> root_node_ = std::make_shared<SAssimpNodeData>();
    std::map<std::string, std::shared_ptr<SBoneInfo>> bone_info_;
};

class CAnimator {
  public:
  CAnimator() = default;
  ~CAnimator() = default;

  CAnimator(std::shared_ptr<CAnimation> animation);

  void UpdateAnimation(float dt);
  std::vector<glm::mat4> GetFinalBoneMatrices() { return final_bone_matrices_; }
  void PlayAnimation(std::shared_ptr<CAnimation> animation);
  void CalculateBoneTransform(std::shared_ptr<SAssimpNodeData> node, glm::mat4 parent_transform);
  void PlayBlendedAnimation(std::shared_ptr<CAnimation> from_animation, std::shared_ptr<CAnimation> to_animation);
  void CalculateBlendedBoneTransform(std::shared_ptr<SAssimpNodeData> from_node, std::shared_ptr<SAssimpNodeData> to_node, glm::mat4 parentTransform);
  glm::mat4 GetBlendedLocalTransform(std::shared_ptr<GEngine::Bone> from_bone, std::shared_ptr<GEngine::Bone> to_bone, float from_current_time, float to_current_time);

private:
  std::vector<glm::mat4> final_bone_matrices_;
  std::shared_ptr<CAnimation> current_animation_;
  float current_time_;
  float delta_time_;

  // blended animation
  float blended_animation_total_time_ = 1800.0f;
  float animation_blend_current_time_ = 0.0f;
  bool playing_blended_animation_ = false;
  std::shared_ptr<CAnimation> from_animation_;
  std::shared_ptr<CAnimation> to_animation_;
  float animation_blend_factor_ = 0.0f;
  float from_current_time_ = 0.0f;
  float to_current_time_ = 0.0f;
};

} // namespace GEngine