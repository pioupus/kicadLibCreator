#ifndef KICADFILE_LIB_H
#define KICADFILE_LIB_H

#include <QString>
#include <QPoint>
#include <QObject>

typedef enum{ksfi_none=-2,ksfi_name=-1,ksfi_value=1,ksfi_reference=0,ksfi_footprint=2,ksfi_datasheet=3}KicadSymbolFieldIndex_t;
typedef enum{ksfo_none=0,ksfo_horizontal,ksfo_vertical}KicadSymbolFieldOrientation_t;
typedef enum{ksfj_none=0,ksfj_left,ksfj_right,ksfj_center,ksfj_bottom,ksfj_top             }KicadSymbolFieldJustify_t;

/*Parameters for DEF :
• name = component name in library (74LS02 ...)
• référence = Reference ( U, R, IC .., which become U3, U8, R1, R45, IC4...)
• unused = 0 (reserved)
• text_offset = offset for pin name position
• draw_pinnumber = Y (display pin number) ou N (do not display pin number).
• draw_pinname = Y (display pin name) ou N (do not display pin name).
• unit_count = Number of part ( or section) in a component package.
• units_locked = = L (units are not identical and cannot be swapped) or F (units are
identical and therefore can be swapped) (Used only if unit_count > 1)
• option_flag = N (normal) or P (component type "power")

Example:
DEF BNC P 0 40 Y NR 1 L NR
F0 “P” 10.120 60 H V L C
F1 “BNC” 110 - 60 40 V V L C
DRAW
*/

class KICADLibSchematicDeviceDefinition{
public:

    KICADLibSchematicDeviceDefinition();
    ~KICADLibSchematicDeviceDefinition();

    void decode(QString str);
    QString encode();

    QString name;
    QString reference;
    int text_offset;
    bool drawPinNumber;
    bool drawPinName;
    int unitCount;
    bool unitLocked;
    bool optionFlag_IsPowersymbol;
};

/*2.3.2 - Description of the fields
format:
F n “text” posx posy dimension orientation visibility hjustify vjustify/italic/bold “name”
with:
• n = field number :
• reference = 0.
• value = 1.
• Pcb FootPrint = 2.
• User doc link = 3. At present time: not used
• n = 4..11 = fields 1 to 8 (since January 2009 more than 8 field allowed, so n can be > 11.
• text (delimited by double quotes)
• position X and Y
• dimension (default = 50)
• orientation = H (horizontal) or V (vertical).
• Visibility = V (visible) or I (invisible)
• hjustify vjustify = L R C B or T
• L= left
• R = Right
• C = centre
• B = bottom
• T = Top
• Style: Italic = I or N ( since January 2009)
• Style Bold = B or N ( since January 2009)
• Name of the field (delimited by double quotes) (only if it is not the default name)

Note: vjustify, Italic and Bold are in the same 3 chars word.
Example:
DEF DIODE D 0 40 Y NR 1 0 NR
F0 “D” 0.100 50 H V L CNN
F1 “DIODE” 0 -100 50 H V L CIB
F5 “2euros” 0 -200 50 H V L CIB ”PRICE”

2.3.2.1 - Important Note 1:
The F1 field is the default component value and the component name in library.
So the F1 field text should be the same as the name.

2.3.2.2 - Important Note 2:
F0 is the reference prefix.

If the prefix starts b # (like #U) the component is not output to netlist or Bill Of Material.
This is a “virtual” component.
Mainly power symbols must have the prefix starting by #.
*/
class KICADLibSchematicDeviceField{
public:

    KICADLibSchematicDeviceField();
    ~KICADLibSchematicDeviceField();

    void decode(QString str);
    QString encode();

    QString text;
    QPoint position;
    int dimension;
    KicadSymbolFieldOrientation_t orientation;
    bool visible;
    KicadSymbolFieldJustify_t hjustify;
    KicadSymbolFieldJustify_t vjustify;
    bool FontstyleItalic;
    bool FontstyleBold;
    QString name;
};

class KICADLibSchematicDrawElement{
public:

    KICADLibSchematicDrawElement();
    ~KICADLibSchematicDrawElement();

    void decode(QString str);
    QString encode();

    QString text;
    QPoint position;
    int dimension;
    KicadSymbolFieldOrientation_t orientation;
    bool visible;
    KicadSymbolFieldJustify_t hjustify;
    KicadSymbolFieldJustify_t vjustify;
    bool FontstyleItalic;
    bool FontstyleBold;
    QString name;
};
/*2.3.3.1 - Polygon :
Format:
P Nb parts convert thickness x0 y0 x1 y1 xi yi cc
With:
• Nb = a number of points.
• unit = 0 if common to the parts; if not, number of part (1. .n).
• convert = 0 if common to the 2 representations, if not 1 or 2.
• thickness = line thickness.
• xi yi coordinates of end i.
• cc = N F or F ( F = filled polygon; f = . filled polygon, N = transparent background)

Example:
P 3 0 1 0 - 50 50 50 0 - 50 - 50 F
P 2 0 1 0 50 50 50 – 50 N
*/

class KICADLibSchematicDrawElement_polygon :public KICADLibSchematicDrawElement{
public:

    KICADLibSchematicDrawElement_polygon();
    ~KICADLibSchematicDrawElement_polygon();

    void decode(QString str);
    QString encode();

    intr units;
    int convert;
    int thickness;
    bool cc_filled;
    QList<QPoints> points;

};

/*
 * 2.3.3.2 - Rectangle
Format:
S startx starty endx endy unit convert thickness cc
With
• unit = 0 if common to the parts; if not, number of part (1. .n).
• convert = 0if common to all parts. If not, number of the part (1. .n).
• thickness = thickness of the outline.
• cc = N F or F ( F = filled Rectangle,; f = . filled Rectangle, N = transparent background)
Example:
S 0 50.900.900 0 1 0 f

*/

class KICADLibSchematicDrawElement_rectangle :public KICADLibSchematicDrawElement{
public:

    KICADLibSchematicDrawElement_rectangle();
    ~KICADLibSchematicDrawElement_rectangle();

    void decode(QString str);
    QString encode();

    intr units;
    int convert;
    int thickness;
    bool cc_filled;

    QPoint start;
    QPoint end;
};

class KICADLibSchematicDevice : public QObject
{
    Q_OBJECT
public:
    explicit KICADLibSchematicDevice(QObject *parent = 0);

    QStringList getDrawSymbol();

    QList<KICADLibSchematicDeviceField> fields;
    KICADLibSchematicDeviceDefinition def;

signals:

public slots:
};

class KICADLibSchematicDeviceLibrary : public QObject
{
    Q_OBJECT
public:
    explicit KICADLibSchematicDeviceLibrary(QObject *parent = 0);

    void loadFile(QString fileName);
    QList<KICADLibSchematicDevice> getSymbolList();

signals:

public slots:
};

#endif // KICADFILE_LIB_H
