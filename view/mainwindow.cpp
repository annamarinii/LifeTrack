#include "view/mainwindow.h"
#include "ui_mainwindow.h"

#include "attivita/rowadapter.h"
#include "attivita/attivita.h"

#include "persistenza/jsonsavevisitor.h"
#include "persistenza/xmlsavevisitor.h"

#include <QGuiApplication>
#include <QScreen>
#include <QListView>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QDateEdit>
#include <QTimeEdit>
#include <QLocale>
#include <QFileInfo>
#include <QDir>
#include <QToolButton>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QPixmap>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDomDocument>
#include <QEvent>
#include <QMouseEvent>
#include <QCoreApplication>
#include <QDebug>
#include <QStandardItemModel>
#include <QAbstractItemModel>
#include <QAbstractItemView>
#include <QFile>

static QString stripAccents(QString s) {
    s = s.normalized(QString::NormalizationForm_D);
    QString out;
    out.reserve(s.size());
    for (QChar ch : s) {
        if (ch.category() != QChar::Mark_NonSpacing &&
            ch.category() != QChar::Mark_SpacingCombining &&
            ch.category() != QChar::Mark_Enclosing) {
            out.append(ch);
        }
    }
    return out;
}

static QString canonicalKey(QString k) {
    k = k.trimmed();
    k = stripAccents(k);

    if (k.compare("Lezione", Qt::CaseInsensitive) == 0)  return "Tipo";
    if (k.compare("Priorita", Qt::CaseInsensitive) == 0) return "Priorita";

    return k;
}

static int colByHeaderCanonical(const QAbstractItemModel* m, const QString& wantedCanonical) {
    for (int c = 0; c < m->columnCount(); ++c) {
        const QString h = canonicalKey(m->headerData(c, Qt::Horizontal, Qt::DisplayRole).toString());
        if (h.compare(wantedCanonical, Qt::CaseInsensitive) == 0) return c;
    }
    return -1;
}

static void setFieldValue(QWidget* w, const QString& v) {
    if (auto le = qobject_cast<QLineEdit*>(w))      { le->setText(v); return; }
    if (auto sb = qobject_cast<QSpinBox*>(w))       { sb->setValue(v.toInt()); return; }
    if (auto ds = qobject_cast<QDoubleSpinBox*>(w)) { ds->setValue(QLocale().toDouble(v)); return; }
    if (auto de = qobject_cast<QDateEdit*>(w)) {
        QDate d;
        if (v.contains('/')) d = QLocale().toDate(v, "dd/MM/yyyy");
        if (!d.isValid() && v.toInt()>0) d = QDate(v.toInt(),1,1);
        if (d.isValid()) de->setDate(d);
        return;
    }
    if (auto te = qobject_cast<QTimeEdit*>(w))      { te->setTime(QTime::fromString(v,"HH:mm")); return; }
    if (auto cb = qobject_cast<QComboBox*>(w))      {
        int i = cb->findText(v, Qt::MatchFixedString|Qt::MatchCaseSensitive);
        if (i>=0) cb->setCurrentIndex(i);
        else if (cb->isEditable()) cb->setEditText(v);
        return;
    }
    if (auto l  = qobject_cast<QLabel*>(w))         { l->setText(v); return; }
}

static bool populateFormFromRow(QWidget* page, const QAbstractItemModel* m, int row) {
    if (!page || !m || row < 0) return false;
    QFormLayout* form = page->findChild<QFormLayout*>();
    if (!form) return false;

    auto at = [&](int c){ return (c >= 0) ? m->index(row, c).data(Qt::DisplayRole).toString() : QString(); };

    for (int r = 0; r < form->rowCount(); ++r) {
        auto *li = form->itemAt(r, QFormLayout::LabelRole);
        auto *fi = form->itemAt(r, QFormLayout::FieldRole);
        auto *lab = li ? qobject_cast<QLabel*>(li->widget()) : nullptr;
        QWidget* field = fi ? fi->widget() : nullptr;
        if (!lab || !field) continue;

        const QString key = canonicalKey(lab->text());
        const int c = colByHeaderCanonical(m, key);
        if (c < 0) continue;
        setFieldValue(field, at(c));
    }

    return true;
}

static void clearForm(QWidget* page) {
    if (!page) return;
    if (auto form = page->findChild<QFormLayout*>()) {
        for (int r=0;r<form->rowCount();++r) {
            if (auto *fi = form->itemAt(r, QFormLayout::FieldRole))
                setFieldValue(fi->widget(), QString());
        }
    }
}

static QWidget* findEditorPageByTipo(Ui::MainWindow* ui, const QString& tipo) {
    if (!ui || !ui->editorStack) return nullptr;

    auto matchAny = [&](std::initializer_list<const char*> prefs)->QWidget* {
        for (int i = 0; i < ui->editorStack->count(); ++i) {
            QWidget* w = ui->editorStack->widget(i);
            if (!w) continue;
            const QString n = w->objectName();
            for (auto p : prefs)
                if (n.startsWith(QLatin1String(p), Qt::CaseInsensitive)) return w;
        }
        return nullptr;
    };

    if (tipo.compare("Impegno",    Qt::CaseInsensitive)==0) return matchAny({"impegnoform"});
    if (tipo.compare("Scadenza",   Qt::CaseInsensitive)==0) return matchAny({"scadenzaform"});
    if (tipo.compare("Universita", Qt::CaseInsensitive)==0) return matchAny({"universitaform"});
    if (tipo.compare("Lavoro",     Qt::CaseInsensitive)==0) return matchAny({"lavoroform"});
    if (tipo.compare("Promemoria", Qt::CaseInsensitive)==0) return matchAny({"promemoriaform"});
    if (tipo.compare("Compleanno", Qt::CaseInsensitive)==0) return matchAny({"compleannoform"});
    return nullptr;
}

static bool readFormIntoMap(QWidget* page, QVariantMap& out){
    out.clear();
    if (!page) return false;

    QFormLayout* form = page->findChild<QFormLayout*>();
    if (!form) return false;

    auto textOf = [](QWidget* w)->QString{
        if (!w) return {};
        if (auto le = qobject_cast<QLineEdit*>(w))        return le->text();
        if (auto sb = qobject_cast<QSpinBox*>(w))         return QString::number(sb->value());
        if (auto ds = qobject_cast<QDoubleSpinBox*>(w))   return QString::number(ds->value());
        if (auto de = qobject_cast<QDateEdit*>(w))        return de->date().toString("dd/MM/yyyy");
        if (auto te = qobject_cast<QTimeEdit*>(w))        return te->time().toString("HH:mm");
        if (auto cb = qobject_cast<QComboBox*>(w))        return cb->currentText();
        if (auto lab= qobject_cast<QLabel*>(w))           return lab->text();
        return {};
    };

    for (int r = 0; r < form->rowCount(); ++r){
        auto *li = form->itemAt(r, QFormLayout::LabelRole);
        auto *fi = form->itemAt(r, QFormLayout::FieldRole);
        auto *lab = li ? qobject_cast<QLabel*>(li->widget()) : nullptr;
        QWidget* field = fi ? fi->widget() : nullptr;

        QString key = lab ? lab->text().trimmed() : QString();
        if (key.isEmpty() || !field) continue;

        key = canonicalKey(key);
        out[key] = textOf(field).trimmed();
    }
    return true;
}

template<typename Obj, typename Func>
static void connectButton(QWidget* root, const char* name, Obj* receiver, Func slot) {
    if (auto tb = root->findChild<QToolButton*>(name)) {
        QObject::connect(tb, &QToolButton::clicked, receiver, slot);
    } else if (auto pb = root->findChild<QPushButton*>(name)) {
        QObject::connect(pb, &QPushButton::clicked, receiver, slot);
    }
}

static QAbstractItemModel* modelByTipo(MainWindow* w, const QString& tipo) {
    if (tipo == "Impegno")     return w->impegniModel();
    if (tipo == "Scadenza")    return w->scadenzeModel();
    if (tipo == "Universita")  return w->universitaModel();
    if (tipo == "Lavoro")      return w->lavoroModel();
    if (tipo == "Promemoria")  return w->promemoriaModel();
    if (tipo == "Compleanno")  return w->compleanniModel();
    return nullptr;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    const QRect screenGeometry = QGuiApplication::primaryScreen()->geometry();
    this->move((screenGeometry.width()-width())/2, (screenGeometry.height()-height())/2);

    auto make = [&](QStringList headers){
        auto *m = new QStandardItemModel(0, headers.size(), this);
        m->setHorizontalHeaderLabels(headers);
        return m;
    };

    m_impegnoModel    = make({"Titolo","Descrizione","Luogo","Data","Orario"});
    m_scadenzaModel   = make({"Titolo","Descrizione","Data","Categoria","Priorita"});
    m_universitaModel = make({"Titolo","Tipo","Docente","Aula","Data","Orario"});
    m_lavoroModel     = make({"Titolo","Azienda","Luogo","Data","Orario"});
    m_promemoriaModel = make({"Titolo","Contenuto","Data", "Tipologia"});
    m_compleannoModel = make({"Nome","Data","Promemoria","Tipologia"});

    m_resultsList = new QStandardItemModel(this);

    if (ui->listResults) {
        ui->listResults->setModel(m_resultsList);
        ui->listResults->setSelectionMode(QAbstractItemView::SingleSelection);
        connect(ui->listResults, &QListView::clicked,       this, &MainWindow::onResultActivated);
        connect(ui->listResults, &QListView::doubleClicked, this, &MainWindow::onResultActivated);
    }

    connect(ui->searchBar, &QLineEdit::returnPressed, this, &MainWindow::onSearchReturnPressed);

    connect(ui->oggettoImpegni,     &QToolButton::clicked, this, &MainWindow::showImpegni);
    connect(ui->oggettoScadenze,    &QToolButton::clicked, this, &MainWindow::showScadenze);
    connect(ui->oggettoUniversita,  &QToolButton::clicked, this, &MainWindow::showUniversita);
    connect(ui->oggettoLavoro,      &QToolButton::clicked, this, &MainWindow::showLavoro);
    connect(ui->oggettoPromemoria,  &QToolButton::clicked, this, &MainWindow::showPromemoria);
    connect(ui->oggettoCompleanni,  &QToolButton::clicked, this, &MainWindow::showCompleanni);

    connect(ui->btnIndietroRisultati, &QToolButton::clicked, this, &MainWindow::goHome);
    connect(ui->btnAggiungi,          &QPushButton::clicked, this, &MainWindow::onAddClicked);
    connect(ui->btnModifica,          &QPushButton::clicked, this, &MainWindow::onEditClicked);
    connect(ui->btnElimina,           &QPushButton::clicked, this, &MainWindow::onDeleteClicked);
    connect(ui->btnCaricaJSON,        &QPushButton::clicked, this, &MainWindow::onLoadJsonClicked);
    connect(ui->btnCaricaXML,         &QPushButton::clicked, this, &MainWindow::onLoadXmlClicked);
    connect(ui->btnSalvaJSON,         &QPushButton::clicked, this, &MainWindow::onSaveJsonClicked);
    connect(ui->btnSalvaXML,          &QPushButton::clicked, this, &MainWindow::onSaveXmlClicked);

    connect(ui->btnAnnulla, &QPushButton::clicked, this, &MainWindow::onbtnAnnullaclicked);
    connect(ui->btnSalva,   &QPushButton::clicked, this, &MainWindow::onbtnSalvaclicked);

    ui->stackedWidget->setCurrentWidget(ui->paginaPrincipale);
    hookModelSignals();
}

MainWindow::~MainWindow() { delete ui; }

QAbstractItemModel* MainWindow::impegniModel()     const { return m_impegnoModel; }
QAbstractItemModel* MainWindow::scadenzeModel()    const { return m_scadenzaModel; }
QAbstractItemModel* MainWindow::universitaModel()  const { return m_universitaModel; }
QAbstractItemModel* MainWindow::lavoroModel()      const { return m_lavoroModel; }
QAbstractItemModel* MainWindow::promemoriaModel()  const { return m_promemoriaModel; }
QAbstractItemModel* MainWindow::compleanniModel()  const { return m_compleannoModel; }

void MainWindow::hookModelSignals() {
    auto wire = [this](QAbstractItemModel* m){
        if (!m) return;
        auto refreshIfVisible = [this]{
            if (ui->stackedWidget->currentWidget() == ui->paginaRisultati)
                refreshResultsList();
        };
        connect(m, &QAbstractItemModel::rowsInserted, this, refreshIfVisible);
        connect(m, &QAbstractItemModel::rowsRemoved,  this, refreshIfVisible);
        connect(m, &QAbstractItemModel::dataChanged,  this, refreshIfVisible);
        connect(m, &QAbstractItemModel::modelReset,   this, refreshIfVisible);
    };
    wire(impegniModel());
    wire(scadenzeModel());
    wire(universitaModel());
    wire(lavoroModel());
    wire(promemoriaModel());
    wire(compleanniModel());
}

void MainWindow::onSearchReturnPressed() { m_category = Category::All; m_term = ui->searchBar->text(); openResultsPage(); }
void MainWindow::showImpegni()     { m_category = Category::Impegno;     m_term = ui->searchBar->text(); openResultsPage(); }
void MainWindow::showScadenze()    { m_category = Category::Scadenza;    m_term = ui->searchBar->text(); openResultsPage(); }
void MainWindow::showUniversita()  { m_category = Category::Universita;  m_term = ui->searchBar->text(); openResultsPage(); }
void MainWindow::showLavoro()      { m_category = Category::Lavoro;      m_term = ui->searchBar->text(); openResultsPage(); }
void MainWindow::showPromemoria()  { m_category = Category::Promemoria;  m_term = ui->searchBar->text(); openResultsPage(); }
void MainWindow::showCompleanni()  { m_category = Category::Compleanno;  m_term = ui->searchBar->text(); openResultsPage(); }

void MainWindow::openResultsPage() {
    static const char* names[] = {"Tutti","Impegni","Scadenze","Universita","Lavoro","Promemoria","Compleanni"};
    ui->lblRisultato->setText(names[(int)m_category]);
    refreshResultsList();
    ui->stackedWidget->setCurrentWidget(ui->paginaRisultati);
}

bool MainWindow::rowMatches(QAbstractItemModel* src, int row, const QString& term) const {
    if (!src) return false;
    const QString needle = term.trimmed().toLower();
    if (needle.isEmpty()) return true;
    for (int c = 0; c < src->columnCount(); ++c) {
        const QString cell = src->index(row, c).data(Qt::DisplayRole).toString().toLower();
        if (cell.contains(needle)) return true;
    }
    return false;
}

QString MainWindow::buildResultName(QAbstractItemModel* src, int row) const {
    auto hdr = [&](int c){ return src->headerData(c, Qt::Horizontal, Qt::DisplayRole).toString().trimmed(); };
    auto val = [&](int c){ return src->index(row, c).data(Qt::DisplayRole).toString().trimmed(); };

    auto colBy = [&](const QString& wanted){
        for (int c=0; c<src->columnCount(); ++c)
            if (hdr(c).compare(wanted, Qt::CaseInsensitive) == 0) return c;
        return -1;
    };

    QString titolo;
    int cTitolo = colBy("Titolo");
    if (cTitolo >= 0) titolo = val(cTitolo);

    if (titolo.isEmpty()) {
        int cNome = colBy("Nome");
        if (cNome >= 0) titolo = val(cNome);
    }

    QString when;
    int cData = colBy("Data");
    if (cData >= 0) when = val(cData);

    QString s = titolo;
    if (!when.isEmpty()) s += (s.isEmpty() ? "" : " — ") + when;
    if (s.isEmpty()) s = "[Senza titolo]";
    return s;
}

void MainWindow::addListItem(const QString& tipo, QAbstractItemModel* src, int row) {
    const QString label = buildResultName(src, row);
    auto *it = new QStandardItem(label);
    it->setEditable(false);
    QPersistentModelIndex pIdx(src->index(row, 0));
    it->setData(QVariant::fromValue(pIdx), Qt::UserRole + 1);
    it->setData(tipo,                      Qt::UserRole + 2);
    m_resultsList->appendRow(it);
}

void MainWindow::refreshResultsList() {
    if (m_resultsList->rowCount())
        m_resultsList->removeRows(0, m_resultsList->rowCount());

    auto scan = [&](const QString& tipo, QAbstractItemModel* model){
        if (!model) return;
        for (int r = 0; r < model->rowCount(); ++r)
            if (rowMatches(model, r, m_term))
                addListItem(tipo, model, r);
    };

    if (m_category == Category::All) {
        scan("Impegno",     impegniModel());
        scan("Scadenza",    scadenzeModel());
        scan("Universita",  universitaModel());
        scan("Lavoro",      lavoroModel());
        scan("Promemoria",  promemoriaModel());
        scan("Compleanno",  compleanniModel());
    } else if (m_category == Category::Impegno)     scan("Impegno", impegniModel());
    else if (m_category == Category::Scadenza)     scan("Scadenza", scadenzeModel());
    else if (m_category == Category::Universita)   scan("Universita", universitaModel());
    else if (m_category == Category::Lavoro)       scan("Lavoro", lavoroModel());
    else if (m_category == Category::Promemoria)   scan("Promemoria", promemoriaModel());
    else if (m_category == Category::Compleanno)   scan("Compleanno", compleanniModel());
}

void MainWindow::openEditorFor(const QString& tipo, const QPersistentModelIndex& pIdx) {
    QWidget* page = findEditorPageByTipo(ui, tipo);
    if (!page) {
        QMessageBox::warning(this, "Editor", "Pagina editor non trovata per " + tipo);
        return;
    }
    auto *model = const_cast<QAbstractItemModel*>(pIdx.model());
    m_detailTipo  = tipo;
    m_detailIndex = pIdx;

    populateFormFromRow(page, model, pIdx.row());
    ui->editorStack->setCurrentWidget(page);
    ui->stackedWidget->setCurrentWidget(ui->paginaEdit);
}

void MainWindow::onResultActivated(const QModelIndex& idx) {
    if (!idx.isValid()) return;

    const QModelIndex mi = m_resultsList->index(idx.row(), 0);
    const QString tipo = mi.data(Qt::UserRole + 2).toString();
    const QVariant v   = mi.data(Qt::UserRole + 1);
    if (!v.isValid()) return;

    const auto pIdx = v.value<QPersistentModelIndex>();
    if (!pIdx.isValid()) return;

    openEditorFor(tipo, pIdx);
}

bool MainWindow::currentSelection(QString& tipoOut, QPersistentModelIndex& idxOut) const {
    if (!m_resultsList || m_resultsList->rowCount() == 0) return false;

    QModelIndex sel = ui->listResults->currentIndex();
    if (!sel.isValid()) return false;

    const QModelIndex mi = m_resultsList->index(sel.row(), 0);
    const QVariant v  = mi.data(Qt::UserRole + 1);
    const QString tp = mi.data(Qt::UserRole + 2).toString();
    if (!v.isValid() || tp.isEmpty()) return false;

    tipoOut = tp;
    idxOut  = v.value<QPersistentModelIndex>();
    return idxOut.isValid();
}

void MainWindow::onAddClicked() {
    QString tipoTarget;
    switch (m_category) {
    case Category::Impegno:     tipoTarget="Impegno"; break;
    case Category::Scadenza:    tipoTarget="Scadenza"; break;
    case Category::Universita:  tipoTarget="Universita"; break;
    case Category::Lavoro:      tipoTarget="Lavoro"; break;
    case Category::Promemoria:  tipoTarget="Promemoria"; break;
    case Category::Compleanno:  tipoTarget="Compleanno"; break;
    case Category::All: {
        const QStringList cats = {"Impegno","Scadenza","Universita","Lavoro","Promemoria","Compleanno"};
        bool ok=false;
        const QString choice = QInputDialog::getItem(this, "Nuovo elemento", "Categoria:", cats, 0, false, &ok);
        if (!ok) return;
        tipoTarget = choice;
        break;
    }
    }

    QWidget* page = findEditorPageByTipo(ui, tipoTarget);
    if (!page) { QMessageBox::warning(this,"Nuovo","Pagina editor non trovata per "+tipoTarget); return; }

    m_detailTipo.clear();
    m_detailIndex = QPersistentModelIndex();
    clearForm(page);

    ui->editorStack->setCurrentWidget(page);
    ui->stackedWidget->setCurrentWidget(ui->paginaEdit);
}

void MainWindow::onEditClicked() {
    QString tipo; QPersistentModelIndex pIdx;
    if (!currentSelection(tipo, pIdx)) {
        QMessageBox::information(this, "Modifica", "Seleziona un elemento dalla lista.");
        return;
    }
    openEditorFor(tipo, pIdx);
}

void MainWindow::onDeleteClicked() {
    QString tipo; QPersistentModelIndex pIdx;
    if (!currentSelection(tipo, pIdx)) {
        QMessageBox::information(this, "Elimina", "Seleziona un elemento dalla lista.");
        return;
    }
    if (QMessageBox::question(this, "Conferma eliminazione",
                              "Vuoi eliminare l'elemento selezionato?") != QMessageBox::Yes) return;

    if (auto *src = const_cast<QAbstractItemModel*>(pIdx.model())) {
        if (!src->removeRow(pIdx.row(), pIdx.parent()))
            QMessageBox::warning(this, "Elimina", "Impossibile eliminare l'elemento dal modello.");
        m_term.clear();
        refreshResultsList();
        ui->stackedWidget->setCurrentWidget(ui->paginaRisultati);
    }
}

bool MainWindow::loadModel(QAbstractItemModel* m, const QVariantList& rows){
    if (!m) return false;
    if (m->rowCount()>0) m->removeRows(0, m->rowCount());
    const int cols = m->columnCount();
    for (const QVariant& v : rows){
        const QVariantMap obj = v.toMap();
        const int row = m->rowCount();
        if (!m->insertRow(row)) return false;
        for (int i=0; i<cols; ++i){
            const QString h = m->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
            QVariant val = obj.value(h);
            if (!val.isValid()) val = obj.value(QString("col%1").arg(i));
            m->setData(m->index(row,i), val);
        }
    }
    return true;
}

void MainWindow::exportAllJson() { onSaveJsonClicked(); }
void MainWindow::exportAllXml()  { onSaveXmlClicked(); }

void MainWindow::onSaveJsonClicked() {
    const QString file = QFileDialog::getSaveFileName(this, "Salva JSON", QString(), "JSON (*.json)");
    if (file.isEmpty()) return;

    JsonSaveVisitor vis;

    auto collect = [&](const QString& tipo, QAbstractItemModel* m){
        if (!m) return;
        for (int r=0; r<m->rowCount(); ++r) {
            if (auto obj = attivitaFromRow(tipo, m, r))
                obj->accept(vis);
        }
    };

    collect("Impegno",     impegniModel());
    collect("Scadenza",    scadenzeModel());
    collect("Universita",  universitaModel());
    collect("Lavoro",      lavoroModel());
    collect("Promemoria",  promemoriaModel());
    collect("Compleanno",  compleanniModel());

    QFile f(file);
    if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Salva JSON", "Impossibile aprire il file.");
        return;
    }
    f.write(vis.document().toJson(QJsonDocument::Indented));
    f.close();
    QMessageBox::information(this,"Salva JSON","Dati salvati (Visitor).");
}

void MainWindow::onLoadJsonClicked() {
    const QString file = QFileDialog::getOpenFileName(this, "Carica JSON", QString(), "JSON (*.json)");
    if (file.isEmpty()) return;

    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)){ QMessageBox::warning(this,"Carica JSON","Impossibile aprire il file."); return; }
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject()){ QMessageBox::warning(this,"Carica JSON","Formato non valido."); return; }
    const QJsonObject root = doc.object();

    loadModel(impegniModel(),     root.value("Impegni").toArray().toVariantList());
    loadModel(scadenzeModel(),    root.value("Scadenze").toArray().toVariantList());
    loadModel(universitaModel(),  root.value("Universita").toArray().toVariantList());
    loadModel(lavoroModel(),      root.value("Lavoro").toArray().toVariantList());
    loadModel(promemoriaModel(),  root.value("Promemoria").toArray().toVariantList());
    loadModel(compleanniModel(),  root.value("Compleanni").toArray().toVariantList());

    m_term.clear();
    refreshResultsList();
    ui->stackedWidget->setCurrentWidget(ui->paginaRisultati);

    QMessageBox::information(this,"Carica JSON","Dati caricati.");
}

void MainWindow::onSaveXmlClicked() {
    const QString file = QFileDialog::getSaveFileName(this, "Salva XML", QString(), "XML (*.xml)");
    if (file.isEmpty()) return;

    XmlSaveVisitor vis;

    auto collect = [&](const QString& tipo, QAbstractItemModel* m){
        if (!m) return;
        for (int r=0; r<m->rowCount(); ++r) {
            if (auto obj = attivitaFromRow(tipo, m, r))
                obj->accept(vis);
        }
    };

    collect("Impegno",     impegniModel());
    collect("Scadenza",    scadenzeModel());
    collect("Universita",  universitaModel());
    collect("Lavoro",      lavoroModel());
    collect("Promemoria",  promemoriaModel());
    collect("Compleanno",  compleanniModel());

    QFile f(file);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this,"Salva XML","Impossibile aprire il file.");
        return;
    }
    f.write(vis.document().toByteArray(2));
    f.close();
    QMessageBox::information(this,"Salva XML","Dati salvati (Visitor).");
}

void MainWindow::onLoadXmlClicked() {
    const QString file = QFileDialog::getOpenFileName(this, "Carica XML", QString(), "XML (*.xml)");
    if (file.isEmpty()) return;

    QDomDocument doc;
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly)){
        QMessageBox::warning(this,"Carica XML","Impossibile aprire il file.");
        return;
    }
    if (!doc.setContent(&f)){
        f.close();
        QMessageBox::warning(this,"Carica XML","Formato non valido.");
        return;
    }
    f.close();

    auto readCat = [&](const QString& tag)->QVariantList{
        QVariantList rows;
        auto tags = doc.elementsByTagName(tag);
        if (tags.isEmpty()) return rows;

        auto catEl = tags.at(0).toElement();
        for (QDomNode n = catEl.firstChild(); !n.isNull(); n = n.nextSibling()){
            if (!n.isElement() || n.nodeName() != "Item") continue;

            QVariantMap obj;
            for (QDomNode c = n.firstChild(); !c.isNull(); c = c.nextSibling()){
                if (!c.isElement()) continue;
                auto fieldEl = c.toElement();
                if (fieldEl.tagName() != "Field") continue;
                const QString headerName = fieldEl.attribute("name");
                const QString text       = fieldEl.text();
                obj[headerName] = text;
            }
            rows << obj;
        }
        return rows;
    };

    loadModel(impegniModel(),     readCat("Impegni"));
    loadModel(scadenzeModel(),    readCat("Scadenze"));
    loadModel(universitaModel(),  readCat("Universita"));
    loadModel(lavoroModel(),      readCat("Lavoro"));
    loadModel(promemoriaModel(),  readCat("Promemoria"));
    loadModel(compleanniModel(),  readCat("Compleanni"));

    m_term.clear();
    refreshResultsList();
    ui->stackedWidget->setCurrentWidget(ui->paginaRisultati);

    QMessageBox::information(this,"Carica XML","Dati caricati.");
}

void MainWindow::goHome() { ui->stackedWidget->setCurrentWidget(ui->paginaPrincipale); }
void MainWindow::goBackToResults() { ui->stackedWidget->setCurrentWidget(ui->paginaRisultati); }

void MainWindow::onbtnAnnullaclicked() {
    ui->stackedWidget->setCurrentWidget(ui->paginaRisultati);
}

void MainWindow::onbtnSalvaclicked() {
    QWidget* page = ui->editorStack->currentWidget();
    if (!page) return;

    QString tipo;
    const QString obj = page->objectName();
    if      (obj.startsWith("impegnoform"))     tipo = "Impegno";
    else if (obj.startsWith("scadenzaform"))    tipo = "Scadenza";
    else if (obj.startsWith("universitaform"))  tipo = "Universita";
    else if (obj.startsWith("lavoroform"))      tipo = "Lavoro";
    else if (obj.startsWith("promemoriaform"))  tipo = "Promemoria";
    else if (obj.startsWith("compleannoform"))  tipo = "Compleanno";
    else { QMessageBox::warning(this,"Salva","Tipo pagina non riconosciuto."); return; }

    QAbstractItemModel* model = modelByTipo(this, tipo);
    if (!model) return;

    QVariantMap map;
    if (!readFormIntoMap(page, map)) {
        QMessageBox::warning(this,"Salva","Form non valido.");
        return;
    }

    int row = -1;
    if (m_detailIndex.isValid() && m_detailIndex.model() == model)
        row = m_detailIndex.row();
    else {
        row = model->rowCount();
        model->insertRow(row);
    }

    const int cols = model->columnCount();
    for (int c=0; c<cols; ++c){
        const QString h = canonicalKey(model->headerData(c, Qt::Horizontal, Qt::DisplayRole).toString().trimmed());
        const QVariant v = map.value(h);
        if (v.isValid()) model->setData(model->index(row,c), v);
    }

    m_detailIndex = QPersistentModelIndex();
    m_detailTipo.clear();
    m_term.clear();
    refreshResultsList();
    ui->stackedWidget->setCurrentWidget(ui->paginaRisultati);
}

bool MainWindow::eventFilter(QObject* obj, QEvent* ev) {
    return QMainWindow::eventFilter(obj, ev);
}

void MainWindow::onoggettoImpegniclicked()     { showImpegni(); }
void MainWindow::onoggettoScadenzeclicked()    { showScadenze(); }
void MainWindow::onoggettoUniversitaclicked()  { showUniversita(); }
void MainWindow::onoggettoLavoroclicked()      { showLavoro(); }
void MainWindow::onoggettoPromemoriaclicked()  { showPromemoria(); }
void MainWindow::onoggettoCompleanniclicked()  { showCompleanni(); }
