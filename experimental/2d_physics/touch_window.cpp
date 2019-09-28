
#include "touch_window.h"
#include "sandbox_window.h"

#include <QLineF>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QRectF>
#include <QVBoxLayout>

TouchSensorWidget::TouchSensorWidget(QWidget* parent) : Canvas(parent) {
  setViewport(QRectF(-kCanvasWidth / 2, kCanvasHeight / 2, kCanvasWidth, -kCanvasHeight));
}

void TouchSensorWidget::setSensor(const phys::TouchSensor* sensor) {
  sensor_ = sensor;
  update();
}

void TouchSensorWidget::paintEvent(QPaintEvent* event) {
  Canvas::paintEvent(event);
  if (sensor_ != nullptr) {
    QPainter painter(this);

    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing |
                           QPainter::SmoothPixmapTransform |
                           QPainter::HighQualityAntialiasing);

    // background (whole widget)
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::white);
    painter.drawRect(rect());

    painter.setTransform(transformFromViewport());

    const QRectF sensorsRect(
        -kSensorWidth / 2, -kSensorHeight / 2, kSensorWidth, kSensorHeight);

    // axes
    painter.setPen(QPen(Qt::lightGray, 0));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(QLineF(0, -kCanvasHeight / 2, 0, kCanvasHeight / 2));
    painter.drawLine(QLineF(-kCanvasWidth / 2, 0, kCanvasWidth / 2, 0));

    // receptor values
    painter.setPen(QPen(Qt::gray, 0));
    const auto receptors = sensor_->receptors();
    if (receptors.size() == 1) {
      painter.setBrush(receptors[0] > 0 ? Qt::green : Qt::white);
      painter.drawEllipse(sensorsRect);
    } else {
      const double slice_angle = 360.0 / receptors.size();
      for (size_t i = 0; i < receptors.size(); ++i) {
        const double angle = i * slice_angle - 90;
        Q_ASSERT(receptors[i] >= 0);
        painter.setBrush(receptors[i] > 0 ? Qt::green : Qt::white);
        painter.drawPie(sensorsRect, int(angle * 16), int(slice_angle * 16));
      }
    }

    painter.setPen(QPen(Qt::gray, 0));
    painter.setBrush(Qt::white);
    painter.drawEllipse(
        sensorsRect.adjusted(+kSkinSize, +kSkinSize, -kSkinSize, -kSkinSize));
  }
}

TouchWindow::TouchWindow(QWidget* parent) : ToolWindow(parent) {
  setWindowTitle("Touch");
  setMinimumSize(64, 64);
  touch_sensor_widget_ = new TouchSensorWidget(this);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(touch_sensor_widget_);
}

void TouchWindow::onSandboxChange(SandboxWindow* sandbox_window) {
  touch_sensor_widget_->setSensor(
      sandbox_window ? sandbox_window->scenePackage().scene->touchSensor() : nullptr);
}
