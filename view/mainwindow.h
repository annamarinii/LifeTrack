#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPersistentModelIndex>

class QStandardItemModel;
class QAbstractItemModel;
class QModelIndex;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QAbstractItemModel* impegniModel() const;
    QAbstractItemModel* scadenzeModel() const;
    QAbstractItemModel* universitaModel() const;
    QAbstractItemModel* lavoroModel() const;
    QAbstractItemModel* promemoriaModel() const;
    QAbstractItemModel* compleanniModel() const;

    void exportAllJson();
    void exportAllXml();

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;

private slots:
    void onSearchReturnPressed();

    void showImpegni();
    void showScadenze();
    void showUniversita();
    void showLavoro();
    void showPromemoria();
    void showCompleanni();

    void onResultActivated(const QModelIndex& idx);

    void onAddClicked();
    void onEditClicked();
    void onDeleteClicked();

    void onLoadJsonClicked();
    void onSaveJsonClicked();
    void onLoadXmlClicked();
    void onSaveXmlClicked();

    void goHome();
    void goBackToResults();

    void onbtnAnnullaclicked();
    void onbtnSalvaclicked();

    void onoggettoImpegniclicked();
    void onoggettoScadenzeclicked();
    void onoggettoUniversitaclicked();
    void onoggettoLavoroclicked();
    void onoggettoPromemoriaclicked();
    void onoggettoCompleanniclicked();

private:
    enum class Category {
        All = 0,
        Impegno,
        Scadenza,
        Universita,
        Lavoro,
        Promemoria,
        Compleanno
    };

    Ui::MainWindow *ui;

    QStandardItemModel* m_impegnoModel = nullptr;
    QStandardItemModel* m_scadenzaModel = nullptr;
    QStandardItemModel* m_universitaModel = nullptr;
    QStandardItemModel* m_lavoroModel = nullptr;
    QStandardItemModel* m_promemoriaModel = nullptr;
    QStandardItemModel* m_compleannoModel = nullptr;

    QStandardItemModel* m_resultsList = nullptr;

    Category m_category = Category::All;
    QString m_term;

    QString m_detailTipo;
    QPersistentModelIndex m_detailIndex;

    void hookModelSignals();
    void openResultsPage();

    bool rowMatches(QAbstractItemModel* src, int row, const QString& term) const;
    QString buildResultName(QAbstractItemModel* src, int row) const;
    void addListItem(const QString& tipo, QAbstractItemModel* src, int row);
    void refreshResultsList();

    void openEditorFor(const QString& tipo, const QPersistentModelIndex& pIdx);

    bool currentSelection(QString& tipoOut, QPersistentModelIndex& idxOut) const;

    bool loadModel(QAbstractItemModel* m, const QVariantList& rows);
};

#endif
