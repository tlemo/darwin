
#include "accelerometer_window.h"
#include "sandbox_window.h"

#include <QLineF>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QRectF>
#include <QVBoxLayout>

AccelerometerWidget::AccelerometerWidget(QWidget* parent) : Canvas(parent) {
  setViewport(QRectF(-kCanvasWidth / 2, kCanvasHeight / 2, kCanvasWidth, -kCanvasHeight));
}

void AccelerometerWidget::setSensor(const phys::Accelerometer* sensor) {
  sensor_ = sensor;
  update();
}

void AccelerometerWidget::paintEvent(QPaintEvent* event) {
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
#if 0  // TODO
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
#endif

    painter.setPen(QPen(Qt::gray, 0));
    painter.setBrush(Qt::white);
    painter.drawEllipse(
        sensorsRect.adjusted(+kSkinSize, +kSkinSize, -kSkinSize, -kSkinSize));
  }
}

AccelerometerWindow::AccelerometerWindow(QWidget* parent) : ToolWindow(parent) {
  setWindowTitle("Accelerometer");
  setMinimumSize(64, 64);
  sensor_widget_ = new AccelerometerWidget(this);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(sensor_widget_);
}

void AccelerometerWindow::onSandboxChange(SandboxWindow* sandbox_window) {
  sensor_widget_->setSensor(
      sandbox_window ? sandbox_window->scenePackage().scene->accelerometer() : nullptr);
}
