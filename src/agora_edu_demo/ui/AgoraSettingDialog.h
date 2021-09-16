#pragma once

#include <QDialog>
#include <QMouseEvent>
#include <QWidget>
#include <memory>
#include <unordered_map>
#include "IEduDeviceManager.h"
#include "ui_AgoraSettingDialog.h"

enum Preferred { FLUENCY_PREFERRED, SHARPNESS_PREFERRED };

struct VideoSource {
  QString device_id;
  QString device_name;
  Preferred preferred;
  VideoSource() {
    device_id = "";
    device_name = "";
    preferred = FLUENCY_PREFERRED;
  }
  VideoSource(const VideoSource &video_source) {
    this->device_id = video_source.device_id;
    this->device_name = video_source.device_name;
    this->preferred = video_source.preferred;
  }
};

struct AudioDevice {
  QString device_id;
  QString device_name;
  AudioDevice() {
    device_id = "";
    device_name = "";
  }
  AudioDevice(const AudioDevice &device) {
    this->device_id = device.device_id;
    this->device_name = device.device_name;
  }
};

using AudioPlayBackDevice = AudioDevice;
using AudioCaptureDevice = AudioDevice;

struct SettingConfig {
  VideoSource video_source_master;
  VideoSource video_source_slave;
  AudioCaptureDevice audio_capture;
  AudioPlayBackDevice audio_playback;
  SettingConfig() {}
  SettingConfig(const SettingConfig &config) {
    this->video_source_master = config.video_source_master;
    this->video_source_slave = config.video_source_slave;
    this->audio_capture = config.audio_capture;
    this->audio_playback = config.audio_playback;
  }
};

class AgoraSettingDialog : public QDialog,
                           public agora::edu::IEduDeviceMangerEvenetHandler {
  Q_OBJECT

 public:
  AgoraSettingDialog(QWidget *parent = Q_NULLPTR);
  virtual ~AgoraSettingDialog();
  void InitViews();
  void ShowViews();
  void showEvent(QShowEvent *event);
  void closeEvent(QCloseEvent *event);
  SettingConfig GetSettingConfig() { return setting_config_; }
  virtual void onAudioVolumeIndication(
      const agora::edu::AudioVolumeInfo *speakers, unsigned int speakerNumber,
      int totalVolume) override;
  void RefreshComboBox(QComboBox *cmb, std::vector<agora::edu::EduDevice>);

 protected:
  void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
  void resizeEvent(QResizeEvent *) Q_DECL_OVERRIDE;

 private slots:
  void OnApplyButtonClicked();
  void OnAudioCaptureTest();
  void OnAudioPlaybackTest();
  void OnSliderCapturePositonChange(qreal real);
  void OnSliderPlaybackPositonChange(qreal real);
  void OnVideoSourceChanged_Master(int idx);
  void OnVideoSourceChanged_Slave(int idx);
  void OnCloseButtonClicked();
  void OnMasterPreferred();
  void OnSlavePreferred();

 signals:
  void SettingConfigChanged(SettingConfig config);

 private:
  bool video_capture_testing_master_;
  bool video_capture_testing_slave_;
  bool audio_capture_testing;
  bool audio_playback_testing;

  Ui::AgoraSettingDialog ui;

  bool is_drag_ = false;
  QPoint mouse_start_point_;
  QPoint window_top_left_point_;

  SettingConfig setting_config_;
  QFrame * widgetFrame_1;
  QFrame * widgetFrame_2;
  agora::edu::IEduDeviceManger *device_manager_;
  std::vector<agora::edu::EduDevice> device_audio_playbacks_;
  std::vector<agora::edu::EduDevice> device_audio_captures_;
  std::vector<agora::edu::EduDevice> device_video_captures_;
  std::unordered_map<std::string, int> device_video_map_;
};
