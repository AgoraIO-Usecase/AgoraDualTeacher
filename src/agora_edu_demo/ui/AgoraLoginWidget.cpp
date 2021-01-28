#include "util.h"

#include <QCloseEvent>
#include <QRegExpValidator>
#include "AgoraChatWidget.h"
#include "AgoraLoginWidget.h"
#include "AgoraSettingDialog.h"
#include "AgoraTipsDialog.h"
#include "login_widget_manager.h"

AgoraLoginWidget::AgoraLoginWidget(
    std::shared_ptr<LoginWidgetManager> login_widget_manager, QWidget* parent)
    : QWidget(parent) {
  ui.setupUi(this);
  setting_dlg_.reset(new AgoraSettingDialog);
  setAttribute(Qt::WA_DeleteOnClose);

  setWindowFlags(Qt::FramelessWindowHint);

  ui.RoomNameLabel->setText(QString::fromLocal8Bit(
      STR(<h4><font color = red> ��������úţ�</ font></ h4>)));

  QRegExp regx("[a-zA-Z0-9]+$");
  ui.RoomNameLineEdit->setValidator(
      new QRegExpValidator(regx, ui.RoomNameLineEdit));

  login_widget_manager_ = login_widget_manager;

  auto setting_config = setting_dlg_->GetSettingConfig();
  login_widget_manager_->SetSettingConfig(setting_config);
  QObject::connect(ui.LoginPushButton, SIGNAL(clicked()), this,
                   SLOT(OnLoginPushButtonClicked()));
  QObject::connect(ui.ExitPushButton, SIGNAL(clicked()), this,
                   SLOT(OnExitPushButtonClicked()));
  QObject::connect(ui.SettingPushButton, SIGNAL(clicked()), this,
                   SLOT(OnSettingPushButtonClicked()));

  QObject::connect(ui.RoomNameLineEdit, SIGNAL(textChanged(const QString&)),
                   this, SLOT(OnRoomNameLineEditTextChanged(const QString&)));
}

void AgoraLoginWidget::OnLoginPushButtonClicked() {
  SetTipsLabelContent("");

  QString room_name = ui.RoomNameLineEdit->text();
  QString user_name = ui.UserNameLineEdit->text();

  if (room_name.isEmpty()||user_name.isEmpty())
  {
    SetTipsLabelContent(QString::fromLocal8Bit(
        STR(<h4><font color = red> ����Ż��û���Ϊ�գ�</ font></ h4>)));
    return;
  }

  EduRoleType type = ui.TeachRadioButton->isChecked() ? EDU_ROLE_TYPE_TEACHER
                                                      : EDU_ROLE_TYPE_STUDENT;

  login_widget_manager_->CreateClassroomManager(room_name.toStdString(),
                                                user_name.toStdString(), type);
}

void AgoraLoginWidget::OnExitPushButtonClicked() {
  if (AgoraTipsDialog::ExecTipsDialog(str2qstr("�˳�Ӧ�� ��")) ==
      QDialog::Accepted) {
    login_widget_manager_->ExitLoginWidget();
  }
}

void AgoraLoginWidget::OnInitializeResult(bool is_success) {
  if (is_success) {
    ui.LoginPushButton->setEnabled(true);
  } else {
    if (AgoraTipsDialog::ExecTipsDialog(str2qstr("��ʼ��ʧ��")) ==
        QDialog::Accepted) {
      login_widget_manager_->InitializeEduManager();
    }
  }
}

void AgoraLoginWidget::OnSettingPushButtonClicked() {
  setting_dlg_->exec();
  auto setting_config = setting_dlg_->GetSettingConfig();
  login_widget_manager_->SetSettingConfig(setting_config);
}

void AgoraLoginWidget::OnRoomNameLineEditTextChanged(const QString& text) {
  if (text.isEmpty()) {
    ui.RoomNameLabel->setText(QString::fromLocal8Bit(
        STR(<h4><font color = red> ��������úţ�</ font></ h4>)));
  } else {
    ui.RoomNameLabel->setText("");
  }
}

void AgoraLoginWidget::SetTipsLabelContent(QString content) {
  ui.TipsLabel->setText(content);
}

void AgoraLoginWidget::showEvent(QShowEvent* event) {
  ui.LoginPushButton->setEnabled(false);
}

void AgoraLoginWidget::closeEvent(QCloseEvent* event) {
  event->ignore();

  login_widget_manager_->ExitLoginWidget();
}

void AgoraLoginWidget::mousePressEvent(QMouseEvent* event) {
  window_top_left_point_ = frameGeometry().topLeft();
  auto current_pos = event->globalPos();
  auto exit_button_pos =
      mapToGlobal(ui.ExitPushButton->geometry().bottomRight());
  auto rect = QRect(window_top_left_point_, exit_button_pos);

  if (event->button() == Qt::LeftButton && rect.contains(current_pos)) {
    is_drag_ = true;
    //������ĳ�ʼλ��
    mouse_start_point_ = event->globalPos();
    // mouseStartPoint = event->pos();
    setCursor(Qt::OpenHandCursor);
  }

  QWidget::mousePressEvent(event);
}

void AgoraLoginWidget::mouseMoveEvent(QMouseEvent* event) {
  if (is_drag_) {
    //�������ƶ��ľ���
    QPoint distance = event->globalPos() - mouse_start_point_;
    // QPoint distance = event->pos() - mouseStartPoint;
    //�ı䴰�ڵ�λ��
    this->move(window_top_left_point_ + distance);
  }

  QWidget::mouseMoveEvent(event);
}

void AgoraLoginWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    is_drag_ = false;
    setCursor(Qt::ArrowCursor);
  }

  QWidget::mouseReleaseEvent(event);
}
