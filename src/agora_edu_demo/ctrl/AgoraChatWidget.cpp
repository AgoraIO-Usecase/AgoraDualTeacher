#include "AgoraChatWidget.h"
#include <QScrollBar>
#include "AgoraInnerChatContent.h"
#include "util.h"
#include <QPainter>
#include <QBitmap>

AgoraChatWidget::AgoraChatWidget(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);
  setWindowFlags(Qt::FramelessWindowHint);
  setWindowTitle("chat room");

  ui.scrollArea->widget()->setMaximumHeight(height());
  ui.scrollArea->widget()->setMinimumHeight(height());

  ui.scrollArea->verticalScrollBar()->setStyleSheet(STR(
	QScrollBar:vertical {
		border: none;
		background-color: rgb(255,255,255);
		width: 6px;
		margin: 0px 0 0px 0;
	}

	QScrollBar::handle:vertical { 
		background:  #ECECF5;
		min-height: 20px;
		width: 6px;
		border: 1px solid  rgba(0, 0, 0, 51);
		border-radius: 4px;
		margin-left:4px;
		margin-right:4px;
	}

	QScrollBar::add-line:vertical {
		background-color: rgb(255,255,255);
		height: 4px;
	}

	QScrollBar::sub-line:vertical {
		background-color: rgb(255,255,255);
		height: 4px;
	}

	QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical {
		height: 0px;
	}
	QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
		background: none;
	}
  ));
  ui.scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  QObject::connect(ui.ExitPushButton, SIGNAL(clicked()), this,
                   SLOT(OnExitPushButtonClicked()));
}

void AgoraChatWidget::OnSendMessage() {
  QString data = ui.lineEdit->text();
  if (data.isEmpty()) return;
  ChatMessage message;
  message.title_text = str2qstr(user_name_);
  message.chat_text = data;
  static_cast<AgoraInnerChatContent *>(ui.scrollArea->widget())
      ->AddChatMessage(message);
  ui.lineEdit->clear();
  auto scroll_bar = ui.scrollArea->verticalScrollBar();
  if (scroll_bar != nullptr) {
    scroll_bar->setSliderPosition(
        static_cast<AgoraInnerChatContent *>(ui.scrollArea->widget())
            ->GetPos());
  }
  emit ChatMessageSendSig(message);
}

AgoraChatWidget::~AgoraChatWidget() {}

void AgoraChatWidget::AddMessage(const QString &title, const QString &msg,
                                 Qt::Alignment align) {
  ChatMessage message;
  message.title_text = title;
  message.chat_text = msg;
  message.align = align;
  static_cast<AgoraInnerChatContent *>(ui.scrollArea->widget())
      ->AddChatMessage(message);
  auto scroll_bar = ui.scrollArea->verticalScrollBar();
  if (scroll_bar != nullptr) {
    scroll_bar->setSliderPosition(
        static_cast<AgoraInnerChatContent *>(ui.scrollArea->widget())
            ->GetPos());
  }
}

void AgoraChatWidget::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
    OnSendMessage();
  }
}

void AgoraChatWidget::mousePressEvent(QMouseEvent *event) {
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

void AgoraChatWidget ::mouseMoveEvent(QMouseEvent *event) {
  if (is_drag_) {
    QPoint distance = event->globalPos() - mouse_start_point_;
    this->move(window_top_left_point_ + distance);
  }

  QWidget::mouseMoveEvent(event);
}

void AgoraChatWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    is_drag_ = false;
    setCursor(Qt::ArrowCursor);
  }

  QWidget::mouseReleaseEvent(event);
}

void AgoraChatWidget::paintEvent(QPaintEvent *event) {
  Q_UNUSED(event);
  QBitmap bmp(this->size());
  bmp.fill();
  QPainter painter(&bmp);
  painter.setPen(Qt::black);
  painter.setBrush(Qt::black);
  painter.setRenderHints(QPainter::HighQualityAntialiasing |
                         QPainter::SmoothPixmapTransform);
  auto top_rect =
      QRect(ui.top_widget->rect().x()+1 , ui.top_widget->rect().y()+1 ,
            ui.top_widget->rect().width(), ui.top_widget->rect().height());
  auto center_rect =
      QRect(top_rect.x(), top_rect.y() + top_rect.height(),
                           ui.scrollArea->rect().width(), ui.scrollArea->rect().height());
  auto bottom_rect =
      QRect(center_rect.x(), center_rect.y() + center_rect.height(),
            ui.bottomWidget->rect().width() ,
            ui.bottomWidget->rect().height() );
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

void AgoraChatWidget::OnExitPushButtonClicked() {
  hide();
  emit ChatWidgetExitSig(false);
}
