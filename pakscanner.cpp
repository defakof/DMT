#include "pakscanner.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>

PakScanner::PakScanner(QObject *parent) : QObject(parent) {
    divine_path = QFileInfo(QCoreApplication::applicationFilePath()).absolutePath() + "/divine.exe";
    temp_path = QFileInfo(QCoreApplication::applicationFilePath()).absolutePath() + "/temp";
    retain_temp_files = false;
    network_manager = new QNetworkAccessManager(this);
    timer = new QTimer(this);
    timer->setSingleShot(true);
}

void PakScanner::set_divine_path(const QString &path) {
    divine_path = path;
}

bool PakScanner::check_divine_exists() {
    return QFile::exists(divine_path);
}

void PakScanner::set_temp_path(const QString &path) {
    temp_path = path;
}

void PakScanner::set_retain_temp_files(bool retain) {
    retain_temp_files = retain;
}

QString PakScanner::get_latest_divine_version() {
    QString owner = "Norbyte";
    QString project = "lslib";
    QUrl url("https://api.github.com/repos/" + owner + "/" + project + "/tags");
    qInfo() << "Fetching latest Divine version from:" << url.toString();

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = network_manager->get(request);

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    // Set up a timer for timeout
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(5000);  // 5 second timeout

    // Wait for either finished() or timeout
    loop.exec();

    if (timer.isActive()) {
        // Request finished before timeout
        timer.stop();
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response_data = reply->readAll();
            QJsonDocument json = QJsonDocument::fromJson(response_data);
            QJsonArray tags = json.array();
            if (!tags.isEmpty()) {
                QString version = tags[0].toObject()["name"].toString();
                reply->deleteLater();
                return version;
            }
        } else {
            qWarning() << "Error fetching latest version:" << reply->errorString();
        }
    } else {
        qWarning() << "Request timed out";
    }

    reply->deleteLater();
    return QString("Timeout");
}

void PakScanner::download_divine() {
    QString latest_version = get_latest_divine_version();
    if (latest_version == "Timeout" || latest_version.isEmpty()) {
        emit progress_updated("Failed to fetch the latest Divine version");
        return;
    }

    // Remove 'v' prefix if present
    if (latest_version.startsWith('v')) {
        latest_version.remove(0, 1);
    }

    QUrl url("https://github.com/Norbyte/lslib/releases/download/v" + latest_version + "/ExportTool-v" + latest_version + ".zip");
    qInfo() << "Downloading Divine from:" << url.toString();

    QNetworkRequest request(url);
    QNetworkReply *reply = network_manager->get(request);

    connect(reply, &QNetworkReply::downloadProgress, this, &PakScanner::divine_download_progress);
    connect(reply, &QNetworkReply::finished, [this, reply, latest_version]() {
        if (reply->error() == QNetworkReply::NoError) {
            QString zip_path = temp_path + "/ExportTool-v" + latest_version + ".zip";
            QFile file(zip_path);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(reply->readAll());
                file.close();
                emit progress_updated("Downloaded ExportTool zip to: " + zip_path);
                // Here you should add code to extract the zip file and set the divine_path
                extract_divine_zip(zip_path, latest_version);
            } else {
                emit progress_updated("Error saving ExportTool zip file to: " + zip_path);
            }
        } else {
            emit progress_updated("Error downloading ExportTool: " + reply->errorString());
        }
        reply->deleteLater();
    });
}

void PakScanner::extract_divine_zip(const QString &zip_path, const QString &version) {
    // Add code here to extract the zip file later xd

    divine_path = temp_path + "/ExportTool-v" + version + "/divine.exe";

    if (QFile::exists(divine_path)) {
        emit progress_updated("Divine extracted successfully to: " + divine_path);
        emit divine_download_finished();
    } else {
        emit progress_updated("Failed to extract Divine executable");
    }
}


void PakScanner::scan_mod_folder(const QString &mod_folder, const QString &mod_manager) {
    if (!checkDivine()) {
        return;  // Stop the scanning process if Divine is not found
    }

    emit progress_updated("Scanning mod folder: " + mod_folder);

    QStringList pak_files;
    if (mod_manager == "Mod Organizer 2" || mod_manager == "Vortex") {
        pak_files = find_pak_files(mod_folder);
    } else if (mod_manager == "BG3 Mod Manager") {
        emit progress_updated("BG3 Mod Manager support not yet implemented");
        return;
    } else {
        emit progress_updated("Unsupported mod manager: " + mod_manager);
        return;
    }

    for (const QString &pak_file : pak_files) {
        process_pak_file(pak_file);
    }
}

QStringList PakScanner::find_pak_files(const QString &folder) {
    QStringList pak_files;
    QDirIterator it(folder, QStringList() << "*.pak", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        pak_files << it.next();
    }
    return pak_files;
}

void PakScanner::process_pak_file(const QString &pak_file) {
    emit progress_updated("Processing PAK file: " + pak_file);

    QString extract_dir = temp_path + "/" + QFileInfo(pak_file).fileName() + "_extracted";
    QDir().mkpath(extract_dir);

    extract_pak(pak_file, extract_dir, QStringList() << "Localization" << "Mods");
    process_extracted_files(extract_dir);
}

void PakScanner::extract_pak(const QString &pak_file, const QString &extract_dir, const QStringList &folders_to_extract) {
    for (const QString &folder : folders_to_extract) {
        QStringList args;
        args << "-g" << "bg3"
             << "-s" << pak_file
             << "-d" << extract_dir
             << "-a" << "extract-package"
             << "-e" << folder + "/*"
             << "-l" << "info";

        QProcess process;
        process.start(divine_path, args);
        process.waitForFinished(-1);

        if (process.exitCode() == 0) {
            emit progress_updated("Extracted " + folder + " from " + pak_file + " to " + extract_dir);
        } else {
            emit progress_updated("Error extracting " + folder + " from " + pak_file + ": " + process.errorString());
        }
    }
}

bool PakScanner::checkDivine() {
    if (!check_divine_exists()) {
        emit divine_not_found();
        return false;
    }
    return true;
}

void PakScanner::process_extracted_files(const QString &extract_dir) {
    QString localization_dir = extract_dir + "/Localization";
    QString mods_dir = extract_dir + "/Mods";

    if (QDir(localization_dir).exists()) {
        process_localization(localization_dir);
    }

    if (QDir(mods_dir).exists()) {
        process_mods(mods_dir);
    }
}

void PakScanner::process_localization(const QString &localization_dir) {
    QDir dir(localization_dir);
    QStringList lang_dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &lang_dir : lang_dirs) {
        emit progress_updated("Found localization for language: " + lang_dir);
        // Add logic to process localization files here later
    }
}

void PakScanner::process_mods(const QString &mods_dir) {
    QDir dir(mods_dir);
    QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &subdir : subdirs) {
        QString mcm_blueprint = mods_dir + "/" + subdir + "/MCM_blueprint.json";
        if (QFile::exists(mcm_blueprint)) {
            emit progress_updated("Found MCM_blueprint.json in " + subdir);
            // Add logic to process MCM_blueprint.json here later
        }
    }
}

void PakScanner::cleanup_temp_files() {
    if (!retain_temp_files && QDir(temp_path).exists()) {
        QDir dir(temp_path);
        if (dir.removeRecursively()) {
            emit progress_updated("Cleaned up temporary files in " + temp_path);
        } else {
            emit progress_updated("Error cleaning up temporary files in " + temp_path);
        }
    }
}