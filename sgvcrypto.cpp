#include "sgvcrypto.h"
#include "./ui_sgvcrypto.h"

SgvCrypto::SgvCrypto(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SgvCrypto)
{
    ui->setupUi(this);
}
QString SgvCrypto::encrypt(const QString &id){
    // Get the prefix and suffix of the ID
    QString prefix = id.left(4);
    QString suffix = id.right(4);

    // Create the mixed key by concatenating the first 4 characters from the prefix and the last 4 characters from the suffix
    QString key = prefix + suffix;

    // Convert the ID to a byte array
    QByteArray idBytes = QByteArray::fromHex(id.toLatin1());

    // Encrypt the ID using XOR with the key
    QByteArray encrypted;
    for (int i = 0; i < idBytes.size(); i++)
    {
        encrypted.append(idBytes.at(i) ^ key.at(i % key.size()).toLatin1());
    }

    // Convert the encrypted bytes to hex and truncate to 16 characters
    QString encryptedHex = encrypted.toHex();

    // Split the encrypted ID into groups of four characters separated by hyphens
    QString formattedEncrypted;
    for (int i = 0; i < encryptedHex.size(); i += 4)
    {
        formattedEncrypted += encryptedHex.mid(i, 4) + '-';
    }
    formattedEncrypted.chop(1); // Remove the last hyphen

    return formattedEncrypted;
}
SgvCrypto::~SgvCrypto()
{
    delete ui;
}


void SgvCrypto::on_generateBtn_clicked()
{
    QString deviceID=encrypt(ui->iDLineEdit->text());
    ui->serialLineEdit->setText(deviceID);
}

