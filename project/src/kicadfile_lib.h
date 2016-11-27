#ifndef KICADFILE_LIB_H
#define KICADFILE_LIB_H

#include <QString>
#include <QStringList>
#include <QPoint>

//https://en.wikibooks.org/wiki/Kicad/file_formats

typedef enum{ksfi_none=-2,ksfi_name=-1,ksfi_value=1,ksfi_reference=0,ksfi_footprint=2,ksfi_datasheet=3}KicadSymbolFieldIndex_t;
typedef enum{ksfo_none=0,ksfo_horizontal,ksfo_vertical}KicadSymbolFieldOrientation_t;
typedef enum{ksfj_none=0,ksfj_left,ksfj_right,ksfj_center,ksfj_bottom,ksfj_top             }KicadSymbolFieldJustify_t;
typedef enum{ksr_none=0,ksr_up,ksr_down,ksr_right,ksr_left             }KicadSymbolOrientation_t;



typedef enum{ksps_none=0,
             ksps_line,
             ksps_inverted,
             ksps_clock,
             ksps_inverted_clock,
             ksps_input_low,
             ksps_clock_low,
             ksps_output_low,
             ksps_falling_edge_clock,
             ksps_non_logic} KicadSymbolPinShape_t;


QStringList splitParams(const QString & params);

/*
DEF name reference unused text_offset draw_pinnumber draw_pinname unit_count units_locked option_flag
Parameters for DEF :
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


    void decode(QString str);
    QString encode();
    void clear();
    bool isValid();

    QString name;
    QString reference;
    int text_offset;
    bool drawPinNumber;
    bool drawPinName;
    int unitCount;
    bool unitLocked;
    bool optionFlag_IsPowersymbol;
};

class KICADLibFieldIndex{
public:
    KICADLibFieldIndex();
    void setRawIndex(int rawindex);

    int getRawIndex();
    QString getFieldIndexDescription();
    int rawIndex;

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

    void decode(QString str);
    QString encode();
    void clear();

    KICADLibFieldIndex fieldIndex;
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

    bool operator<(const KICADLibSchematicDeviceField &R) const;
    bool operator>(const KICADLibSchematicDeviceField &R) const;

};

enum class DrawType{none,polygon,rectangle,circle,arc,text,pin};
#if 0
typedef enum{kspt_none=0,
             kspt_input ='I'             ,
             kspt_output = 'O',
             kspt_bidirectional = 'B',
             kspt_tristate = 'T',
             kspt_passive='P',
             kspt_unsepcified='U',
             kspt_power_in='W',
             kspt_power_out ='w',
             kspt_open_collector='C',
             kspt_open_emitter='E',
             kspt_not_connected = 'N'} KicadSymbolPinElectrcalType_t;
#endif
enum class ElType {
                 None,
                 Input,
                 Output,
                 Bidirectional,
                 Tristate,
                 Passive,
                 Unsepcified,
                 Power_in,
                 Power_out,
                 Open_collector,
                 Open_emitter,
                 Not_connected
   };

class ElectricalType{
public:

    void decode(char ch);


    ElType getType();
private:
    ElType t;
};

enum class PShape {
                 None,
                 Line,
                 Inverted,
                 Clock,
                 Inverted_clock,
                 Input_low,
                 Clock_low,
                 OutputLow,
                 FallingEdgeClock,
                 NonLocgic
   };

class PinShape{
public:
    void decode(QString str);


    PShape getShape();
    void setShape(PShape s);
private:
    PShape s;
};



class KICADLibSchematicDrawElement{
public:

    KICADLibSchematicDrawElement(QString str);


    QString encode();
    //KICADLibSchematicDrawElement *getDrawSmybol();

    DrawType getDrawType();


    DrawType drawtype;

    int unit;
    int convert;
    int thickness;
    bool cc_filled;
    QList<QPoint> polygon_points;

    QPoint start;
    QPoint end;

    QPoint position;

    int radius;

    int angle_start;
    int angle_end;


    bool orientation_vertical;
    int dimension;

    QString name;
    int number;
    int length;
    int pinNumberTextSize;
    int pinNameTextSize;

    KicadSymbolOrientation_t orientation;

    ElectricalType etype;
    PinShape shape;

private:
    QString originalText;


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



/*
 * 2.3.3.3 - Circle
Format:
C posx posy radius unit convert thickness cc
With
• posx posy = circle center position
• unit = 0 if common to the parts; if not, number of part (1. .n).
• convert = 0 if common to all parts. If not, number of the part (1. .n).
• thickness = thickness of the outline.
• cc = N F or F ( F = filled circle,; f = . filled circle, N = transparent background)
Example:
C 0 0 70 0 1 0 F
C 0 0 20 0 1 0 N

*/


/*
 * 2.3.3.4 - Arc of circle
Format:
A posx posy radius start end part convert thickness cc start_pointX start_pointY end_pointX end_pointY.
With:
• posx posy = arc center position
• start = angle of the starting point (in 0,1 degrees).
• end = angle of the end point (in 0,1 degrees).
• unit = 0 if common to all parts; if not, number of the part (1. .n).
• convert = 0 if common to the representations, if not 1 or 2.
• thickness = thickness of the outline or 0 to use the default line thickness.
• cc = N F or F ( F = filled arc,; f = . filled arc, N = transparent background)
• start_pointX start_pointY = coordinate of the starting point (role similar to start)
• end_pointX end_pointY = coordinate of the ending point (role similar to end)
Example:
A -1 -200 49 900 -11 0 1 0 N -50 -200 0 -150
A 0 -199 49 0 -911 0 1 0 N 0 -150 50 -200
*/



/*
 * 2.3.3.5 - Text field
Format:
T orientation posx posy dimension unit convert Text
With:
• orientation = horizontal orientation (=0) or vertical (=1).
• type = always 0.
• unit = 0 if common to the parts. If not, the number of the part (1. .n).
• convert = 0 if common to the representations, if not 1 or 2.
Example:
T 0 - 320 - 10 100 0 0 1 VREF
*/


/*
 * 2.3.4 - Description of pins
Format:
X name number posx posy length orientation Snum Snom unit convert Etype [shape].
With:
• orientation = U (up) D (down) R (right) L (left).
• name = name (without space) of the pin. if ~: no name
• number = n pin number (4 characters maximum).
• length = pin length.
• Snum = pin number text size.
• Snom = pin name text size.
• unit = 0 if common to all parts. If not, number of the part (1. .n).
• convert = 0 if common to the representations, if not 1 or 2.
• Etype = electric type (1 character)
• shape = if present: pin shape (clock, inversion...).
Example:
X TO 1 - 200 0.150 R 40 40 1 1 P
X K 2.200 0.150 L 40 40 1 1 P
X 0 1 0 0 0 R 40 40 1 1 W NC
X ~ 2 0 - 250 200 U 40 40 1 1 P
Etype list:
INPUT           I
OUTPUT          O
BIDI            B
TRISTATE        T
PASSIVE         P
UNSPECIFIED     U
POWER INPUT     W
POWER OUTPUT    w
OPEN COLLECTOR  C
OPEN EMITTER    E
NOT CONNECTED   N

Shape list:
• If invisible pin, the shape identifier starts by N
• Next character is:
Line                None(default)
Inverted            I
Clock               C
Inverted clock      CI
Input low           L
Clock low           CL
Output low          V
Falling edge clock  F
Non Logic           X

Example:
A clock is coded C if visible, and NC if invisible.
*/


class KICADLibDCMEntry{
public:
    KICADLibDCMEntry();
    void clear();

    bool hasFields();
    QString name;
    QString description;
    QString keywords;
    QString datasheetlink;
};

class KICADLibDCMFile{
public:
    void loadFile(QString libfileName);
    KICADLibDCMEntry getEntryByName(QString name, bool &ok);
    QList<KICADLibDCMEntry> dcmEntries;
    QStringList componentNames;
    void clear();

    static QString getdcmFileNameFromLibFileName(QString libfileName);
};

class KICADLibSchematicDevice
{

public:
    KICADLibSchematicDevice();
    void clear();
    bool isValid();

    void setField(KICADLibSchematicDeviceField field);
    void removeAllFields();

    QList<KICADLibSchematicDeviceField> fields;

    QList<KICADLibSchematicDrawElement> getDrawSymbols();


    KICADLibSchematicDeviceDefinition def;
    QStringList fpList;
    QStringList alias;

    QList<KICADLibSchematicDrawElement> drawSymbols;

    KICADLibDCMEntry dcmEntry;
};

class KICADLibSchematicDeviceLibrary
{

public:
    explicit KICADLibSchematicDeviceLibrary();
    void clear();

    void loadFile(QString fileName);
    void saveFile(QString fileName);
    QString getName();

    QList<KICADLibSchematicDevice> getSymbolList();
    int indexOf(QString deviceName);
    void insertDevice(KICADLibSchematicDevice device);



private:
    QString fileName;
    QList<KICADLibSchematicDevice> symbolList;
};

class KICADLibFootprintLibrary{
public:
    KICADLibFootprintLibrary();

    void scan(QString path);
    QStringList getFootprintList();
private:
    QString path;
    QStringList footprintnames;
};

#endif // KICADFILE_LIB_H
