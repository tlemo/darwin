
#include "seg_tree_world.h"

#include <core/math_2d.h>
#include <core/random.h>
#include <core/exception.h>
#include <core/parallel_for_each.h>
#include <core/random.h>

#include <utility>
#include <cmath>
#include <limits>
#include <array>
using namespace std;

namespace seg_tree {

Genotype::Genotype() {
  root_ = newSegment();
}

Genotype::Genotype(const Genotype& other) {
  root_ = deepCopy(other.root_);
  CHECK(segments_.size() == other.segments_.size());
}

Genotype& Genotype::operator=(const Genotype& other) {
  Genotype copy(other);
  swap(*this, copy);
  return *this;
}

void swap(Genotype& a, Genotype& b) noexcept {
  using std::swap;
  swap(a.segments_, b.segments_);
  swap(a.root_, b.root_);
}

void Genotype::mutate() {
  const auto segment = core::randomElem(segments_)->get();

  struct MutationType {
    double weight = 0;
    function<void()> mutate;
  };

  vector<MutationType> mutagen = {
    { 10, [&] { axialSplit(segment, 0.5); } },
    { 20, [&] { axialSplit(segment, 1.0); } },
    { 50, [&] { lateralSplit(segment, 0.5); } },
    { 50, [&] { lateralSplit(segment, 1.0); } },
    { 5, [&] { segment->suppressed = !segment->suppressed; } },
    { 50, [&] { growAppendage(segment); } },
    { 25, [&] { growSideAppendage(segment); } },
    { 100, [&] { mutateLength(segment, 0.5); } },
    { 100, [&] { mutateWidth(segment, 0.5); } },
    { 50, [&] { mutateSliceWidth(segment, 0.5); } },
  };

  core::randomWeightedElem(mutagen)->mutate();
}

Segment* Genotype::deepCopy(const Segment* segment) {
  if (segment == nullptr) {
    return nullptr;
  }

  unordered_map<const Segment*, Segment*> orig_to_clone;

  // do a DFS and a shallow copy of all reachable segments
  vector<const Segment*> stack = { segment };
  while (!stack.empty()) {
    const auto s = stack.back();
    stack.pop_back();
    if (orig_to_clone.find(s) == orig_to_clone.end()) {
      orig_to_clone.insert({ s, newSegment(*s) });

      if (s->side_appendage != nullptr) {
        stack.push_back(s->side_appendage);
      }

      for (const auto& slice : s->slices) {
        if (slice.appendage != nullptr) {
          stack.push_back(slice.appendage);
        }
      }
    }
  }

  // fixup references
  for (auto kv : orig_to_clone) {
    const auto clone = kv.second;
    if (clone->side_appendage != nullptr) {
      clone->side_appendage = orig_to_clone.at(clone->side_appendage);
    }
    for (auto& slice : clone->slices) {
      if (slice.appendage != nullptr) {
        slice.appendage = orig_to_clone.at(slice.appendage);
      }
    }
  }

  return orig_to_clone.at(segment);
}

void Genotype::growAppendage(Segment* segment) {
  auto& slice = *core::randomElem(segment->slices);
  slice.appendage = newSegment(1.0, 1.0, slice.appendage);
}

void Genotype::growSideAppendage(Segment* segment) {
  segment->side_appendage = newSegment(1.0, 1.0, segment->side_appendage);
}

// pick a random slice and split it
// (replacing it with two slices with width = old_width * fraction)
void Genotype::lateralSplit(Segment* segment, double fraction) {
  CHECK(fraction > 0);

  const auto slice_it = core::randomElem(segment->slices);
  const auto new_width = slice_it->relative_width * fraction;

  slice_it->relative_width = new_width;

  Slice new_slice;
  new_slice.relative_width = new_width;
  new_slice.appendage = deepCopy(slice_it->appendage);

  segment->slices.insert(slice_it, new_slice);
}

// split the segment into two segments, chained together
void Genotype::axialSplit(Segment* segment, double fraction) {
  CHECK(fraction > 0);
  const auto new_length = segment->length * fraction;
  for (auto& slice : segment->slices) {
    slice.appendage = newSegment(new_length, segment->width, slice.appendage);
  }
  if (segment->slices.size() == 1) {
    segment->slices.front().appendage->side_appendage = deepCopy(segment->side_appendage);
  }
  segment->length = new_length;
}

void Genotype::mutateLength(Segment* segment, double std_dev) {
  segment->length =
      core::clampValue(core::mutateNormalValue(segment->length, std_dev), 0.01, 1000.0);
}

void Genotype::mutateWidth(Segment* segment, double std_dev) {
  segment->width =
      core::clampValue(core::mutateNormalValue(segment->width, std_dev), 0.01, 1000.0);
}

void Genotype::mutateSliceWidth(Segment* segment, double std_dev) {
  auto& slice = *core::randomElem(segment->slices);
  slice.relative_width = core::clampValue(
      core::mutateNormalValue(slice.relative_width, std_dev), 0.01, 100.0);
}

Organism::Organism(World* world,
                   const b2Vec2& pos,
                   float angle,
                   const Genotype& parent_genotype)
    : world_(world), genotype_(parent_genotype), birth_pos_(pos), birth_angle_(angle) {
  CHECK(world_);
  genotype_.mutate();
  alive_ = true;
}

bool Organism::simStep(float dt) {
  CHECK(alive_);
  CHECK(dt > 0);
  age_ += dt;

  // for testing only
  if (camera_) {
    const auto vision = camera_->render();
  }

#if 0
  if (health_ < 0 || age_ > 5.0) {
    return false;
  }

  gestation_time_ += dt;
  if (gestation_time_ > 3.0) {
    bernoulli_distribution dist(0.05);
    if (dist(rnd_)) {
      gestation_time_ = 0;
      reproduce();
    }
  }
#endif

  animateJoint(root_, current_phase_);
  current_phase_ += kPhaseVelocity;
  return true;
}

void Organism::developBody() {
  // develop the organism
  // (this may throw if the genotype doesn't result in a viable phenotype)
  constexpr float kTipSize = 0.1f;
  const float dx = cosf(birth_angle_) * kTipSize;
  const float dy = sinf(birth_angle_) * kTipSize;
  root_ = createSegment(genotype_.root(),
                        nullptr,
                        birth_pos_ - b2Vec2(dx, dy),
                        birth_pos_ + b2Vec2(dx, dy),
                        false);
  if (body_parts_.empty()) {
    throw core::Exception("Organism not viable");
  }

  const auto head = body_parts_[0];
  camera_ = make_unique<sim::Camera>(head, 90, 0.1f, 10.0f, 3);
}

Organism::Joint Organism::createSegment(const Segment* segment,
                                        b2Body* parent_body,
                                        const b2Vec2& base_left,
                                        const b2Vec2& base_right,
                                        bool mirror) {
  using std::swap;

  CHECK(segment != nullptr);
  CHECK(!segment->slices.empty());

  const auto b2_world = world_->box2dWorld();

  Joint joint;
  joint.mirror = mirror;

  if (segment->suppressed) {
    return joint;
  }

  const auto d = base_right - base_left;

  // body
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = (base_left + base_right) * 0.5f;
  body_def.angle = atan2f(d.y, d.x);
  b2Body* body = b2_world->CreateBody(&body_def);
  body_parts_.push_back(body);

  const float scale = parent_body ? d.Length() : 1.0f;
  const float base_width = parent_body ? 1.0f * scale : 0.0f;
  const float width = segment->width * scale;
  const float length = segment->length * scale;

  float total_rel_width = 0;
  for (const auto& slice : segment->slices) {
    total_rel_width += slice.relative_width;
  }
  CHECK(total_rel_width > 0);

  // calculate base/extremity points
  float base_x = -(base_width / 2);
  float extremity_x = -(width / 2);
  vector<b2Vec2> base_points = { b2Vec2(base_x, 0) };
  vector<b2Vec2> extremity_points = { b2Vec2(extremity_x, length) };
  for (const auto& slice : segment->slices) {
    const auto fraction = slice.relative_width / total_rel_width;
    base_x += fraction * base_width;
    extremity_x += fraction * width;
    base_points.emplace_back(base_x, 0);
    extremity_points.emplace_back(extremity_x, length);
  }

  // adjust extremity points:
  // 1. fixed length from the base counterpart
  // 2. y-coordinate mirroring
  for (size_t i = 0; i < extremity_points.size(); ++i) {
    const auto& bp = base_points[i];
    auto& ep = extremity_points[i];
    ep = (ep - bp).Normalized() * length + bp;
    ep.y = -ep.y;
  }

  // mirroring x coordinates?
  if (mirror) {
    for (auto& point : base_points) {
      point.x = -point.x;
    }
    for (auto& point : extremity_points) {
      point.x = -point.x;
    }
  }

  // create fixtures for each of the slices
  CHECK(segment->slices.size() + 1 == base_points.size());
  CHECK(segment->slices.size() + 1 == extremity_points.size());
  for (size_t i = 0; i < segment->slices.size(); ++i) {
    array<b2Vec2, 4> points;
    points[0] = base_points[i];
    points[1] = extremity_points[i];
    points[2] = extremity_points[i + 1];
    points[3] = base_points[i + 1];

    b2PolygonShape shape;
    shape.Set(points.data(), points.size());
    shape.m_radius = 0;

    b2FixtureDef fixture_def;
    fixture_def.shape = &shape;
    fixture_def.density = 1.0f;
    fixture_def.material.color = b2Color(0.8f, 0.5f, 0.5f);
    fixture_def.material.emit_intensity = 0.5f;
    body->CreateFixture(&fixture_def);
  }

  // create appendages
  for (size_t i = 0; i < segment->slices.size(); ++i) {
    const auto& slice = segment->slices[i];
    if (slice.appendage) {
      auto left = body->GetWorldPoint(extremity_points[i]);
      auto right = body->GetWorldPoint(extremity_points[i + 1]);
      if (mirror) {
        swap(left, right);
      }
      joint.children.push_back(createSegment(slice.appendage, body, left, right, mirror));
    }
  }

  // side appendage, if any
  if (segment->side_appendage) {
    // left appendage
    auto la_left = body->GetWorldPoint(base_points.front());
    auto la_right = body->GetWorldPoint(extremity_points.front());
    if (mirror) {
      swap(la_left, la_right);
    }
    joint.children.push_back(
        createSegment(segment->side_appendage, body, la_left, la_right, mirror));

    // right appendage (mirror)
    auto ra_left = body->GetWorldPoint(extremity_points.back());
    auto ra_right = body->GetWorldPoint(base_points.back());
    if (mirror) {
      swap(ra_left, ra_right);
    }
    joint.children.push_back(
        createSegment(segment->side_appendage, body, ra_left, ra_right, !mirror));
  }

  // parent joint
  if (parent_body) {
    b2RevoluteJointDef hinge_def;
    hinge_def.bodyA = parent_body;
    hinge_def.bodyB = body;
    hinge_def.localAnchorA = parent_body->GetLocalPoint(body_def.position);
    hinge_def.localAnchorB.Set(0, 0);
    hinge_def.lowerAngle = -0.5f * b2_pi;
    hinge_def.upperAngle = 0.5f * b2_pi;
    hinge_def.referenceAngle = body->GetAngle() - parent_body->GetAngle();
    hinge_def.enableLimit = true;
    hinge_def.maxMotorTorque = 25.0f;
    hinge_def.motorSpeed = 0;
    hinge_def.enableMotor = true;
    joint.box2d_joint = static_cast<b2RevoluteJoint*>(b2_world->CreateJoint(&hinge_def));
  }

  return joint;
}

void Organism::animateJoint(const Organism::Joint& joint, float phase) {
  if (const auto b2_joint = joint.box2d_joint) {
    auto target_speed = cos(phase) * kJointSpeed * (joint.mirror ? -1 : 1);
    if (b2_joint->GetJointAngle() > 0) {
      target_speed -= kJointResetSpeed;
    } else {
      target_speed += kJointResetSpeed;
    }
    b2_joint->SetMotorSpeed(target_speed);
  }

  for (const auto& child_joint : joint.children) {
    animateJoint(child_joint, phase + kPhaseLag);
  }
}

void Organism::reproduce() {
  CHECK(!body_parts_.empty());
  const auto ref_body = body_parts_.front();
  const auto child_pos = ref_body->GetPosition();
  const auto child_angle = ref_body->GetAngle();
  world_->newOrganism(child_pos, child_angle, genotype_);
}

void Organism::die() {
  CHECK(alive_, "Already dead");
  alive_ = false;
  const auto b2_world = world_->box2dWorld();
  for (auto body_part : body_parts_) {
    CHECK(body_part);
    b2_world->DestroyBody(body_part);
  }
}

World::World() : ::World(sim::Rect(-kWidth / 2, -kHeight / 2, kWidth, kHeight)) {
  // walls
  b2BodyDef walls_def;
  auto walls = world_.CreateBody(&walls_def);

  b2EdgeShape wall_shape;
  b2FixtureDef wall_fixture_def;
  wall_fixture_def.shape = &wall_shape;
  wall_fixture_def.friction = 10.0f;
  wall_fixture_def.restitution = 0.8f;
  wall_fixture_def.material.color = b2Color(0.2, 0.5, 0.1);
  wall_fixture_def.material.emit_intensity = 0.1f;

  const b2Vec2 top_left(-kWidth / 2, kHeight / 2);
  const b2Vec2 top_right(kWidth / 2, kHeight / 2);
  const b2Vec2 bottom_left(-kWidth / 2, -kHeight / 2);
  const b2Vec2 bottom_right(kWidth / 2, -kHeight / 2);

  wall_shape.Set(bottom_left, bottom_right);
  walls->CreateFixture(&wall_fixture_def);
  wall_shape.Set(bottom_left, top_left);
  walls->CreateFixture(&wall_fixture_def);
  wall_shape.Set(bottom_right, top_right);
  walls->CreateFixture(&wall_fixture_def);
  wall_shape.Set(top_left, top_right);
  walls->CreateFixture(&wall_fixture_def);

  std::default_random_engine rnd(core::randomSeed());
  uniform_real_distribution<float> dist_x(-kWidth / 2, kWidth / 2);
  uniform_real_distribution<float> dist_y(-kHeight / 2, kHeight / 2);
  uniform_real_distribution<float> dist_angle(0, 2 * math::kPi);

  for (int i = 0; i < 5000; ++i) {
    const auto pos = b2Vec2(dist_x(rnd), dist_y(rnd));
    newFood(pos);
  }

  for (int i = 0; i < 2500; ++i) {
    const auto pos = b2Vec2(dist_x(rnd), dist_y(rnd));
    const auto angle = dist_angle(rnd);
    newOrganism(pos, angle, Genotype());
  }

  applyPopulationChanges();
}

void World::newOrganism(const b2Vec2& pos, float angle, const Genotype& parent_genotype) {
  unique_lock<mutex> guard(new_organisms_lock_);
  new_organisms_.push_back(make_unique<Organism>(this, pos, angle, parent_genotype));
}

void World::newFood(const b2Vec2& pos) {
  b2BodyDef body_def;
  body_def.type = b2_dynamicBody;
  body_def.position = pos;
  body_def.userData = reinterpret_cast<void*>(1);  // HACK to easily identify food bodies
  auto body = world_.CreateBody(&body_def);

  b2CircleShape shape;
  shape.m_radius = 0.1f;

  b2FixtureDef fixture_def;
  fixture_def.shape = &shape;
  fixture_def.density = 1.0f;
  fixture_def.friction = 0.3f;
  fixture_def.restitution = 0.8f;
  fixture_def.material.color = b2Color(0.1, 0.9, 0.1);
  fixture_def.material.emit_intensity = 0.1f;
  body->CreateFixture(&fixture_def);
}

void World::applyPopulationChanges() {
  unique_lock<mutex> guard_new(new_organisms_lock_);
  unique_lock<mutex> guard_dead(dead_organisms_lock_);

  for (auto index : dead_organisms_) {
    const auto& organism = organisms_[index];
    if (organism->alive()) {
      organism->die();
    }
  }

  for (auto& new_organism : new_organisms_) {
    try {
      new_organism->developBody();
    } catch (const std::exception& e) {
      printf("Organism not viable.\n");
      continue;
    }

    if (!dead_organisms_.empty()) {
      // reuse slot
      organisms_[dead_organisms_.back()] = std::move(new_organism);
      dead_organisms_.pop_back();
    } else {
      organisms_.push_back(std::move(new_organism));
    }
  }

  new_organisms_.clear();
}

void World::postStep(float dt) {
  for (auto body : reaped_bodies_) {
    world_.DestroyBody(body);
  }
  reaped_bodies_.clear();

  pp::for_each(organisms_, [&](int index, const unique_ptr<Organism>& organism) {
    if (organism->alive()) {
      if (!organism->simStep(dt)) {
        unique_lock<mutex> guard(dead_organisms_lock_);
        dead_organisms_.push_back(index);
      }
    }
  });

  applyPopulationChanges();
}

void World::onContact(b2Contact* contact) {
  if (contact->IsTouching()) {
    const auto body_a = contact->GetFixtureA()->GetBody();
    const auto body_b = contact->GetFixtureB()->GetBody();
    if (body_a->GetUserData() && !body_b->GetUserData()) {
      reaped_bodies_.insert(body_a);
    } else if (body_b->GetUserData() && !body_a->GetUserData()) {
      reaped_bodies_.insert(body_b);
    }
  }
}

}  // namespace seg_tree
