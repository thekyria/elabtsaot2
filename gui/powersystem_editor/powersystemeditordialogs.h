/*!
\file powersystemeditordialogs.h
\brief Definition file for namespace pwsEditorDialogs

This class is part of the elab-tsaot project in the Electronics Laboratory of
the Ecole Polytechnique Federal de Lausanne.

\author Fragkiskos Koufogiannis, fkoufis at gmail dot com
\author Theodoros Kyriakidis, thekyria at gmail dot com, Electronics Laboratory
EPFL
*/

#ifndef POWERSYSTEMEDITORDIALOGS_H
#define POWERSYSTEMEDITORDIALOGS_H

class QString;

namespace elabtsaot{

class Bus;
class Branch;
class Generator;
class Load;

namespace pwsEditorDialogs{

int slackDialog( int oldSlackBusExtId,
                 int oldSlackGenExtId,
                 int* newSlackGenExtId );
int busDialog( Bus* bus );
int branchDialog( Branch* branch );
int genDialog( Generator* gen );
int loadDialog( Load* load );

} // end of namespace pwsEditorDialogs

} // end of namespace elabtsaot

#endif // POWERSYSTEMEDITORDIALOGS_H
