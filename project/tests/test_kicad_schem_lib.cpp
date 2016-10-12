#include "test_kicad_schem_lib.h"
#include "kicadfile_lib.h"

#include "gmock/gmock.h"  // Brings in Google Mock.

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
    KICADLibSchematicDeviceLibrary schemDevLib;
    schemDevLib.loadFile("scripts/74xx.lib");
    QList<KICADLibSchematicDevice> symList = schemDevLib.getSymbolList();
    QCOMPARE(symList.count(),139);
}

void TestKICADLibSchematicDeviceLibrary::test_loadDef()
{
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

}


void TestKICADLibSchematicDeviceLibrary::test_BasicMocking(){

    MockFooClass turtle;
    EXPECT_CALL(turtle, PenDown())
            .Times(AtLeast(1));

    turtle.PenDown();


}
