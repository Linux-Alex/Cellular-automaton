#include "cell_2d.h"

Cell_2D::Cell_2D() {
    isAlive = false;
}

Material Cell_2D::selectedMaterial = air; // Define and initialize the static member variable

int Cell_2D::lifeExpectancy = 10; // Define and initialize the static member variable

QList<QColor> Cell_2D::waterColors = {
    QColor(135, 206, 250, 100), // Light Blue - RGBA: 135, 206, 250, 100
    QColor(135, 206, 250, 150), // Medium Blue - RGBA: 135, 206, 250, 150
    QColor(135, 206, 250, 200), // Dark Blue - RGBA: 135, 206, 250, 200
    QColor(135, 206, 250, 255)  // Very Dark Blue - RGBA: 135, 206, 250, 255 (Fully opaque)
}; // Define and initialize the static member variable
