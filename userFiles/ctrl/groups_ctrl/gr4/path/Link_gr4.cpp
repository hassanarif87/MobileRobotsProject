#include "Link_gr4.h"


NAMESPACE_INIT(ctrlGr4);

Link::Link()
{
}

Link::Link(int id, double length) : m_goalNodeId(id), m_length(length)
{
}

NAMESPACE_CLOSE();
