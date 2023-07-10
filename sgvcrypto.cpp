#include "sgvcrypto.h"
#include "./ui_sgvcrypto.h"
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QMessageBox>
SgvCrypto::SgvCrypto(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SgvCrypto)
{

    ui->setupUi(this);
    setAcceptDrops(true);

    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(4);
    model->setHeaderData(0, Qt::Horizontal, "File name");
    model->setHeaderData(1, Qt::Horizontal, "Path");
    model->setHeaderData(2, Qt::Horizontal, "Video name"); // New column
    model->setHeaderData(3, Qt::Horizontal, "Description"); // New column
    ui->tableView->setModel(model);

    ui->tableView->setDragEnabled(true); // Enable drag
    ui->tableView->setAcceptDrops(true); // Enable drop
    ui->tableView->setDropIndicatorShown(true);
    ui->tableView->setDefaultDropAction(Qt::MoveAction);
    ui->tableView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // Set the stretch factor for the second column to make it take up all available width
   // ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
   // ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
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

                // Create QStandardItem for each column and set the data
                QStandardItem *nameItem = new QStandardItem(fileName);
                QStandardItem *pathItem = new QStandardItem(filePath);
                QStandardItem *videoNameItem = new QStandardItem(); // New column
                QStandardItem *descriptionItem = new QStandardItem(); // New column

                // Add the items to the model
                QList<QStandardItem*> rowItems;
                rowItems << nameItem << pathItem << videoNameItem << descriptionItem; // Add the new items
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
        createProjectFile(folderPath);
    }
}

void SgvCrypto::createProjectFile(const QString& exportPath)
{
    // Get the project name from projectName_lienEdit
    QString projectName = ui->projectName_lineEdit->text();

    // Create a JSON object for the project
    QJsonObject projectObject;
    projectObject["Pack_name"] = projectName;

    // Create a JSON array for the videos
    QJsonArray videosArray;

    // Iterate through the rows in the table and add them to the JSON array
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->tableView->model());
    if (model)
    {
        int rowCount = model->rowCount();
        for (int row = 0; row < rowCount; ++row)
        {
            QString vbaseName = model->data(model->index(row, 0)).toString();
            QString vName = model->data(model->index(row, 2)).toString();
            QString desc = model->data(model->index(row, 3)).toString();
            // Modify the file extension to ".dat0"
            QString modifiedVbaseName = vbaseName;
            QString extension = ".dat0";
            int dotIndex = modifiedVbaseName.lastIndexOf(".");
            if (dotIndex != -1)
            {
                modifiedVbaseName = modifiedVbaseName.left(dotIndex) + extension;
            }
            else
            {
                modifiedVbaseName += extension;
            }
            QJsonObject videoObject;
            videoObject["vbaseName"] = modifiedVbaseName;
            videoObject["vName"] = vName;
            videoObject["desc"] = desc;

            videosArray.append(videoObject);
        }
    }

    // Add the videos array to the project object
    projectObject["videos"] = videosArray;

    // Create the JSON document
    QJsonDocument jsonDoc(projectObject);

    // Convert the JSON document to a string
    QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Indented);

    // Create the output file path
    QString filePath = exportPath + "/" + projectName + ".json";

    // Open the file for writing
    QFile outputFile(filePath);
    if (outputFile.open(QIODevice::WriteOnly))
    {
        // Write the JSON data to the file
        outputFile.write(jsonData);
        outputFile.close();

        QMessageBox::information(this, "Project File Created", "Project file has been created successfully.");
    }
    else
    {
        QMessageBox::critical(this, "Error", "Failed to create project file.");
    }
}

