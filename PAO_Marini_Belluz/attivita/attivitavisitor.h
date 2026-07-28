#ifndef ATTIVITAVISITOR_H
#define ATTIVITAVISITOR_H

class Impegno;
class Scadenza;
class Universita;
class Lavoro;
class Promemoria;
class Compleanno;

class AttivitaVisitor {
public:
    virtual ~AttivitaVisitor() {}
    virtual void visit(const Impegno&)     = 0;
    virtual void visit(const Scadenza&)    = 0;
    virtual void visit(const Universita&)  = 0;
    virtual void visit(const Lavoro&)      = 0;
    virtual void visit(const Promemoria&)  = 0;
    virtual void visit(const Compleanno&)  = 0;
};

#endif
