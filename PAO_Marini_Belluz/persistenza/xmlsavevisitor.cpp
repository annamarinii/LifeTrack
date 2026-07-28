#include "persistenza/xmlsavevisitor.h"

static QDomElement addField(QDomDocument& d, QDomElement& parent,
                            const QString& name, const QString& value)
{
    QDomElement f = d.createElement("Field");
    f.setAttribute("name", name);
    f.appendChild(d.createTextNode(value));
    parent.appendChild(f);
    return f;
}

XmlSaveVisitor::XmlSaveVisitor()
{
    m_doc  = QDomDocument("Archivio");
    auto pi = m_doc.createProcessingInstruction("xml","version=\"1.0\" encoding=\"UTF-8\"");
    m_doc.appendChild(pi);
    m_root = m_doc.createElement("Archivio");
    m_doc.appendChild(m_root);
}

void XmlSaveVisitor::visit(const Impegno& v) {
    QDomElement cat = m_doc.createElement("Impegni");
    m_root.appendChild(cat);
    QDomElement it  = m_doc.createElement("Item");
    cat.appendChild(it);

    addField(m_doc, it, "Titolo",      v.titolo());
    addField(m_doc, it, "Descrizione", v.descrizione());
    addField(m_doc, it, "Luogo",       v.luogo());
    addField(m_doc, it, "Data",        v.data());
    addField(m_doc, it, "Orario",      v.orario());
}

void XmlSaveVisitor::visit(const Promemoria& v) {
    QDomElement cat = m_doc.createElement("Promemoria");
    m_root.appendChild(cat);
    QDomElement it  = m_doc.createElement("Item");
    cat.appendChild(it);

    addField(m_doc, it, "Titolo",    v.titolo());
    addField(m_doc, it, "Contenuto", v.contenuto());
    addField(m_doc, it, "Data",      v.data());
}

void XmlSaveVisitor::visit(const Scadenza& v) {
    QDomElement cat = m_doc.createElement("Scadenze");
    m_root.appendChild(cat);
    QDomElement it  = m_doc.createElement("Item");
    cat.appendChild(it);

    addField(m_doc, it, "Titolo",      v.titolo());
    addField(m_doc, it, "Descrizione", v.descrizione());
    addField(m_doc, it, "Data",        v.data());
    addField(m_doc, it, "Categoria",   v.categoria());
    addField(m_doc, it, "Priorita",    QString::number(v.priorita()));
}

void XmlSaveVisitor::visit(const Universita& v) {
    QDomElement cat = m_doc.createElement("Universita");
    m_root.appendChild(cat);
    QDomElement it  = m_doc.createElement("Item");
    cat.appendChild(it);

    addField(m_doc, it, "Titolo",   v.titolo());
    addField(m_doc, it, "Tipo",     v.tipo());
    addField(m_doc, it, "Docente",  v.docente());
    addField(m_doc, it, "Aula",     v.aula());
    addField(m_doc, it, "Data",     v.data());
    addField(m_doc, it, "Orario",   v.orario());
}

void XmlSaveVisitor::visit(const Lavoro& v) {
    QDomElement cat = m_doc.createElement("Lavoro");
    m_root.appendChild(cat);
    QDomElement it  = m_doc.createElement("Item");
    cat.appendChild(it);

    addField(m_doc, it, "Titolo",   v.titolo());
    addField(m_doc, it, "Azienda",  v.azienda());
    addField(m_doc, it, "Luogo",    v.luogo());
    addField(m_doc, it, "Data",     v.data());
    addField(m_doc, it, "Orario",   v.orario());
}

void XmlSaveVisitor::visit(const Compleanno& v) {
    QDomElement cat = m_doc.createElement("Compleanni");
    m_root.appendChild(cat);
    QDomElement it  = m_doc.createElement("Item");
    cat.appendChild(it);

    addField(m_doc, it, "Nome",       v.nome());
    addField(m_doc, it, "Data",       v.data());
    addField(m_doc, it, "Promemoria", v.promemoria());
    addField(m_doc, it, "Tipologia",  v.tipologia());
}
