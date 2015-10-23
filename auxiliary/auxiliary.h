/*!
\file auxiliary.h
\brief Header file containing inline definitions of useful utility functions

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef AUXILIARY_H
#define AUXILIARY_H

#include <vector> // std::vector
#include <string> // std::string
#include <sstream> // std::stringstream
#include <cstring> // std::memcpy
#include <iostream> // std::cout, std::endl, std::right, std::hex, std::dec
#include <cmath> // std::floor, std::ceil
#include <complex> // std::complex
#include <ctime> // std::time
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/random.hpp>
#include <windows.h> // required for winuser.h
#include <winuser.h> // MSG, ::PeekMessage, ::TranslateMessage, ::DispatchMessage
#include <stdint.h>

class QLayout;

namespace elabtsaot{

namespace auxiliary{

// ----- Functions implemented in auxiliary.cpp -----
/*! Logger for a vector (to output stream)
  Pretty prints the vector argument to the specified output stream

  \param vec vector to be pretty printed
  \param indexOffset index that will be printed for element vec[0]; that is
                     vec[0] will be printed out as: [indexOffset]: vec[0]
  \param mode mode accepts the following strings:
               - "none" default behaviour
               - "dec" decimal representation after 32bit binary word
               - "hex" hexadecimal representation after 32bit binary word
               - "both" both of the above
  \param str output stream

  \return integer exit code; 0 when successful */
int log_vector( std::vector<uint32_t> const& vec,
                int indexOffset,
                std::string const& mode,
                std::ostream& ostr );
/*! Logger for a vector (to file)
  Pretty prints the vector argument to the specified output file

  \param vec vector to be pretty printed
  \param indexOffset index that will be printed for element vec[0]; that is
                     vec[0] will be printed out as: [indexOffset]: vec[0]
  \param mode mode accepts the following strings:
               - "none" default behaviour
               - "dec" decimal representation after 32bit binary word
               - "hex" hexadecimal representation after 32bit binary word
               - "both" both of the above
  \param fname string with the filename of the file to be created where the
               configuration is to be written to

  \return integer exit code; 0 when successful */
int log_vector( std::vector<uint32_t> const& vec,
                int indexOffset,
                std::string const& mode,
                std::string const& fname);
//! Clears a Qt layout (QLayout) from all its contents
void clearQLayout(QLayout* layout);

// ----- Functions implemented with templates -----
//! Random number generator
template<typename T_>
T_ getRandom(T_ min, T_ max) {
  typedef typename boost::mpl::if_<
    boost::is_floating_point<T_>, // if we have a floating point type
    boost::uniform_real<>,        // use this, or
    boost::uniform_int<>          // else use this one
  >::type distro_type;

  boost::mt19937 seed( static_cast<unsigned int>(std::time(0)) );
  distro_type dist(min,max);
  boost::variate_generator<boost::mt19937&, distro_type > random(seed,dist);
  return random();
}

//! Converts the std::string representation of the input argument T t
/*! Internally based on std::stringstream::operator<< */
template<class T_>
std::string to_string(T_ const& t) {
  std::stringstream ss;
  ss << t;
  return ss.str();
}

template<class E, class T, class VE>
std::basic_ostream<E,T>& operator<<(std::basic_ostream<E,T>& os, std::vector<VE> const& v) {
  size_t size = v.size();
  std::basic_ostringstream<E, T, std::allocator<E> > s;
  s.flags (os.flags());
  s.imbue (os.getloc());
  s.precision (os.precision());
  s << '[' << size << "](";
  if (size > 0)
    s << v.at(0);
  for (size_t i = 1; i < size; ++ i)
    s << ',' << v.at(i);
  s << ')';
  return os << s.str().c_str ();
}

// ----- Functions implemented inline -----
//! MS Windows specific function to keep the calling process alive
inline void stayAlive(){
  MSG stMsg;
  while (::PeekMessage(&stMsg, NULL, 0, 0, PM_REMOVE)) {
    ::TranslateMessage(&stMsg);
    ::DispatchMessage(&stMsg);
  }
}

inline void sleep(int microseconds) {
    __int64 start, ticksPerSecond, tick;
    QueryPerformanceCounter((LARGE_INTEGER *)&start);
    QueryPerformanceFrequency((LARGE_INTEGER *)&ticksPerSecond);
    double wait =   static_cast<double>(ticksPerSecond)
                  * static_cast<double>(microseconds)/1000000.0;
    __int64 stop = start + static_cast<__int64>(ceil(wait));
    do {
      QueryPerformanceCounter((LARGE_INTEGER *)&tick);
    } while(tick<stop);
}

/*! Returns a string binary representation of a decimal number
  \param n decimal number to be converted to a binary string representation
  \return std::string binary representation of the decimal input argument */
inline std::string dec2bin(unsigned int n){
  size_t const size = sizeof(n) * 8;
  char result[size];

  unsigned index = size;
  do {
    result[--index] = '0' + (n & 1);
  } while (n >>= 1);

  return std::string(result + index, result + size);
}

//! Converts the std::string argument to a char array
inline char* str2chararr(std::string const& str){
  char* a = new char[str.size()+1];
  a[str.size()]=0;
  memcpy(a,str.c_str(),str.size());
  return a;
}

inline int str2int (std::string const& str) {
  std::stringstream ss(str);
  int n;
  ss >> n;
  return n;
}

/*! Symmetric rounding function;
  That's how the C99 round function works (rounding halfway numbers always away
  from zero).  e.g. The function will round -0.5 down to -1.0.
  You can also use floor(n + 0.5). It will round halfway numbers such as -0.5
  up to 0.0. */
inline double round(double r) {
  return (r > 0.0) ? floor(r + 0.5) : ceil(r - 0.5);
}

/*! Count decimals to first non zero digit
  e.g. countToFirstDecimal(0.002) -> 3
       countToFirstDecimal(0.0052) -> 3
       countToFirstDecimal(-1.102) -> 1 */
inline int countToFirstDecimal(double num){
  int count(0);
  num = std::abs(num);
  num -= round(num);
  while ( num < 1.0 ){
    num *= 10.0;
    ++count;
  }
  return count;
}

inline double deg2rad(double deg){ return deg*M_PI/180; } //!< Degrees to rad converter
inline double rad2deg(double rad){ return rad*180/M_PI; } //!< Rad to degrees conveter

/*! Display SHA1 Message Digest array
  Displays the 5 unsigned integers of a SHA1 message digest in human readable
  form
  \param sha1_digest SHA1 message digest of 5 unsigned ints
  \return integer exit code; 0 when successful */
inline int displaySHA1(unsigned int* sha1_digest) {

  std::ios::fmtflags flags;

  std::cout << '\t';

  flags = std::cout.setf(std::ios::hex|std::ios::uppercase,std::ios::basefield);
  std::cout.setf(std::ios::uppercase);

  for(int i = 0; i < 5 ; i++)
    std::cout << sha1_digest[i] << ' ';
  std::cout << std::endl;

  std::cout.setf(flags);

  return 0;
}

/*! Convert SHA1 message digest array to std::string
  \param sha1_digest SHA1 message digest of 5 unsigned ints
  \return std::string containing the 5 unsigned integers of the SHA1 message
          digest in human readable form */
inline std::string SHA1toStr( unsigned int* sha1_digest ){

  std::stringstream ss;
  std::ios::fmtflags flags;

  flags = ss.setf(std::ios::hex|std::ios::uppercase,std::ios::basefield);
  ss.setf(std::ios::uppercase);

  for(int i = 0; i < 5 ; i++)
    ss << sha1_digest[i] << ' ';
  ss << std::endl;

  ss.setf(flags);

  return ss.str();
}

template<class T_>
void invertVector(std::vector<std::vector<T_> >& vec ){

  // Find size of the 2d vector to be inverted
  size_t rows = vec.size();
  size_t cols = 0;
  for ( size_t r = 0 ; r != rows ; ++r )
    if ( vec[r].size() > cols )
      cols = vec[r].size();

  // Create temporary storage
  std::vector<std::vector<T_> > tmp(cols, std::vector<T_>(rows) );
  for ( size_t r = 0 ; r != rows ; ++r )
    for ( size_t c = 0 ; c != cols ; ++c )
      tmp[c][r] = vec[r][c];

  // Invert input(&output in the same time) argument
  vec = tmp;
}

inline void printVector(std::vector<std::vector<double> > vec ){
  for ( size_t k = 0 ; k != vec.size() ; ++k ){
    for ( size_t m = 0 ; m != vec[k].size() ; ++m ){
      std::cout << "("<<k<<","<<m<<"):" << vec[k][m] << " ";
    }
    std::cout << std::endl;
  }
}
inline void printVector(std::vector<std::vector<std::complex<double> > > vec){
  for ( size_t k = 0 ; k != vec.size() ; ++k ){
    for ( size_t m = 0 ; m != vec[k].size() ; ++m ){
      std::cout << "("<<k<<","<<m<<"):" << vec[k][m] << " ";
    }
    std::cout << std::endl;
  }
}
inline void print1DVector(std::vector<double> vec){
  for ( size_t k = 0 ; k != vec.size() ; ++k )
    std::cout << "("<<k<<"):" << vec[k] << " ";
  std::cout << std::endl;
}
inline void print1DVector(std::vector<std::complex<double> > vec){
  for ( size_t k = 0 ; k != vec.size() ; ++k )
    std::cout << "("<<k<<"):" << vec[k] << " ";
  std::cout << std::endl;
}

} // end of namespace auxiliary
} // end of namespace elabtsaot

#endif // AUXILIARY_H
