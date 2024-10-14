#ifndef PAKSCANNER_H
#define PAKSCANNER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QDirIterator>
#include <QProcess>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTimer>
#include <QEventLoop>

class PakScanner : public QObject {
    Q_OBJECT

public:
    bool checkDivine();
    explicit PakScanner(QObject *parent = nullptr);
    void set_temp_path(const QString &path);
    void set_retain_temp_files(bool retain);
    void scan_mod_folder(const QString &mod_folder, const QString &mod_manager);

    signals:
        void progress_updated(const QString &message);
    void divine_not_found();
    void divine_download_progress(qint64 bytesReceived, qint64 bytesTotal);
    void divine_download_finished();

    public slots:
        void set_divine_path(const QString &path);
    void download_divine();

private:
    QString divine_path;
    QString temp_path;
    bool retain_temp_files;
    QNetworkAccessManager *network_manager;
    QTimer *timer;

    bool check_divine_exists();
    QString get_latest_divine_version();
    QStringList find_pak_files(const QString &folder);
    void process_pak_file(const QString &pak_file);
    void extract_pak(const QString &pak_file, const QString &extract_dir, const QStringList &folders_to_extract);
    void process_extracted_files(const QString &extract_dir);
    void process_localization(const QString &localization_dir);
    void process_mods(const QString &mods_dir);
    void cleanup_temp_files();
    void extract_divine_zip(const QString &zip_path, const QString &version);
};

#endif