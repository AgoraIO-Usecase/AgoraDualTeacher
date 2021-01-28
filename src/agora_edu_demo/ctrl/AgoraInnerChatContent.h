#pragma once
#include <QColor>
#include <QVBoxLayout>
#include <QWidget>
#include <atomic>

static std::atomic<int> g_index{0};

struct ChatMessage {
  int index;
  QString title_text;
  QFont title_font;
  QString chat_text;
  QFont chat_font;
  QColor bk_color;
  int roundRadious;
  Qt::Alignment align;  // left right
  ChatMessage() {
    index = g_index++;
    title_font = QFont(QString::fromLocal8Bit("Î¢ÈíÑÅºÚ"), 10);
    title_text = "title_text";
    chat_font = QFont(QString::fromLocal8Bit("Î¢ÈíÑÅºÚ"), 12);
    chat_text = "chat_text";
    bk_color = QColor(255, 255, 255);
    roundRadious = 4;
    align = Qt::AlignLeft;
  }
};

class AgoraChatItem : public QWidget {
  Q_OBJECT
 public:
  explicit AgoraChatItem(QWidget *parent,ChatMessage message);
  ~AgoraChatItem() = default;

 protected:
  void paintEvent(QPaintEvent *event);

 private:
  ChatMessage m_chatMessage;
  int m_lines;
  int m_width;
  int m_height;
};

class AgoraInnerChatContent : public QWidget {
 public:
  Q_OBJECT

 public:
  AgoraInnerChatContent(QWidget *parent = nullptr);
  virtual ~AgoraInnerChatContent();
  void AddChatMessage(ChatMessage message);
  int GetPos();
 protected:
  void paintEvent(QPaintEvent *e);

 private:
  int m_pos;
  std::list<ChatMessage> m_chatMessages;
  QVBoxLayout *m_layout;
};
