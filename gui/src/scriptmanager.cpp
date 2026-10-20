#include "scriptmanager.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJSEngine>
#include <QJSValue>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QUrl>

namespace {
class ScriptApi final : public QObject
{
    Q_OBJECT
public:
    explicit ScriptApi(ScriptManager *manager, QString scriptName)
        : QObject(manager), manager(manager), scriptName(std::move(scriptName)) {}

    Q_INVOKABLE void log(const QString &message) { emit manager->scriptMessage(QStringLiteral("info"), prefix(message)); }
    Q_INVOKABLE void warn(const QString &message) { emit manager->scriptMessage(QStringLiteral("warning"), prefix(message)); }
    Q_INVOKABLE void error(const QString &message) { emit manager->scriptMessage(QStringLiteral("error"), prefix(message)); }
    Q_INVOKABLE void notify(const QString &title, const QString &message)
    {
        emit manager->scriptMessage(QStringLiteral("notify"), title + QStringLiteral(": ") + message);
    }
    Q_INVOKABLE QString appVersion() const { return QCoreApplication::applicationVersion(); }
    Q_INVOKABLE QString scriptsDirectory() const;

private:
    QString prefix(const QString &message) const
    {
        return QStringLiteral("[%1] %2").arg(scriptName, message);
    }

    ScriptManager *manager;
    QString scriptName;
};

QString ScriptApi::scriptsDirectory() const
{
    return manager->scriptsDirectory();
}
}

ScriptManager::ScriptManager(QObject *parent)
    : QObject(parent)
{
    directory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/scripts");
    QDir().mkpath(directory);
    load();
}

QVariantList ScriptManager::scripts() const
{
    QVariantList result;
    for (const Entry &entry : entries) {
        QVariantMap item;
        item.insert(QStringLiteral("fileName"), entry.fileName);
        item.insert(QStringLiteral("name"), QFileInfo(entry.fileName).completeBaseName());
        item.insert(QStringLiteral("enabled"), entry.enabled);
        item.insert(QStringLiteral("error"), entry.error);
        result.append(item);
    }
    return result;
}

QString ScriptManager::scriptsDirectory() const
{
    return directory;
}

void ScriptManager::load()
{
    entries.clear();
    QDir dir(directory);
    const QStringList files = dir.entryList({QStringLiteral("*.js")}, QDir::Files, QDir::Name);
    QSettings settings(directory + QStringLiteral("/scripts.ini"), QSettings::IniFormat);
    for (const QString &fileName : files) {
        Entry entry;
        entry.fileName = fileName;
        entry.enabled = settings.value(QStringLiteral("enabled/%1").arg(fileName), true).toBool();
        entries.append(entry);
    }
}

void ScriptManager::save() const
{
    QSettings settings(directory + QStringLiteral("/scripts.ini"), QSettings::IniFormat);
    settings.remove(QStringLiteral("enabled"));
    for (const Entry &entry : entries)
        settings.setValue(QStringLiteral("enabled/%1").arg(entry.fileName), entry.enabled);
    settings.sync();
}

ScriptManager::Entry *ScriptManager::find(const QString &fileName)
{
    for (Entry &entry : entries)
        if (entry.fileName == fileName)
            return &entry;
    return nullptr;
}

const ScriptManager::Entry *ScriptManager::find(const QString &fileName) const
{
    for (const Entry &entry : entries)
        if (entry.fileName == fileName)
            return &entry;
    return nullptr;
}

bool ScriptManager::validFileName(const QString &fileName) const
{
    return !fileName.isEmpty() && QFileInfo(fileName).fileName() == fileName && fileName.endsWith(QStringLiteral(".js"), Qt::CaseInsensitive);
}

bool ScriptManager::addScriptFromFile(const QUrl &source)
{
    const QString sourcePath = source.isLocalFile() ? source.toLocalFile() : QString();
    QFileInfo info(sourcePath);
    if (!info.exists() || !info.isFile() || info.suffix().compare(QStringLiteral("js"), Qt::CaseInsensitive) != 0)
        return false;

    QString fileName = info.fileName();
    if (find(fileName)) {
        const QString base = info.completeBaseName();
        int index = 2;
        do {
            fileName = QStringLiteral("%1-%2.js").arg(base).arg(index++);
        } while (find(fileName));
    }
    if (!QFile::copy(sourcePath, directory + QStringLiteral("/") + fileName))
        return false;
    Entry entry;
    entry.fileName = fileName;
    entries.append(entry);
    save();
    emit scriptsChanged();
    return true;
}

bool ScriptManager::createScript(const QString &name)
{
    QString base = name.trimmed();
    if (base.isEmpty())
        base = QStringLiteral("new-script");
    base.replace(QRegularExpression(QStringLiteral("[^A-Za-z0-9_-]")), QStringLiteral("-"));
    QString fileName = base.endsWith(QStringLiteral(".js")) ? base : base + QStringLiteral(".js");
    int index = 2;
    while (find(fileName))
        fileName = QStringLiteral("%1-%2.js").arg(base).arg(index++);

    QFile file(directory + QStringLiteral("/") + fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::NewOnly | QIODevice::Text))
        return false;
    file.write("// Ruby RemotePlay user script\n// Available hooks: onAppStart(api, event), onBeforeConnect(api, event),\n// onSessionStarted(api, event), onBeforeDisconnect(api, event)\n\nfunction onAppStart(api, event) {\n    api.log('Script loaded');\n}\n");
    file.close();
    Entry entry;
    entry.fileName = fileName;
    entries.append(entry);
    save();
    emit scriptsChanged();
    return true;
}

bool ScriptManager::removeScript(const QString &fileName)
{
    if (!validFileName(fileName) || !find(fileName))
        return false;
    if (!QFile::remove(directory + QStringLiteral("/") + fileName))
        return false;
    entries.removeIf([&](const Entry &entry) { return entry.fileName == fileName; });
    save();
    emit scriptsChanged();
    return true;
}

bool ScriptManager::setScriptEnabled(const QString &fileName, bool enabled)
{
    Entry *entry = find(fileName);
    if (!entry)
        return false;
    entry->enabled = enabled;
    save();
    emit scriptsChanged();
    return true;
}

bool ScriptManager::evaluate(const Entry &entry, const QString &hook, const QVariantMap &event)
{
    QFile file(directory + QStringLiteral("/") + entry.fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    const QString source = QString::fromUtf8(file.readAll());
    file.close();

    QJSEngine engine;
    ScriptApi api(this, QFileInfo(entry.fileName).completeBaseName());
    engine.globalObject().setProperty(QStringLiteral("api"), engine.newQObject(&api));
    engine.globalObject().setProperty(QStringLiteral("event"), engine.toScriptValue(event));
    QJSValue result = engine.evaluate(source, entry.fileName);
    if (result.isError()) {
        const QString error = QStringLiteral("%1:%2: %3").arg(entry.fileName).arg(result.property(QStringLiteral("lineNumber")).toInt()).arg(result.toString());
        if (Entry *mutableEntry = find(entry.fileName))
            mutableEntry->error = error;
        emit scriptMessage(QStringLiteral("error"), error);
        return false;
    }
    if (!hook.isEmpty()) {
        const QJSValue function = engine.globalObject().property(hook);
        if (function.isCallable()) {
            QJSValueList args;
            args << engine.newQObject(&api) << engine.toScriptValue(event);
            result = function.call(args);
            if (result.isError()) {
                const QString error = QStringLiteral("%1:%2: %3").arg(entry.fileName).arg(result.property(QStringLiteral("lineNumber")).toInt()).arg(result.toString());
                if (Entry *mutableEntry = find(entry.fileName))
                    mutableEntry->error = error;
                emit scriptMessage(QStringLiteral("error"), error);
                return false;
            }
        }
    }
    if (Entry *mutableEntry = find(entry.fileName))
        mutableEntry->error.clear();
    return true;
}

bool ScriptManager::runScript(const QString &fileName)
{
    const Entry *entry = find(fileName);
    if (!entry)
        return false;
    const bool ok = evaluate(*entry, QString(), {});
    emit scriptsChanged();
    return ok;
}

int ScriptManager::runHook(const QString &hook, const QVariantMap &event)
{
    int executed = 0;
    const QList<Entry> snapshot = entries;
    for (const Entry &entry : snapshot) {
        if (!entry.enabled)
            continue;
        if (evaluate(entry, hook, event))
            ++executed;
    }
    emit scriptsChanged();
    return executed;
}

void ScriptManager::openScriptsFolder()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(directory));
}

#include "scriptmanager.moc"
