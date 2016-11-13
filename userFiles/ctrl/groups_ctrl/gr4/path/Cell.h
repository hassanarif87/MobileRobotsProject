#ifndef CELL_H
#define CELL_H

#include "namespace_ctrl.h"
#include "CtrlStruct_gr4.h"
#include "Link.h"
#include <vector>

NAMESPACE_INIT(ctrlGr4);

class Cell
{
public:
    using LinkIt = std::vector<Link*>::iterator;

    Cell();
    ~Cell();
    Cell(double x, double y, double size);

    double getX(){ return m_x; }
    double getY(){ return m_y; }
    double getS(){ return m_size; }

private:
    double m_x;
    double m_y;
    double m_size; // cm, assuming square cells

    std::vector<Link*> m_linkVector;
};

NAMESPACE_CLOSE();

#endif // CELL_H
