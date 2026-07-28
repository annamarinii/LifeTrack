#ifndef XMLSAVEVISITOR_H
#define XMLSAVEVISITOR_H

#include <QString>
#include <QDomDocument>

#include "attivita/attivitavisitor.h"
#include "attivita/impegno.h"
#include "attivita/promemoria.h"
#include "attivita/scadenza.h"
#include "attivita/universita.h"
#include "attivita/lavoro.h"
#include "attivita/compleanno.h"

class XmlSaveVisitor : public AttivitaVisitor {
public:
    XmlSaveVisitor();

    void visit(const Impegno& v) override;
    void visit(const Promemoria& v) override;
    void visit(const Scadenza& v) override;
    void visit(const Universita& v) override;
    void visit(const Lavoro& v) override;
    void visit(const Compleanno& v) override;

    const QDomDocument& document() const { return m_doc; }

private:
    QDomDocument m_doc;
    QDomElement  m_root;
};

#endif // XMLSAVEVISITOR_H
