#include "AgoraChatWidget.h"
#include <QScrollBar>
#include "AgoraInnerChatContent.h"
#include "util.h"

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
    //获得鼠标的初始位置
    mouse_start_point_ = event->globalPos();
    // mouseStartPoint = event->pos();
    setCursor(Qt::OpenHandCursor);
  }

  QWidget::mousePressEvent(event);
}

void AgoraChatWidget ::mouseMoveEvent(QMouseEvent *event) {
  if (is_drag_) {
    //获得鼠标移动的距离
    QPoint distance = event->globalPos() - mouse_start_point_;
    // QPoint distance = event->pos() - mouseStartPoint;
    //改变窗口的位置
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

void AgoraChatWidget::OnExitPushButtonClicked() {
  hide();
  emit ChatWidgetExitSig(false);
}
