#include "moddingtoolsui.h"
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>
#include <QDialogButtonBox>
#include <QPainter>
#include <QApplication>
#include <QStyle>
#include <QListWidget>
#include <QSettings>

ModdingToolsUI::ModdingToolsUI(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Defakof's Modding Tools");
    setGeometry(100, 100, 1200, 800);

    pakScanner = new PakScanner(this);
    connect(pakScanner, &PakScanner::progress_updated, this, &ModdingToolsUI::updateStatus);
    connect(pakScanner, &PakScanner::divine_not_found, this, &ModdingToolsUI::onDivineNotFound);
    connect(pakScanner, &PakScanner::divine_download_progress, this, &ModdingToolsUI::handleDivineDownloadProgress);
    connect(pakScanner, &PakScanner::divine_download_finished, this, &ModdingToolsUI::handleDivineDownloadFinished);

    downloadProgressDialog = nullptr;

    loadSettings();
    createInitialUI();
}


void ModdingToolsUI::onDivineNotFound() {
    static bool handlingDivineNotFound = false;
    if (handlingDivineNotFound) {
        return;  // Prevent multiple dialogs
    }
    handlingDivineNotFound = true;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Divine.exe Not Found",
                                  "Divine.exe is required but not found. Would you like to download it?",
                                  QMessageBox::Yes|QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        pakScanner->download_divine();
        downloadProgressDialog = new QProgressDialog("Downloading Divine.exe...", "Cancel", 0, 100, this);
        downloadProgressDialog->setWindowModality(Qt::WindowModal);
        downloadProgressDialog->show();
    } else {
        QString divineFilePath = QFileDialog::getOpenFileName(this, "Select Divine.exe", "", "Executable (*.exe)");
        if (!divineFilePath.isEmpty()) {
            pakScanner->set_divine_path(divineFilePath);
        }
    }

    handlingDivineNotFound = false;
}

void ModdingToolsUI::handleDivineDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if (downloadProgressDialog) {
        downloadProgressDialog->setMaximum(bytesTotal);
        downloadProgressDialog->setValue(bytesReceived);
    }
}

void ModdingToolsUI::handleDivineDownloadFinished() {
    if (downloadProgressDialog) {
        downloadProgressDialog->close();
        delete downloadProgressDialog;
        downloadProgressDialog = nullptr;
    }
    QMessageBox::information(this, "Download Complete", "Divine.exe has been downloaded and extracted successfully.");
}

void ModdingToolsUI::loadSettings() {
    QSettings settings("DefakofModdingTools", "ModOrganizer");
    moExePath = settings.value("MOExePath", "").toString();
}

void ModdingToolsUI::saveSettings() {
    QSettings settings("DefakofModdingTools", "ModOrganizer");
    settings.setValue("MOExePath", moExePath);
}

void ModdingToolsUI::createInitialUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    QLabel *label = new QLabel("Welcome to Defakof's Modding Tools", this);
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    QPushButton *selectMOButton = new QPushButton("Select Mod Organizer Executable", this);
    connect(selectMOButton, &QPushButton::clicked, this, &ModdingToolsUI::selectModOrganizerExe);
    layout->addWidget(selectMOButton);

    moPathLabel = new QLabel(this);
    updateMOPathLabel();
    layout->addWidget(moPathLabel);

    profileComboBox = new QComboBox(this);
    layout->addWidget(profileComboBox);

    QPushButton *loadButton = new QPushButton("Load", this);
    connect(loadButton, &QPushButton::clicked, this, &ModdingToolsUI::loadMainUI);
    layout->addWidget(loadButton);

    statusLabel = new QLabel("Ready", this);
    layout->addWidget(statusLabel);

    if (!moExePath.isEmpty()) {
        updateProfileList();
    }
}

void ModdingToolsUI::updateMOPathLabel() {
    if (moExePath.isEmpty()) {
        moPathLabel->setText("No Mod Organizer executable selected");
    } else {
        moPathLabel->setText("Selected: " + moExePath);
    }
}

void ModdingToolsUI::selectModOrganizerExe() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select Mod Organizer 2 Executable", "", "Executable (*.exe)");
    if (!fileName.isEmpty()) {
        moExePath = fileName;
        saveSettings();
        updateMOPathLabel();
        updateProfileList();
    }
}

void ModdingToolsUI::updateProfileList() {
    profileComboBox->clear();
    QString profilesDir = QFileInfo(moExePath).absolutePath() + "/profiles";
    QDir dir(profilesDir);
    QStringList profiles = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    if (profiles.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No profiles found in Mod Organizer 2 directory.");
    } else {
        profileComboBox->addItems(profiles);
    }
}

void ModdingToolsUI::loadMainUI() {
    if (moExePath.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select Mod Organizer executable first.");
        return;
    }

    if (profileComboBox->currentText().isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a profile.");
        return;
    }

    profilePath = QFileInfo(moExePath).absolutePath() + "/profiles/" + profileComboBox->currentText();

    // Clear the current central widget
    QWidget *oldCentralWidget = centralWidget();
    setCentralWidget(nullptr);
    delete oldCentralWidget;

    // Create and set the new main UI
    createMainUI();
    loadModList();
}

void ModdingToolsUI::createMainUI() {
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);

    // Top bar with categorized buttons and search bar
    QWidget *topBarWidget = new QWidget(this);
    QHBoxLayout *topBar = new QHBoxLayout(topBarWidget);
    topBar->setContentsMargins(0, 0, 0, 0);

    // Buttons container (1/4 of the space)
    QWidget *buttonsContainer = new QWidget(this);
    QHBoxLayout *buttonsLayout = new QHBoxLayout(buttonsContainer);
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    buttonsLayout->setSpacing(5);

    // Category 1
    QGroupBox *category1 = new QGroupBox(this);
    QHBoxLayout *category1Layout = new QHBoxLayout(category1);
    category1Layout->addWidget(new QPushButton("Filter", this));
    buttonsLayout->addWidget(category1);

    // Category 2
    QGroupBox *category2 = new QGroupBox(this);
    QHBoxLayout *category2Layout = new QHBoxLayout(category2);
    QPushButton* scanButton = new QPushButton("Scan", this);
    connect(scanButton, &QPushButton::clicked, this, &ModdingToolsUI::onScanButtonClicked);
    category2Layout->addWidget(scanButton);
    category2Layout->addWidget(new QPushButton("Deploy", this));
    buttonsLayout->addWidget(category2);

    // Category 3
    QGroupBox *category3 = new QGroupBox(this);
    QHBoxLayout *category3Layout = new QHBoxLayout(category3);
    category3Layout->addWidget(new QPushButton("Sort", this));
    buttonsLayout->addWidget(category3);

    topBar->addWidget(buttonsContainer, 1);

    // Search bar (3/4 of the space)
    QLineEdit *searchBar = new QLineEdit(this);
    searchBar->setPlaceholderText("Search...");
    topBar->addWidget(searchBar, 3);

    topBarWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    mainLayout->addWidget(topBarWidget);

    // Plugin count
    pluginLabel = new QLabel("Plugins: 0", this);
    pluginLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    mainLayout->addWidget(pluginLabel);

    // Main content area with QSplitter
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(1);
    splitter->setChildrenCollapsible(false);

    // Left frame: Mod tree
    modTree = new QTreeWidget(this);
    modTree->setHeaderHidden(true);
    modTree->setIndentation(0);
    modTree->setItemDelegate(new ModItemDelegate(this));
    modTree->setStyleSheet("QTreeWidget::item { padding: 2px; }");
    splitter->addWidget(modTree);

    // Right frame: Tabs
    QTabWidget *tabs = new QTabWidget(this);

    // Translations tab
    QWidget *translationsTab = new QWidget(this);
    QVBoxLayout *translationsLayout = new QVBoxLayout(translationsTab);

    QHBoxLayout *translationButtonsLayout = new QHBoxLayout();
    translationButtonsLayout->addWidget(new QPushButton("Filter", this));
    translationButtonsLayout->addWidget(new QPushButton("Add", this));
    translationButtonsLayout->addWidget(new QPushButton("Connect", this));
    translationsLayout->addLayout(translationButtonsLayout);

    translationCount = new QLabel("Translations: 0", this);
    translationsLayout->addWidget(translationCount);

    translationList = new QListWidget(this);
    translationsLayout->addWidget(translationList);

    tabs->addTab(translationsTab, "Translations");

    // Downloads tab
    QWidget *downloadsTab = new QWidget(this);
    QVBoxLayout *downloadsLayout = new QVBoxLayout(downloadsTab);

    QHBoxLayout *downloadButtonsLayout = new QHBoxLayout();
    downloadButtonsLayout->addWidget(new QPushButton("Filter", this));
    downloadButtonsLayout->addWidget(new QPushButton("Add", this));
    downloadButtonsLayout->addWidget(new QPushButton("Connect", this));
    downloadsLayout->addLayout(downloadButtonsLayout);

    downloadCount = new QLabel("Downloads: 0", this);
    downloadsLayout->addWidget(downloadCount);

    QListWidget *downloadList = new QListWidget(this);
    downloadsLayout->addWidget(downloadList);

    tabs->addTab(downloadsTab, "Downloads");

    splitter->addWidget(tabs);

    splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainLayout->addWidget(splitter);

    // Set initial sizes for splitter (2:1 ratio)
    splitter->setSizes(QList<int>() << 200 << 100);

    // Status bar
    statusLabel = new QLabel("Ready", this);
    statusBar()->addPermanentWidget(statusLabel);

    setupConnections();
}

void ModdingToolsUI::onScanButtonClicked() {
    scanMods();
}

void ModdingToolsUI::setupConnections() {
    connect(modTree, &QTreeWidget::itemClicked, this, &ModdingToolsUI::onItemClicked);
}

void ModdingToolsUI::loadModList() {
    QString modlistPath = profilePath + "/modlist.txt";
    QFile file(modlistPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Unable to open modlist.txt");
        return;
    }

    QTextStream in(&file);
    modTree->clear();
    int modCount = 0;

    // Add the four main separators
    QTreeWidgetItem* recentlyAdded = new QTreeWidgetItem(modTree);
    recentlyAdded->setText(0, "Recently Added");
    recentlyAdded->setData(0, Qt::UserRole, true);
    recentlyAdded->setExpanded(true);

    QTreeWidgetItem* translated = new QTreeWidgetItem(modTree);
    translated->setText(0, "Translated");
    translated->setData(0, Qt::UserRole, true);
    translated->setExpanded(true);

    QTreeWidgetItem* requireTranslation = new QTreeWidgetItem(modTree);
    requireTranslation->setText(0, "Require Translation");
    requireTranslation->setData(0, Qt::UserRole, true);
    requireTranslation->setExpanded(true);

    QTreeWidgetItem* requireUpdate = new QTreeWidgetItem(modTree);
    requireUpdate->setText(0, "Requires Update");
    requireUpdate->setData(0, Qt::UserRole, true);
    requireUpdate->setExpanded(true);

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("+")) {
            QString modName = line.mid(1).trimmed(); // Remove the '+' and any leading whitespace

            if (!modName.endsWith("_separator")) {
                QTreeWidgetItem* item = new QTreeWidgetItem();
                item->setText(0, modName);

                // Add all mods to the "Recently Added" category by default
                recentlyAdded->addChild(item);
                modCount++;
            }
        }
    }
    file.close();

    pluginLabel->setText("Plugins: " + QString::number(modCount));
}

void ModdingToolsUI::scanMods() {
    QString modsDir = QFileInfo(moExePath).absolutePath() + "/mods";
    QDir dir(modsDir);
    if (!dir.exists()) {
        QMessageBox::critical(this, "Error", "Mods folder not found");
        return;
    }

    if (!pakScanner->checkDivine()) {
        return;  // Stop if divine is not found
    }

    QTreeWidgetItemIterator it(modTree);
    while (*it) {
        QTreeWidgetItem* item = *it;
        if (!item->data(0, Qt::UserRole).toBool()) { // If it's not a separator
            QString modName = item->text(0);
            QString modDir = modsDir + "/" + modName;
            if (QDir(modDir).exists()) {
                pakScanner->scan_mod_folder(modDir, "Mod Organizer 2");
            }
        }
        ++it;
    }
}

void ModdingToolsUI::onItemClicked(QTreeWidgetItem *item, int column) {
    if (item->data(0, Qt::UserRole).toBool()) { // If it's a separator
        item->setExpanded(!item->isExpanded());
    }
}

void ModdingToolsUI::updateStatus(const QString &message) {
    statusLabel->setText(message);
    // Update other UI elements as needed
}

void ModItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    if (index.data(Qt::UserRole).toBool()) {
        // This is a separator item
        QStyleOptionButton buttonOption;
        buttonOption.rect = option.rect;
        buttonOption.palette = option.palette;
        buttonOption.state = QStyle::State_Enabled;
        if (option.state & QStyle::State_MouseOver)
            buttonOption.state |= QStyle::State_MouseOver;
        if (option.state & QStyle::State_Sunken)
            buttonOption.state |= QStyle::State_Sunken;

        // Draw the background
        painter->fillRect(option.rect, QColor(255, 170, 127)); // Light orange background

        // Draw the expand/collapse arrow
        QStyleOption arrowOption;
        arrowOption.rect = QRect(option.rect.left() + 5, option.rect.top() + (option.rect.height() - 9) / 2, 9, 9);
        arrowOption.palette = option.palette;
        arrowOption.state = QStyle::State_Enabled;
        if (index.model()->hasChildren(index)) {
            if (option.state & QStyle::State_Open)
                QApplication::style()->drawPrimitive(QStyle::PE_IndicatorArrowDown, &arrowOption, painter);
            else
                QApplication::style()->drawPrimitive(QStyle::PE_IndicatorArrowRight, &arrowOption, painter);
        }

        // Draw the text
        painter->drawText(option.rect.adjusted(20, 0, 0, 0), Qt::AlignVCenter, index.data().toString());
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize ModItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    if (index.data(Qt::UserRole).toBool()) {
        // Make separators slightly taller
        size.setHeight(size.height() + 4);
    }
    return size;
}