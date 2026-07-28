#ifndef IMPEGNO_H
#define IMPEGNO_H

#include <QString>
#include "attivita.h"

class Impegno : public Attivita {
    QString m_titolo;
    QString m_descrizione;
    QString m_luogo;
    QString m_data;
    QString m_orario;

public:
    void setTitolo(const QString& v)       { m_titolo = v; }
    void setDescrizione(const QString& v)  { m_descrizione = v; }
    void setLuogo(const QString& v)        { m_luogo = v; }
    void setData(const QString& v)         { m_data = v; }
    void setOrario(const QString& v)       { m_orario = v; }

    const QString& titolo()       const { return m_titolo; }
    const QString& descrizione()  const { return m_descrizione; }
    const QString& luogo()        const { return m_luogo; }
    const QString& data()         const { return m_data; }
    const QString& orario()       const { return m_orario; }

    void accept(AttivitaVisitor& v) const override { v.visit(*this); }
};

#endif
