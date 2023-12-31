#include "sgvcrypto.h"
#include "./ui_sgvcrypto.h"

#include <iostream>

SgvCrypto::SgvCrypto(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SgvCrypto)
{

    ui->setupUi(this);
    setAcceptDrops(true);
    infoLabel = new QLabel(this);
    verLabel = new QLabel(this);
    infoLabel->setText("Ready");
    infoLabel->setAlignment(Qt::AlignCenter);
    verLabel->setText("SoneGX ver 0.1");
    verLabel->setAlignment(Qt::AlignCenter);
    progressb = new QProgressBar(this);
    progressb->setVisible(false);
    progressb->setAlignment(Qt::AlignCenter);

    ui->statusbar->setSizeGripEnabled(false);
    ui->statusbar->addPermanentWidget(verLabel,1);
    ui->statusbar->addPermanentWidget(progressb,1);
    ui->statusbar->addPermanentWidget(infoLabel,1);

    model = new QStandardItemModel(this);
    model->setColumnCount(4);
    model->setHeaderData(0, Qt::Horizontal, "File name");
    model->setHeaderData(1, Qt::Horizontal, "Path");
    model->setHeaderData(2, Qt::Horizontal, "Video name"); // New column
    model->setHeaderData(3, Qt::Horizontal, "Description"); // New column

    ui->tableView->setModel(model);
    // Set edit triggers to AnyKeyPressed for the "Action" column

    // Set edit triggers to DoubleClicked for the rest of the columns
    //   ui->tableView->setEditTriggers(QAbstractItemView::DoubleClicked);
    ui->tableView->setDragEnabled(true); // Enable drag
    ui->tableView->setAcceptDrops(true); // Enable drop
    ui->tableView->setDropIndicatorShown(true);
    ui->tableView->setDefaultDropAction(Qt::MoveAction);
    ui->tableView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    // Set the stretch factor for the second column to make it take up all available width
    // ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    // ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);


    progressTimer = new QTimer(this);
    progressTimer->setInterval(100);  // Check progress every 500 milliseconds

    connect(this, &SgvCrypto::startTimerSignal, progressTimer, [=]() {
        qDebug()<<"START";
        progressTimer->start();
    });
    connect(this, &SgvCrypto::stopTimerSignal, progressTimer, [=]() {
        qDebug()<<"STOP";
        progressTimer->stop();
    });

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
        m_folderPath=folderPath;
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

    // Clear the videosArray before populating it with new videos
    videosArray = QJsonArray();
    // Iterate through the rows in the table and add them to the JSON array
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->tableView->model());
    if (model)
    {
        int rowCount = model->rowCount();

        // Create lists to store the input and output file paths
        QStringList inputFilePaths;
        QStringList outputFilePaths;

        for (int row = 0; row < rowCount; ++row)
        {
            QString vbaseName = model->data(model->index(row, 0)).toString();
            QString vName = model->data(model->index(row, 2)).toString();
            QString desc = model->data(model->index(row, 3)).toString();

            // Remove spaces and use underscores instead
            QString modifiedVbaseName = vbaseName.replace(" ", "_");
            QString extension = ".mp4";
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

            // Get the input file path from the table view
            QString inputFilePath = model->data(model->index(row, 1)).toString();

            // Construct the output file path in the export directory
            QString outputFilePath = exportPath + "/" + modifiedVbaseName;

            // Add the input and output file paths to the lists
            inputFilePaths.append(inputFilePath);
            outputFilePaths.append(outputFilePath);
        }

        // Start processing the files recursively
        processFilesRecursive(inputFilePaths, outputFilePaths, 0,rowCount);
    }


}
QFuture<bool> SgvCrypto::encryptVideo(const QString &inputFilePath, const QString &outputFilePath)
{

    return QtConcurrent::run([=]() -> bool {

        qDebug() << "Starting encryption...";

        QByteArray inputFilePathUtf = inputFilePath.toUtf8();
        const char* gf_inputFilePathUtf = inputFilePathUtf.constData();
        qDebug()<<"Input file : "<<gf_inputFilePathUtf;

        GF_ISOFile *infile = gf_isom_open(gf_inputFilePathUtf, GF_ISOM_OPEN_READ, 0);

        if (!infile || gf_isom_last_error(infile) != GF_OK) {
            qDebug() << "Error opening input file or file not found";
            return 1;
        }
        // Encrypt the video file
        QByteArray outputFilePathUtf = outputFilePath.toUtf8();
        const char* gf_outputFilePathUtf = outputFilePathUtf.constData();
        qDebug()<<"gf_outputFilePathUtf file : "<<outputFilePathUtf;
        const char *drm_file = "drm_file.xml";  // Assuming no DRM configuration needed
        emit startTimerSignal();
        GF_Err err = gf_crypt_file(infile, drm_file, gf_outputFilePathUtf, 0.0, 1);
        emit stopTimerSignal();
        if (err != GF_OK) {
            qDebug() << "Error encrypting file. Error code: " << err;
            qDebug() << "ISO file last error: " << gf_isom_last_error(infile);
            gf_isom_close(infile);
            return 1;
        }

        // Clean up
        gf_isom_close(infile);

        return true;
    });
}
void SgvCrypto::processFilesRecursive(const QStringList& inputFilePaths, const QStringList& outputFilePaths, int index, int rowCount)
{
    if (index >= inputFilePaths.size()) {
        // All files processed, continue with saving project file
        saveProjectFile();
        return;
    }
    currentIndex = index; // Update the currentIndex

    QString inputFilePath = inputFilePaths.at(index);
    QString outputFilePath = outputFilePaths.at(index);



    QFileSystemWatcher* fileWatcher = new QFileSystemWatcher();
    fileWatcher->addPath(outputFilePath);

    connect(progressTimer, &QTimer::timeout, [=]() {
        qint64 inputfileSize;
        qint64 lastProgress = 0;
        QFileInfo inputFileInfo(inputFilePath);
        // QFileInfo outputFileInfo(outputFilePath); // Don't need this here

        if (inputFileInfo.exists()) {
            inputfileSize = inputFileInfo.size();
        } else {
            qDebug() << "Input file not found info";
        }

        QList<QString> modifiedFiles = fileWatcher->files();
        if (modifiedFiles.contains(outputFilePath)) {
            QFileInfo outputFileInfo(outputFilePath);

            qint64 currentSize = outputFileInfo.size();

            int percentage = (currentSize * 100) / inputfileSize;
            if (percentage > lastProgress) {
                qDebug() << "\rProgress: " << percentage << "%";
                emit encryptionVideoProgressChanged(percentage);
                lastProgress = percentage;
            }
        } else {
            qDebug() << "Output file not found info";
        }
    });

    // Connect slot for fileChanged signal
    QObject::connect(fileWatcher, &QFileSystemWatcher::fileChanged, [=](const QString& path) {
        //   qDebug() << "File changed: " << path;
        // You can handle file changes here
    });

    // Encrypt the video asynchronously
    QFuture<bool> encryptionFuture = encryptVideo(inputFilePath, outputFilePath);

    // Create a QFutureWatcher to monitor the encryption process
    QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>(this);

    // Connect the progress signal to update the progress bar and info label
    connect(this, &SgvCrypto::encryptionVideoProgressChanged, this, [&](int progress) {
        // Update the progress bar
        progressb->setVisible(true);
        progressb->setValue(progress);

    });

    QString info = QString("%1/%2").arg(currentIndex + 1).arg(rowCount);
    infoLabel->setText("Files : "+info);

    // Connect the finished signal of the watcher
    connect(watcher, &QFutureWatcher<bool>::finished, this, [=]() {
        // Disconnect the progress signal to avoid updating progress of previous files
        disconnect(this, &SgvCrypto::encryptionVideoProgressChanged, nullptr, nullptr);

        // Clean up the watcher
        watcher->deleteLater();

        // Move to the next file
        processFilesRecursive(inputFilePaths, outputFilePaths, currentIndex + 1,rowCount);
    });

    // Associate the QFuture with the QFutureWatcher
    watcher->setFuture(encryptionFuture);
}
void SgvCrypto::saveProjectFile()
{

    // Get the export path from the UI
    QString exportPath =m_folderPath;

    // Get the project name from projectName_lienEdit
    QString projectName = ui->projectName_lineEdit->text();

    // Create a JSON object for the project
    QJsonObject projectObject;
    projectObject["Pack_name"] = projectName;
    projectObject["videos"] = videosArray;

    // Create the JSON document
    QJsonDocument jsonDoc(projectObject);

    // Convert the JSON document to a string
    QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Indented);

    // Create the output file path
    QString filePath = exportPath + "/" + projectName + ".sngv";

    // Open the file for writing
    QFile outputFile(filePath);
    if (outputFile.open(QIODevice::WriteOnly))
    {
        // Write the JSON data to the file
        outputFile.write(jsonData);
        outputFile.close();
        progressb->setVisible(false);
        infoLabel->setText("Done !");
        QMessageBox::information(this, "Project Files Created", "Project files has been created successfully.");

    }
    else
    {
        QMessageBox::critical(this, "Error", "Failed to create project files.");
    }
}

void SgvCrypto::on_moveUpBtn_clicked()
{
    QModelIndexList selectedIndexes = ui->tableView->selectionModel()->selectedRows();

    // Move selected rows up
    for (const QModelIndex &index : selectedIndexes) {
        int row = index.row();
        if (row > 0) {
            // Swap with the row above
            model->insertRow(row - 1, model->takeRow(row));
        }
    }

    // Restore selection
    for (const QModelIndex &index : selectedIndexes) {
        int row = index.row();
        ui->tableView->selectRow(row - 1);
    }
}


void SgvCrypto::on_moveDownBtn_clicked()
{
    QModelIndexList selectedIndexes = ui->tableView->selectionModel()->selectedRows();

    // Move selected rows down
    for (const QModelIndex &index : selectedIndexes) {
        int row = index.row();
        if (row < model->rowCount() - 1) {
            // Swap with the row below
            model->insertRow(row + 1, model->takeRow(row));
        }
    }

    // Restore selection
    for (const QModelIndex &index : selectedIndexes) {
        int row = index.row();
        ui->tableView->selectRow(row + 1);
    }
}


void SgvCrypto::on_deleteRowBtn_clicked()
{
    QModelIndexList selectedIndexes = ui->tableView->selectionModel()->selectedRows();

    // Delete selected rows
    for (const QModelIndex &index : selectedIndexes) {
        model->removeRow(index.row());
    }
}

