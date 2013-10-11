/*!
\file graph.h
\brief Header file containing inline definitions of useful graph functions

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef ELABTSAOT_GRAPH_H
#define ELABTSAOT_GRAPH_H

#include "powersystem.h"

#include <vector>   // std::vector, std::pair
#include <iostream> // std::cout, std::endl

namespace elabtsaot{
namespace graph{

//! Calculates a planarization of a given graph
std::vector< std::pair<int,int> >
planarizeGraph( int vertexCount, std::vector<std::pair<int,int> > e );
int countIslands( Powersystem const& pws );
bool isConnected( int vertexCount, std::vector<std::pair<int,int> > e );
bool isPlanar( Powersystem const& pws );
bool isPlanar( int vertexCount, std::vector<std::pair<int,int> > e );

} // end of namespace graph
} // end of namespace elabtsaot

#endif // ELABTSAOT_GRAPH_H
