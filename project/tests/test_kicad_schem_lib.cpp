#include <QDebug>
#include "test_kicad_schem_lib.h"
#include "../src/kicadfile_lib.h"

#include "gmock/gmock.h"  // Brings in Google Mock.

#define ALL_TESTS 1
class MockFooClass : public FooClass {
 public:

  MOCK_METHOD0(PenUp, void());
  MOCK_METHOD0(PenDown, void());
};

void FooClass::PenUp(){

}

void FooClass::PenDown(){

}



void TestKICADLibSchematicDeviceLibrary::initTestCase(){

}

void TestKICADLibSchematicDeviceLibrary::cleanupTestCase(){

}

void TestKICADLibSchematicDeviceLibrary::test_loadfromFile()
{
#if ALL_TESTS
    KICADLibSchematicDeviceLibrary schemDevLib;
    schemDevLib.loadFile("scripts/74xx.lib");
    QList<KICADLibSchematicDevice> symList = schemDevLib.getSymbolList();
    QCOMPARE(symList.count(),139);
#endif
}

void TestKICADLibSchematicDeviceLibrary::test_loadfromFile_brooktre()
{
#if ALL_TESTS
    KICADLibSchematicDeviceLibrary schemDevLib;
    schemDevLib.loadFile("scripts/brooktre.lib");
    QList<KICADLibSchematicDevice> symList = schemDevLib.getSymbolList();
    QCOMPARE(symList.count(),9);
#endif
}

void TestKICADLibSchematicDeviceLibrary::test_description()
{
#if ALL_TESTS
    KICADLibSchematicDeviceLibrary schemDevLib;
    schemDevLib.loadFile("scripts/74xx.lib");
    QList<KICADLibSchematicDevice> symList = schemDevLib.getSymbolList();
    QCOMPARE(symList[2].dcmEntry.description,QString("inverting buffer with Schmitt trigger."));
    QCOMPARE(symList[2].dcmEntry.datasheetlink,QString("74xx/74ahc_ahct1g14.pdf"));
    QCOMPARE(symList[4].dcmEntry.keywords,QString("HCMOS SR 3State"));

#endif
}

void TestKICADLibSchematicDeviceLibrary::test_loadDef()
{
#if ALL_TESTS
    KICADLibSchematicDeviceLibrary schemDevLib;
    schemDevLib.loadFile("scripts/74xx.lib");
    QList<KICADLibSchematicDevice> symList = schemDevLib.getSymbolList();
    QCOMPARE(symList[0].def.name,QString("74469"));
    QCOMPARE(symList[0].def.reference,QString("U"));
    QCOMPARE(symList[0].def.text_offset,40);
    QCOMPARE(symList[0].def.drawPinNumber,true);
    QCOMPARE(symList[0].def.drawPinName,true);
    QCOMPARE(symList[0].def.unitCount,1);
    QCOMPARE(symList[0].def.unitLocked,false);
    QCOMPARE(symList[0].def.optionFlag_IsPowersymbol,false);
    QCOMPARE(symList[0].fpList.count(),0);
    //qDebug() << symList[9].fpList;
    QCOMPARE(symList[8].fpList.count(),2);
    QCOMPARE(symList[8].def.name,QString("74LS00"));

#endif
}

void TestKICADLibSchematicDeviceLibrary::test_loadFields()
{
#if ALL_TESTS
    /*
     *

    DEF 74469 U 0 40 Y Y 1 F N
    F0 "U" 0 0 50 H V C CNN
    F1 "74469" 0 -200 50 H V C CNN
    F2 "" 0 0 50 H V C CNN
    F3 "" 0 0 50 H V C CNN
    DRAW
    S -350 650 350 -650 0 1 0 N
    X CLK 1 -650 -300 300 R 50 50 1 1 I
    */

    KICADLibSchematicDeviceLibrary schemDevLib;
    schemDevLib.loadFile("scripts/74xx.lib");
    QList<KICADLibSchematicDevice> symList = schemDevLib.getSymbolList();
    KICADLibSchematicDeviceField field_0 = symList[0].fields.getFieldbyIndex(0);
    KICADLibSchematicDeviceField field_1 = symList[0].fields.getFieldbyIndex(1);
    KICADLibSchematicDeviceField field_2 = symList[0].fields.getFieldbyIndex(2);
    KICADLibSchematicDeviceField field_3 = symList[0].fields.getFieldbyIndex(3);
    QCOMPARE(symList[0].fields.count(),4);
    QCOMPARE(field_0.text,QString("U"));
    QCOMPARE(field_0.fieldIndex.getRawIndex(),0);
    QCOMPARE(field_1.fieldIndex.getRawIndex(),1);
    QCOMPARE(field_2.fieldIndex.getRawIndex(),2);
    QCOMPARE(field_3.fieldIndex.getRawIndex(),3);
    QCOMPARE(field_0.position,QPoint(0,0));
    QCOMPARE(field_0.dimension,50);
    QCOMPARE(field_0.orientation,ksfo_horizontal);
    QCOMPARE(field_0.visible,true);
    QCOMPARE(field_0.hjustify,ksfj_center);
    QCOMPARE(field_0.vjustify,ksfj_center);
    QCOMPARE(field_0.FontstyleBold,false);
    QCOMPARE(field_0.FontstyleItalic,false);
    QCOMPARE(field_0.name,QString(""));
    QCOMPARE(field_2.name,QString("NAME"));

#endif
}

void TestKICADLibSchematicDeviceLibrary::test_loadDrawElements()
{
    KICADLibSchematicDeviceLibrary schemDevLib;
    schemDevLib.loadFile("scripts/74xx.lib");
    QList<KICADLibSchematicDrawElement> drawElements = schemDevLib.getSymbolList()[0].getDrawSymbols();
    QCOMPARE(drawElements.count(), 25);
    QCOMPARE(drawElements[0].getDrawType(), DrawType::rectangle);

    // S -350 650 350 -650 0 1 0 N
    //X CLK 1 -650 -300 300 R 50 50 1 1 I

    QCOMPARE(drawElements[0].start, QPoint(-350,650));
    QCOMPARE(drawElements[0].end, QPoint(350,-650));
    QCOMPARE(drawElements[0].unit, 0);
    QCOMPARE(drawElements[0].convert, 1);
    QCOMPARE(drawElements[0].thickness, 0);
    QCOMPARE(drawElements[0].cc_filled, false);

    QCOMPARE(drawElements[1].getDrawType(), DrawType::pin);
    QCOMPARE(drawElements[1].name, QString("CLK"));
    QCOMPARE(drawElements[1].number, 1);
    QCOMPARE(drawElements[1].position, QPoint(-650,-300));
    QCOMPARE(drawElements[1].length, 300);
    QCOMPARE(drawElements[1].orientation, ksr_right);
    QCOMPARE(drawElements[1].pinNumberTextSize, 50);
    QCOMPARE(drawElements[1].pinNameTextSize, 50);

    QCOMPARE(drawElements[1].unit, 1);
    QCOMPARE(drawElements[1].convert, 1);

    QCOMPARE(drawElements[1].etype.getType(), ElType::Input);
    QCOMPARE(drawElements[1].shape.getShape(), PShape::Line);
}

void TestKICADLibSchematicDeviceLibrary::test_saveDeviceIntoLibrary()
{
    KICADLibSchematicDeviceLibrary schemDevLib;
    schemDevLib.loadFile("scripts/74xx.lib");
    schemDevLib.saveFile("scripts/74xx_output.lib");

    KICADLibSchematicDeviceLibrary schemDevLibNew;
    schemDevLibNew.loadFile("scripts/74xx.lib");
    QList<KICADLibSchematicDevice> devList1 = schemDevLib.getSymbolList();
    QList<KICADLibSchematicDevice> devList2 = schemDevLib.getSymbolList();

    QCOMPARE(devList1.count(), devList2.count());
    for (int i=0;i<devList1.count();i++){
        QCOMPARE(devList1[i].def.name, devList2[i].def.name);
    }
}

void TestKICADLibSchematicDeviceLibrary::test_indexOf()
{
    KICADLibSchematicDeviceLibrary schemDevLib;
    schemDevLib.loadFile("scripts/74xx.lib");
    QCOMPARE(1, schemDevLib.indexOf("74AHC1G04"));
}

void TestKICADLibSchematicDeviceLibrary::test_insertDevice()
{
    KICADLibSchematicDeviceLibrary schemDevLib1;
    schemDevLib1.loadFile("scripts/74xx.lib");

    KICADLibSchematicDeviceLibrary schemDevLib2;
    schemDevLib2.loadFile("scripts/brooktre.lib");

    KICADLibSchematicDevice devToinsert= schemDevLib1.getSymbolList()[0];

    schemDevLib2.insertDevice(devToinsert);
    schemDevLib2.saveFile("scripts/brooktre_out.lib");
    QCOMPARE(9, schemDevLib2.indexOf(devToinsert.def.name));
}


void TestKICADLibSchematicDeviceLibrary::test_BasicMocking(){

    MockFooClass turtle;
    EXPECT_CALL(turtle, PenDown())
            .Times(AtLeast(1));

    turtle.PenDown();


}

void TestKICADLibSchematicDeviceLibrary::test_splitParams()
{
#if ALL_TESTS
    QString in = "hallo1 hallo2 \"hallo3 und 4\" hallo5 \"hallo6\" \"\"";
    QStringList out = splitParams(in);
    QCOMPARE(out.length(),6);
    QCOMPARE(out[0],QString("hallo1"));
    QCOMPARE(out[1],QString("hallo2"));
    QCOMPARE(out[2],QString("hallo3 und 4"));
    QCOMPARE(out[3],QString("hallo5"));
    QCOMPARE(out[4],QString("hallo6"));
    QCOMPARE(out[5],QString(""));
#endif
}
