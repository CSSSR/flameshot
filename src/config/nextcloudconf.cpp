
// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2017-2019 Alejandro Sirgo Rica & Contributors

#include "nextcloudconf.h"
#include "src/core/controller.h"
#include "src/utils/confighandler.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QFileDialog>
#include <QGroupBox>
#include <QImageWriter>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSizePolicy>
#include <QSpinBox>
#include <QStandardPaths>
#include <QTextCodec>
#include <QVBoxLayout>

NextcloudConf::NextcloudConf(QWidget* parent)
  : QWidget(parent)
  , m_historyConfirmationToDelete(nullptr)
  , m_undoLimit(nullptr)
{
    m_layout = new QVBoxLayout(this);
    m_layout->setAlignment(Qt::AlignTop);

	initCredentials();

    m_layout->addStretch();

    updateComponents();
}

void NextcloudConf::_updateComponents(bool allowEmptySavePath)
{
    ConfigHandler config;

    //m_uploadHistoryMax->setValue(config.uploadHistoryMax());
}

void NextcloudConf::updateComponents()
{
    _updateComponents(false);
}

void NextcloudConf::showHelpChanged(bool checked)
{
    ConfigHandler().setShowHelp(checked);
}

void NextcloudConf::showSidePanelButtonChanged(bool checked)
{
    ConfigHandler().setShowSidePanelButton(checked);
}

void NextcloudConf::showDesktopNotificationChanged(bool checked)
{
    ConfigHandler().setShowDesktopNotification(checked);
}

void NextcloudConf::checkForUpdatesChanged(bool checked)
{
    ConfigHandler().setCheckForUpdates(checked);
    Controller::getInstance()->setCheckForUpdatesEnabled(checked);
}

void NextcloudConf::allowMultipleGuiInstancesChanged(bool checked)
{
    ConfigHandler().setAllowMultipleGuiInstances(checked);
}

void NextcloudConf::autoCloseIdleDaemonChanged(bool checked)
{
    ConfigHandler().setAutoCloseIdleDaemon(checked);
}

void NextcloudConf::autostartChanged(bool checked)
{
    ConfigHandler().setStartupLaunch(checked);
}

void NextcloudConf::importConfiguration()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Import"));
    if (fileName.isEmpty()) {
        return;
    }
    QFile file(fileName);
    QTextCodec* codec = QTextCodec::codecForLocale();
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox::about(this, tr("Error"), tr("Unable to read file."));
        return;
    }
    QString text = codec->toUnicode(file.readAll());
    file.close();

    QFile config(ConfigHandler().configFilePath());
    if (!config.open(QFile::WriteOnly)) {
        QMessageBox::about(this, tr("Error"), tr("Unable to write file."));
        return;
    }
    config.write(codec->fromUnicode(text));
    config.close();
}

void NextcloudConf::exportFileConfiguration()
{
    QString defaultFileName = QSettings().fileName();
    QString fileName =
      QFileDialog::getSaveFileName(this, tr("Save File"), defaultFileName);

    // Cancel button or target same as source
    if (fileName.isNull() || fileName == defaultFileName) {
        return;
    }

    QFile targetFile(fileName);
    if (targetFile.exists()) {
        targetFile.remove();
    }
    bool ok = QFile::copy(ConfigHandler().configFilePath(), fileName);
    if (!ok) {
        QMessageBox::about(this, tr("Error"), tr("Unable to write file."));
    }
}

void NextcloudConf::resetConfiguration()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(
      this,
      tr("Confirm Reset"),
      tr("Are you sure you want to reset the configuration?"),
      QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        m_savePath->setText(
          QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
        ConfigHandler().setDefaultSettings();
        _updateComponents(true);
    }
}

void NextcloudConf::initScrollArea()
{
    m_scrollArea = new QScrollArea(this);
    m_layout->addWidget(m_scrollArea);

    QWidget* content = new QWidget(m_scrollArea);
    m_scrollArea->setWidget(content);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    content->setObjectName("content");
    m_scrollArea->setObjectName("scrollArea");
    m_scrollArea->setStyleSheet(
      "#content, #scrollArea { background: transparent; border: 0px; }");
    m_scrollAreaLayout = new QVBoxLayout(content);
    m_scrollAreaLayout->setContentsMargins(0, 0, 20, 0);
}

void NextcloudConf::initHistoryConfirmationToDelete()
{
    m_historyConfirmationToDelete = new QCheckBox(
      tr("Confirmation required to delete screenshot from the latest uploads"),
      this);
    m_historyConfirmationToDelete->setToolTip(
      tr("Confirmation required to delete screenshot from the latest uploads"));
    m_scrollAreaLayout->addWidget(m_historyConfirmationToDelete);

    connect(m_historyConfirmationToDelete,
            &QCheckBox::clicked,
            this,
            &NextcloudConf::historyConfirmationToDelete);
}

void NextcloudConf::initConfigButtons()
{
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QGroupBox* box = new QGroupBox(tr("Configuration File"));
    box->setFlat(true);
    box->setLayout(buttonLayout);
    m_layout->addWidget(box);

    m_exportButton = new QPushButton(tr("Export"));
    buttonLayout->addWidget(m_exportButton);
    connect(m_exportButton,
            &QPushButton::clicked,
            this,
            &NextcloudConf::exportFileConfiguration);

    m_importButton = new QPushButton(tr("Import"));
    buttonLayout->addWidget(m_importButton);
    connect(m_importButton,
            &QPushButton::clicked,
            this,
            &NextcloudConf::importConfiguration);

    m_resetButton = new QPushButton(tr("Reset"));
    buttonLayout->addWidget(m_resetButton);
    connect(m_resetButton,
            &QPushButton::clicked,
            this,
            &NextcloudConf::resetConfiguration);
}

void NextcloudConf::initCheckForUpdates()
{
    m_checkForUpdates = new QCheckBox(tr("Automatic check for updates"), this);
    m_checkForUpdates->setToolTip(tr("Automatic check for updates"));
    m_scrollAreaLayout->addWidget(m_checkForUpdates);

    connect(m_checkForUpdates,
            &QCheckBox::clicked,
            this,
            &NextcloudConf::checkForUpdatesChanged);
}

void NextcloudConf::initAllowMultipleGuiInstances()
{
    m_allowMultipleGuiInstances = new QCheckBox(
      tr("Allow multiple flameshot GUI instances simultaneously"), this);
    m_allowMultipleGuiInstances->setToolTip(tr(
      "This allows you to take screenshots of flameshot itself for example."));
    m_scrollAreaLayout->addWidget(m_allowMultipleGuiInstances);
    connect(m_allowMultipleGuiInstances,
            &QCheckBox::clicked,
            this,
            &NextcloudConf::allowMultipleGuiInstancesChanged);
}

void NextcloudConf::initAutoCloseIdleDaemon()
{
    m_autoCloseIdleDaemon = new QCheckBox(
      tr("Automatically close daemon when it is not needed"), this);
    m_autoCloseIdleDaemon->setToolTip(
      tr("Automatically close daemon when it is not needed"));
    m_scrollAreaLayout->addWidget(m_autoCloseIdleDaemon);
    connect(m_autoCloseIdleDaemon,
            &QCheckBox::clicked,
            this,
            &NextcloudConf::autoCloseIdleDaemonChanged);
}

void NextcloudConf::initAutostart()
{
    m_autostart = new QCheckBox(tr("Launch at startup"), this);
    m_autostart->setToolTip(tr("Launch Flameshot"));
    m_scrollAreaLayout->addWidget(m_autostart);

    connect(
      m_autostart, &QCheckBox::clicked, this, &NextcloudConf::autostartChanged);
}

void NextcloudConf::initShowStartupLaunchMessage()
{
    m_showStartupLaunchMessage =
      new QCheckBox(tr("Show welcome message on launch"), this);
    ConfigHandler config;
    m_showStartupLaunchMessage->setToolTip(
      tr("Show welcome message on launch"));
    m_scrollAreaLayout->addWidget(m_showStartupLaunchMessage);

    connect(m_showStartupLaunchMessage, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setShowStartupLaunchMessage(checked);
    });
}

void NextcloudConf::initPredefinedColorPaletteLarge()
{
    m_predefinedColorPaletteLarge =
      new QCheckBox(tr("Use large predefined color palette"), this);
    m_predefinedColorPaletteLarge->setToolTip(
      tr("Use large predefined color palette"));
    m_scrollAreaLayout->addWidget(m_predefinedColorPaletteLarge);

    connect(
      m_predefinedColorPaletteLarge, &QCheckBox::clicked, [](bool checked) {
          ConfigHandler().setPredefinedColorPaletteLarge(checked);
      });
}

void NextcloudConf::initCopyAndCloseAfterUpload()
{
    m_copyAndCloseAfterUpload =
      new QCheckBox(tr("Copy URL after upload"), this);
    m_copyAndCloseAfterUpload->setToolTip(
      tr("Copy URL and close window after upload"));
    m_scrollAreaLayout->addWidget(m_copyAndCloseAfterUpload);

    connect(m_copyAndCloseAfterUpload, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setCopyAndCloseAfterUpload(checked);
    });
}

void NextcloudConf::initSaveAfterCopy()
{
    m_saveAfterCopy = new QCheckBox(tr("Save image after copy"), this);
    m_saveAfterCopy->setToolTip(tr("Save image file after copying it"));
    m_scrollAreaLayout->addWidget(m_saveAfterCopy);
    connect(m_saveAfterCopy,
            &QCheckBox::clicked,
            this,
            &NextcloudConf::saveAfterCopyChanged);

    QGroupBox* box = new QGroupBox(tr("Save Path"));
    box->setFlat(true);
    m_layout->addWidget(box);

    QVBoxLayout* vboxLayout = new QVBoxLayout();
    box->setLayout(vboxLayout);

    QHBoxLayout* pathLayout = new QHBoxLayout();

    QString path = ConfigHandler().savePath();
    m_savePath = new QLineEdit(path, this);
    m_savePath->setDisabled(true);
    QString foreground = this->palette().windowText().color().name();
    m_savePath->setStyleSheet(QStringLiteral("color: %1").arg(foreground));
    pathLayout->addWidget(m_savePath);

    m_changeSaveButton = new QPushButton(tr("Change..."), this);
    pathLayout->addWidget(m_changeSaveButton);
    connect(m_changeSaveButton,
            &QPushButton::clicked,
            this,
            &NextcloudConf::changeSavePath);

    m_screenshotPathFixedCheck =
      new QCheckBox(tr("Use fixed path for screenshots to save"), this);
    connect(m_screenshotPathFixedCheck,
            SIGNAL(toggled(bool)),
            this,
            SLOT(togglePathFixed()));

    vboxLayout->addLayout(pathLayout);
    vboxLayout->addWidget(m_screenshotPathFixedCheck);

    QHBoxLayout* extensionLayout = new QHBoxLayout();

    extensionLayout->addWidget(
      new QLabel(tr("Preferred save file extension:")));
    m_setSaveAsFileExtension = new QComboBox(this);
    m_setSaveAsFileExtension->addItem("");

    QStringList imageFormatList;
    foreach (auto mimeType, QImageWriter::supportedImageFormats())
        imageFormatList.append(mimeType);

    m_setSaveAsFileExtension->addItems(imageFormatList);

    int currentIndex =
      m_setSaveAsFileExtension->findText(ConfigHandler().saveAsFileExtension());
    m_setSaveAsFileExtension->setCurrentIndex(currentIndex);

    connect(m_setSaveAsFileExtension,
            SIGNAL(currentTextChanged(QString)),
            this,
            SLOT(setSaveAsFileExtension(QString)));

    extensionLayout->addWidget(m_setSaveAsFileExtension);
    vboxLayout->addLayout(extensionLayout);
}

void NextcloudConf::historyConfirmationToDelete(bool checked)
{
    ConfigHandler().setHistoryConfirmationToDelete(checked);
}

void NextcloudConf::initCredentials()
{
	QGroupBox* box = new QGroupBox(tr("Nextcloud Credentials"));
	box->setFlat(true);
	m_layout->addWidget(box);

    QVBoxLayout* vboxLayout = new QVBoxLayout();
    box->setLayout(vboxLayout);

	QString login = ConfigHandler().nextcloudLogin();
	m_nextcloudLogin = new QLineEdit(login, this);

	QString password = ConfigHandler().nextcloudPassword();
	m_nextcloudPassword = new QLineEdit(password, this);
	m_nextcloudPassword->setEchoMode(QLineEdit::Password);

	QString uid = ConfigHandler().nextcloudUid();
	m_nextcloudUid = new QLineEdit(uid, this);

    vboxLayout->addWidget(new QLabel(tr("Login:")));
	vboxLayout->addWidget(m_nextcloudLogin);
    vboxLayout->addWidget(new QLabel(tr("Password:")));
	vboxLayout->addWidget(m_nextcloudPassword);
    vboxLayout->addWidget(new QLabel(tr("Slack Id:")));
	vboxLayout->addWidget(m_nextcloudUid);

    m_saveButton = new QPushButton(tr("Save"), this);
    vboxLayout->addWidget(m_saveButton);
	connect(m_saveButton,
			&QPushButton::clicked,
			this,
			&NextcloudConf::changeNextcloudCredentials);
}

void NextcloudConf::changeNextcloudCredentials()
{
	QString login = m_nextcloudLogin->text();
	ConfigHandler().setNextcloudLogin(login);

	QString password = m_nextcloudPassword->text();
	ConfigHandler().setNextcloudPassword(password);

	QString uid = m_nextcloudUid->text();
	ConfigHandler().setNextcloudUid(uid);
}

void NextcloudConf::inituploadHistoryMax()
{
    QGroupBox* box = new QGroupBox(tr("Latest Uploads Max Size"));
    box->setFlat(true);
    m_layout->addWidget(box);

    QVBoxLayout* vboxLayout = new QVBoxLayout();
    box->setLayout(vboxLayout);

    m_uploadHistoryMax = new QSpinBox(this);
    m_uploadHistoryMax->setMaximum(50);
    QString foreground = this->palette().windowText().color().name();
    m_uploadHistoryMax->setStyleSheet(
      QStringLiteral("color: %1").arg(foreground));

    connect(m_uploadHistoryMax,
            SIGNAL(valueChanged(int)),
            this,
            SLOT(uploadHistoryMaxChanged(int)));
    vboxLayout->addWidget(m_uploadHistoryMax);
}

void NextcloudConf::uploadHistoryMaxChanged(int max)
{
    ConfigHandler().setUploadHistoryMax(max);
}

void NextcloudConf::initUndoLimit()
{
    QGroupBox* box = new QGroupBox(tr("Undo limit"));
    box->setFlat(true);
    m_layout->addWidget(box);

    QVBoxLayout* vboxLayout = new QVBoxLayout();
    box->setLayout(vboxLayout);

    m_undoLimit = new QSpinBox(this);
    m_undoLimit->setMinimum(1);
    m_undoLimit->setMaximum(999);
    QString foreground = this->palette().windowText().color().name();
    m_undoLimit->setStyleSheet(QStringLiteral("color: %1").arg(foreground));

    connect(m_undoLimit, SIGNAL(valueChanged(int)), this, SLOT(undoLimit(int)));

    vboxLayout->addWidget(m_undoLimit);
}

void NextcloudConf::undoLimit(int limit)
{
    ConfigHandler().setUndoLimit(limit);
}

void NextcloudConf::initUseJpgForClipboard()
{
    m_useJpgForClipboard =
      new QCheckBox(tr("Use JPG format for clipboard (PNG default)"), this);
    m_useJpgForClipboard->setToolTip(
      tr("Use JPG format for clipboard (PNG default)"));
    m_scrollAreaLayout->addWidget(m_useJpgForClipboard);

#if defined(Q_OS_MACOS)
    // FIXME - temporary fix to disable option for MacOS
    m_useJpgForClipboard->hide();
#endif
    connect(m_useJpgForClipboard,
            &QCheckBox::clicked,
            this,
            &NextcloudConf::useJpgForClipboardChanged);
}

void NextcloudConf::saveAfterCopyChanged(bool checked)
{
    ConfigHandler().setSaveAfterCopy(checked);
}

void NextcloudConf::changeSavePath()
{
    QString path = ConfigHandler().savePath();
    path = chooseFolder(path);
    if (!path.isEmpty()) {
        m_savePath->setText(path);
        ConfigHandler().setSavePath(path);
    }
}

void NextcloudConf::initCopyPathAfterSave()
{
    m_copyPathAfterSave = new QCheckBox(tr("Copy file path after save"), this);
    m_copyPathAfterSave->setToolTip(tr("Copy file path after save"));
    m_scrollAreaLayout->addWidget(m_copyPathAfterSave);
    connect(m_copyPathAfterSave, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setCopyPathAfterSave(checked);
    });
}

void NextcloudConf::initAntialiasingPinZoom()
{
    m_antialiasingPinZoom =
      new QCheckBox(tr("Anti-aliasing image when zoom the pinned image"), this);
    m_antialiasingPinZoom->setToolTip(
      tr("After zooming the pinned image, should the image get smoothened or "
         "stay pixelated"));
    m_scrollAreaLayout->addWidget(m_antialiasingPinZoom);
    connect(m_antialiasingPinZoom, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setAntialiasingPinZoom(checked);
    });
}

void NextcloudConf::initUploadWithoutConfirmation()
{
    m_uploadWithoutConfirmation =
      new QCheckBox(tr("Upload image without confirmation"), this);
    m_uploadWithoutConfirmation->setToolTip(
      tr("Upload image without confirmation"));
    m_scrollAreaLayout->addWidget(m_uploadWithoutConfirmation);
    connect(m_uploadWithoutConfirmation, &QCheckBox::clicked, [](bool checked) {
        ConfigHandler().setUploadWithoutConfirmation(checked);
    });
}

const QString NextcloudConf::chooseFolder(const QString pathDefault)
{
    QString path;
    if (pathDefault.isEmpty()) {
        path =
          QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    }
    path = QFileDialog::getExistingDirectory(
      this,
      tr("Choose a Folder"),
      path,
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (path.isEmpty()) {
        return path;
    }
    if (!path.isEmpty()) {
        if (!QFileInfo(path).isWritable()) {
            QMessageBox::about(
              this, tr("Error"), tr("Unable to write to directory."));
            return QString();
        }
    }
    return path;
}

void NextcloudConf::togglePathFixed()
{
    ConfigHandler().setSavePathFixed(m_screenshotPathFixedCheck->isChecked());
}

void NextcloudConf::setSaveAsFileExtension(QString extension)
{
    ConfigHandler().setSaveAsFileExtension(extension);
}

void NextcloudConf::useJpgForClipboardChanged(bool checked)
{
    ConfigHandler().setUseJpgForClipboard(checked);
}
