
#include "main_window.h"

#include <core/thread_pool.h>

#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  Q_INIT_RESOURCE(resources);

  // initialize thread pool
  pp::ParallelForSupport::init(nullptr);

  MainWindow main_window;
  main_window.show();
  return app.exec();
}
