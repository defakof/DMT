#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QTabWidget>
#include <QStatusBar>
#include <QGroupBox>
#include <QSplitter>
#include <QSizePolicy>

class ModdingToolsUI : public QMainWindow {
    Q_OBJECT

public:
    ModdingToolsUI(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Defakof's Modding Tools");
        setGeometry(100, 100, 1200, 800);

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
        category2Layout->addWidget(new QPushButton("Scan", this));
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
        QLabel *pluginLabel = new QLabel("Plugins: 0", this);
        pluginLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        mainLayout->addWidget(pluginLabel);

        // Main content area with QSplitter
        QSplitter *splitter = new QSplitter(Qt::Horizontal);
        splitter->setHandleWidth(1);
        splitter->setChildrenCollapsible(false);

        // Left frame: Mod list
        QListWidget *modList = new QListWidget(this);
        modList->addItems({"Mod 1", "Mod 2", "Mod 3"}); // Add some items for visibility
        splitter->addWidget(modList);

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

        QLabel *translationCount = new QLabel("Translations: 0", this);
        translationsLayout->addWidget(translationCount);

        QListWidget *translationList = new QListWidget(this);
        translationList->addItems({"Translation 1", "Translation 2"}); // Add some items for visibility
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

        QLabel *downloadCount = new QLabel("Downloads: 0", this);
        downloadsLayout->addWidget(downloadCount);

        QListWidget *downloadList = new QListWidget(this);
        downloadList->addItems({"Download 1", "Download 2"}); // Add some items for visibility
        downloadsLayout->addWidget(downloadList);

        tabs->addTab(downloadsTab, "Downloads");

        splitter->addWidget(tabs);

        splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        mainLayout->addWidget(splitter);

        // Set initial sizes for splitter (2:1 ratio)
        splitter->setSizes(QList<int>() << 200 << 100);

        // Status bar
        statusBar()->showMessage("Ready");
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ModdingToolsUI window;
    window.show();
    return app.exec();
}

#include "main.moc"