#ifndef JSONSAVEVISITOR_H
#define JSONSAVEVISITOR_H

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "attivita/attivitavisitor.h"
#include "attivita/impegno.h"
#include "attivita/promemoria.h"
#include "attivita/scadenza.h"
#include "attivita/universita.h"
#include "attivita/lavoro.h"
#include "attivita/compleanno.h"

class JsonSaveVisitor : public AttivitaVisitor {
public:
    JsonSaveVisitor();
    void reset();

    void visit(const Impegno&     v) override;
    void visit(const Promemoria&  v) override;
    void visit(const Scadenza&    v) override;
    void visit(const Universita&  v) override;
    void visit(const Lavoro&      v) override;
    void visit(const Compleanno&  v) override;

    const QJsonDocument& document() const { return m_doc; }

private:
    QJsonDocument m_doc;
    QJsonObject   m_root;

    static void append(QJsonObject& root, const char* key, const QJsonObject& item) {
        QJsonArray arr = root.value(QLatin1String(key)).toArray();
        arr.append(item);
        root.insert(QLatin1String(key), arr);
    }

    static inline QString s(int v)             { return QString::number(v); }
    static inline QString s(double v)          { return QString::number(v, 'f', 2); }
    static inline QString s(const QString& v)  { return v; }
};

#endif // JSONSAVEVISITOR_H
