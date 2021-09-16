#include "AgoraTipsDialog.h"
#include <QBitmap>
#include <QDebug>
#include <QPainter>

int AgoraTipsDialog::ExecTipsDialog(const QString& title, const QString& tips) {
  static AgoraTipsDialog tips_dialog_;

  tips_dialog_.ui.TitleLabel->setText(title);
  tips_dialog_.ui.TipsLabel->setText(tips);
  return tips_dialog_.exec();
}

AgoraTipsDialog::AgoraTipsDialog(QWidget* parent) : QDialog(parent) {
  ui.setupUi(this);

  setWindowFlags(Qt::FramelessWindowHint);

  QObject::connect(ui.ExitPushButton, SIGNAL(clicked()), this,
                   SLOT(OnCancelPushButtonClicked()));
  QObject::connect(ui.CancelPushButton, SIGNAL(clicked()), this,
                   SLOT(OnCancelPushButtonClicked()));
  QObject::connect(ui.ConfirmPushButton, SIGNAL(clicked()), this,
                   SLOT(OnConfirmPushButtonClicked()));
}

AgoraTipsDialog::~AgoraTipsDialog() {}

void AgoraTipsDialog::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QBitmap bmp(this->size());
  bmp.fill();
  QPainter painter(&bmp);
  painter.setPen(Qt::black);
  painter.setBrush(Qt::black);
  painter.setRenderHints(QPainter::HighQualityAntialiasing |
                         QPainter::SmoothPixmapTransform);
  auto top_rect =
      QRect(ui.widget->rect().x() + 1, ui.widget->rect().y() + 1,
            ui.widget->rect().width() + 2, ui.widget->rect().height());
  auto center_rect =
      QRect(top_rect.x(), top_rect.y()+ top_rect.height(),
            ui.TipsLabel->rect().width()+2,
            ui.TipsLabel->rect().height());
  auto bottom_rect =
      QRect(center_rect.x(), center_rect.y() + center_rect.height(),
            ui.bottomWidget->rect().width()+2, ui.bottomWidget->rect().height()+2);
  QPainterPath path;
  path.setFillRule(Qt::WindingFill);
  path.addRoundedRect(top_rect, 8, 8);
  path.addRoundedRect(bottom_rect, 8, 8);
  QRect temp_top_rect(top_rect.left(), top_rect.top() + top_rect.height() / 2,
                      top_rect.width(), top_rect.height());
  QRect temp_bottom_rect(bottom_rect.left(), bottom_rect.top(),
                         bottom_rect.width(), bottom_rect.height() / 2);
  path.addRect(temp_top_rect);
  path.addRect(center_rect);
  path.addRect(temp_bottom_rect);
  painter.fillPath(path, QBrush(QColor(93, 201, 87)));
  setMask(bmp);
}

void AgoraTipsDialog::OnCancelPushButtonClicked() { reject(); }

void AgoraTipsDialog::OnConfirmPushButtonClicked() { accept(); }
