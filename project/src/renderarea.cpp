#include "renderarea.h"

#include <QDebug>
#include <QFont>
#include <QPainter>
#include <QStaticText>
#include <QTransform>
#include <cmath>
#include <complex>

using namespace std::complex_literals;

RenderArea::RenderArea(QWidget *parent)
    : QWidget(parent) {
    shape = Polygon;
    antialiased = false;
    transformed = false;
    pixmap.load(":/images/qt-logo.png");

    brushForeground.setColor(QColor(153, 0, 0));
    brushForeground.setStyle(Qt::SolidPattern);
    brushBackground.setColor(QColor(255, 255, 204));
    brushBackground.setStyle(Qt::SolidPattern);
    brushEmpty.setStyle(Qt::NoBrush);

    penPinName.setColor(QColor(32, 96, 96));

    penForeground.setColor(brushForeground.color());
    penForeground.setWidth(2);
    penForeground.setJoinStyle(Qt::RoundJoin);
    penForeground.setCapStyle(Qt::RoundCap);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

QSize RenderArea::minimumSizeHint() const {
    return QSize(100, 100);
}

QSize RenderArea::sizeHint() const {
    return QSize(400, 200);
}

void RenderArea::paintSymbol(KICADLibSchematicDevice symbol) {
    this->symbol = symbol;

    update();
}

void RenderArea::clear() {
    symbol.clear();
    update();
}

void RenderArea::setAntialiased(bool antialiased) {
    this->antialiased = antialiased;
    update();
}

void RenderArea::setTransformed(bool transformed) {
    this->transformed = transformed;
    update();
}

void drawText(QPainter &painter, QPointF p, TextDirection textDirection, KicadSymbolFieldJustify_t text_hjustify, KicadSymbolFieldJustify_t text_vjustify,
              TextStyle textStyle, int textSize, const QString &text) {
    bool vertical = false;

    auto align = Qt::Alignment();

    if (text_hjustify == ksfj_left) {
        align |= Qt::AlignLeft;
    } else if (text_hjustify == ksfj_right) {
        align |= Qt::AlignRight;
    } else if (text_hjustify == ksfj_center) {
        align |= Qt::AlignHCenter;
    }

    if (text_vjustify == ksfj_top) {
        align |= Qt::AlignTop;
    } else if (text_vjustify == ksfj_bottom) {
        align |= Qt::AlignBottom;
    } else if (text_vjustify == ksfj_center) {
        align |= Qt::AlignVCenter;
    }
    if (textDirection == TextDirection::Vertical) {
        vertical = true;
    }
    (void)vertical;
    QFont font = painter.font();
    if (textStyle == TextStyle::bold) {
        font.setItalic(false);
        font.setBold(true);
    }
    if (textStyle == TextStyle::italic) {
        font.setItalic(true);
        font.setBold(false);
    }
    if (textStyle == TextStyle::bold_italic) {
        font.setItalic(true);
        font.setBold(true);
    }
    if (textStyle == TextStyle::normal) {
        font.setItalic(false);
        font.setBold(false);
    }
    font.setPixelSize(textSize);
    painter.setFont(font);

    const qreal size = 32767.0;
    QPointF corner = p;
    if (vertical && 0) {
        corner.setX(p.x() - size);
        corner.setY(p.y());

        if (align & Qt::AlignHCenter) {
            corner.ry() -= size / 2.0;
        } else if (align & Qt::AlignRight) {
            corner.ry() -= size;
        }
        if (align & Qt::AlignVCenter) {
            corner.rx() += size / 2.0;
        } else if (align & Qt::AlignTop) {
            corner.rx() += size;
        } else {
            align |= Qt::AlignBottom;
        }

    } else {
        corner.setX(p.x());
        corner.setY(p.y() - size);

        if (align & Qt::AlignHCenter) {
            corner.rx() -= size / 2.0;
        } else if (align & Qt::AlignRight) {
            corner.rx() -= size;
        }
        if (align & Qt::AlignVCenter) {
            corner.ry() += size / 2.0;
        } else if (align & Qt::AlignTop) {
            corner.ry() += size;
        } else {
            align |= Qt::AlignBottom;
        }
    }

    QRectF rect(corner, QSizeF(size, size));
    ;
    if (vertical && 0) {
        QTransform transform = painter.transform();
        transform.rotate(90);
        painter.setTransform(transform);
#if 0
        QRectF rect1 = rect;
        rect.setTop(rect1.left());
        rect.setBottom(rect1.right());

        rect.setLeft(rect1.top());
        rect.setRight(rect1.bottom());
#endif
    }
    painter.drawText(rect, align, text);
    if (vertical) {
        QTransform transform = painter.transform();
        transform.rotate(0);
        painter.setTransform(transform);
    }
}

void RenderArea::paintEvent(QPaintEvent * /* event */) {
    QPoint centerPoint = {width() / 2, height() / 2};
    float SCALE = 2;
    QPainter painter(this);

    painter.setBrush(brushEmpty);
    painter.setCompositionMode(QPainter::CompositionMode_Darken);

    QTransform transform;
    transform.scale(1.0 / SCALE, 1.0 / SCALE);
    painter.setTransform(transform);
    centerPoint.setX(centerPoint.x() * SCALE);
    centerPoint.setY(centerPoint.y() * SCALE);
    symbol.paint(painter, centerPoint);
}
