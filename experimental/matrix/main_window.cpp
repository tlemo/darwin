
#include "main_window.h"
#include "camera_window.h"
#include "touch_window.h"
#include "accelerometer_window.h"
#include "compass_window.h"
#include "visual_map_window.h"
#include "ui_main_window.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QDockWidget>
#include <QTransform>

#include <math.h>

MainWindow::MainWindow() : QMainWindow(nullptr), ui(new Ui::MainWindow) {
  ui->setupUi(this);

  setupToolbar();
  setupToolWindows();

  ui->action_pan_view->setChecked(true);
  ui->action_select->setChecked(false);
  ui->map_view->setMode(MapView::Mode::Pan);

  connect(ui->map_view, &MapView::zoomIn, this, &MainWindow::zoomIn);
  connect(ui->map_view, &MapView::zoomOut, this, &MainWindow::zoomOut);

  status_label = new QLabel;
  status_label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->status_bar->addPermanentWidget(status_label);

  updateZoom();

  connect(&timer_, &QTimer::timeout, this, &MainWindow::refresh);
  timer_.setInterval(20);
  timer_.start();
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::updateToolWindows() {
  for (auto tool_window : tool_windows_) {
    tool_window->update();
  }
}

void MainWindow::zoomIn() {
  zoom_slider->setValue(zoom_slider->value() + 2);
}

void MainWindow::zoomOut() {
  zoom_slider->setValue(zoom_slider->value() - 2);
}

void MainWindow::updateZoom() {
  const double scale = std::pow(2, (zoom_slider->value() + 22) / 12.0);

  QTransform transform;
  transform.scale(scale, -scale);

  ui->map_view->setTransform(transform);
}

void MainWindow::setupToolbar() {
  zoom_slider = new QSlider;
  zoom_slider->setMinimum(0);
  zoom_slider->setMaximum(75);
  zoom_slider->setTickInterval(5);
  zoom_slider->setValue(kDefaultZoom);
  zoom_slider->setOrientation(Qt::Horizontal);
  zoom_slider->setTickPosition(QSlider::TicksBelow);

  connect(zoom_slider, &QSlider::valueChanged, this, &MainWindow::updateZoom);

  auto zoom_in = new QToolButton;
  zoom_in->setDefaultAction(ui->action_zoom_in);
  zoom_in->setIconSize(ui->tool_bar->iconSize());
  zoom_in->setAutoRepeat(true);
  zoom_in->setAutoRepeatInterval(50);
  zoom_in->setAutoRepeatDelay(0);

  connect(zoom_in, &QToolButton::clicked, this, &MainWindow::zoomIn);

  auto zoom_out = new QToolButton;
  zoom_out->setDefaultAction(ui->action_zoom_out);
  zoom_out->setIconSize(ui->tool_bar->iconSize());
  zoom_out->setAutoRepeat(true);
  zoom_out->setAutoRepeatInterval(50);
  zoom_out->setAutoRepeatDelay(0);

  connect(zoom_out, &QToolButton::clicked, this, &MainWindow::zoomOut);

  QHBoxLayout* zoom_layout = new QHBoxLayout;
  zoom_layout->addWidget(zoom_out);
  zoom_layout->addWidget(zoom_slider);
  zoom_layout->addWidget(zoom_in);

  QWidget* toolbar_controls = new QWidget;
  toolbar_controls->setLayout(zoom_layout);

  ui->tool_bar->addWidget(toolbar_controls);
}

void MainWindow::setupToolWindows() {
  // main toolbar menu entry
  ui->menu_windows->addAction(ui->tool_bar->toggleViewAction());

  // configure docking
  setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);
  setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
  setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);
  setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

  // camera window
  auto camera_window = new CameraWindow(this);
  dockWindow(camera_window, Qt::AllDockWidgetAreas, Qt::BottomDockWidgetArea);

  // touch window
  auto touch_window = new TouchWindow(this);
  dockWindow(touch_window, Qt::AllDockWidgetAreas, Qt::LeftDockWidgetArea);

  // accelerometer window
  auto accelerometer_window = new AccelerometerWindow(this);
  dockWindow(accelerometer_window, Qt::AllDockWidgetAreas, Qt::LeftDockWidgetArea);

  // compass window
  auto compass_window = new CompassWindow(this);
  dockWindow(compass_window, Qt::AllDockWidgetAreas, Qt::LeftDockWidgetArea);

  // visual map window
#if 0
  auto visual_map_window = new VisualMapWindow(this);
  dockWindow(visual_map_window, Qt::AllDockWidgetAreas, Qt::TopDockWidgetArea);
  visual_map_window->setScene(ui->map_view->mapScene());
#endif
}

void MainWindow::dockWindow(ToolWindow* tool_window,
                            Qt::DockWidgetAreas allowed_areas,
                            Qt::DockWidgetArea area) {
  auto dock = new QDockWidget;
  dock->setAllowedAreas(allowed_areas);
  dock->setWindowTitle(tool_window->windowTitle());
  dock->setMinimumSize(100, 75);
  dock->setWidget(tool_window);
  addDockWidget(area, dock);

  ui->menu_windows->addAction(dock->toggleViewAction());

  tool_windows_.push_back(tool_window);
}

void MainWindow::refresh() {
  ui->map_view->refresh();

  const double timestamp = ui->map_view->timestamp();
  const double ups = ui->map_view->ups();
  const double fps = ui->map_view->fps();
  status_label->setText(
      QString::asprintf("Time: %.2f (%.2f fps, %.2f ups)", timestamp, fps, ups));
}

void MainWindow::on_action_select_toggled(bool checked) {
  ui->action_pan_view->setChecked(!checked);
  ui->map_view->setMode(checked ? MapView::Mode::Select : MapView::Mode::Pan);
}

void MainWindow::on_action_pan_view_toggled(bool checked) {
  ui->action_select->setChecked(!checked);
  ui->map_view->setMode(checked ? MapView::Mode::Pan : MapView::Mode::Select);
}
