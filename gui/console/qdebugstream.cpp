
#include "qdebugstream.h"
using namespace elabtsaot;

#include <QTextEdit>

QDebugStream::QDebugStream(std::ostream& stream, QTextEdit* text_edit) :
    m_stream(stream), m_old_buf(stream.rdbuf()), log_window(text_edit) {
  stream.rdbuf(this);
}

QDebugStream::~QDebugStream() {
  // output anything that is left
  if (!m_string.empty())
    log_window->append(m_string.c_str());

  m_stream.rdbuf(m_old_buf);
}

std::streambuf::int_type QDebugStream::overflow(int_type v) {
  if (v == '\n') {
    log_window->append(m_string.c_str());
    m_string.erase(m_string.begin(), m_string.end());
  } else
    m_string += v;

  return v;
}

std::streamsize QDebugStream::xsputn(char const* p, std::streamsize n) {
  m_string.append(p, p + n);

  size_t pos = 0;
  while (pos != std::string::npos) {
    pos = m_string.find('\n');
    if (pos != std::string::npos) {
      std::string tmp(m_string.begin(), m_string.begin() + pos);
      log_window->append(tmp.c_str());
      m_string.erase(m_string.begin(), m_string.begin() + pos + 1);
    }
  }

  return n;
}
