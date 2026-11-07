#include "stash.h"
#include "networkstate.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSaveFile>

namespace {
constexpr int version = 1;
constexpr qint64 maximumSize = 16384;

bool validUuid(const QString &uuid) {
  static const QRegularExpression pattern("^[0-9a-fA-F-]{1,64}$");
  return pattern.match(uuid).hasMatch();
}
} // namespace

QString Stash::path(const QString &directory, const QString &uuid) {
  return validUuid(uuid) ? directory + '/' + uuid + ".json" : QString();
}

bool Stash::validValues(const QStringList &values) {
  if (values.size() != 6) {
    return false;
  }
  if ((values.at(0) != "yes" && values.at(0) != "no") ||
      (values.at(3) != "yes" && values.at(3) != "no")) {
    return false;
  }
  bool ipv4PriorityValid = false;
  bool ipv6PriorityValid = false;
  values.at(2).toInt(&ipv4PriorityValid);
  values.at(5).toInt(&ipv6PriorityValid);
  return ipv4PriorityValid && ipv6PriorityValid &&
         values.at(1).size() <= 4096 && values.at(4).size() <= 4096;
}

QString Stash::write(const QString &directory, const QString &uuid,
                     const QStringList &values) {
  const QString filePath = path(directory, uuid);
  if (filePath.isEmpty() || !validValues(values)) {
    return "The DNS backup is invalid.";
  }
  if (!QDir().mkpath(directory)) {
    return "The DNS backup directory could not be created.";
  }
  QJsonArray array;
  for (const QString &value : values) {
    array.append(value);
  }
  const QByteArray data =
      QJsonDocument(
          QJsonObject{{"version", version}, {"uuid", uuid}, {"values", array}})
          .toJson(QJsonDocument::Compact);
  QSaveFile file(filePath);
  if (!file.open(QIODevice::WriteOnly)) {
    return file.errorString();
  }
  if (file.write(data) != data.size() || !file.commit()) {
    return file.errorString();
  }
  if (!QFile::setPermissions(filePath, QFileDevice::ReadOwner |
                                           QFileDevice::WriteOwner)) {
    return "The DNS backup permissions could not be set.";
  }
  return QString();
}

Stash::Result Stash::read(const QString &directory, const QString &uuid) {
  const QString filePath = path(directory, uuid);
  if (filePath.isEmpty()) {
    return {Status::Invalid, {}, "The connection identifier is invalid."};
  }
  QFile file(filePath);
  bool legacy = false;
  if (!file.exists()) {
    file.setFileName(directory + '/' + uuid);
    legacy = true;
  }
  if (!file.exists()) {
    return {Status::Missing, {}, QString()};
  }
  if (!file.open(QIODevice::ReadOnly)) {
    return {Status::Invalid, {}, file.errorString()};
  }
  if (file.size() > maximumSize) {
    return {Status::Invalid, {}, "The DNS backup is too large."};
  }
  if (legacy) {
    const QStringList values =
        NetworkState::profileValues(QString::fromUtf8(file.readAll()));
    return validValues(values)
               ? Result{Status::Valid, values, QString()}
               : Result{Status::Invalid,
                        {},
                        "The legacy DNS backup contains invalid DNS settings."};
  }
  QJsonParseError parseError;
  const QJsonDocument document =
      QJsonDocument::fromJson(file.readAll(), &parseError);
  if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
    return {Status::Invalid, {}, "The DNS backup is malformed."};
  }
  const QJsonObject object = document.object();
  if (object.value("version").toInt(-1) != version ||
      object.value("uuid").toString() != uuid ||
      !object.value("values").isArray()) {
    return {
        Status::Invalid, {}, "The DNS backup does not match this connection."};
  }
  QStringList values;
  for (const QJsonValue &value : object.value("values").toArray()) {
    if (!value.isString()) {
      return {Status::Invalid, {}, "The DNS backup contains an invalid value."};
    }
    values.append(value.toString());
  }
  return validValues(values)
             ? Result{Status::Valid, values, QString()}
             : Result{Status::Invalid,
                      {},
                      "The DNS backup contains invalid DNS settings."};
}

QString Stash::remove(const QString &directory, const QString &uuid) {
  const QString filePath = path(directory, uuid);
  if (filePath.isEmpty()) {
    return "The connection identifier is invalid.";
  }
  const QString legacyPath = directory + '/' + uuid;
  if (QFile::exists(filePath) && !QFile::remove(filePath)) {
    return "The DNS backup could not be removed.";
  }
  if (QFile::exists(legacyPath) && !QFile::remove(legacyPath)) {
    return "The legacy DNS backup could not be removed.";
  }
  return QString();
}
