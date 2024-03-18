#ifndef CELL_2D_H
#define CELL_2D_H

#include <QGraphicsRectItem>
#include <QBrush>

enum Material {
    sand,
    water,
    wall,
    air,
    fire,
    darkSmoke,
    lightSmoke,
    wood,
    border,
};

enum CellType {
    CaveAutomaton,
    GameOfLife,
};

class Cell_2D : public QGraphicsRectItem
{
public:
    Cell_2D();

    bool isAlive;
    Material material;
    CellType cellType;

    int waterDensity = 0;

    int age = 0;

    static Material selectedMaterial; // Define and initialize the static member variable

    static int lifeExpectancy; // Define and initialize the static member variable

    static QList<QColor> waterColors; // Define and initialize the static member variable

    Cell_2D(QGraphicsItem* parent = nullptr) : QGraphicsRectItem(parent), isAlive(false) {
        setRect(0, 0, 10, 10); // Set cell size
    }

    void setState(bool alive) {
        isAlive = alive;
        setBrush(QBrush(isAlive ? Qt::white : Qt::black)); // Set cell color based on state
        // cellType = GameOfLife;
    }

    void setMaterial(Material material, int density = 0) {
        this->material = material;
        this->waterDensity = density;

        if(material != water) {
            this->age = 0;
        }

        if(material == sand) {
            setBrush(QBrush(Qt::yellow));
        }
        else if(material == water) {
            // setBrush(QBrush(Qt::blue));
            if(density < 0) {
                this->setMaterial(Material::air);
            }
            else {
                this->waterDensity = qMin(this->waterDensity, 3);
                setBrush(QBrush(this->waterColors[this->waterDensity]));
            }
        }
        else if(material == wall) {
            setBrush(QBrush(Qt::black));
        }
        else if(material == air) {
            setBrush(QBrush(Qt::white));
        }
        else if(material == fire) {
            setBrush(QBrush(Qt::red));
        }
        else if(material == darkSmoke) {
            setBrush(QBrush(Qt::darkGray));
        }
        else if(material == lightSmoke) {
            setBrush(QBrush(Qt::lightGray));
        }
        else if(material == wood) {
            setBrush(QBrush(Qt::darkGreen));
        }

        cellType = CaveAutomaton;
    }

    void ageMaterial() {
        this->age++;
    }

    void setAge(int age) {
        this->age = age;

        if(this->age > lifeExpectancy) {
            setMaterial(Material::air);
        }
    }

    // Copy constructor definition
    Cell_2D(const Cell_2D& other) : QGraphicsRectItem(other.parentItem()), isAlive(other.isAlive), material(other.material), cellType(other.cellType) {
        setRect(other.rect()); // Set cell size
        setBrush(other.brush()); // Set cell color
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        if(cellType == CellType::GameOfLife) {
            isAlive = !isAlive; // Toggle the cell state
            setState(isAlive); // Update the cell appearance
        }
        else if(cellType == CellType::CaveAutomaton) {
            if(this->material != Material::water) {
                this->waterDensity = 0;
            }
            else {
                this->waterDensity = qMin(this->waterDensity + 1, 3);
            }
            setMaterial(selectedMaterial, waterDensity);
        }
    }
};

#endif // CELL_2D_H
