#pragma once

#include <QObject>
#include <QUrl>
#include <QVariantMap>
#include <QVariantList>
#include <QString>

class ScriptManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList scripts READ scripts NOTIFY scriptsChanged)
    Q_PROPERTY(QString scriptsDirectory READ scriptsDirectory CONSTANT)

public:
    explicit ScriptManager(QObject *parent = nullptr);

    QVariantList scripts() const;
    QString scriptsDirectory() const;

    Q_INVOKABLE bool addScriptFromFile(const QUrl &source);
    Q_INVOKABLE bool createScript(const QString &name);
    Q_INVOKABLE bool removeScript(const QString &fileName);
    Q_INVOKABLE bool setScriptEnabled(const QString &fileName, bool enabled);
    Q_INVOKABLE bool runScript(const QString &fileName);
    Q_INVOKABLE int runHook(const QString &hook, const QVariantMap &event = {});
    Q_INVOKABLE void openScriptsFolder();

signals:
    void scriptsChanged();
    void scriptMessage(const QString &level, const QString &message);

private:
    struct Entry {
        QString fileName;
        bool enabled = true;
        QString error;
    };

    void load();
    void save() const;
    Entry *find(const QString &fileName);
    const Entry *find(const QString &fileName) const;
    bool validFileName(const QString &fileName) const;
    bool evaluate(const Entry &entry, const QString &hook, const QVariantMap &event);

    QString directory;
    QList<Entry> entries;
};
