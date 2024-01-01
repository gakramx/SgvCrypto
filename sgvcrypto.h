#ifndef SGVCRYPTO_H
#define SGVCRYPTO_H

#include <QMainWindow>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QFileInfo>
#include <QStandardItemModel>
#include <QtConcurrent>
#include <QProgressBar>
#include <QLabel>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QMessageBox>
#include <QTimer>
#include <gpac/isomedia.h>
#include <gpac/constants.h>
#include <gpac/crypt.h>
#include <gpac/crypt_tools.h>

QT_BEGIN_NAMESPACE
namespace Ui { class SgvCrypto; }
QT_END_NAMESPACE

class SgvCrypto : public QMainWindow
{
    Q_OBJECT

public:
    SgvCrypto(QWidget *parent = nullptr);
    ~SgvCrypto();
    QString encrypt(const QString& id);
    bool createProjectFile(const QString& exportPath);
    QFuture<bool> encryptVideo(const QString& inputFilePath, const QString& outputFilePath);
    void processFilesRecursive(const QStringList& inputFilePaths, const QStringList& outputFilePaths, int index,int rowCount);
    void  saveProjectFile();
signals:
    void encryptionVideoProgressChanged(int progress);
    void startTimerSignal();
    void stopTimerSignal();
private slots:
    void on_generateBtn_clicked();
    void on_exportBtn_clicked();
    void on_moveUpBtn_clicked();
    void on_moveDownBtn_clicked();

    void on_deleteRowBtn_clicked();
    void openProject();
    void saveProject();
    void exportProject();
    bool readProjectFile(const QString& filePath);
    bool writeProjectFile(const QString& filePath);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    Ui::SgvCrypto *ui;
    QProgressBar* progressb;
    QLabel* infoLabel;
    QLabel* verLabel;
    QString m_folderPath;
    QJsonArray videosArray;
    int currentIndex = 0;
    QTimer* progressTimer;
    QStandardItemModel *model;
    QString currentProjectFilePath;  // To store the path of the currently open project file
        QMenu* fileMenu;  // File menu

};
#endif // SGVCRYPTO_H
