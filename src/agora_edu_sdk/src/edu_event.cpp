#include "edu_event.h"
#include <chrono>

namespace agora {
namespace edu {
EduEvent::EduEvent(bool initialState) : m_mutex(), m_cv() {
  m_setState.store(initialState);
}

EduEvent::~EduEvent() { m_setState.store(true); }

void EduEvent::Wait() {
  std::unique_lock<std::mutex> lk(m_mutex);
  m_cv.wait(lk, [this] { return (bool)this->m_setState; });
}

EduEvent::WaitResult EduEvent::WaitFor(unsigned int ms) {
  std::unique_lock<std::mutex> lk(m_mutex);
  bool result = m_cv.wait_for(lk, std::chrono::milliseconds(ms),
                              [this] { return (bool)this->m_setState; });
  return result ? WAIT_RESULT_SET : WAIT_RESULT_TIMEOUT;
}

void EduEvent::notifyOne() {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (!m_setState) {
    m_setState.store(true);
    m_cv.notify_one();
  }
}

void EduEvent::notifyAll() {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (!m_setState) {
    m_setState.store(true);
    m_cv.notify_all();
  }
}

void EduEvent::reset() {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (!m_setState) {
    m_setState.store(false);
  }
}
}  // namespace edu
}  // namespace agora