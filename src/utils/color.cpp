#include "color.h"

QList<QColor> ColorGenerator::predefinedColors = {
    QColor("#FF4136"), // Red
    QColor("#0074D9"), // Blue
    QColor("#2ECC40"), // Green
    QColor("#FF851B"), // Orange
    QColor("#B10DC9"), // Purple
    QColor("#39CCCC"), // Teal
    QColor("#FFDC00"), // Yellow
    QColor("#F012BE"), // Magenta
    QColor("#01FF70"), // Lime
    QColor("#7FDBFF")  // Light Blue
};

int ColorGenerator::hashString(const QString& str) {
    int hash = 0;
    for (QChar c : str) {
        hash = ((hash << 5) + hash) + c.unicode();
    }
    return qAbs(hash);
}

QColor ColorGenerator::generateNickColor(const QString& nickname) {
    int index = hashString(nickname) % predefinedColors.size();
    return predefinedColors[index];
}