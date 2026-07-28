#ifndef ROWADAPTER_H
#define ROWADAPTER_H

#include <memory>
#include <QString>

class QAbstractItemModel;
class Attivita;

std::unique_ptr<Attivita> attivitaFromRow(const QString& tipo,
                                          const QAbstractItemModel* m,
                                          int row);

#endif
