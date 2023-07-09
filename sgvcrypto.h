#ifndef SGVCRYPTO_H
#define SGVCRYPTO_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class SgvCrypto; }
QT_END_NAMESPACE

class SgvCrypto : public QMainWindow
{
    Q_OBJECT

public:
    SgvCrypto(QWidget *parent = nullptr);
    ~SgvCrypto();

private:
    Ui::SgvCrypto *ui;
};
#endif // SGVCRYPTO_H
