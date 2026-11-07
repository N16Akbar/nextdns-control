#pragma once

#include <QString>
#include <QStringList>

namespace Stash {
enum class Status { Missing, Valid, Invalid };

struct Result {
  Status status;
  QStringList values;
  QString error;
};

QString path(const QString &directory, const QString &uuid);
QString write(const QString &directory, const QString &uuid,
              const QStringList &values);
Result read(const QString &directory, const QString &uuid);
QString remove(const QString &directory, const QString &uuid);
bool validValues(const QStringList &values);
} // namespace Stash
