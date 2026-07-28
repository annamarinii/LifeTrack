#ifndef PROMEMORIA_H
#define PROMEMORIA_H

#include <QString>
#include "attivita.h"

class Promemoria : public Attivita {
    QString m_titolo;
    QString m_contenuto;
    QString m_data;
    QString m_tipologia;

public:
    void setTitolo(const QString& v)    { m_titolo = v; }
    void setContenuto(const QString& v) { m_contenuto = v; }
    void setData(const QString& v)      { m_data = v; }
    void setTipologia(const QString& v) { m_tipologia = v; }

    const QString& titolo()    const { return m_titolo; }
    const QString& contenuto() const { return m_contenuto; }
    const QString& data()      const { return m_data; }
    const QString& tipologia() const { return m_tipologia; }

    void accept(AttivitaVisitor& v) const override { v.visit(*this); }
};

#endif
