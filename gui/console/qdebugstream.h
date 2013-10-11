/*!
\file qdebugstream.h
\brief Definition file for class QDebugStream

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
*/

#ifndef QDEBUGSTREAM_H
#define QDEBUGSTREAM_H

class QTextEdit;

#include <iostream>
#include <streambuf>
#include <string>

namespace elabtsaot{

//! Class to redirect the output of a std::ostream to a QTextEdit
class QDebugStream : public std::basic_streambuf<char> {

 public:

  //! Constructor
  /*!
    \param stream to be redirected to the text_edit QTextEdit
    \param text_edit receives the redirection of the stream std::ostream
  */
  QDebugStream(std::ostream& stream, QTextEdit* text_edit);
  //! Destructor - resets the stream output back to its normal state
  virtual ~QDebugStream();

 protected:

  //! Reimplement parent class function that deals with overflow
  virtual int_type overflow(int_type v);
  //! Performs the actual redirection
  virtual std::streamsize xsputn(char const* p, std::streamsize n);

 private:

  //! Reference to the redirected stream
  std::ostream& m_stream;
  //! Old rdbuf of the redirected stream - stored for reseting purposes after
  //! the QDebugStream has been destroyed
  std::streambuf* m_old_buf;
  //! Auxiliary string
  std::string m_string;
  //! Output QTextEdit of the redirection
  QTextEdit* log_window;

};

} // end of namespace elabtsaot

#endif // QDEBUGSTREAM_H
