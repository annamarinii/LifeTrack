#ifndef COMPLEANNO_H
#define COMPLEANNO_H

#include <QString>
#include "attivita.h"

class Compleanno : public Attivita {
    QString m_nome;
    QString m_data;
    QString m_promemoria;
    QString m_tipologia;

public:
    void setNome(const QString& v)        { m_nome = v; }
    void setData(const QString& v)        { m_data = v; }
    void setPromemoria(const QString& v)  { m_promemoria = v; }
    void setTipologia(const QString& v)   { m_tipologia = v; }

    const QString& nome()       const { return m_nome; }
    const QString& data()       const { return m_data; }
    const QString& promemoria() const { return m_promemoria; }
    const QString& tipologia()  const { return m_tipologia; }

    void accept(AttivitaVisitor& v) const override { v.visit(*this); }
};

#endif
