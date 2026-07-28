#ifndef UNIVERSITA_H
#define UNIVERSITA_H

#include <QString>
#include "attivita.h"

class Universita : public Attivita {
    QString m_titolo;
    QString m_tipo;
    QString m_docente;
    QString m_aula;
    QString m_data;
    QString m_orario;

public:
    void setTitolo(const QString& v)     { m_titolo = v; }
    void setTipo(const QString& v)       { m_tipo = v; }
    void setDocente(const QString& v)    { m_docente = v; }
    void setAula(const QString& v)       { m_aula = v; }
    void setData(const QString& v)       { m_data = v; }
    void setOrario(const QString& v)     { m_orario = v; }

    const QString& titolo()     const { return m_titolo; }
    const QString& tipo()       const { return m_tipo; }
    const QString& docente()    const { return m_docente; }
    const QString& aula()       const { return m_aula; }
    const QString& data()       const { return m_data; }
    const QString& orario()     const { return m_orario; }

    void accept(AttivitaVisitor& v) const override { v.visit(*this); }
};

#endif
