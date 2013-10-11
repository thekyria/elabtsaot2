
#include "pwsmappermodelelement.h"
using namespace elabtsaot;

PwsMapperModelElement::PwsMapperModelElement(int type, int extId):
    PwsModelElement(type,extId), mapped(false), tab(-1), row(-1), col(-1) {}
