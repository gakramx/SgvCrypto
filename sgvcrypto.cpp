#include "sgvcrypto.h"
#include "./ui_sgvcrypto.h"
#include <QFileDialog>
SgvCrypto::SgvCrypto(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SgvCrypto)
{

    ui->setupUi(this);
    setAcceptDrops(true);

    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(2); // Set the number of columns to 2
    model->setHeaderData(0, Qt::Horizontal, "Name"); // Set the header for the first column
    model->setHeaderData(1, Qt::Horizontal, "Path"); // Set the header for the second column
    ui->tableView->setModel(model);

    ui->tableView->setDragEnabled(true); // Enable drag
    ui->tableView->setAcceptDrops(true); // Enable drop
    ui->tableView->setDropIndicatorShown(true);
    ui->tableView->setDefaultDropAction(Qt::MoveAction);
    ui->tableView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Set the stretch factor for the second column to make it take up all available width
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
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
void SgvCrypto::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls())
    {
        qDebug() << "Heeeu";
        event->acceptProposedAction();
    }
}

void SgvCrypto::dropEvent(QDropEvent *event)
{

    if (event->mimeData()->hasUrls())
    {
        QList<QUrl> fileUrls = event->mimeData()->urls();
        QStandardItemModel *model = qobject_cast<QStandardItemModel *>(ui->tableView->model());
        if (model)
        {
            foreach (const QUrl &fileUrl, fileUrls)
            {
                QString fileName = QFileInfo(fileUrl.toLocalFile()).fileName();
                QString filePath = fileUrl.toLocalFile();

                // Create QStandardItem for each file and set the data
                QStandardItem *nameItem = new QStandardItem(fileName);
                QStandardItem *pathItem = new QStandardItem(filePath);

                // Add the items to the model
                QList<QStandardItem*> rowItems;
                rowItems << nameItem << pathItem;
                model->appendRow(rowItems);
            }
        }
        event->acceptProposedAction();
    }
}

void SgvCrypto::on_exportBtn_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, "Select Folder", QDir::homePath());
    if (!folderPath.isEmpty())
    {
        qDebug()<<folderPath;
    }
}

