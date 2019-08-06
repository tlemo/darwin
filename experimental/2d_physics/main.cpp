
#include "main_window.h"

#include <QApplication>

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);
  Q_INIT_RESOURCE(resources);
  
  MainWindow main_window;
  main_window.show();
  return app.exec();
}
