#include <QDebug>
#include "test_kicad_schem_lib.h"
#include "kicadfile_lib.h"

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
    QCOMPARE(symList[9].fpList.count(),2);
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
    QList<KICADLibSchematicDeviceField> fields = symList[0].fields;
    QCOMPARE(fields.length(),4);
    QCOMPARE(fields[0].text,QString("U"));
    QCOMPARE(fields[0].fieldIndex.getRawIndex(),0);
    QCOMPARE(fields[1].fieldIndex.getRawIndex(),1);
    QCOMPARE(fields[2].fieldIndex.getRawIndex(),2);
    QCOMPARE(fields[3].fieldIndex.getRawIndex(),3);
    QCOMPARE(fields[0].position,QPoint(0,0));
    QCOMPARE(fields[0].dimension,50);
    QCOMPARE(fields[0].orientation,ksfo_horizontal);
    QCOMPARE(fields[0].visible,true);
    QCOMPARE(fields[0].hjustify,ksfj_center);
    QCOMPARE(fields[0].vjustify,ksfj_center);
    QCOMPARE(fields[0].FontstyleBold,false);
    QCOMPARE(fields[0].FontstyleItalic,false);
    QCOMPARE(fields[0].name,QString(""));
    QCOMPARE(fields[2].name,QString("NAME"));

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
    QCOMPARE(drawElements[1].start, QPoint(-650,-300));
    QCOMPARE(drawElements[1].length, 300);
    QCOMPARE(drawElements[1].orientation, ksr_right);
    QCOMPARE(drawElements[1].pinNumberTextSize, 50);
    QCOMPARE(drawElements[1].pinNameTextSize, 50);

    QCOMPARE(drawElements[1].unit, 1);
    QCOMPARE(drawElements[1].convert, 1);

    QCOMPARE(drawElements[1].etype.getType(), ElectricalType::Input);
    QCOMPARE(drawElements[1].shape.getShape(), PinShape::None);
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
