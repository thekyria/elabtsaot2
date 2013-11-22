
#include "importerexporter.h"
using namespace elabtsaot;

#include "messagehandler.h"
#include "powersystemxmlhandler.h"
#include "schematicxmlhandler.h"
#include "mappingxmlhandler.h"
#include "scenariosetxmlhandler.h"
#include "calibrationxmlhandler.h"
#include "pwsschematicmodel.h"
#include "pwsmappermodel.h"
#include "powersystem.h"
#include "emulatorhw.h"
#include "emulator.h"
#include "scenarioset.h"
#include "tdresults.h"
#include "auxiliary.h"
#include "calibrationeditor.h"

#include <QFileDialog>
#include <QFile>
#include <QtXmlPatterns>
#include <QXmlInputSource>
#include <QXmlStreamReader>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>



#include <string>
using std::string;
#include <iostream>
using std::ostream;
using std::ios;
using std::cout;
using std::endl;
#include <limits>
using std::numeric_limits;
#include <fstream>
using std::ofstream;

using std::vector;

int io::importProject(string filename_,
                      string& pwsfilename,
                      string& schfilename,
                      string& mapfilename,
                      string& scsfilename){
    QString filename(QString::fromStdString(filename_));
    QFile file( filename );
    if ( !file.open(QFile::ReadOnly | QFile::Text) ) // open() returns true if ok
        return 1;
    QTextStream fin(&file);
    fin.setCodec("UTF-8");

    QString temp;
    QString tempBuffer;
    // ----- start '$$' delimeter -----
    bool startDelimeterRead = false;
    while ( !startDelimeterRead ){
        temp = fin.readLine();
        if ( temp.isNull() )
            return 2; // EOF reach prematurely
        if ( temp.startsWith("$$") )
            startDelimeterRead = true;
    }

    // ----- title/description section -----
    string title;
    bool mainDelimeterRead = false;
    tempBuffer.clear();
    while ( !mainDelimeterRead ){
        temp = fin.readLine();
        if ( temp.isNull() )
            return 5; // EOF reach prematurely
        if ( temp.startsWith("$$") )
            mainDelimeterRead = true;
        else
            tempBuffer.append( temp );
    }
    title = tempBuffer.toStdString();

    // ----- main part -----
    QFileInfo fileinfo(filename);
    QDir dir( fileinfo.dir() );
    bool footerDelimeterRead = false;
    while ( !footerDelimeterRead ){
        temp = fin.readLine();
        temp = temp.simplified(); // change all whitespace to ASCII #32 (space)
        temp.replace( " ", "" );  // delete all whitespace
        if ( temp.isNull() ){
            return 7; // EOF reach prematurely
        } else if ( temp.startsWith("$$") ){
            footerDelimeterRead = true;
        } else {
            if ( temp.startsWith("pws") ){
                temp.remove(0,4);
                if ( temp.endsWith(".xml") )
                    pwsfilename = dir.absoluteFilePath(temp).toStdString();
            } else if ( temp.startsWith("sch") ){
                temp.remove(0,4);
                if ( temp.endsWith(".xml") )
                    schfilename = dir.absoluteFilePath(temp).toStdString();
            } else if ( temp.startsWith("map") ){
                temp.remove(0,4);
                if ( temp.endsWith(".xml") )
                    mapfilename = dir.absoluteFilePath(temp).toStdString();
            } else if ( temp.startsWith("scs") ){
                temp.remove(0,4);
                if ( temp.endsWith(".xml") )
                    scsfilename = dir.absoluteFilePath(temp).toStdString();
            }
        }
    }
    return 0;
}

int io::importPowersystem(string filename_, Powersystem* pws){
    QString filename(QString::fromStdString(filename_));

    if ( pws == NULL )
        return -1;

    // Check xml's consistency according to schema
    if( validateSchema( ":/xml_schemas/powersystem.xsd", filename_ ) ){
        cout << "The XML file is not built according to the powersystem schema."
             << endl;
        return 1;
    }

    QFile xmlFile( filename );
    QXmlInputSource source( &xmlFile );
    QXmlSimpleReader reader;
    Powersystem tempws("xml import power system", 100, 50);
    PowersystemXMLHandler* handler = new PowersystemXMLHandler( &tempws );
    reader.setContentHandler( handler );
    if ( !reader.parse( source )){
        cout << "Error parsing xml file!" << endl;
        return 2;
    }

    if( int ans = tempws.validate() ){
        cout << "Powersystem contained in the XML file cannot be validated" << endl;
        cout << "Exit code: " << ans << " Aborting ... " << endl;
        return 3;
    } else{
        // Powersystem properly validated - copy to the provided argument
        *pws = tempws;
    }

    return 0;
}

int io::logPowerSystem(Powersystem const& pws, ostream& ostr){

    if ( ostr.bad() )
        // Error writing to ostr!
        return 10;

    size_t k;                               // counter

    ostr.precision(5);        // sets the decimal precision to be used by output
    // operations at 5 significant digits
    //ostr.width(xxx);        // set field width to xxx BUT works only for the
    // next insertion operation; no turn around.
    ostr.setf ( ios::fixed, ios::floatfield );  // use fixed-point notation
    //ostr.setf ( ios::right, ios::adjustfield ); // adjust fields right (TODO!)

    ostr << "========================================" ;
    ostr << "=======================================" ;
    ostr << endl;
    ostr << "\tBus Data" << endl;
    ostr << "========================================" ;
    ostr << "=======================================" ;
    ostr << endl;
    ostr << endl;

    ostr.width(5);
    ostr << "Id";
    ostr.width(9);
    ostr << "V";
    ostr.width(9);
    ostr << "theta";
    ostr.width(9);
    ostr << "P";
    ostr.width(9);
    ostr << "Q" << endl;
    for (k=0; k!=pws.getBusCount(); ++k){
        Bus const* bus = pws.getBus(k);
        ostr.width(5);
        ostr << bus->extId;          // bus ext id
        ostr.width(9);
        ostr << bus->V;            // bus voltage magnitude
        ostr.width(9);
        ostr << bus->theta;           // bus voltage angle
        ostr.width(9);
        ostr << bus->P;            // P at bus
        ostr.width(9);
        ostr << bus->Q;            // Q at bus
        ostr << endl;
    }
    ostr << endl;

    ostr << "========================================" ;
    ostr << "=======================================" ;
    ostr << endl;
    ostr << "\tBranch Data (* denotes online)" << endl;
    ostr << "========================================" ;
    ostr << "=======================================" ;
    ostr << endl;
    ostr << endl;

    // TODO: fix I flows
    ostr.width(5);
    ostr << "Id";
    ostr.width(5);
    ostr << "from";
    ostr.width(5);
    ostr << "to";
    ostr.width(9);
    ostr << "P_f";
    ostr.width(9);
    ostr << "Q_f";
    ostr.width(9);
    ostr << "P_t";
    ostr.width(9);
    ostr << "Q_t";
    ostr.width(9);
    ostr << "I_f";
    ostr.width(9);
    ostr << "I_t";
    ostr << endl;
    for (k=0; k!=pws.getBranchCount(); ++k){
        Branch const* branch = pws.getBranch(k);
        if ( branch->status )
            ostr << "*";
        else
            ostr << " ";
        ostr.width(4);
        ostr << branch->extId;        // branch ext id
        ostr.width(5);
        ostr << branch->fromBusExtId; // branch from bus ext id
        ostr.width(5);
        ostr << branch->toBusExtId;   // branch to bus ext id
        ostr.width(9);
    }
    ostr << endl;

    ostr << "========================================" ;
    ostr << "=======================================" ;
    ostr << endl;
    ostr << "\tGenerator Data (* denotes online)" << endl;
    ostr << "========================================" ;
    ostr << "=======================================" ;
    ostr << endl;
    ostr << endl;

    ostr.width(5);
    ostr << "Id";
    ostr.width(9);
    ostr << "P";
    ostr.width(9);
    ostr << "Q";
    ostr.width(9);
    ostr << "E";
    ostr.width(9);
    ostr << "delta";
    ostr << endl;
    for (k=0; k!=pws.getGenCount(); ++k){
        Generator const* gen = pws.getGenerator(k);
        if (gen->status)
            ostr << "*";
        else
            ostr << " ";
        ostr.width(4);
        ostr << gen->extId;
        ostr.width(9);
        ostr << gen->Pgen;
        ostr.width(9);
        ostr << gen->Qgen;
        ostr.width(9);
        ostr << gen->Ess();
        ostr.width(9);
        ostr << gen->deltass();
        ostr << endl;
    }
    ostr << endl;

    return 0;
}

int io::logPowerSystem(Powersystem const& pws, string const& filename){

    // Convert string fname to char* a
    char *a=new char[filename.size()+1];
    a[filename.size()]=0;
    memcpy(a,filename.c_str(),filename.size());

    int ans(0);
    ofstream ofstr;
    ofstr.open(a, std::ios::trunc);
    if (ofstr.is_open()){
        ans = logPowerSystem(pws,ofstr);
        ofstr.close();
    } else {
        // Error opening the file
        return 2;
    }
    return ans;
}

int io::importSchematic( string filename_, PwsSchematicModel* smd ){
    QString filename(QString::fromStdString(filename_));

    if ( smd == NULL )
        return -1;

    // Check xml's consistency according to schema
    if( validateSchema( ":/xml_schemas/schematic.xsd", filename_ ) ){
        cout << "The XML file is not built according to the schematic schema."
             << endl;
        return 1;
    }

    QFile xmlFile( filename );
    QXmlInputSource source( &xmlFile );
    QXmlSimpleReader reader;
    PwsSchematicModel temp_smd;
    SchematicXMLHandler* handler = new SchematicXMLHandler( &temp_smd );
    reader.setContentHandler( handler );
    if ( !reader.parse( source )){
        cout << "Error parsing xml file!" << endl;
        return 2;
    }

    return smd->copy( temp_smd );
}

int io::importMapping( string filename_,
                       PwsMapperModel* mmd,
                       EmulatorHw const* emuhw,
                       Powersystem const* pws){
    QString filename(QString::fromStdString(filename_));

    if ( mmd == NULL )
        return -1;

    // Check xml's consistency according to schema
    if( validateSchema( ":/xml_schemas/mapping.xsd", filename_ ) ){
        cout << "The XML file is not built according to the mapping schema."
             << endl;
        return 1;
    }

    QFile xmlFile( filename );
    QXmlInputSource source( &xmlFile );
    QXmlSimpleReader reader;
    PwsMapperModel* temp_mmd = new PwsMapperModel( pws, emuhw );
    MappingXMLHandler* handler = new MappingXMLHandler( temp_mmd );
    reader.setContentHandler( handler );
    if ( !reader.parse( source )){
        cout << "Error parsing xml file!" << endl;
        return 2;
    }

    // Validate the imported mapping model against the emulator and the
    // powersystem provided as arguments
    int ans = temp_mmd->validate();
    if( ans ){
        cout << "Mapping contained in the XML file cannot be validated" << endl;
        cout << "Exit code: " << ans << " Aborting ... " << endl;
        return 3;
    }

    return mmd->copy( *temp_mmd );
}

int io::importEncoding(string filename_, size_t sliceId, Emulator& emu, bool force){
    QString filename(QString::fromStdString(filename_));
    QFile file( filename );
    if (!file.open(QFile::ReadOnly | QFile::Text)) // open() returns true if ok
        return 1;
    QTextStream fin(&file);
    fin.setCodec("UTF-8");

    QString temp;
    vector<uint32_t> tempEncoding;
    temp = fin.readLine();
    while (!temp.isNull()){
        bool ok(true);
        uint32_t tempUint = temp.toUInt(&ok,16);
        if (!ok){
            file.close();
            return 2;
        }
        tempEncoding.push_back(tempUint);
        temp = fin.readLine();
    }
    file.close(); // Close file

    // If force set, then emu.encoding is resized to accomodate the imported bitstream
    if (force)
        if (emu.encoding.size()<=sliceId)
            emu.encoding.resize(sliceId+1);

    if (tempEncoding.size()>0)
        emu.encoding.at(sliceId) = tempEncoding;

    return 0;
}

int io::importScenarioSet(string filename_, ScenarioSet* scs){
    QString filename(QString::fromStdString(filename_));

    if (scs==NULL) return -1;

    // Check xml's consistency according to schema
    if( validateSchema( ":/xml_schemas/scenariosset.xsd", filename_ ) ){
        cout << "The XML file is not built according to the scenario set schema."
             << endl;
        return 1;
    }

    QFile xmlFile( filename );
    QXmlInputSource source( &xmlFile );
    QXmlSimpleReader reader;
    ScenarioSet* temp_scs = new ScenarioSet();
    ScenarioSetXMLHandler* handler = new ScenarioSetXMLHandler( temp_scs );
    reader.setContentHandler( handler );
    if ( !reader.parse( source ) ){
        cout << "Error parsing xml file!" << endl;
        return 2;
    }

    *scs = *temp_scs;
    return 0;
}

int io::importTDResults( string filename_, TDResults* res ){
    QString filename(QString::fromStdString(filename_));

    if ( res == NULL )
        return -1;

    QFile file( filename );
    if ( !file.open(QFile::ReadOnly | QFile::Text) ) // open() returns true if ok
        return 1;
    QTextStream fin(&file);
    fin.setCodec("UTF-8");

    QString temp;
    QString tempBuffer;
    // Temp variables that correspond to a TDResults structure
    string sha1Digest;
    string title;
    string description;
    vector<bool> dataExists;
    vector<TDResultIdentifier> identifiers;
    vector<string> names;
    vector<string> units;
    vector<vector<double> > data;
    string footer;

    // ----- File beginning -----
    //  bool sha1DigestDelimeterRead = false;
    temp = fin.readLine();
    if ( !temp.startsWith("$$") )
        return 2;
    //  sha1DigestDelimeterRead = true;

    // ----- sha1digest section -----
    bool titleDelimeterRead = false;
    temp = fin.readLine();
    if ( !temp.contains("$$") ){
        sha1Digest = temp.toStdString();
    } else {
        if ( temp.startsWith("$$") ){
            sha1Digest = "UNKNOWN";
            titleDelimeterRead = true;
        } else {
            // Weird sha1Digest read!
            return 3;
        }
    }
    if ( !titleDelimeterRead ){
        temp = fin.readLine();
        if ( !temp.contains("$$") )
            return 4;
    }

    // ----- title section -----
    bool descDelimeterRead = false;
    tempBuffer.clear();
    while ( !descDelimeterRead ){
        temp = fin.readLine();
        if ( temp.isNull() )
            return 5; // EOF reach prematurely
        if ( temp.startsWith("$$") )
            descDelimeterRead = true;
        else
            tempBuffer.append( temp );
    }
    title = tempBuffer.toStdString();

    // ----- desc section -----
    bool formatDelimeterRead = false;
    tempBuffer.clear();
    while ( !formatDelimeterRead ){
        temp = fin.readLine();
        if ( temp.isNull() )
            return 6; // EOF reach prematurely
        if ( temp.startsWith("$$") )
            formatDelimeterRead = true;
        else
            tempBuffer.append( temp );
    }
    description = tempBuffer.toStdString();

    // ----- format section -----
    bool dataDelimeterRead = false;
    while ( !dataDelimeterRead ){
        temp = fin.readLine();
        if ( temp.isNull() ){
            return 7; // EOF reach prematurely
        } else if ( temp.startsWith("$$") ){
            dataDelimeterRead = true;
        } else {
            // Parse format section proper
            QStringList list = temp.split( QRegExp("\\s*\\$\\s*") );
            if ( list.size() < 7 )
                return 8; // inconsistent format-line format
            dataExists.push_back( true );
            names.push_back( list.at(0).toStdString() );
            units.push_back( list.at(1).toStdString() );
            TDResultIdentifier tempTdri;
            bool ok = true;
            tempTdri.elementType = list.at(2).toInt(&ok);
            if ( !ok ) return 81;
            tempTdri.variable = list.at(3).toInt(&ok);
            if ( !ok ) return 82;
            tempTdri.extId = list.at(4).toInt(&ok);
            if ( !ok ) return 83;
            identifiers.push_back( tempTdri );
        }
    }
    int numberOfVariables = dataExists.size();
    data.resize( numberOfVariables );

    // ----- data section -----
    bool footerDelimeterRead = false;
    while ( !footerDelimeterRead ){
        temp = fin.readLine();
        if ( temp.isNull() ){
            return 9; // EOF reached prematurely
        } else if ( temp.startsWith("$$") ){
            footerDelimeterRead = true;
        } else {
            // Parse data section proper
            QStringList list = temp.split( QRegExp("\\s*\\$\\s*") );
            if ( list.size() < numberOfVariables )
                return 10; // malformed data format line
            bool ok = true;
            for ( int k = 0 ; k != numberOfVariables ; ++k ){
                data[k].push_back( list[k].toDouble(&ok) );
                if ( !ok ) return 11; // malformed data format line
            }
        }
    }

    // ----- footer section -----
    temp = fin.readLine();
    while ( !temp.isNull() ){
        footer.append( temp.toStdString() );
        temp = fin.readLine();
    }

    // Close file
    file.close();

    // Write temp variables read from the file into the TDResults structure
    TDResults tempRes( sha1Digest, title, description, dataExists, identifiers,
                       names, units, data, footer );
    *res = tempRes;

    return 0;
}

int io::importCalibrationValues(string filename_, CalibrationEditor* cal){
    QString filename(QString::fromStdString(filename_));
    cout << "Importing from filename: " << filename.toStdString() << endl;

    // Cal is the object pointer of calibrator in the mainwindow
    // We need the cal to writeback the xml
    cal->hardreset();
    if ( cal == NULL)
        return -1;

    // TODO TODO TODO Check xml's consistency according to schema

    QFile xmlfile( filename );
    if (xmlfile.exists()){
        QDomDocument doc("XMLFile");
        if(!xmlfile.open(QIODevice::ReadOnly))
            return 0;
        if( !doc.setContent( &xmlfile ) )
        {
            xmlfile.close();
            return 0;
        }
        xmlfile.close();

        QDomElement root = doc.documentElement(); //Take the root
        QDomNode n1,n2,n3,node ,n5,realimag,test, data; //6 nodes

        n1 = root.firstChild(); //<Info>
        cout << "Number of device in XML: " << n1.namedItem("Devicenumber").toElement().text().toStdString() <<endl;
        n1 = n1.nextSibling(); //<Devices>
        n2 = n1.firstChild(); //<Device>
        while( !n2.isNull() ) //Enumerate the devices
        {
            QVector<QString> namereal;
            QVector<double> offsetreal;
            QVector<double> gainreal;
            QVector<double> rabreal;
            QVector<double> rwreal;

            QVector<QString> nameimag;
            QVector<double> offsetimag;
            QVector<double> gainimag;
            QVector<double> rabimag;
            QVector<double> rwimag;

            QVector <QVector<QString> > names;
            QVector <QVector<double> > offsets;
            QVector <QVector<double> > gains;
            QVector <QVector<double> > rab;
            QVector <QVector<double> > rw;
            QString devName;

            n3 = n2.namedItem("Id");
            string devid = n3.toElement().text().toStdString();
            n3 = n2.namedItem("Name");
            string name = n3.toElement().text().toStdString();
            cout << "Importing device: " << devid << " with name " <<  name << endl;
            devName = QString::fromStdString(name);

            n3 = n2.namedItem("Nodes"); //Nodes


            QVector<QString> TEST_NAMES;
            TEST_NAMES.append("ADC");//0
            TEST_NAMES.append("DAC_ADC"); //1
            TEST_NAMES.append("Convertion");//2
            TEST_NAMES.append("Internal");//3
            TEST_NAMES.append("P0Chip1-2");//4
            TEST_NAMES.append("P1Chip1-2");//5
            TEST_NAMES.append("P2Chip1-2");//6
            TEST_NAMES.append("P3Chip1-2");//7
            TEST_NAMES.append("P0P2Chip3");//8
            TEST_NAMES.append("P1P3Chip3");//9
            TEST_NAMES.append("P0P2EXT");//10
            TEST_NAMES.append("P1P3EXT");//11

            QVector <QString> REAL_IMAG;
            REAL_IMAG.append("Real");
            REAL_IMAG.append("Imag");



            for(size_t i=0; i!=TEST_NAMES.size();++i){
                node = n3.namedItem("Node"); //Node
                while( !node.isNull() ) //Enumerate the nodes
                {
                    QString nodeid(node.namedItem("Id").toElement().text());
                    n5 = node.namedItem("Data"); //Data
                    // ----------------- REAL ------------------------------------
                    for (size_t ri = 0; ri!=REAL_IMAG.size(); ++ri){
                        realimag = n5.namedItem(REAL_IMAG[ri]);

                        test = realimag.namedItem(TEST_NAMES[i]);
                        if (!test.isNull()){
                            QString testname(TEST_NAMES[i]);
                            data = test.namedItem("Offset");
                            if (!data.isNull()){
                                namereal.append(testname.append(" real ").append(" of node ").append(nodeid));
                                offsetreal.append(data.toElement().text().toDouble());
                            }
                            data = test.namedItem("Gain");
                            if (!data.isNull()){//Only second has gain, the first is just 0
                                gainreal.append(data.toElement().text().toDouble());
                            }else if(data.isNull() && i ==0){
                                gainreal.append(0); //Zero for the first test
                            }

                            data = test.namedItem("rab");
                            if (!data.isNull()){
                                namereal.append(testname.append(" real ").append(" of node ").append(nodeid));
                                rabreal.append(data.toElement().text().toDouble());
                            }
                            data = test.namedItem("rw");
                            if (!data.isNull()){
                                rwreal.append(data.toElement().text().toDouble());
                            }
                        }
                    }
                    node = node.nextSibling();
                }
            }



            n2 = n2.nextSibling(); //Next device

        }

    } else {
        cout << "File not found" <<endl;
        return 0;
    }


    return 1;
}

int io::exportPowersystem( string filename, Powersystem const* pws){

    FILE *f;
    char name[200];
    strcpy(name, filename.c_str());
    f = fopen( name, "w" );

    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
    fprintf(f, "<powersystem xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n");

    // --------------- Output network's info ---------------
    fprintf(f, "<info>\n");
    fprintf(f, "\t<name> %s </name>\n", pws->name.c_str());
    fprintf(f, "\t<baseMVA> %.12f </baseMVA>\n", pws->baseS);
    fprintf(f, "\t<baseFreq> %.12f </baseFreq>\n", pws->baseF);
    fprintf(f, "</info>\n");
    fflush(f);

    // --------------- Output buses ---------------
    fprintf(f, "<buses>\n");
    for( size_t k = 0 ; k != pws->getBusCount(); ++k ){
        // Get bus at pos (intId) k
        Bus const* bus = pws->getBus(k);
        // Write its parameters to the xml file
        fprintf(f, "\t<bus>\n");
        fprintf(f, "\t\t<extId> %d </extId>\n", bus->extId);
        fprintf(f, "\t\t<name> %s </name>\n", bus->name.c_str());
        fprintf(f, "\t\t<type> %d </type>\n", bus->type);
        fprintf(f, "\t\t<Gsh> %.12f </Gsh>\n", bus->Bsh);
        fprintf(f, "\t\t<Bsh> %.12f </Bsh>\n", bus->Gsh);
        fprintf(f, "\t\t<baseKV> %.12f </baseKV>\n", bus->baseKV);
        fprintf(f, "\t\t<P> %.12f </P>\n", bus->P);
        fprintf(f, "\t\t<Q> %.12f </Q>\n", bus->Q);
        fprintf(f, "\t\t<V> %.12f </V>\n", bus->V);
        fprintf(f, "\t\t<theta> %.12f </theta>\n", bus->theta);
        fprintf(f, "\t</bus>\n");
        fflush(f);
    }
    fprintf(f, "</buses>\n\n");

    // --------------- Output branches --------------------
    fprintf(f, "<branches>\n");
    for( size_t k = 0 ; k != pws->getBranchCount() ; ++k ){
        // Get branch at pos (intId) k
        Branch const* br = pws->getBranch(k);
        // Write its parameters to the xml file
        fprintf(f, "\t<branch>\n");
        fprintf(f, "\t\t<extId> %d </extId>\n", br->extId);
        fprintf(f, "\t\t<status> %d </status>\n", static_cast<int>( br->status ) );
        fprintf(f, "\t\t<fromBusExtId> %d </fromBusExtId>\n", br->fromBusExtId);
        fprintf(f, "\t\t<toBusExtId> %d </toBusExtId>\n", br->toBusExtId);
        fprintf(f, "\t\t<R> %.12f </R>\n", br->R);
        fprintf(f, "\t\t<X> %.12f </X>\n", br->X);
        fprintf(f, "\t\t<Bfrom> %.12f </Bfrom>\n", br->Bfrom);
        fprintf(f, "\t\t<Bto> %.12f </Bto>\n", br->Bto);
        fprintf(f, "\t\t<Gfrom> %.12f </Gfrom>\n", br->Gfrom);
        fprintf(f, "\t\t<Gto> %.12f </Gto>\n", br->Gto);
        fprintf(f, "\t\t<Xratio> %.12f </Xratio>\n", br->Xratio);
        fprintf(f, "\t\t<Xshift> %.12f </Xshift>\n", br->Xshift);
        fprintf(f, "\t</branch>\n");
        fflush(f);
    }
    fprintf(f, "</branches>\n\n");

    // -------------------- Output generators --------------------
    fprintf(f, "<generators>\n");
    for( size_t k = 0 ; k != pws->getGenCount() ; ++k ){
        // Get generator at pos (intId) k
        Generator const* gen = pws->getGenerator(k);
        // Write its parameters to the xml file
        fprintf(f, "\t<generator>\n");

        fprintf(f, "\t\t<extId> %d </extId>\n", gen->extId);
        fprintf(f, "\t\t<status>%d</status>\n", static_cast<int>(gen->status) );
        fprintf(f, "\t\t<name> %s </name>\n", gen->name.c_str());
        fprintf(f, "\t\t<busExtId> %d </busExtId>\n", gen->busExtId );

        fprintf(f, "\t\t<Pgen> %.12f </Pgen>\n", gen->Pgen);
        fprintf(f, "\t\t<Qgen> %.12f </Qgen>\n", gen->Qgen);
        fprintf(f, "\t\t<Vss_real> %.12f </Vss_real>\n", gen->Vss.real());
        fprintf(f, "\t\t<Vss_imag> %.12f </Vss_imag>\n", gen->Vss.imag());

        fprintf(f, "\t\t<model> %d </model>\n", gen->model);
        fprintf(f, "\t\t<xl> %.12f </xl>\n", gen->xl);
        fprintf(f, "\t\t<ra> %.12f </ra>\n", gen->ra);
        fprintf(f, "\t\t<xd> %.12f </xd>\n", gen->xd);
        fprintf(f, "\t\t<xd_1> %.12f </xd_1>\n", gen->xd_1);
        fprintf(f, "\t\t<xd_2> %.12f </xd_2>\n", gen->xd_2);
        fprintf(f, "\t\t<Td0_1> %.12f </Td0_1>\n", gen->Td0_1);
        fprintf(f, "\t\t<Td0_2> %.12f </Td0_2>\n", gen->Td0_2);
        fprintf(f, "\t\t<xq> %.12f </xq>\n", gen->xq);
        fprintf(f, "\t\t<xq_1> %.12f </xq_1>\n", gen->xq_1);
        fprintf(f, "\t\t<xq_2> %.12f </xq_2>\n", gen->xq_2);
        fprintf(f, "\t\t<Tq0_1> %.12f </Tq0_1>\n", gen->Tq0_1);
        fprintf(f, "\t\t<Tq0_2> %.12f </Tq0_2>\n", gen->Tq0_2);
        fprintf(f, "\t\t<M> %.12f </M>\n", gen->M);
        fprintf(f, "\t\t<D> %.12f </D>\n", gen->D);

        fprintf(f, "\t</generator>\n");
        fflush(f);
    }
    fprintf(f, "</generators>\n\n");

    // --------------- Output loads ---------------
    fprintf(f, "<loads>\n");
    for( size_t k = 0 ; k != pws->getLoadCount() ; ++k ){
        // Get load at pos (intId) k
        Load const* load = pws->getLoad(k);
        // And write its parameters to the xml file
        fprintf(f, "\t<load>\n");

        fprintf(f, "\t\t<extId> %d </extId>\n", load->extId);
        fprintf(f, "\t\t<busExtId> %d </busExtId>\n", load->busExtId);
        fprintf(f, "\t\t<Pdemand> %.12f </Pdemand>\n", load->Pdemand);
        fprintf(f, "\t\t<Qdemand> %.12f </Qdemand>\n", load->Qdemand);
        fprintf(f, "\t\t<Vss_real> %.12f </Vss_real>\n", load->Vss.real());
        fprintf(f, "\t\t<Vss_imag> %.12f </Vss_imag>\n", load->Vss.imag());
        fprintf(f, "\t\t<Vexpa> %.12f </Vexpa>\n", load->Vexpa);
        fprintf(f, "\t\t<Vexpb> %.12f </Vexpb>\n", load->Vexpb);
        fprintf(f, "\t\t<kpf> %.12f </kpf>\n", load->kpf);
        fprintf(f, "\t\t<kqf> %.12f </kqf>\n", load->kqf);

        fprintf(f, "\t</load>\n");
        fflush(f);
    }
    fprintf(f, "</loads>\n\n");
    fprintf(f, "</powersystem>\n");
    fflush(f);
    fclose(f);

    return 0;
}

int io::exportSchematic( string filename, PwsSchematicModel const* smd ){

    FILE *f;
    char name[200];
    strcpy(name, filename.c_str());
    f = fopen( name, "w" );

    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
    fprintf(f, "<schematic xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n");
    fflush(f);

    size_t mdlId;
    PwsSchematicModelElement const* cd;
    // --------------- Write bus information ---------------
    for( mdlId = 0; mdlId != smd->busElements_size(); ++mdlId ){
        cd = smd->elementByIndex(PWSMODELELEMENTTYPE_BUS, mdlId);

        fprintf(f, "<element>\n");
        fprintf(f, "\t<type> %d </type>\n", cd->type );
        fprintf(f, "\t<id> %d </id>\n", cd->extId);
        fprintf(f, "\t<x1> %.12f </x1>\n", cd->x1);
        fprintf(f, "\t<y1> %.12f </y1>\n", cd->y1);
        fprintf(f, "\t<x2> %.12f </x2>\n", cd->x2);
        fprintf(f, "\t<y2> %.12f </y2>\n", cd->y2);
        fprintf(f, "</element>\n");
        fflush(f);
    }

    // --------------- Write branch information ---------------
    for( mdlId = 0; mdlId!=smd->branchElements_size(); ++mdlId ){
        cd = smd->elementByIndex(PWSMODELELEMENTTYPE_BR, mdlId);

        fprintf(f, "<element>\n");
        fprintf(f, "\t<type> %d </type>\n", cd->type);
        fprintf(f, "\t<id> %d </id>\n", cd->extId);
        fprintf(f, "\t<x1> %.12f </x1>\n", cd->x1);
        fprintf(f, "\t<y1> %.12f </y1>\n", cd->y1);
        fprintf(f, "\t<x2> %.12f </x2>\n", cd->x2);
        fprintf(f, "\t<y2> %.12f </y2>\n", cd->y2);
        fprintf(f, "</element>\n");
        fflush(f);
    }

    // --------------- Write generator information ---------------
    for( mdlId = 0; mdlId!=smd->genElements_size(); ++mdlId ){
        cd = smd->elementByIndex(PWSMODELELEMENTTYPE_GEN, mdlId);

        fprintf(f, "<element>\n");
        fprintf(f, "\t<type> %d </type>\n", cd->type);
        fprintf(f, "\t<id> %d </id>\n", cd->extId);
        fprintf(f, "\t<x1> %.12f </x1>\n", cd->x1);
        fprintf(f, "\t<y1> %.12f </y1>\n", cd->y1);
        fprintf(f, "\t<x2> %.12f </x2>\n", cd->x2);
        fprintf(f, "\t<y2> %.12f </y2>\n", cd->y2);
        fprintf(f, "</element>\n");
        fflush(f);
    }

    // --------------- Write load information ---------------
    for( mdlId = 0; mdlId!=smd->loadElements_size(); ++mdlId ){
        cd = smd->elementByIndex(PWSMODELELEMENTTYPE_LOAD, mdlId);

        fprintf(f, "<element>\n");
        fprintf(f, "\t<type> %d </type>\n", cd->type);
        fprintf(f, "\t<id> %d </id>\n", cd->extId);
        fprintf(f, "\t<x1> %.12f </x1>\n", cd->x1);
        fprintf(f, "\t<y1> %.12f </y1>\n", cd->y1);
        fprintf(f, "\t<x2> %.12f </x2>\n", cd->x2);
        fprintf(f, "\t<y2> %.12f </y2>\n", cd->y2);
        fprintf(f, "</element>\n");
        fflush(f);
    }

    fprintf(f, "</schematic>\n");
    fflush(f);
    fclose(f);

    return 0;
}

int io::exportMapping( string filename, PwsMapperModel const* mmd ){

    FILE *f;
    char name[200];
    strcpy(name, filename.c_str());
    f = fopen( name, "w" );

    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
    fprintf(f, "<mapping xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n");
    fflush(f);

    size_t mdlId;
    PwsMapperModelElement const* cd;
    // --------------- Write bus information ---------------
    for( mdlId = 0; mdlId != mmd->busElements_size(); ++mdlId ){
        cd = mmd->elementByIndex(PWSMODELELEMENTTYPE_BUS, mdlId);

        fprintf(f, "<element>\n");
        fprintf(f, "\t<type> %d </type>\n", cd->type );
        fprintf(f, "\t<id> %d </id>\n", cd->extId);
        fprintf(f, "\t<mapped> %d </mapped>\n", static_cast<int>(cd->mapped));
        fprintf(f, "\t<tab> %d </tab>\n", cd->tab);
        fprintf(f, "\t<row> %d </row>\n", cd->row);
        fprintf(f, "\t<col> %d </col>\n", cd->col);
        fprintf(f, "</element>\n");
        fflush(f);
    }

    // --------------- Write branch information ---------------
    for( mdlId = 0; mdlId!=mmd->branchElements_size(); ++mdlId ){
        cd = mmd->elementByIndex(PWSMODELELEMENTTYPE_BR, mdlId);

        fprintf(f, "<element>\n");
        fprintf(f, "\t<type> %d </type>\n", cd->type);
        fprintf(f, "\t<id> %d </id>\n", cd->extId);
        fprintf(f, "\t<mapped> %d </mapped>\n", static_cast<int>(cd->mapped));
        fprintf(f, "\t<tab> %d </tab>\n", cd->tab);
        fprintf(f, "\t<row> %d </row>\n", cd->row);
        fprintf(f, "\t<col> %d </col>\n", cd->col);
        fprintf(f, "</element>\n");
        fflush(f);
    }

    // --------------- Write generator information ---------------
    for( mdlId = 0; mdlId!=mmd->genElements_size(); ++mdlId ){
        cd = mmd->elementByIndex(PWSMODELELEMENTTYPE_GEN, mdlId);

        fprintf(f, "<element>\n");
        fprintf(f, "\t<type> %d </type>\n", cd->type);
        fprintf(f, "\t<id> %d </id>\n", cd->extId);
        fprintf(f, "\t<mapped> %d </mapped>\n", static_cast<int>(cd->mapped));
        fprintf(f, "\t<tab> %d </tab>\n", cd->tab);
        fprintf(f, "\t<row> %d </row>\n", cd->row);
        fprintf(f, "\t<col> %d </col>\n", cd->col);
        fprintf(f, "</element>\n");
        fflush(f);
    }

    // --------------- Write load information ---------------
    for( mdlId = 0; mdlId!=mmd->loadElements_size(); ++mdlId ){
        cd = mmd->elementByIndex(PWSMODELELEMENTTYPE_LOAD, mdlId);

        fprintf(f, "<element>\n");
        fprintf(f, "\t<type> %d </type>\n", cd->type);
        fprintf(f, "\t<id> %d </id>\n", cd->extId);
        fprintf(f, "\t<mapped> %d </mapped>\n", static_cast<int>(cd->mapped));
        fprintf(f, "\t<tab> %d </tab>\n", cd->tab);
        fprintf(f, "\t<row> %d </row>\n", cd->row);
        fprintf(f, "\t<col> %d </col>\n", cd->col);
        fprintf(f, "</element>\n");
        fflush(f);
    }

    fprintf(f, "</mapping>\n");
    fflush(f);
    fclose(f);

    return 0;
}

int io::exportScenarioSet( string filename, ScenarioSet const* scs ){

    FILE *f;
    char name[200];
    strcpy(name, filename.c_str());
    f = fopen( name, "w" );

    fprintf(f, "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
    fprintf(f, "<scenarioSet xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n");
    fprintf(f, "\n");

    // --------------- Output scenarioSet's general info ---------------
    fprintf(f, "<extId> %d </extId>\n", scs->extId() );
    fprintf(f, "<name> %s </name>\n", scs->name().c_str() );
    fprintf(f, "<description> %s </description>\n", scs->description().c_str() );
    fprintf(f, "\n");
    fflush(f);

    // --------------- Output scenarios ---------------
    for ( size_t k = 0 ; k != scs->scenarios.size() ; ++k ){
        fprintf(f, "<scenario>\n");

        fprintf(f, "\t<extId> %d </extId>\n", scs->scenarios[k].extId() );
        fprintf(f, "\t<name> %s </name>\n", scs->scenarios[k].name().c_str() );
        fprintf(f, "\t<description> %s </description>\n", scs->scenarios[k].description().c_str() );
        fprintf(f, "\t<startTime> %.12f </startTime>\n", scs->scenarios[k].startTime() );
        fprintf(f, "\t<stopTime> %.12f </stopTime>\n", scs->scenarios[k].stopTime() );
        fprintf(f, "\t<status> %d </status>\n", scs->scenarios[k].status() );
        fflush(f);

        for ( size_t m = 0 ; m != scs->scenarios[k].getEventSetSize() ; ++m ){
            fprintf(f, "\t<event>\n");
            Event evn = scs->scenarios[k].getEvent(m);

            fprintf(f, "\t\t<name> %s </name>\n", evn.name().c_str() );
            fprintf(f, "\t\t<time> %.12f </time>\n", evn.time() );
            fprintf(f, "\t\t<description> %s </description>\n", evn.description().c_str() );
            fprintf(f, "\t\t<status> %d </status>\n", evn.status() );
            fprintf(f, "\t\t<element_type> %d </element_type>\n", evn.element_type() );
            fprintf(f,"\t\t<element_extId> %d </element_extId>\n", evn.element_extId());
            fprintf(f, "\t\t<event_type> %d </event_type>\n", evn.event_type() );
            fprintf(f, "\t\t<bool_arg> %d </bool_arg>\n", evn.bool_arg() );
            fprintf(f, "\t\t<double_arg> %.12f </double_arg>\n", evn.double_arg() );
            fprintf(f,"\t\t<double_arg_1> %.12f </double_arg_1>\n", evn.double_arg_1());
            fprintf(f,"\t\t<double_arg_2> %.12f </double_arg_2>\n", evn.double_arg_2());
            fprintf(f,"\t\t<uint_arg> %d </uint_arg>\n", evn.uint_arg());

            fprintf(f, "\t</event>\n");
            fprintf(f, "\n");
            fflush(f);
        }

        fprintf(f, "</scenario>\n");
        fprintf(f, "\n");
        fflush(f);
    }
    fprintf(f, "\n");

    fprintf(f, "</scenarioSet>\n");
    fflush(f);
    fclose(f);

    return 0;
}

/*! \todo for large results chunks it crashes */
int io::exportTDResults( string filename, TDResults const* res ){

    // Open file
    FILE *f;
    char name[200];
    strcpy(name, filename.c_str());
    f = fopen( name, "w" );

    // Retrieve references to TDResults members
    vector<bool> const& dataExists( res->dataExists() );
    vector<vector<double> > const& data( res->data() );

    // ----- SHA1 digest -----
    fprintf(f, "$$\n");
    //  fprintf(f, "%s\n", res->sha1Digest().c_str() );
    fprintf(f, "%s", res->sha1Digest().c_str() ); // sha1Digest() string has \n

    // ----- Title -----
    fprintf(f, "$$\n");
    fprintf(f, "%s\n", res->title().c_str() );

    // ----- Header section -----
    fprintf(f, "$$\n");
    fprintf(f, "Description %s\n", res->description().c_str() );

    // ----- Format section -----
    fprintf(f, "$$\n");
    fflush(f);
    for ( size_t k = 0 ; k != dataExists.size() ; ++k ){
        if ( !dataExists.at(k) )
            continue;
        fprintf(f, "%s $ ", res->names().at(k).c_str());
        fprintf(f, "%s $ ", res->units().at(k).c_str());
        fprintf(f, "%d $ ", res->identifiers().at(k).elementType);
        fprintf(f, "%d $ ", res->identifiers().at(k).variable);
        fprintf(f, "%d $ ", res->identifiers().at(k).extId);
        fprintf(f, "2 $ 9\n"); // Q2.9 format for all results // TODO change!
        fflush(f);
        auxiliary::stayAlive();
    }

    // ----- Data section -----
    fprintf(f, "$$\n");
    // Find the minimum length of the data vectors
    // Actually all vectors should normally be of the same length
    size_t minDataLength = numeric_limits<int>::max();
    for ( size_t k = 0 ; k != data.size() ; ++k ){
        if ( !dataExists.at(k) )
            continue;
        if ( data.at(k).size() < minDataLength )
            minDataLength = data.at(k).size();
    }
    // Output results to file
    for ( size_t k = 0 ; k != minDataLength ; ++k ){
        for ( size_t m = 0 ; m != data.size() ; ++m ){
            if ( !dataExists.at(m) )
                continue;
            fprintf(f, "%.9f $", data[m][k]);
            fflush(f);
            auxiliary::stayAlive();
        }
        fprintf(f, "\n");
    }

    // ----- Footer section -----
    fprintf(f, "$$\n");
    fprintf(f, "%s\n", res->footer().c_str());
    fflush(f);
    fclose(f);

    return 0;
}

int io::exportCalibrationValues(std::string filename_, CalibrationEditor* cal){
    QString filename(QString::fromStdString(filename_));
    int ans = cal->calexport(filename);
    if (ans)
        cout << "Exported successfully";
}

int io::validateSchema(string xsdPath_, string xmlPath_){
    QString xsdPath(QString::fromStdString(xsdPath_));
    QString xmlPath(QString::fromStdString(xmlPath_));

    // Setup schema object
    QXmlSchema schema;
    MessageHandler messenger;
    schema.setMessageHandler(&messenger);

    // Retrieve schema data
    QFile xsdFile(xsdPath);
    xsdFile.open(QIODevice::ReadOnly);
    QByteArray const schemaData = xsdFile.readAll();

    // Exec schema validation
    schema.load(schemaData);
    if( schema.isValid() ){
        // XML instance validator setup
        QXmlSchemaValidator validator(schema);
        QFile xmlFile(xmlPath);
        xmlFile.open(QIODevice::ReadOnly);
        QByteArray const instanceData = xmlFile.readAll();
        // Exec XML instance validator
        if( validator.validate(instanceData) ){
            cout << "XML instance validation was successful." << endl;
            return 0;
        }else{
            cout << "XML instance is invalid." << endl;
            cout << "Error '" << messenger.statusMessage().toStdString()
                 << "' at line " << messenger.line()
                 << ", column " << messenger.column() << "." << endl;
            return 1;
        }
    }else{
        cout << "XML schema is invalid." << endl;
        cout << "Error '" << messenger.statusMessage().toStdString()
             << "' at line " << messenger.line()
             << ", column " << messenger.column() << "." << endl;
        return 1;
    }
}
