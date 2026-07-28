#ifndef ATTIVITA_H
#define ATTIVITA_H

#include "attivitavisitor.h"

class Attivita {
public:
    virtual ~Attivita() {}
    virtual void accept(AttivitaVisitor& v) const = 0;
};

#endif
