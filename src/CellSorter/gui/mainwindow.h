#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "AcquisitionApp/src/ironmanwidget.h"
#include "acquisitioninterface.h"
#include "configurator.h"
#include "experimentsetup.h"
#include "imagedisplayerwidget.h"
#include "processinterface.h"

namespace Ui {
class MainWindow;
}
class Analyzer;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const QString& projectFilePath = QString(), QWidget* parent = 0);
    ~MainWindow();

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version) const;

private slots:
    void cameraSelectedWithoutAcqBuilt();
    void acqSelectionChanged(int index);
    void ffStateChanged(int state);

    void loadProjectFile(const QString& filename);
    void on_actionLoad_project_file_triggered();
    void on_actionStore_project_file_triggered();

private:
    Ui::MainWindow* ui;

    ProcessInterface* m_processInterface;
    Configurator* m_configurator;
    Analyzer* m_analyzer;
    IronManWidget* m_acquisitionWdiget;
    ImageDisplayerWidget* m_imageDisplayerWidget;
    AcquisitionInterface* m_acqInterface;
    ExperimentSetup* m_experimentSetup;

    void setupAcqCombobox();
};

#endif  // MAINWINDOW_H
