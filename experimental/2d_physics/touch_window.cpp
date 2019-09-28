
#include "touch_window.h"
#include "sandbox_window.h"

#include <QPainter>
#include <QPen>
#include <QVBoxLayout>
#include <QLineF>
#include <QRectF>
#include <QPointF>

TouchSensorWidget::TouchSensorWidget(QWidget *parent) : Canvas(parent) {
  setViewport(QRectF(-10, 10, 20, -20));
}

void TouchSensorWidget::setSensor(const phys::TouchSensor *sensor) {
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

    // axes
    painter.setPen(QPen(Qt::gray, 0));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(QLineF(0, -1000, 0, 1000));
    painter.drawLine(QLineF(-1000, 0, 1000, 0));
    painter.drawEllipse(QRectF(-5, -5, 10, 10));

    // TODO
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
