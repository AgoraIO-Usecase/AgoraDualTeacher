#include "src/login_widget_manager.h"
#include "util/util.h"
#include <QIcon>
#include <QTextCodec>
#include <QtWidgets/QApplication>

int main(int argc, char *argv[]) {
  QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
  QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
  QApplication app(argc, argv);
  app.setWindowIcon(QIcon(":/image/resource/logo-dual-teacher@2x.png"));
  std::shared_ptr<LoginWidgetManager> login_widget_manager =
      std::make_shared<LoginWidgetManager>();
  login_widget_manager->ShowLoginDialog();
  return app.exec();
}
