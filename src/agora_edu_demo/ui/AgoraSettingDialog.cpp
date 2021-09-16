#include "AgoraSettingDialog.h"
#include <QApplication>
#include <QBitmap>
#include <QListView>
#include <QPainter>
#include <QPalette>
#include <QScreen>
#include <QStyledItemDelegate>
#include "AgoraTipsDialog.h"
#include "util.h"
using namespace agora::edu;

AgoraSettingDialog::AgoraSettingDialog(QWidget *parent) : QDialog(parent) {
  ui.setupUi(this);
  video_capture_testing_master_ = false;
  video_capture_testing_slave_ = false;
  audio_playback_testing = false;
  audio_capture_testing = false;
  connect(ui.ExitPushButton, SIGNAL(clicked()), this,
          SLOT(OnCloseButtonClicked()));
  connect(ui.btn_applay, SIGNAL(clicked()), this, SLOT(OnApplyButtonClicked()));

  connect(ui.btn_audio_cap_test, SIGNAL(clicked()), this,
          SLOT(OnAudioCaptureTest()));

  connect(ui.btn_audio_playback_test, SIGNAL(clicked()), this,
          SLOT(OnAudioPlaybackTest()));

  setWindowFlags(Qt::FramelessWindowHint);
  ui.sldier_cap->setBkColor(QColor(0x00, 0x73, 0xff));
  ui.sldier_cap->setFrontColor(QColor(0xE2, 0xE2, 0xf0));
  ui.sldier_cap->setCircleColor(QColor(0xff, 0xff, 0xff));
  ui.sldier_cap->setMaxValue(255);
  ui.sldier_playback->setBkColor(QColor(0x00, 0x73, 0xff));
  ui.sldier_playback->setFrontColor(QColor(0xE2, 0xE2, 0xf0));
  ui.sldier_playback->setCircleColor(QColor(0xff, 0xff, 0xff));
  ui.sldier_playback->setMaxValue(255);

  ui.cmb_audio_cap->setView(new QListView());
  ui.cmb_audio_playback->setView(new QListView());
  ui.cmb_video_source_1->setView(new QListView());
  ui.cmb_video_source_2->setView(new QListView());
  ui.video_render_1->setAutoFillBackground(true);
  ui.video_render_1->setLayout(new QHBoxLayout(ui.video_render_1));
  ui.video_render_2->setAutoFillBackground(true);
  ui.video_render_2->setLayout(new QHBoxLayout(ui.video_render_2));
  widgetFrame_1 = new QFrame(ui.video_render_1);
  widgetFrame_1->setStyleSheet(
      QLatin1String("background-color:#FFD8D8D8; background-image: "
                    "url(\":/image/resource/icon-camera-off.png\"); "
                    "background-repeat: none; background-position: "
                    "center;"));
  widgetFrame_2 = new QFrame(ui.video_render_2);
  widgetFrame_2->setStyleSheet(
      QLatin1String("background-color:#FFD8D8D8; background-image: "
                    "url(\":/image/resource/icon-camera-off.png\"); "
                    "background-repeat: none; background-position: "
                    "center;"));
  ui.video_render_1->layout()->addWidget(widgetFrame_1);
  ui.video_render_2->layout()->addWidget(widgetFrame_2);
  connect(ui.sldier_cap, SIGNAL(PositonChangeSignal(qreal)), this,
          SLOT(OnSliderCapturePositonChange(qreal)));
  connect(ui.sldier_playback, SIGNAL(PositonChangeSignal(qreal)), this,
          SLOT(OnSliderPlaybackPositonChange(qreal)));
  ui.cap_widget_render->setRange(255);
  ui.cap_widget_render->setImage(
      QPixmap(":/image/resource/icon-microphone@2x.png"));
  ui.playback_widget_render->setRange(255);
  ui.playback_widget_render->setImage(
      QPixmap(":/image/resource/icon-speaker@2x.png"));
  device_manager_ = agora::edu::createaAgoraEduDeviceManger();
  auto config = GetConfig();
  device_manager_->Initialize(config.app_id.c_str(), 2);
  device_manager_->RegisterEventHandler(this);
  InitViews();
}

AgoraSettingDialog::~AgoraSettingDialog() {
  if (device_manager_) {
    device_manager_->Release();
    device_manager_ = nullptr;
  }
}

void AgoraSettingDialog::showEvent(QShowEvent *event) {
  auto screens = QApplication::screens();
  int appWindowWidth = this->geometry().width();
  int appWindowHeight = this->geometry().height();
  auto screen = screens.at(0);
  if (screen) {
    int center_y = screen->size().height() / 2 - appWindowHeight / 2;
    int center_x = screen->size().width() / 2 - appWindowWidth / 2;
    move(center_x, center_y);
    ShowViews();
  }
}

void AgoraSettingDialog::closeEvent(QCloseEvent *event) {
  if (!device_manager_) {
    return;
  }
  if (audio_capture_testing) {
    device_manager_->StopRecordingDeviceTest();
  }
  if (audio_playback_testing) {
    device_manager_->StopPlaybackDeviceTest();
  }
  if (video_capture_testing_master_ || video_capture_testing_slave_) {
    device_manager_->StopDeviceTest(0);
    device_manager_->StopDeviceTest(1);
  }
  audio_capture_testing = false;
  audio_playback_testing = false;
}

void AgoraSettingDialog::onAudioVolumeIndication(
    const agora::edu::AudioVolumeInfo *speakers, unsigned int speakerNumber,
    int totalVolume) {
  if (audio_capture_testing) {
    ui.cap_widget_render->setValue(totalVolume);
  } else {
    ui.cap_widget_render->setValue(0);
  }
  if (audio_playback_testing) {
    ui.playback_widget_render->setValue(totalVolume);
  } else {
    ui.playback_widget_render->setValue(0);
  }
}

void AgoraSettingDialog::RefreshComboBox(
    QComboBox *cmb, std::vector<agora::edu::EduDevice> vec_device) {
  for (int i = 1; i < cmb->count(); i++) {
    if (i != cmb->currentIndex()) cmb->removeItem(i--);
  }
  auto str = cmb->currentText();
  for (int i = 0; i < vec_device.size(); i++) {
    if (cmb->currentText() != vec_device[i].name)
      cmb->insertItem(device_video_map_[vec_device[i].name] + 1,
                      vec_device[i].name);
  }
}

void AgoraSettingDialog::mousePressEvent(QMouseEvent *event) {
  window_top_left_point_ = frameGeometry().topLeft();
  auto current_pos = event->globalPos();
  auto exit_button_pos =
      mapToGlobal(ui.ExitPushButton->geometry().bottomRight());
  auto rect = QRect(window_top_left_point_, exit_button_pos);

  if (event->button() == Qt::LeftButton && rect.contains(current_pos)) {
    is_drag_ = true;
    mouse_start_point_ = event->globalPos();
    setCursor(Qt::OpenHandCursor);
  }

  QWidget::mousePressEvent(event);
}

void AgoraSettingDialog::mouseMoveEvent(QMouseEvent *event) {
  if (is_drag_) {
    QPoint distance = event->globalPos() - mouse_start_point_;
    this->move(window_top_left_point_ + distance);
  }

  QWidget::mouseMoveEvent(event);
}

void AgoraSettingDialog::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    is_drag_ = false;
    setCursor(Qt::ArrowCursor);
  }

  QWidget::mouseReleaseEvent(event);
}

void AgoraSettingDialog::resizeEvent(QResizeEvent *e) {
  QWidget::resizeEvent(e);
}

void AgoraSettingDialog::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QBitmap bmp(this->size());
  bmp.fill();
  QPainter painter(&bmp);
  painter.setPen(Qt::black);
  painter.setBrush(Qt::black);
  painter.setRenderHints(QPainter::HighQualityAntialiasing |
                         QPainter::SmoothPixmapTransform);
  painter.drawRoundedRect(bmp.rect(), 7, 7);
  setMask(bmp);

  {
    QPainter p(this);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    QPen pen(QColor(0xDB, 0xDB, 0xEA), 3);
    p.setPen(pen);
    p.drawRoundRect(QRect(bmp.rect().x() + 1, bmp.rect().y() + 1,
                          bmp.rect().width() - 1, bmp.rect().height() - 1),
                    2, 2);
  }
}

void AgoraSettingDialog::InitViews() {
  if (!device_manager_) {
    return;
  }
  {
    auto devices = device_manager_->EnumeratePlaybackDevices();
    for (int i = 0; i < devices->Count(); i++) {
      EduDevice dc;
      devices->GetDevice(i, dc);
      device_audio_playbacks_.push_back(dc);
      ui.cmb_audio_playback->addItem(QString(dc.name));
    }
    if (devices->Count() > 0) {
      setting_config_.audio_playback.device_id = device_audio_playbacks_[0].id;
    }
  }
  {
    auto devices = device_manager_->EnumerateRecordingDevices();
    for (int i = 0; i < devices->Count(); i++) {
      EduDevice dc;
      devices->GetDevice(i, dc);
      device_audio_captures_.push_back(dc);
      ui.cmb_audio_cap->addItem(QString(dc.name));
    }
    if (devices->Count() > 0) {
      setting_config_.audio_capture.device_id = device_audio_captures_[0].id;
    }
  }
  {
    auto devices = device_manager_->EnumerateVideoDevices();
    int count = devices->Count();
    for (int i = 0; i < count; i++) {
      EduDevice dc;
      devices->GetDevice(i, dc);
      device_video_captures_.push_back(dc);
      ui.cmb_video_source_2->addItem(QString(dc.name));
      ui.cmb_video_source_1->addItem(QString(dc.name));
      device_video_map_.insert(std::make_pair(std::string(dc.name), i));
    }
    if (device_video_captures_.size() >= 1) {
      setting_config_.video_source_master.device_id =
          device_video_captures_[0].id;
      setting_config_.video_source_master.device_name =
          device_video_captures_[0].name;
      setting_config_.video_source_slave.device_id = "";
      setting_config_.video_source_slave.device_name = "";
    }
    if (device_video_captures_.size() > 1) {
      setting_config_.video_source_slave.device_id =
          device_video_captures_[1].id;
      setting_config_.video_source_slave.device_name =
          device_video_captures_[1].name;
    }
  }
  connect(ui.cmb_video_source_1,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &AgoraSettingDialog::OnVideoSourceChanged_Master,
          Qt::DirectConnection);
  connect(ui.cmb_video_source_2,
          QOverload<int>::of(&QComboBox::currentIndexChanged), this,
          &AgoraSettingDialog::OnVideoSourceChanged_Slave,
          Qt::DirectConnection);
}

void AgoraSettingDialog::ShowViews() {
  if (video_capture_testing_master_) {
    setting_config_.video_source_master.device_id =
        device_video_captures_[device_video_map_[ui.cmb_video_source_1
                                                     ->currentText()
                                                     .toStdString()]]
            .id;
    device_manager_->SetDevice(
        0, setting_config_.video_source_master.device_id.toStdString().c_str());

    device_manager_->StartDeviceTest(0, (HWND)ui.video_render_1->winId());
  } else {
    if (setting_config_.video_source_master.device_id.isEmpty()) {
      ui.cmb_video_source_1->setCurrentIndex(0);
      video_capture_testing_master_ = false;
    } else {
      int idx = 0;
      for (int i = 1; i < ui.cmb_video_source_1->count(); i++) {
        QString tmp = ui.cmb_video_source_1->itemText(i);
        if (ui.cmb_video_source_1->itemText(i) ==
            setting_config_.video_source_master.device_name) {
          idx = i;
        }
      }
      ui.cmb_video_source_1->setCurrentIndex(idx);
      video_capture_testing_master_ = true;
    }
  }
  if (video_capture_testing_slave_) {
    setting_config_.video_source_slave.device_id =
        device_video_captures_[device_video_map_[ui.cmb_video_source_2
                                                     ->currentText()
                                                     .toStdString()]]
            .id;
    device_manager_->SetDevice(
        1, setting_config_.video_source_slave.device_id.toStdString().c_str());
    device_manager_->StartDeviceTest(1, (HWND)ui.video_render_2->winId());
  } else {
    if (setting_config_.video_source_slave.device_id.isEmpty()) {
      ui.cmb_video_source_2->setCurrentIndex(0);
      video_capture_testing_slave_ = false;
    } else {
      int idx = 0;
      for (int i = 1; i < ui.cmb_video_source_2->count(); i++) {
        if (ui.cmb_video_source_2->itemText(i) ==
            setting_config_.video_source_slave.device_name) {
          idx = i;
        }
      }
      ui.cmb_video_source_2->setCurrentIndex(idx);
      video_capture_testing_master_ = true;
    }
  }

  {
    int vol = 0;
    device_manager_->GetPlaybackDeviceVolume(&vol);
    ui.sldier_playback->setValue(vol);
    device_manager_->GetRecordingDeviceVolume(&vol);
    ui.sldier_cap->setValue(vol);
  }
}

void AgoraSettingDialog::OnAudioCaptureTest() {
  if (!device_manager_) {
    return;
  }
  int sel = ui.cmb_audio_cap->currentIndex();
  if (sel < 0) return;
  if (sel >= device_audio_captures_.size()) return;
  if (audio_capture_testing) {
    device_manager_->StopRecordingDeviceTest();
    ui.btn_audio_cap_test->setText(QString::fromLocal8Bit("检测"));
    ui.cap_widget_render->setValue(0);
  } else {
    device_manager_->SetRecordingDevice(device_audio_captures_[sel].id);
    device_manager_->StartRecordingDeviceTest(10);
    ui.btn_audio_cap_test->setText(QString::fromLocal8Bit("停止"));
  }
  audio_capture_testing = !audio_capture_testing;
}

void AgoraSettingDialog::OnAudioPlaybackTest() {
  if (!device_manager_) {
    return;
  }
  int sel = ui.cmb_audio_playback->currentIndex();
  if (sel < 0) return;
  if (sel >= device_audio_playbacks_.size()) return;
  if (audio_playback_testing) {
    device_manager_->StopPlaybackDeviceTest();
    ui.btn_audio_playback_test->setText(QString::fromLocal8Bit("检测"));
    ui.playback_widget_render->setValue(0);
  } else {
    device_manager_->SetPlaybackDevice(device_audio_playbacks_[sel].id);
    QString exec_dir = QApplication::applicationDirPath();
    exec_dir += "/ID_TEST_AUDIO.wav";
    device_manager_->StartPlaybackDeviceTest(exec_dir.toStdString().c_str());
    ui.btn_audio_playback_test->setText(QString::fromLocal8Bit("停止"));
  }
  audio_playback_testing = !audio_playback_testing;
}

void AgoraSettingDialog::OnSliderCapturePositonChange(qreal real) {
  if (!device_manager_) {
    return;
  }
  device_manager_->SetRecordingDeviceVolume(real);
}

void AgoraSettingDialog::OnSliderPlaybackPositonChange(qreal real) {
  if (!device_manager_) {
    return;
  }
  device_manager_->SetPlaybackDeviceVolume(real);
}

void AgoraSettingDialog::OnVideoSourceChanged_Master(int idx) {
  if (!device_manager_) {
    return;
  }
  if (idx < 0) return;
  if (idx == 0) {
    setting_config_.video_source_master.device_id.clear();
    RefreshComboBox(ui.cmb_video_source_2, device_video_captures_);
    device_manager_->StopDeviceTest(0);
    ui.video_render_1->update();
    ui.video_render_1->setStyleSheet(
        "border-iamge:url(:/image/resource/icon-camera-off@2x.png);");

    video_capture_testing_master_ = false;
    return;
  }
  idx -= 1;  //去掉
  if (idx >= device_video_captures_.size()) return;
  auto copy = device_video_captures_;
  std::string current_device =
      ui.cmb_video_source_1->currentText().toStdString();
  copy.erase(copy.begin() + device_video_map_[current_device]);
  RefreshComboBox(ui.cmb_video_source_2, copy);
  device_manager_->SetDevice(
      0, device_video_captures_[device_video_map_[current_device]].id);
  setting_config_.video_source_master.device_id =
      device_video_captures_[idx].id;
  device_manager_->StartDeviceTest(0, (HWND)ui.video_render_1->winId());
  video_capture_testing_master_ = true;
}

void AgoraSettingDialog::OnVideoSourceChanged_Slave(int idx) {
  if (!device_manager_) {
    return;
  }
  if (idx < 0) return;
  if (idx == 0) {
    setting_config_.video_source_slave.device_id.clear();
    RefreshComboBox(ui.cmb_video_source_1, device_video_captures_);
    device_manager_->StopDeviceTest(1);
    ui.video_render_2->update();
    ui.video_render_2->setStyleSheet(
        "border-iamge:url(:/image/resource/icon-camera-off@2x.png);");

    video_capture_testing_slave_ = false;
    return;
  }
  idx -= 1;  //去掉
  if (idx >= device_video_captures_.size()) return;
  auto copy = device_video_captures_;
  std::string current_device =
      ui.cmb_video_source_2->currentText().toStdString();
  copy.erase(copy.begin() + device_video_map_[current_device]);
  RefreshComboBox(ui.cmb_video_source_1, copy);
  device_manager_->SetDevice(
      1, device_video_captures_[device_video_map_[current_device]].id);
  setting_config_.video_source_slave.device_id = device_video_captures_[idx].id;
  device_manager_->StartDeviceTest(1, (HWND)ui.video_render_2->winId());
  video_capture_testing_slave_ = true;
}

void AgoraSettingDialog::OnApplyButtonClicked() {
  emit SettingConfigChanged(setting_config_);
  close();
}

void AgoraSettingDialog::OnCloseButtonClicked() {
  if (AgoraTipsDialog::ExecTipsDialog(
          QString::fromLocal8Bit("设置确认"),
          QString::fromLocal8Bit("是否应用当前设置？")) == QDialog::Accepted) {
    OnApplyButtonClicked();
  } else {
    close();
  }
}

void AgoraSettingDialog::OnMasterPreferred() {
  if (ui.rdo_hight_level_video_1->isChecked()) {
    setting_config_.video_source_master.preferred = SHARPNESS_PREFERRED;
  } else {
    setting_config_.video_source_master.preferred = FLUENCY_PREFERRED;
  }
}

void AgoraSettingDialog::OnSlavePreferred() {
  if (ui.rdo_hight_level_video_2->isChecked()) {
    setting_config_.video_source_master.preferred = SHARPNESS_PREFERRED;
  } else {
    setting_config_.video_source_master.preferred = FLUENCY_PREFERRED;
  }
}
