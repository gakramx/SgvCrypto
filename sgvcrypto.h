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
    void createProjectFile(const QString& exportPath);
    QFuture<bool> encryptVideo(const QString& inputFilePath, const QString& outputFilePath,const QByteArray& encryptionKey);
signals:
    void encryptionVideoProgressChanged(int progress);
private slots:
    void on_generateBtn_clicked();
    void on_exportBtn_clicked();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    Ui::SgvCrypto *ui;
    QProgressBar* progress;
    QLabel* infoLabel;
    QLabel* verLabel;
};
#endif // SGVCRYPTO_H
