#include "AgoraInnerChatContent.h"
#include <QPainter>
#include <QVBoxLayout>

#define SPACER 4
#define CHAT_SPACER 10
#define MAX_LINE_WIDTH 246
AgoraChatItem::AgoraChatItem(QWidget* parent, ChatMessage message)
    : QWidget(parent) {
  m_chatMessage = message;
  QFontMetrics fm_title(m_chatMessage.title_font);
  QFontMetrics fm_chat(m_chatMessage.chat_font);
  m_width =
      fm_title.width(m_chatMessage.title_text) + SPACER * 2 + CHAT_SPACER * 2;
  int chat_text_width = fm_chat.width(m_chatMessage.chat_text);
  m_width = std::max(m_width, chat_text_width + SPACER * 2 + CHAT_SPACER * 2);
  int k = 0;
  m_lines = 0;
  while (k < m_chatMessage.chat_text.size()) {
    int len;
    for (len = 1; len <= m_chatMessage.chat_text.size() - k; len++) {
      if (fm_chat.width(m_chatMessage.chat_text.mid(k, len)) > MAX_LINE_WIDTH) {
        len--;
        break;
      }
    }
    m_lines++;
    k += len;
  }
  m_width = std::min(m_width, CHAT_SPACER + MAX_LINE_WIDTH + CHAT_SPACER);
  m_height = fm_title.height() + SPACER + CHAT_SPACER +
             fm_chat.height() * m_lines + (m_lines - 1) * SPACER + CHAT_SPACER;
  setMaximumWidth(m_width);
  setMaximumHeight(m_height);
  setMinimumWidth(m_width);
  setMinimumHeight(m_height);
}

void AgoraChatItem::paintEvent(QPaintEvent* event) {
  QPainter painter(this);
  QFontMetrics fm_title(m_chatMessage.title_font);
  QFontMetrics fm_chat(m_chatMessage.chat_font);
  painter.setRenderHints(QPainter::Antialiasing |
                         QPainter::SmoothPixmapTransform);
  QPen pen(1);
  QPoint pos;
  static QPen text_pen(1);
  text_pen.setColor(QColor(0x19, 0x19, 0x19));
  pos = QPoint(0, fm_title.height());
  painter.setBrush(m_chatMessage.bk_color);
  painter.setFont(m_chatMessage.title_font);
  painter.setPen(text_pen);
  painter.drawText(pos, m_chatMessage.title_text);
  pen.setColor(QColor(0xD7, 0xD9, 0xE6));
  painter.setPen(pen);
  painter.drawRoundedRect(
      QRect(pos.x(), fm_title.height() + SPACER, m_width - pos.x() - SPACER,
            m_height - (fm_title.height() + SPACER) - 2),
      m_chatMessage.roundRadious, m_chatMessage.roundRadious);
  painter.setFont(m_chatMessage.chat_font);
  painter.setPen(text_pen);

  int k = 0;
  for (int i = 0; i < m_lines; i++) {
    int len;
    for (len = 1; len < m_chatMessage.chat_text.size() - k; len++) {
      if (fm_chat.width(m_chatMessage.chat_text.mid(k, len)) > MAX_LINE_WIDTH) {
        break;
      }
    }
    if (i != m_lines - 1) len = len - 1;
    painter.drawText(
        QPoint(pos.x() + CHAT_SPACER, fm_title.height() + SPACER +
                                          fm_chat.height() + CHAT_SPACER / 2 +
                                          i * (SPACER + fm_chat.height())),
        m_chatMessage.chat_text.mid(k, len));
    k += len;
  }
}

AgoraInnerChatContent::AgoraInnerChatContent(QWidget* parent)
    : QWidget(parent) {
  setMaximumWidth(width());
  setMinimumWidth(width());
  m_layout = new QVBoxLayout(this);
  m_layout->setContentsMargins(10, 18, 10, 18);
  m_layout->setSizeConstraint(QVBoxLayout::SetFixedSize);
  this->setLayout(m_layout);
  m_pos = 0;
}

AgoraInnerChatContent::~AgoraInnerChatContent() {}

void AgoraInnerChatContent::AddChatMessage(ChatMessage message) {
  m_chatMessages.push_back(message);
  auto p = new AgoraChatItem(this, message);
  m_layout->addWidget(p, 0, message.align);
  p->show();
  m_pos += p->height();
  m_pos += 36;
}

int AgoraInnerChatContent::GetPos() { return m_pos; }

void AgoraInnerChatContent::paintEvent(QPaintEvent* e) {}
