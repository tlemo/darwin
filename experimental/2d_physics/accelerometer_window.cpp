
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

    const QPen vector_pen(Qt::darkGreen, kVectorWidth, Qt::SolidLine, Qt::RoundCap);

    QRectF sensor_rect(
        -kSensorWidth / 2, -kSensorHeight / 2, kSensorWidth, kSensorHeight);

    // axes
    painter.setPen(QPen(Qt::lightGray, 0));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(QLineF(0, -kCanvasHeight / 2, 0, kCanvasHeight / 2));
    painter.drawLine(QLineF(-kCanvasWidth / 2, 0, kCanvasWidth / 2, 0));
    painter.drawEllipse(sensor_rect);

    // angular acceleration
    sensor_rect.adjust(+kSkinSize, +kSkinSize, -kSkinSize, -kSkinSize);
    const float arc_angle = -sensor_->angularAcceleration() * 90;
    painter.setPen(vector_pen);
    painter.drawArc(sensor_rect, -90 * 16, int(arc_angle * 16));

    // linear acceleration
    const b2Vec2 linear_acceleration = sensor_->linearAcceleration() * kVectorLength;
    painter.setPen(vector_pen);
    painter.drawLine(QLineF(0, 0, linear_acceleration.x, linear_acceleration.y));

    // inner circle
    painter.setPen(QPen(Qt::lightGray, 0));
    sensor_rect.adjust(+kSkinSize, +kSkinSize, -kSkinSize, -kSkinSize);
    painter.drawEllipse(sensor_rect);
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
