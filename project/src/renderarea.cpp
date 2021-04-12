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
    QList<KICADLibSchematicDrawElement> drawElements = symbol.getDrawSymbols();

    painter.setBrush(brushEmpty);
    painter.setCompositionMode(QPainter::CompositionMode_Darken);

    QTransform transform;
    transform.scale(1.0 / SCALE, 1.0 / SCALE);
    painter.setTransform(transform);
    centerPoint.setX(centerPoint.x() * SCALE);
    centerPoint.setY(centerPoint.y() * SCALE);

    for (KICADLibSchematicDrawElement &da : drawElements) {
        QPen pen = penForeground;
        int thickness = da.thickness;
        if (thickness < 6) {
            thickness = 6;
        }
        pen.setWidth(thickness);
        painter.setPen(pen);

        if (da.cc_filled == FillType::backgroundColor) {
            painter.setBrush(brushBackground);
        } else if (da.cc_filled == FillType::foregroundColor) {
            painter.setBrush(brushForeground);
        } else {
            painter.setBrush(brushEmpty);
        }

        switch (da.getDrawType()) {
            case DrawType::none:
                break;
            case DrawType::arc: {
                QRect rect;
                QPoint p = da.positions[0];
                p.setY(-p.y());
                p = p + centerPoint;
                rect.setTop(p.y() - da.radius);
                rect.setLeft(p.x() - da.radius);
                rect.setHeight(da.radius * 2);
                rect.setWidth(da.radius * 2);
                if (thickness == 20) {
                    qDebug() << "thickness 20";
                }
                double start_deg = da.angle_start;
                double end_deg = da.angle_end;
                if (start_deg < 0) {
                    start_deg = 360 + start_deg;
                }
                if (end_deg < 0) {
                    end_deg = 360 + end_deg;
                }
                double span_deg = end_deg - start_deg;
                if (((start_deg < 90) && (end_deg > 270)) || ((start_deg > 270) && (end_deg < 90))) {
                    if (span_deg < 0) {
                        span_deg = 360 + span_deg;

                    } else {
                        span_deg = 360 - span_deg;
                        span_deg = -span_deg;
                    }
                }

                span_deg = fmodf(span_deg, 360);
                if (fabs(span_deg) > 180.0) {
                    start_deg = end_deg;
                }
                span_deg = fmodf(span_deg, 180);
                if (da.cc_filled == FillType::none) {
                    painter.drawArc(rect, start_deg * 16, span_deg * 16);
                } else {
                    painter.drawChord(rect, start_deg * 16, span_deg * 16);
                }
            } break;
            case DrawType::circle: {
                QRect rect;
                QPoint p = da.positions[0];
                p.setY(-p.y());
                p = p + centerPoint;
                rect.setTop(p.y() - da.radius);
                rect.setLeft(p.x() - da.radius);
                rect.setHeight(da.radius * 2);
                rect.setWidth(da.radius * 2);
                painter.drawEllipse(rect);
                break;
            }
            case DrawType::pin: {
                QPoint points[3];
                TextDirection textDirection = TextDirection::Horizontal;
                KicadSymbolFieldJustify_t text_hjustify = ksfj_center;
                KicadSymbolFieldJustify_t text_vjustify = ksfj_center;
                const int TEXTSIZE = da.pinNameTextSize;
                const int TEXTMARGIN = symbol.def.text_offset;

                bool drawPinName = symbol.def.drawPinName;
                points[0] = da.positions[0];
                points[0].setY(-points[0].y());

                points[1] = points[0] + centerPoint;
                points[0] = points[0] + centerPoint;
                if (da.orientation == ksr_up) {
                    points[1].setY(points[0].y() - da.length);
                    textDirection = TextDirection::Vertical;
                    points[2] = points[1];
                    points[2].setY(points[2].y() - TEXTMARGIN);
                    text_hjustify = ksfj_center;
                    text_vjustify = ksfj_bottom;

                } else if (da.orientation == ksr_down) {
                    points[1].setY(points[0].y() + da.length);
                    textDirection = TextDirection::Vertical;
                    points[2] = points[1];
                    points[2].setY(points[2].y() + TEXTMARGIN);
                    text_hjustify = ksfj_center;
                    text_vjustify = ksfj_top;
                } else if (da.orientation == ksr_right) {
                    points[1].setX(points[0].x() + da.length);
                    points[2] = points[1];
                    points[2].setX(points[2].x() + TEXTMARGIN);
                    text_hjustify = ksfj_left;
                    text_vjustify = ksfj_center;
                } else if (da.orientation == ksr_left) {
                    points[1].setX(points[0].x() - da.length);
                    points[2] = points[1];
                    points[2].setX(points[2].x() - TEXTMARGIN);
                    text_hjustify = ksfj_right;
                    text_vjustify = ksfj_center;
                }
                painter.drawPolyline(points, 2);
                QRect connectCircle;
                const int CONNNECTCIRCLE_DIAMETER = 20;
                connectCircle.setTop(points[0].y() - CONNNECTCIRCLE_DIAMETER / 2);
                connectCircle.setLeft(points[0].x() - CONNNECTCIRCLE_DIAMETER / 2);
                connectCircle.setWidth(CONNNECTCIRCLE_DIAMETER);
                connectCircle.setHeight(CONNNECTCIRCLE_DIAMETER);
                painter.drawEllipse(connectCircle);
                painter.setPen(penPinName);
                if ((da.name != "~") && (drawPinName)) {
                    drawText(painter, points[2], textDirection, text_hjustify, text_vjustify, TextStyle::normal, TEXTSIZE, da.name);
                }
            } break;
            case DrawType::polygon: {
                QPolygon polygon;
                for (QPoint p : da.positions) {
                    p.setY(-p.y());
                    polygon << p + centerPoint;
                }
                if (da.cc_filled == FillType::none) {
                    painter.drawPolyline(polygon);
                } else {
                    painter.drawPolygon(polygon);
                }
            } break;
            case DrawType::text: {
                QPoint p = da.positions[0];
                p.setY(-p.y());

                drawText(painter, p + centerPoint, da.textDirection, da.text_hjustify, da.text_vjustify, da.textStyle, da.textSize, da.text);

                break;
            }
            case DrawType::rectangle: {
                QRect rect;
                QPoint s = da.start;
                QPoint e = da.end;
                s.setY(-s.y());
                e.setY(-e.y());
                rect.setTopLeft(s + centerPoint);
                rect.setBottomRight(e + centerPoint);
                painter.drawRect(rect);
                break;
            }
        }
    }
}
