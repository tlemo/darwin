// Copyright The Darwin Neuroevolution Framework Authors.
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

#pragma once

#include "replicators.h"

#include <core_ui/sim/box2d_widget.h>

#include <memory>
using namespace std;

namespace experimental::replicators {

class PhenotypeWidget : public physics_ui::Box2dWidget {
  Q_OBJECT

  const QColor kBackgroundColor{ 255, 255, 255 };
  const QColor kHighlightedColor{ 240, 240, 255 };

 public:
  PhenotypeWidget(QWidget* parent, unique_ptr<Phenotype> phenotype);

  void animate();

 signals:
  void sigClicked();

 protected:
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void enterEvent(QEvent* event) override;
  void leaveEvent(QEvent* event) override;

  void setUniformBackgroundColor(const QColor& color);

 private:
  unique_ptr<Phenotype> phenotype_;
};

}  // namespace experimental::replicators
