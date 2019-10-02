
#include "compass_window.h"

#include <QLineF>
#include <QPainter>
#include <QPen>
#include <QPointF>
#include <QRectF>
#include <QVBoxLayout>

#include "sandbox_window.h"

CompassWidget::CompassWidget(QWidget* parent) : Canvas(parent) {
  setViewport(QRectF(-kCanvasWidth / 2, kCanvasHeight / 2, kCanvasWidth, -kCanvasHeight));
}

void CompassWidget::setSensor(const phys::Compass* sensor) {
  sensor_ = sensor;
  update();
}

void CompassWidget::paintEvent(QPaintEvent* event) {
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

    const QRectF sensor_rect(
        -kSensorWidth / 2, -kSensorHeight / 2, kSensorWidth, kSensorHeight);

    // axes
    painter.setPen(QPen(Qt::lightGray, 0));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(QLineF(0, -kCanvasHeight / 2, 0, kCanvasHeight / 2));
    painter.drawLine(QLineF(-kCanvasWidth / 2, 0, kCanvasWidth / 2, 0));
    painter.drawEllipse(sensor_rect);

    // heading
    const auto heading = sensor_->heading() * kCompassLength;
    painter.setPen(QPen(Qt::blue, kCompassWidth, Qt::SolidLine, Qt::RoundCap));
    painter.drawLine(QLineF(0, 0, heading.x, heading.y));
  }
}

CompassWindow::CompassWindow(QWidget* parent) : ToolWindow(parent) {
  setWindowTitle("Compass");
  setMinimumSize(64, 64);
  sensor_widget_ = new CompassWidget(this);
  auto layout = new QVBoxLayout(this);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->addWidget(sensor_widget_);
}

void CompassWindow::onSandboxChange(SandboxWindow* sandbox_window) {
  sensor_widget_->setSensor(
      sandbox_window ? sandbox_window->scenePackage().scene->compass() : nullptr);
}
