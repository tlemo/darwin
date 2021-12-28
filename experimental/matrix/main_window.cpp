
#include "main_window.h"
#include "camera_window.h"
#include "touch_window.h"
#include "accelerometer_window.h"
#include "compass_window.h"
#include "ui_main_window.h"

#include <QDockWidget>

MainWindow::MainWindow() : QMainWindow(nullptr), ui(new Ui::MainWindow) {
  ui->setupUi(this);

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

  status_label = new QLabel;
  status_label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  ui->status_bar->addPermanentWidget(status_label);

  world_.generateWorld();

  connect(&timer_, &QTimer::timeout, this, &MainWindow::simStep);
  timer_.setInterval(0);
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

void MainWindow::simStep() {
  world_.simStep();
  update();

  fps_tracker_.update();
  status_label->setText(QString::asprintf("%.2f fps", fps_tracker_.currentRate()));
}
