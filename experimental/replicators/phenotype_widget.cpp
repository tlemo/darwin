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

#include "phenotype_widget.h"

#include <QMouseEvent>

namespace experimental::replicators {

PhenotypeWidget::PhenotypeWidget(QWidget* parent, unique_ptr<Phenotype> phenotype)
    : physics_ui::Box2dWidget(parent), phenotype_(std::move(phenotype)) {
  setBorderSize(4);
  setFrameStyle(QFrame::StyledPanel | QFrame::Plain);
  setUniformBackgroundColor(kBackgroundColor);
  setViewport(QRectF(0, 0, 0.01f, 0.01f));
  setViewportPolicy(ViewportPolicy::AutoExpanding);
  setDebugRender(true);
  setWorld(phenotype_->specimen());
}

void PhenotypeWidget::animate() {
  phenotype_->animate();
  update();
}

void PhenotypeWidget::mousePressEvent(QMouseEvent* /*event*/) {
  // TODO
}

void PhenotypeWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (rect().contains(event->pos()) && event->button() == Qt::MouseButton::LeftButton) {
    emit sigClicked();
  }
}

void PhenotypeWidget::mouseMoveEvent(QMouseEvent* /*event*/) {
  // TODO
}

void PhenotypeWidget::enterEvent(QEvent* /*event*/) {
  setUniformBackgroundColor(kHighlightedColor);
}

void PhenotypeWidget::leaveEvent(QEvent* /*event*/) {
  setUniformBackgroundColor(kBackgroundColor);
}

void PhenotypeWidget::setUniformBackgroundColor(const QColor& color) {
  setBackgroundColor(color);
  setViewportColor(color);
}

}  // namespace experimental::replicators
