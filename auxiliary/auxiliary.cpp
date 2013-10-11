
#include "auxiliary.h"
using namespace elabtsaot;

#include <fstream> // ofstream;
#include <QLayout>
#include <QWidget>

int auxiliary::log_vector( std::vector<uint32_t> vec,
                           int indexOffset,
                           std::string const& mode,
                           std::ostream& ostr ){

  for (int k = 0 ; k != static_cast<int>(vec.size()) ; ++k){
    if ( ostr.bad() )
      // Error outputing conf to ostr!
      return 1;
    ostr.width(4);
    ostr.fill('0');
    ostr << (k+indexOffset);
    ostr << " : ";
    ostr.width(32);
    ostr.fill('0');
    ostr << std::right << dec2bin( vec[k] );

    ostr << " ";
//    ostr.fill(' ');
    if ( mode == "none" ){ ; }
    else if ( mode == "dec" ){
      ostr.width(10);
      ostr << vec[k];
    } else if ( mode == "hex" ){
      ostr << "0x";
      ostr.width(8);
      ostr << std::hex << vec[k] << std::dec;
    } else if ( mode == "both" ){
      ostr.width(10);
      ostr << vec[k] << " ";
      ostr << "0x";
      ostr.width(8);
      ostr << std::hex << vec[k] << std::dec;
    } else { ; }
    ostr << std::endl;
  }

  ostr.fill(' ');
  return 0;
}

int auxiliary::log_vector( std::vector<uint32_t> vec,
                           int indexOffset,
                           std::string const& mode,
                           std::string const& fname) {

  // Convert string fname to char* a
  char* a = new char[fname.size()+1];
  a[fname.size()]=0;
  memcpy(a,fname.c_str(),fname.size());

  std::ofstream ofstr;
  ofstr.open( a , std::ios::trunc);
  if ( ofstr.is_open() ){
    for (int k = 0 ; k != static_cast<int>(vec.size()) ; ++k){
      if ( ofstr.bad() )
        // Error outputing conf to ofstr!
        return 3;
      ofstr.width(4);
      ofstr.fill('0');
      ofstr << (k+indexOffset);
      ofstr << " : ";
      ofstr.width(32);
      ofstr.fill('0');
      ofstr << std::right << dec2bin( vec[k] );

      ofstr << " ";
//      ofstr.fill(' ');
      if ( mode == "none" ){ ; }
      else if ( mode == "dec" ){
        ofstr.width(10);
        ofstr << vec[k];
      } else if ( mode == "hex" ){
        ofstr << "0x";
        ofstr.width(8);
        ofstr << std::hex << vec[k] << std::dec;
      }else if ( mode == "both" ){
        ofstr.width(10);
        ofstr << vec[k] << " ";
        ofstr << "0x";
        ofstr.width(8);
        ofstr << std::hex << vec[k] << std::dec;
      } else { ; }
      ofstr << std::endl;
    }
    ofstr.close();
  } else{
    // Error opening the file
    return 2;
  }

  return 0;
}

void auxiliary::clearQLayout(QLayout *layout){
  QLayoutItem *item;

  if ( layout != NULL ){
    while((item = layout->takeAt(0))) {
      if (item->layout()) {
        clearQLayout(item->layout());
        delete item->layout();
      }
      if (item->widget()) {
        delete item->widget();
      }
      if (item->spacerItem()){
        delete item->spacerItem();
      }
      delete item;
    }
  }
  return;
}
