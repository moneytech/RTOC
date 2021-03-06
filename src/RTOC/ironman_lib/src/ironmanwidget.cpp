#include "ironmanwidget.h"

#include "../gui/guihelpers.h"
#include "ui_ironmanwidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QOpenGLWidget>
#include <QPushButton>

IronManWidget::IronManWidget(QWidget* parent) : QWidget(parent), m_ui(new Ui::IronManWidget) {
    m_ui->setupUi(this);

    // connect image request from ImageDisplayer

    m_ui->graphicsView->setScene(&m_imageDisplayer);
    m_ui->graphicsView->setViewport(new QOpenGLWidget());

    m_logger = new Logger(this);
    m_logger->setLog(m_ui->log);

    // Setup ImageDisplayer requests to the acquisitor
    connect(&m_imageDisplayer, &CameraDisplayerWidget::requestImage, Acquisitor::get(),
            &Acquisitor::requestImageData, Qt::QueuedConnection);

    // connect Acquisitor to UI
    connect(Acquisitor::get(), &Acquisitor::sendImageData, &m_imageDisplayer,
            &CameraDisplayerWidget::setImage, Qt::QueuedConnection);
    connect(Acquisitor::get(), &Acquisitor::writeToLog, m_logger, &Logger::writeLineToLog,
            Qt::QueuedConnection);
    connect(Acquisitor::get(), &Acquisitor::stateChanged, this, &IronManWidget::acqStateChanged,
            Qt::QueuedConnection);
    connect(Acquisitor::get(), &Acquisitor::imageDimensionsChanged, &m_imageDisplayer,
            &CameraDisplayerWidget::setImageDimensions, Qt::QueuedConnection);

    // connect Acquisitor initializer
    connect(m_ui->initialize, &QPushButton::clicked, this, &IronManWidget::initializeFramegrabber);

    // Disable start-acq button by default
    m_ui->start->setEnabled(false);

    // Connect start functionality for frame grabber
    connect(m_ui->start, &QPushButton::toggled, [=](bool state) {
        if (state) {
            QMetaObject::invokeMethod(Acquisitor::get(), "startAcq", Qt::QueuedConnection);
        } else {
            QMetaObject::invokeMethod(Acquisitor::get(), "stopAcq", Qt::QueuedConnection);
        }
    });
}

void IronManWidget::acqStateChanged(AcqState state) {
    switch (state) {
        case AcqState::Idle: {
            m_logger->setDotPrinterState(false);
            m_imageDisplayer.stopRequestingImages();
            break;
        }
        case AcqState::Initializing: {
            m_logger->setDotPrinterState(true);
            break;
        }
        case AcqState::Initialized: {
            m_logger->setDotPrinterState(false);
            break;
        }
        case AcqState::Acquiring: {
            m_imageDisplayer.startRequestingImages();
            break;
        }
    }

    // set UI button states
    setButtonStates(state);
}

void IronManWidget::setButtonStates(AcqState state) {
    switch (state) {
        case AcqState::Idle: {
            // Acquisitor is no longer initialized/initialization failed
            m_ui->initialize->setEnabled(true);
            m_ui->initialize->setText("Initialize framegrabber");
            m_ui->initWithConfig->setEnabled(true);
            break;
        }
        case AcqState::Initializing: {
            m_ui->initialize->setEnabled(false);
            m_ui->initialize->setText("Initializing...");
            m_ui->initWithConfig->setEnabled(false);
            break;
        }
        case AcqState::Initialized: {
            // Acquisitor is initialized, disable button and set text
            m_ui->initialize->setEnabled(false);
            m_ui->initialize->setText("Initialized");
            m_ui->start->setEnabled(true);
            m_ui->start->setText("Start acquisition");

            break;
        }
        case AcqState::Acquiring: {
            m_ui->start->setText("Stop acquisition");
            break;
        }
    }
}

void IronManWidget::initializeFramegrabber() {
    if (m_ui->initWithConfig->isChecked() &&
        (m_ui->xmlPath->text().isEmpty() || m_ui->configPath->text().isEmpty())) {
        QString errMsg = m_ui->configPath->text().isEmpty()
                             ? "Missing path to GenICam .txt parameters file"
                             : "Missing path to GenICam .xml configuration file";
        QMessageBox::warning(this, "Invalid file path", errMsg);
        return;
    }
    // Start acquisitor initialization and disable button
    QMetaObject::invokeMethod(Acquisitor::get(), "initialize", Qt::QueuedConnection,
                              Q_ARG(const QString&, m_ui->xmlPath->text()),
                              Q_ARG(const QString&, m_ui->configPath->text()),
                              Q_ARG(bool, m_ui->initWithConfig->isChecked()));
}

IronManWidget::~IronManWidget() {
    delete m_ui;
}

void IronManWidget::on_actionExit_triggered() {
    // Safely deinitialize the framegrabber
    Acquisitor::get()->stopAcq();
    Acquisitor::get()->deInitialize();
    QApplication::exit();
}

void IronManWidget::on_txtPathButton_clicked() {
    auto filename =
        QFileDialog::getOpenFileName(this, "Open parameters file", "", "txt file (*.txt)");
    if (!filename.isNull()) {
        m_ui->configPath->setText(filename);
    }
}

void IronManWidget::on_xmlPathButton_clicked() {
    auto filename =
        QFileDialog::getOpenFileName(this, "Open configuration file", "", "XML file (*.xml)");
    if (!filename.isNull()) {
        m_ui->xmlPath->setText(filename);
    }
}

void IronManWidget::on_clearLog_clicked() {
    m_ui->log->clear();
}

void IronManWidget::on_scale_currentIndexChanged(const QString& arg1) {
    if (arg1 == QString("Fit to view")) {
        m_ui->graphicsView->fitInView(m_ui->graphicsView->sceneRect(), Qt::KeepAspectRatio);
    } else {
        // remove % and convert to a percentage
        QString s = arg1;
        qreal scale = s.replace('%', "").toDouble() / 100.0;
        m_ui->graphicsView->resetMatrix();
        m_ui->graphicsView->scale(scale, scale);
    }
}

void IronManWidget::on_spinBox_valueChanged(int arg1) {
    m_imageDisplayer.setImageInterval(arg1);
}

template <class Archive>
void IronManWidget::serialize(Archive& ar, const unsigned int version) const {
    SERIALIZE_CHECKBOX(ar, m_ui->initWithConfig, InitWithConfig);
    SERIALIZE_LINEEDIT(ar, m_ui->xmlPath, XmlPath);
    SERIALIZE_LINEEDIT(ar, m_ui->configPath, ConfigPath);
}

EXPLICIT_INSTANTIATE_CONST_XML_ARCHIVE(IronManWidget)
