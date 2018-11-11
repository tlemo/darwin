// Copyright 2018 The Darwin Neuroevolution Framework Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "world_widget.h"

#include <core/math_2d.h>
#include <domains/harvester/robot.h>
#include <domains/harvester/world_map.h>

#include <QBrush>
#include <QPainter>
#include <QPen>
#include <QPointF>

namespace harvester_ui {

WorldWidget::WorldWidget(QWidget* parent) : core_ui::Canvas(parent) {
  setAutoFillBackground(false);

  setBorderSize(15);
  setViewport(QRectF(0, 0, 1, 1));
}

void WorldWidget::setWorld(const harvester::World* world) {
  assert(world != nullptr);
  world_ = world;
  const auto& world_map = world_->worldMap();
  rows_ = int(world_map.cells.rows);
  cols_ = int(world_map.cells.cols);
  setViewport(QRectF(0, 0, cols_, rows_));
  update();
}

void WorldWidget::paintWorld(QPainter& painter) const {
  const auto& world_map = world_->worldMap();

  // map "frame"
  painter.setPen(QPen(kGridColor, 0));
  painter.setBrush(kEmptyColor);
  painter.drawRect(0, 0, cols_, rows_);

  auto wall_brush = QBrush(kWallColor);
  auto bad_fruit_brush = QBrush(kBadFruitColor);
  auto junk_fruit_brush = QBrush(kJunkFruitColor);
  auto good_fruit_brush = QBrush(kGoodFruitColor);
  auto visited_brush = QBrush(kVisitedColor);

  // walls/fruits
  painter.setPen(Qt::NoPen);
  for (int i = 0; i < rows_; ++i)
    for (int j = 0; j < cols_; ++j)
      switch (world_map.cells[i][j]) {
        case harvester::WorldMap::Cell::Wall:
          painter.setBrush(wall_brush);
          painter.drawRect(QRectF(j, i, 1, 1));
          break;

        case harvester::WorldMap::Cell::FruitBad:
          painter.setBrush(bad_fruit_brush);
          painter.drawRect(QRectF(j, i, 1, 1));
          break;

        case harvester::WorldMap::Cell::FruitJunk:
          painter.setBrush(junk_fruit_brush);
          painter.drawRect(QRectF(j, i, 1, 1));
          break;

        case harvester::WorldMap::Cell::FruitGood:
          painter.setBrush(good_fruit_brush);
          painter.drawRect(QRectF(j, i, 1, 1));
          break;

        case harvester::WorldMap::Cell::Visited:
          painter.setBrush(visited_brush);
          painter.drawRect(QRectF(j, i, 1, 1));
          break;

        case harvester::WorldMap::Cell::Empty:
          // nothing to draw
          break;
      }

  // grid
  if (scale() > 7.0) {
    auto grid_pen = QPen(kGridColor, 0, Qt::DotLine);
    painter.setPen(grid_pen);

    for (int i = 1; i < rows_; ++i)
      painter.drawLine(0, i, cols_, i);

    for (int i = 1; i < cols_; ++i)
      painter.drawLine(i, 0, i, rows_);
  }
}

void WorldWidget::paintRobot(QPainter& painter) const {
  const auto& world_map = world_->worldMap();
  const auto robot = world_->robot();
  const auto& pos = robot->position();

  QPointF robot_location(pos.x, pos.y);

  // the robot itself
  painter.setPen(Qt::NoPen);
  painter.setBrush(kRobotColor);
  const double robot_radius = harvester::g_config.robot_size / 2;
  painter.drawEllipse(robot_location, robot_radius, robot_radius);

  // dead robot?
  if (!robot->alive()) {
    painter.setPen(QPen(Qt::red, 0));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(robot_location, robot_radius * 2, robot_radius * 2);
    return;
  }

  // field of view
  const double fov = math::radiansToDegrees(harvester::g_config.vision_fov);
  const double angle = math::radiansToDegrees(robot->angle());
  constexpr double fov_size = 1e6;
  QRectF fov_rect(pos.x - fov_size, pos.y - fov_size, fov_size * 2, fov_size * 2);
  painter.setPen(Qt::NoPen);
  painter.setBrush(kFovColor);
  painter.setClipRect(0, 0, cols_, rows_);
  painter.drawPie(fov_rect, -(angle + fov / 2) * 16, fov * 16);
  painter.setClipping(false);

  // vision rays
  QColor wall_ray_color(kWallColor);
  QColor bad_fruit_ray_color(kBadFruitColor);
  QColor junk_fruit_ray_color(kJunkFruitColor);
  QColor good_fruit_ray_color(kGoodFruitColor);

  for (const auto& ray : robot->vision()) {
    if (ray.row == -1)
    {
      assert(ray.col == -1);
      continue;
    }

    QColor color;
    switch (world_map.cells[ray.row][ray.col]) {
      case harvester::WorldMap::Cell::Wall:
        color = wall_ray_color;
        break;
      case harvester::WorldMap::Cell::FruitBad:
        color = bad_fruit_ray_color;
        break;
      case harvester::WorldMap::Cell::FruitJunk:
        color = junk_fruit_ray_color;
        break;
      case harvester::WorldMap::Cell::FruitGood:
        color = good_fruit_ray_color;
        break;
      default:
        FATAL("unexpected cell type");
    }

    QPointF ray_hit_location = robot_location + QPointF(ray.ray.x, ray.ray.y);
    painter.setPen(QPen(color, 0));
    painter.drawLine(robot_location, ray_hit_location);
    painter.setBrush(color);
    painter.setPen(QPen(kGridColor, 0));
    painter.drawEllipse(ray_hit_location, 0.2, 0.2);
  }
}

void WorldWidget::paintEvent(QPaintEvent*) {
  QPainter painter(this);

  // background
  painter.setPen(Qt::NoPen);
  painter.setBrush(kBackgroundColor);
  painter.drawRect(rect());

  painter.setTransform(transformFromViewport());

  if (world_ != nullptr) {
    paintWorld(painter);
    paintRobot(painter);
  }
}

void WorldWidget::mousePressEvent(QMouseEvent*) {
  emit sigSingleStep();
  update();
}

}  // namespace harvester_ui
