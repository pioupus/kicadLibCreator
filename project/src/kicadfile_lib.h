#ifndef KICADFILE_LIB_H
#define KICADFILE_LIB_H
#define SEXPRESSO_OPT_OUT_PIKESTYLE

#include "sexpresso/sexpresso/sexpresso.hpp"
#include <QColor>
#include <QMap>
#include <QPainter>
#include <QPoint>
#include <QString>
#include <QStringList>
#include <QVariant>

//https://en.wikibooks.org/wiki/Kicad/file_formats

typedef enum { ksfi_none = -2, ksfi_name = -1, ksfi_value = 1, ksfi_reference = 0, ksfi_footprint = 2, ksfi_datasheet = 3 } KicadSymbolFieldIndex_t;
typedef enum { ksfo_none = 0, ksfo_horizontal, ksfo_vertical } KicadSymbolFieldOrientation_t;
typedef enum { ksfj_none = 0, ksfj_left, ksfj_right, ksfj_center, ksfj_bottom, ksfj_top } KicadSymbolFieldJustify_t;
typedef enum { ksr_none = 0, ksr_up, ksr_down, ksr_right, ksr_left } KicadSymbolOrientation_t;
enum class FillType { none, foregroundColor, backgroundColor };

QStringList splitParams(const QString &params);
//https://dev-docs.kicad.org/en/file-formats/sexpr-intro/index.html#_text_effects
class sKicadPositionIdentifier {
    /*
  (at
    X
    Y
    [ANGLE]
  )
     */
    public:
    sKicadPositionIdentifier();
    sKicadPositionIdentifier(const sexpresso::Sexp &from);
    QPointF pos;
    std::optional<double> angle;
    sexpresso::Sexp to_sexp() const;
};

class sKicadTextSize {
    //for s-exp format
    public:
    sKicadTextSize();
    sKicadTextSize(const sexpresso::Sexp &from);
    double height;
    double width;
    sexpresso::Sexp to_sexp() const;
};

class sKicadTextJustify {
    //for s-exp format
    /*
[(justify [left | right] [top | bottom] [mirror])]
)
 )*/
    public:
    sKicadTextJustify();
    sKicadTextJustify(const sexpresso::Sexp &from);

    enum Justify_LR { left, right };
    enum Justify_TB { top, bottom };
    bool is_mirror;
    std::optional<Justify_LR> lr;
    std::optional<Justify_TB> tb;

    sexpresso::Sexp to_sexp() const;
};

class sKicadTextFont {
    //for s-exp format
    /*

    (font
      [(face FACE_NAME)]
      (size HEIGHT WIDTH)
      [(thickness THICKNESS)]
      [bold]
      [italic]
      [(line_spacing LINE_SPACING)]
    )
 )*/
    public:
    sKicadTextFont();
    sKicadTextFont(const sexpresso::Sexp &from);
    std::optional<QString> face;
    sKicadTextSize size;
    std::optional<double> thickness;
    bool is_bold;
    bool is_italic;
    std::optional<double> line_spacing;
    sexpresso::Sexp to_sexp() const;
};

class sKicadTextEffects {
    //for s-exp format
    /*
  (effects
    (font
      [(face FACE_NAME)]
      (size HEIGHT WIDTH)
      [(thickness THICKNESS)]
      [bold]
      [italic]
      [(line_spacing LINE_SPACING)]
    )
 [(justify [left | right] [top | bottom] [mirror])]
 [hide]
)
)*/
    public:
    sKicadTextEffects();
    sKicadTextEffects(const sexpresso::Sexp &from);
    QFont get_font() const;

    sKicadTextFont font;
    std::optional<sKicadTextJustify> justify;
    bool is_hide;
    sexpresso::Sexp to_sexp() const;
};

class sKicadPinNameNumber {
    //for s-exp format
    /*
(name "NAME" TEXT_EFFECTS)
(number "NUMBER" TEXT_EFFECTS) */
    public:
    sKicadPinNameNumber();
    sKicadPinNameNumber(QString type, QString &label);
    sKicadPinNameNumber(const sexpresso::Sexp &from);
    QPen get_pen_name() const;

    QString label;
    sKicadTextEffects effects;
    QString node_type;
    sexpresso::Sexp to_sexp() const;
};

class sKicadStrokeDefinition {
    //for s-exp format
    /*
  (stroke
    (width WIDTH)
    (type TYPE)
    (color R G B A)
  )
)*/

    public:
    sKicadStrokeDefinition();
    sKicadStrokeDefinition(const sexpresso::Sexp &from);

    enum TypeEnum { dash, dash_dot, dash_dot_dot, dot, default_, solid };
    QPen get_pen() const;
    TypeEnum type;
    double width;
    QColor color;
    sexpresso::Sexp to_sexp() const;
};

class sKicadFillDefinition {
    //for s-exp format
    /*
  (fill
    (type none | outline | background)
  )
)*/
    public:
    sKicadFillDefinition();
    sKicadFillDefinition(FillType &from);
    sKicadFillDefinition(const sexpresso::Sexp &from);

    enum TypeEnum { none, outline, background };
    QBrush get_brush() const;
    TypeEnum type;
    sexpresso::Sexp to_sexp() const;
};

class sKicadPinName {
    //for s-exp format
    /*
[(pin_names [(offset OFFSET)] hide)]
)*/
    public:
    sKicadPinName();
    //  sKicadPinName(QString &from);
    sKicadPinName(const sexpresso::Sexp &from);
    std::optional<double> offset; //double
    bool is_hidden;
    sexpresso::Sexp to_sexp() const;
};

class sKicadPointList {
    //for s-exp format
    /*
  (pts
    (xy X Y)
    ...
    (xy X Y)
  )
)*/
    public:
    sKicadPointList(const sexpresso::Sexp &from);
    sKicadPointList();
    QList<QPointF> pts;
    sexpresso::Sexp to_sexp() const;
};

class FieldDesignSettingsItem {
    public:
    FieldDesignSettingsItem();

    int index;

    QPoint position;
    int dimension;
    KicadSymbolFieldOrientation_t orientation;
    bool visible;
    KicadSymbolFieldJustify_t hjustify;
    KicadSymbolFieldJustify_t vjustify;
    bool FontstyleItalic;
    bool FontstyleBold;
};

class sSymbolProperty {
    //for s-exp format

    /*
      (property
          "KEY"
          "VALUE"
          (id N)
          POSITION_IDENTIFIER
          TEXT_EFFECTS
        }


            Key         Ordinal 	Description                         Empty Allowed
            Reference   0           Symbol reference designator         No
            Value       1           Symbol value string                 No
            Footprint   2           Symbol footprint library identifier Yes
            Datasheet   3           Symbol datasheet link               Yes



        Reserved Symbol Property Keys
            ki_keywords,
            ki_description,
            ki_locked
            ki_fp_filters

        */

    public:
    sSymbolProperty();
    sSymbolProperty(const sexpresso::Sexp &from);
    void set_design(FieldDesignSettingsItem design_item, bool overwrite_text_position);
    QString key;
    QString value;
    std::optional<int> id; //integer
    sKicadPositionIdentifier at;
    sKicadTextEffects text_effects;
    sexpresso::Sexp to_sexp() const;
};
#if 0
class KICADLibFieldIndex {
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

class KICADLibSchematicDeviceField {
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

    void setDesign(FieldDesignSettingsItem designItem, bool overwriteTextPosition);

    bool operator<(const KICADLibSchematicDeviceField &R) const;
    bool operator>(const KICADLibSchematicDeviceField &R) const;
};
#endif

enum class DrawType { none, polygon, rectangle, circle, arc, text, pin };
enum class TextDirection { Horizontal, Vertical };
enum class TextStyle { normal, bold, italic, bold_italic };

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
//enum class ElType { None, Input, Output, Bidirectional, Tristate, Passive, Unsepcified, Power_in, Power_out, Open_collector, Open_emitter, Not_connected };

enum class sGraphicalStyle { none = 0, line, inverted, clock, inverted_clock, input_low, clock_low, output_low, falling_edge_clock, non_logic };

enum class sElectricalType {
    input,
    output,
    bidirectional,
    tri_state,
    passive,
    free,
    unspecified,
    power_in,
    power_out,
    open_collector,
    open_emitter,
    no_connect
};

class ElectricalType {
    public:
    void decode(char ch);

    sElectricalType getType();

    private:
    sElectricalType t;
};

enum class PShape { None, Line, Inverted, Clock, Inverted_clock, Input_low, Clock_low, OutputLow, FallingEdgeClock, NonLocgic };

class PinShape {
    public:
    void decode(QString str);

    PShape getShape();
    void setShape(PShape s);

    private:
    PShape s;
};

class sKicadGraphSymbol {
    public:
    virtual sexpresso::Sexp to_sexp() const;
    virtual void paint(QPainter &painter, const QPointF &centerPoint, const std::optional<sKicadPinName> &sexp_pin_names) const;
};

class sKicadGraphSymbolArc : public sKicadGraphSymbol {
    /*  (arc
    (start X Y)
    (mid X Y)
    (end X Y)
    STROKE_DEFINITION
    FILL_DEFINITION
  )*/

    public:
    sKicadGraphSymbolArc(const sexpresso::Sexp &from);
    void paint(QPainter &painter, const QPointF &centerPoint, const std::optional<sKicadPinName> &sexp_pin_names) const;
    QPointF start;
    QPointF mid;
    QPointF end;
    sKicadStrokeDefinition stroke;
    sKicadFillDefinition fill;
    std::optional<int> unit;    //old fashioned fileformat declares within the element to what unit this belongs to
    std::optional<int> convert; //old fashioned fileformat declare
    sexpresso::Sexp to_sexp() const;
};

class sKicadGraphSymbolCircle : public sKicadGraphSymbol {
    /*
 (circle
    (center X Y)
    (radius RADIUS)
    STROKE_DEFINITION
    FILL_DEFINITION
  )
  */

    public:
    sKicadGraphSymbolCircle(const sexpresso::Sexp &from);
    void paint(QPainter &painter, const QPointF &centerPoint, const std::optional<sKicadPinName> &sexp_pin_names) const;
    QPointF center;
    double radius;
    sKicadStrokeDefinition stroke;
    sKicadFillDefinition fill;
    sexpresso::Sexp to_sexp() const;
};

class sKicadGraphSymbolCurve : public sKicadGraphSymbol {
    /*
  (bezier
    COORDINATE_POINT_LIST
    STROKE_DEFINITION
    FILL_DEFINITION
  )
     */

    public:
    sKicadGraphSymbolCurve(const sexpresso::Sexp &from);

    void paint(QPainter &painter, const QPointF &centerPoint, const std::optional<sKicadPinName> &sexp_pin_names) const;
    sKicadPointList points;
    sKicadStrokeDefinition stroke;
    sKicadFillDefinition fill;
    sexpresso::Sexp to_sexp() const;
};

class sKicadGraphSymbolLine : public sKicadGraphSymbol {
    /*
  (polyline
    COORDINATE_POINT_LIST
    STROKE_DEFINITION
    FILL_DEFINITION
  )
     */

    public:
    sKicadGraphSymbolLine(const sexpresso::Sexp &from);
    void paint(QPainter &painter, const QPointF &centerPoint, const std::optional<sKicadPinName> &sexp_pin_names) const;
    sKicadPointList points;
    sKicadStrokeDefinition stroke;
    sKicadFillDefinition fill;
    sexpresso::Sexp to_sexp() const;
};

class sKicadGraphSymbolRectangle : public sKicadGraphSymbol {
    /*
  (rectangle
    (start X Y)
    (end X Y)
    STROKE_DEFINITION
    FILL_DEFINITION
  )
     */

    public:
    sKicadGraphSymbolRectangle(const sexpresso::Sexp &from);

    void paint(QPainter &painter, const QPointF &centerPoint, const std::optional<sKicadPinName> &sexp_pin_names) const;
    QPointF start;
    QPointF end;
    sKicadStrokeDefinition stroke;
    sKicadFillDefinition fill;
    sexpresso::Sexp to_sexp() const;
};

class sKicadGraphSymbolText : public sKicadGraphSymbol {
    /*
  (text
    "TEXT"
    POSITION_IDENTIFIER
    (effects TEXT_EFFECTS)
  )
     */

    public:
    sKicadGraphSymbolText(const sexpresso::Sexp &from);

    void paint(QPainter &painter, const QPointF &centerPoint, const std::optional<sKicadPinName> &sexp_pin_names) const;

    QString text;
    sKicadPositionIdentifier at;
    sKicadTextEffects text_effects;
    sexpresso::Sexp to_sexp() const;
};

class sKicadGraphSymbolPin : public sKicadGraphSymbol {
    /*
  (pin
    PIN_ELECTRICAL_TYPE
    PIN_GRAPHIC_STYLE
    POSITION_IDENTIFIER
    (length LENGTH)
    (name "NAME" TEXT_EFFECTS)
    (number "NUMBER" TEXT_EFFECTS)
  )
     */

    public:
    sKicadGraphSymbolPin(const sexpresso::Sexp &from);
    void paint(QPainter &painter, const QPointF &centerPoint, const std::optional<sKicadPinName> &sexp_pin_names) const;
    sGraphicalStyle grapical_style;
    sElectricalType electrical_type;

    sKicadPositionIdentifier at;
    double length;
    sKicadPinNameNumber name;
    sKicadPinNameNumber number;
    std::optional<bool> is_hide;
    sexpresso::Sexp to_sexp() const;
};

class sKICADLibSchematicDeviceChildUnit {
    public:
    sKICADLibSchematicDeviceChildUnit();
    sKICADLibSchematicDeviceChildUnit(const sexpresso::Sexp &from);
    sKICADLibSchematicDeviceChildUnit(const sKICADLibSchematicDeviceChildUnit &from);

    sKICADLibSchematicDeviceChildUnit operator=(const sKICADLibSchematicDeviceChildUnit &from);
    std::vector<std::unique_ptr<sKicadGraphSymbol>> sexp_graphical_symbols;
    QString unit_id;
    //optional:
    std::optional<QString> unit_name;
    sexpresso::Sexp to_sexp() const;
};

class KICADLibSchematicDevice {
    public:
    KICADLibSchematicDevice();

    // KICADLibSchematicDevice(KICADLibSchematicDevice &&from);
    KICADLibSchematicDevice(const sexpresso::Sexp &from);
    KICADLibSchematicDevice(const KICADLibSchematicDevice &from);

    KICADLibSchematicDevice &operator=(const KICADLibSchematicDevice &from);

    void clear();
    bool isValid();

    // QList<std::unique_ptr<sKicadGraphSymbol>> getDrawSymbols();

    QString get_property_value_str(QString key) const;

    sSymbolProperty get_property_value(QString key) const;
    void set_property_value(QString key, QString value);
    void set_property_value(const sSymbolProperty &value);

    void set_symbol_name(QString name);
    QString get_symbol_name() const;
    int get_num_of_graphical_symbols() const;

    void paint(QPainter &painter, const QPointF &centerPoint) const;
    //s-expression's new fields:
    //https://dev-docs.kicad.org/en/file-formats/sexpr-intro/index.html

    /*
    (symbol
     "LIBRARY_ID" | "UNIT_ID"
         [(extends "LIBRARY_ID")]
         [(pin_numbers hide)]
         [(pin_names [(offset OFFSET)] hide)]
     (in_bom yes | no)
     (on_board yes | no)
     SYMBOL_PROPERTIES...
     GRAPHIC_ITEMS...
     PINS...
     UNITS...
         [(unit_name "UNIT_NAME")]
     )
     */

    bool sexp_in_bom;
    bool sexp_on_board;
    std::optional<QString> sexp_extend;
    bool sexp_pin_numbers_hide;
    bool sexp_exclude_from_sim;

    std::optional<sKicadPinName> sexp_pin_names;

    QMap<QString, sSymbolProperty> sexp_properties;
    std::vector<std::unique_ptr<sKicadGraphSymbol>> sexp_graphical_symbols;
    QList<sKICADLibSchematicDeviceChildUnit> sexp_units;
    sexpresso::Sexp to_sexp() const;
    QString symbol_name;
};

class KICADLibSchematicDeviceLibrary {
    public:
    explicit KICADLibSchematicDeviceLibrary();
    void clear();

    void loadFile(QString fileName);
    bool saveFile(QString fileName);
    QString getName();

    const QList<KICADLibSchematicDevice> &getSymbolList() const;

    int indexOf(QString deviceName);
    void insertDevice(const KICADLibSchematicDevice &device);

    private:
    QString fileName;
    QString lib_version;
    QString generator;
    QString generator_version;

    QList<KICADLibSchematicDevice> symbolList;
};

class KICADLibFootprintLibrary {
    public:
    KICADLibFootprintLibrary();

    void scan(QString path);
    QStringList getFootprintList();

    private:
    QString path;
    QStringList footprintnames;
};

#endif // KICADFILE_LIB_H
