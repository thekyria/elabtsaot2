/*!
\file precision_timer.h
\brief Definition file for class PrecisionTimer

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef PRECISIONTIMER_H
#define PRECISIONTIMER_H

#include <windows.h> // Required for type LARGE_INTEGER

namespace elabtsaot{

//! Precision time class
/*!
  Class implementing a precision timer.

  Example of use:
    PrecisionTimer timer;
    timer.Start();
    cout << "1: " << timer.Stop() << endl;
    timer.Start();

  \author thekyria
*/
class PrecisionTimer {

 public:

  //! Constructor
  PrecisionTimer() {
    QueryPerformanceFrequency(&lFreq);
  }

  //! Starts the counter
  inline void Start() {
    QueryPerformanceCounter(&lStart);
  }

  //! Stops the counter and returns the result in seconds
  /*!
    \return seconds elapsed from last PrecisionTimer::Start() to this stop
  */
  inline double Stop() {
    LARGE_INTEGER lEnd;
    QueryPerformanceCounter(&lEnd);
    return (double(lEnd.QuadPart - lStart.QuadPart) / lFreq.QuadPart);
  }

 private:

  LARGE_INTEGER lFreq, lStart;

};

} // end of namespace elabtsaot

#endif // PRECISIONTIMER_H
