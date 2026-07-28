#include "persistenza/jsonsavevisitor.h"

JsonSaveVisitor::JsonSaveVisitor() {
    reset();
}

void JsonSaveVisitor::reset() {
    m_root = QJsonObject{
        { "Impegni",     QJsonArray() },
        { "Promemoria",  QJsonArray() },
        { "Scadenze",    QJsonArray() },
        { "Universita",  QJsonArray() },
        { "Lavoro",      QJsonArray() },
        { "Compleanni",  QJsonArray() }
    };
    m_doc.setObject(m_root);
}

void JsonSaveVisitor::visit(const Impegno& v) {
    QJsonObject item{
        { "Titolo",      s(v.titolo())      },
        { "Descrizione", s(v.descrizione()) },
        { "Luogo",       s(v.luogo())       },
        { "Data",        s(v.data())        },
        { "Orario",      s(v.orario())      }
    };
    append(m_root, "Impegni", item);
    m_doc.setObject(m_root);
}

void JsonSaveVisitor::visit(const Promemoria& v) {
    QJsonObject item{
        { "Titolo",    s(v.titolo())    },
        { "Contenuto", s(v.contenuto()) },
        { "Data",      s(v.data())      }
    };
    append(m_root, "Promemoria", item);
    m_doc.setObject(m_root);
}

void JsonSaveVisitor::visit(const Scadenza& v) {
    QJsonObject item{
        { "Titolo",     s(v.titolo())     },
        { "Descrizione",s(v.descrizione())},
        { "Data",       s(v.data())       },
        { "Categoria",  s(v.categoria())  },
        { "Priorita",   s(v.priorita())   }
    };
    append(m_root, "Scadenze", item);
    m_doc.setObject(m_root);
}

void JsonSaveVisitor::visit(const Universita& v) {
    QJsonObject item{
        { "Titolo",   s(v.titolo())   },
        { "Tipo",     s(v.tipo())     },
        { "Docente",  s(v.docente())  },
        { "Aula",     s(v.aula())     },
        { "Data",     s(v.data())     },
        { "Orario",   s(v.orario())   }
    };
    append(m_root, "Universita", item);
    m_doc.setObject(m_root);
}

void JsonSaveVisitor::visit(const Lavoro& v) {
    QJsonObject item{
        { "Titolo",   s(v.titolo())   },
        { "Azienda",  s(v.azienda())  },
        { "Luogo",    s(v.luogo())    },
        { "Data",     s(v.data())     },
        { "Orario",   s(v.orario())   }
    };
    append(m_root, "Lavoro", item);
    m_doc.setObject(m_root);
}

void JsonSaveVisitor::visit(const Compleanno& v) {
    QJsonObject item{
        { "Nome",       s(v.nome())       },
        { "Data",       s(v.data())       },
        { "Promemoria", s(v.promemoria()) },
        { "Tipologia",  s(v.tipologia())  }
    };
    append(m_root, "Compleanni", item);
    m_doc.setObject(m_root);
}
