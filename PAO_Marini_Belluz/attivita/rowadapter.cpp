#include "attivita/rowadapter.h"

#include <QAbstractItemModel>

#include "attivita/attivita.h"
#include "attivita/impegno.h"
#include "attivita/scadenza.h"
#include "attivita/universita.h"
#include "attivita/lavoro.h"
#include "attivita/promemoria.h"
#include "attivita/compleanno.h"

static int colByHeader(const QAbstractItemModel* m, const QString& wanted) {
    for (int c = 0; c < m->columnCount(); ++c) {
        if (m->headerData(c, Qt::Horizontal).toString().compare(wanted, Qt::CaseInsensitive) == 0)
            return c;
    }
    return -1;
}

static QString atS(const QAbstractItemModel* m, int row, const QString& header) {
    int c = colByHeader(m, header);
    if (c < 0) return {};
    return m->index(row, c).data().toString().trimmed();
}

std::unique_ptr<Attivita> attivitaFromRow(const QString& tipo,
                                          const QAbstractItemModel* m,
                                          int row)
{
    if (!m || row < 0 || row >= m->rowCount()) return nullptr;

    const QString t = tipo.toLower();

    if (t == "impegno") {
        auto x = std::make_unique<Impegno>();
        x->setTitolo(atS(m,row,"Titolo"));
        x->setDescrizione(atS(m,row,"Descrizione"));
        x->setLuogo(atS(m,row,"Luogo"));
        x->setData(atS(m,row,"Data"));
        x->setOrario(atS(m,row,"Orario"));
        return x;
    }

    if (t == "scadenza") {
        auto x = std::make_unique<Scadenza>();
        x->setTitolo(atS(m,row,"Titolo"));
        x->setDescrizione(atS(m,row,"Descrizione"));
        x->setData(atS(m,row,"Data"));
        x->setCategoria(atS(m,row,"Categoria"));
        x->setPriorita(atS(m,row,"Priorita").toInt());
        return x;
    }

    if (t == "universita") {
        auto x = std::make_unique<Universita>();
        x->setTitolo(atS(m,row,"Titolo"));
        x->setTipo(atS(m,row,"Tipo"));
        x->setDocente(atS(m,row,"Docente"));
        x->setAula(atS(m,row,"Aula"));
        x->setData(atS(m,row,"Data"));
        x->setOrario(atS(m,row,"Orario"));
        return x;
    }

    if (t == "lavoro") {
        auto x = std::make_unique<Lavoro>();
        x->setTitolo(atS(m,row,"Titolo"));
        x->setAzienda(atS(m,row,"Azienda"));
        x->setLuogo(atS(m,row,"Luogo"));
        x->setData(atS(m,row,"Data"));
        x->setOrario(atS(m,row,"Orario"));
        return x;
    }

    if (t == "promemoria") {
        auto x = std::make_unique<Promemoria>();
        x->setTitolo(atS(m,row,"Titolo"));
        x->setContenuto(atS(m,row,"Contenuto"));
        x->setData(atS(m,row,"Data"));
        x->setTipologia(atS(m,row,"Tipologia"));
        return x;
    }

    if (t == "compleanno") {
        auto x = std::make_unique<Compleanno>();
        x->setNome(atS(m,row,"Nome"));
        x->setData(atS(m,row,"Data"));
        x->setPromemoria(atS(m,row,"Promemoria"));
        x->setTipologia(atS(m,row,"Tipologia"));
        return x;
    }

    return nullptr;
}
