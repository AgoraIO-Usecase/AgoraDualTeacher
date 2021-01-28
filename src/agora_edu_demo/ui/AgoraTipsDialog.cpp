#include "AgoraTipsDialog.h"

int AgoraTipsDialog::ExecTipsDialog(QString tips) {
 static AgoraTipsDialog tips_dialog_;

  tips_dialog_.ui.TipsLabel->setText(tips);
  return tips_dialog_.exec();
}

AgoraTipsDialog::AgoraTipsDialog(QWidget *parent) : QDialog(parent) {
  ui.setupUi(this);

  setWindowFlags(Qt::FramelessWindowHint);

  QObject::connect(ui.CancelPushButton, SIGNAL(clicked()), this,
                   SLOT(OnCancelPushButtonClicked()));
  QObject::connect(ui.ConfirmPushButton, SIGNAL(clicked()), this,
                   SLOT(OnConfirmPushButtonClicked()));
}

AgoraTipsDialog::~AgoraTipsDialog() {}

void AgoraTipsDialog::OnCancelPushButtonClicked() { reject(); }

void AgoraTipsDialog::OnConfirmPushButtonClicked() { accept(); }
