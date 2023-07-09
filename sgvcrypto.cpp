#include "sgvcrypto.h"
#include "./ui_sgvcrypto.h"

SgvCrypto::SgvCrypto(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SgvCrypto)
{
    ui->setupUi(this);
}

SgvCrypto::~SgvCrypto()
{
    delete ui;
}

