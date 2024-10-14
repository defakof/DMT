#ifndef MODDINGTOOLSUI_H
#define MODDINGTOOLSUI_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QTabWidget>
#include <QStatusBar>
#include <QGroupBox>
#include <QSplitter>
#include <QSizePolicy>
#include <QFileDialog>
#include <QDialog>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QComboBox>
#include <QStyledItemDelegate>
#include "pakscanner.h"
#include <QListWidget>
#include <QMessageBox>
#include <QProgressDialog>

class ModItemDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    ModItemDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class ModdingToolsUI : public QMainWindow {
    Q_OBJECT

public:
    ModdingToolsUI(QWidget *parent = nullptr);

private:
    void createInitialUI();
    void createMainUI();
    void setupConnections();
    void selectModOrganizerExe();
    void loadModList();
    void scanMods();
    void loadSettings();
    void saveSettings();
    void updateMOPathLabel();
    void updateProfileList();
    void loadMainUI();
    void handleDivineNotFound();
    void handleDivineDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void handleDivineDownloadFinished();

    PakScanner *pakScanner;
    QString moExePath;
    QString profilePath;
    QTreeWidget *modTree;
    QListWidget *translationList;
    QLabel *statusLabel;
    QLabel *pluginLabel;
    QLabel *translationCount;
    QLabel *downloadCount;
    QLabel *moPathLabel;
    QComboBox *profileComboBox;
    QProgressDialog *downloadProgressDialog;

    private slots:
        void updateStatus(const QString &message);
        void onDivineNotFound();
    void onItemClicked(QTreeWidgetItem *item, int column);
    void onScanButtonClicked();
};

#endif