#ifndef SGVCRYPTO_H
#define SGVCRYPTO_H

#include <QMainWindow>
 #include <QDragEnterEvent>
 #include <QMimeData>
 #include <QFileInfo>
 #include <QStandardItemModel>
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

private slots:
    void on_generateBtn_clicked();
    void on_exportBtn_clicked();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    Ui::SgvCrypto *ui;
};
#endif // SGVCRYPTO_H
