#pragma once
#include <QColor>
#include <QString>

class ColorGenerator {
public:
    static QColor generateNickColor(const QString& nickname);
    
private:
    static QList<QColor> predefinedColors;
    static int hashString(const QString& str);
};