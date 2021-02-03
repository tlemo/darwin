
#include "main_window.h"
#include "tests.h"

#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  Q_INIT_RESOURCE(resources);

#ifndef NDEBUG
  experimental::replicators::tests::runTests();
#endif

  experimental::replicators::MainWindow main_window;
  main_window.show();
  return app.exec();
}
