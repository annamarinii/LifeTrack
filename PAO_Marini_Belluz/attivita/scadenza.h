#ifndef SCADENZA_H
#define SCADENZA_H

#include <QString>
#include "attivita.h"

class Scadenza : public Attivita {
    QString m_titolo;
    QString m_descrizione;
    QString m_data;
    QString m_categoria;
    int     m_priorita = 0;

public:
    void setTitolo(const QString& v)        { m_titolo = v; }
    void setDescrizione(const QString& v)   { m_descrizione = v; }
    void setData(const QString& v)          { m_data = v; }
    void setCategoria(const QString& v)     { m_categoria = v; }
    void setPriorita(int v)                 { m_priorita = v; }

    const QString& titolo()       const { return m_titolo; }
    const QString& descrizione()  const { return m_descrizione; }
    const QString& data()         const { return m_data; }
    const QString& categoria()    const { return m_categoria; }
    int            priorita()     const { return m_priorita; }

    void accept(AttivitaVisitor& v) const override { v.visit(*this); }
};

#endif
