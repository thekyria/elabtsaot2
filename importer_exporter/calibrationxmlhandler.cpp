#include "calibrationxmlhandler.h"
using namespace elabtsaot;
#include "calibrationeditor.h"

#include <QFile>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNode>

#include <iostream>
using std::cout;
using std::endl;
using std::string;


CalibrationXMLHandler::CalibrationXMLHandler(QFile* file, CalibrationEditor* cal):_cal(cal),_doc("XMLFile") {

    _xmlfile = file;
}

CalibrationXMLHandler::~CalibrationXMLHandler(){
}

int CalibrationXMLHandler::importfile(){
    if(!_xmlfile->open(QIODevice::ReadOnly))
        return 0;
    if( !_doc.setContent( _xmlfile ) )
    {
        _xmlfile->close();
        return 1;
    }
    _xmlfile->close();
}

int CalibrationXMLHandler::importXML(){

    QDomElement root = _doc.documentElement(); //Take the root
    QDomNode n1,n2,n3,node ,n5,realimag,test, data; //6 nodes

    n1 = root.firstChild(); //<Info>
    QString checkname = "Calibration values of devices";
    QString xmlname = n1.namedItem("Name").toElement().text().trimmed();
    if ( xmlname != checkname)
        return 0;
    cout << "Number of devices in XML: " << n1.namedItem("Devicenumber").toElement().text().toStdString() <<endl;
    n1 = n1.nextSibling(); //<Devices>
    n2 = n1.firstChild(); //<Device>
    QVector<bool> imported(_cal->_master_store.size(), false); //Track the correctly imported device values
    while( !n2.isNull() ) //Enumerate the devices
    {

        size_t devid;
        n3 = n2.namedItem("Name");
        string name = n3.toElement().text().trimmed().toStdString();

        //Try to map the detected devices by the calibration module with the xml stored ones
        bool flagfound = false;

        for(size_t k=0; k!= _cal->_master_store.size(); ++k){
            if (_cal->_master_store[k]->deviceName.toStdString() == name){
                devid = k; //Device found
                flagfound = true;
                break;
            }
        }
        //This device of XML is not connected
        if (!flagfound){//Going to next XML device
            cout << "Device with name: "<< name << " in XML is ignored due to no USB connection with it" << endl;
            n2 = n2.nextSibling(); //First peak the next one
            continue;
        }

        //Resize to accept both real and imag
        _cal->_master_store[devid]->calibrationnamedatanew.resize(2);
        _cal->_master_store[devid]->calibrationoffsetdatanew.resize(2);
        _cal->_master_store[devid]->calibrationgaindatanew.resize(2);
        _cal->_master_store[devid]->calibrationrabnew.resize(2);
        _cal->_master_store[devid]->calibrationrwnew.resize(2);


        cout << "Importing device: " << devid << " with name " <<  name << endl;
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
                for (size_t ri = 0; ri!=REAL_IMAG.size(); ++ri){
                    realimag = n5.namedItem(REAL_IMAG[ri]);

                    test = realimag.namedItem(TEST_NAMES[i]);

                    if (!test.isNull()){
                        QString testname(TEST_NAMES[i]);
                        data = test.namedItem("Offset");
                        if (!data.isNull()){
                            _cal->_master_store[devid]->calibrationnamedatanew[ri].append(testname.append(" ").append(REAL_IMAG[ri]).append(" ").append(" of node ").append(nodeid));
                            _cal->_master_store[devid]->calibrationoffsetdatanew[ri].append(data.toElement().text().toDouble());
                        }
                        data = test.namedItem("Gain");
                        if (!data.isNull()){//Only second has gain, the first is just 0
                            _cal->_master_store[devid]->calibrationgaindatanew[ri].append(data.toElement().text().toDouble());
                        }else if(data.isNull() && i ==0){
                            _cal->_master_store[devid]->calibrationgaindatanew[ri].append(0); //Zero for the first test
                        }

                        data = test.namedItem("rab");
                        if (!data.isNull()){
                            _cal->_master_store[devid]->calibrationnamedatanew[ri].append(testname.append(" ").append(REAL_IMAG[ri]).append(" ").append(" of node ").append(nodeid));
                            _cal->_master_store[devid]->calibrationrabnew[ri].append(data.toElement().text().toDouble());
                        }
                        data = test.namedItem("rw");
                        if (!data.isNull()){
                            _cal->_master_store[devid]->calibrationrwnew[ri].append(data.toElement().text().toDouble());
                        }
                    }
                }
                node = node.nextSibling(); //Next Node
            }
        }
        imported[devid] = true; //Set the flag as imported
        n2 = n2.nextSibling(); //Next device
    }
    for (size_t k=0; k!=imported.size(); ++k){
        if (!imported.at(k)){
            cout << "Device " << _cal->_master_store.at(k)->deviceName.toStdString() << " was not found in XML file";
            cout << ", default values will be used for it." << endl;
        }
        else{
            cout << "Device " << _cal->_master_store.at(k)->deviceName.toStdString() << " was found in XML file and is imported" << endl;
        }
    }
    return 1;
}
