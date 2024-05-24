#ifndef JSON_H
#define JSON_H

#include <QObject>
#include <QJsonObject>
#include <QJsonDocument>
#include <QString>
#include <QMap>

class Json : public QObject
{
    Q_OBJECT

public:
    Json(QObject *parent = nullptr);

    template<typename... Args>
    void encodeData(const QString& key, const Args&... args)
    {
        QJsonObject jsonObj;
        addData(jsonObj, key, args...);
        QJsonDocument doc(jsonObj);
        QString jsonString = doc.toJson(QJsonDocument::Compact);
        emit dataEncoded(jsonString);
    }

    void decodeData(const QString& jsonString)
    {
        QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
        QMap<QString, QVariant> resultMap;
        if (!doc.isNull()) {
            QJsonObject jsonObj = doc.object();
            for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
                resultMap.insert(it.key(), it.value().toVariant());
            }
            emit dataDecoded(resultMap); // 发送包含解析数据的 QMap
        }
    }


signals:
    void dataEncoded(const QString& json);
    void dataDecoded(const QMap<QString, QVariant>& data);

private:
    template<typename T, typename... Args>
    void addData(QJsonObject& jsonObj, const QString& key, const T& value, const Args&... args)
    {
        jsonObj[key] = QJsonValue::fromVariant(QVariant(value));
        addData(jsonObj, args...);
    }

    void addData(QJsonObject& jsonObj) { /* 递归终止函数 */ }
};


#endif // JSON_H
