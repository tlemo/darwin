
#pragma once

#include "world.h"

#include <core/sim/camera.h>
#include <third_party/box2d/box2d.h>

#include <vector>
#include <memory>
#include <random>
using namespace std;

namespace seg_tree {

struct Segment;
class Phenotype;
class World;

struct Slice {
  // width relative to the sibling slices
  double relative_width = 1.0;

  // optional appendage attached to the slice's end
  Segment* appendage = nullptr;
};

struct Segment {
  double length = 0.3;
  double width = 0.2;
  bool suppressed = false;
  Segment* side_appendage = nullptr;
  vector<Slice> slices = { Slice() };

  Segment() = default;

  Segment(double length, double width, Segment* appendage = nullptr)
      : length(length), width(width), slices({ { 1.0, appendage } }) {}
};

class Genotype {
 public:
  Genotype();

  Genotype(const Genotype& other);
  Genotype& operator=(const Genotype& other);

  friend void swap(Genotype& a, Genotype& b) noexcept;

  void mutate();

  Segment* root() const { return root_; }

  template <class... Args>
  Segment* newSegment(Args&&... args) {
    const auto segment = new Segment(std::forward<Args>(args)...);
    segments_.emplace_back(segment);
    return segment;
  }

  // mutations
  void growAppendage(Segment* segment);
  void growSideAppendage(Segment* segment);
  void lateralSplit(Segment* segment, double fraction);
  void axialSplit(Segment* segment, double fraction);
  void mutateLength(Segment* segment, double std_dev);
  void mutateWidth(Segment* segment, double std_dev);
  void mutateSliceWidth(Segment* segment, double std_dev);

 private:
  // deep copy a DAG of segments
  // source segment may be from a different Genotype instance
  Segment* deepCopy(const Segment* segment);

 private:
  // the Genotype owns all the Segment instances
  vector<unique_ptr<Segment>> segments_;

  Segment* root_ = nullptr;
};

class Organism {
  static constexpr float kPhaseVelocity = b2_pi / 64;
  static constexpr float kPhaseLag = b2_pi / 4;
  static constexpr float kJointSpeed = 0.15f;
  static constexpr float kJointResetSpeed = 0.05f;

  struct Joint {
    b2RevoluteJoint* box2d_joint = nullptr;
    bool mirror = false;
    vector<Joint> children;
  };

 public:
  Organism(World* world, const b2Vec2& pos, float angle, const Genotype& parent_genotype);

  // returns `true` if still alive
  bool simStep(float dt);

 private:
  Joint createSegment(const Segment* segment,
                      b2Body* parent_body,
                      const b2Vec2& base_left,
                      const b2Vec2& base_right,
                      bool mirror);

  void animateJoint(const Joint& joint, float phase);

  void reproduce();
  void die();

 private:
  Joint root_;
  float current_phase_ = 0;

  World* world_ = nullptr;

  vector<b2Body*> body_parts_;

  unique_ptr<sim::Camera> camera_;

  Genotype genotype_;

  float age_ = 0;
  float gestation_time_ = 0;
  float health_ = 0;
  bool alive_ = false;  // TODO: remove?

  std::default_random_engine rnd_{ std::random_device{}() };
};

class World : public ::World {
  static constexpr float kWidth = 200;
  static constexpr float kHeight = 100;

 public:
  World();

  void newOrganism(const b2Vec2& pos, float angle, const Genotype& parent_genotype);
  void newFood(const b2Vec2& pos);

 private:
  void postStep(float dt) override;

 private:
  vector<Organism> organisms_;
};

}  // namespace seg_tree
