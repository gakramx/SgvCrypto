#include "sgvcrypto.h"
#include "./ui_sgvcrypto.h"

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

    connect(ui->tableView, &QTableView::doubleClicked, this, &SgvCrypto::onTableDoubleClicked);
    fileMenu = menuBar()->addMenu(tr("&File"));

    QAction* openAction = new QAction(tr("&Open"), this);
    connect(openAction, &QAction::triggered, this, &SgvCrypto::openProject);
    fileMenu->addAction(openAction);

    QAction* saveAction = new QAction(tr("&Save"), this);
    connect(saveAction, &QAction::triggered, this, &SgvCrypto::saveProject);
    fileMenu->addAction(saveAction);
    QAction* saveAsAction = new QAction(tr("Save &As..."), this);
    connect(saveAsAction, &QAction::triggered, this, &SgvCrypto::saveAsProject);
    fileMenu->addAction(saveAsAction);

    QAction* exportAction = new QAction(tr("&Export"), this);
    connect(exportAction, &QAction::triggered, this, &SgvCrypto::exportProject);
    fileMenu->addAction(exportAction);

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

bool SgvCrypto::createProjectFile(const QString& exportPath)
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


            QJsonObject videoObject;
            videoObject["vbaseName"] = vbaseName;
            videoObject["vName"] = vName;
            videoObject["desc"] = desc;

            videosArray.append(videoObject);

            // Get the input file path from the table view
            QString inputFilePath = model->data(model->index(row, 1)).toString();

            // Construct the output file path in the export directory
            QString outputFilePath = exportPath + "/" + vbaseName;

            // Add the input and output file paths to the lists
            inputFilePaths.append(inputFilePath);
            outputFilePaths.append(outputFilePath);
        }

        // Start processing the files recursively
        processFilesRecursive(inputFilePaths, outputFilePaths, 0,rowCount);
    }

    return true;
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
        QString tfile;
        QTemporaryDir tempDir;
        if (tempDir.isValid()) {
            const QString tempFile = tempDir.path() + "/yourfile.xlsx";
            if (QFile::copy(":/drm/drm_file.xml", tempFile)) {
                tfile=tempFile;
            }
            else
                tfile="";
        }
         const char *drm_file = tfile.toUtf8().constData();

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


void SgvCrypto::openProject()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open Project File", QDir::homePath(), "Project Files (*.sngvproject)");

    if (!filePath.isEmpty()) {
        // Read the project file and populate the UI
        if (readProjectFile(filePath)) {
            currentProjectFilePath = filePath;
            QMessageBox::information(this, "Open Project", "Project opened successfully.");
        } else {
            QMessageBox::warning(this, "Open Project", "Failed to open the project.");
        }
    }
}

void SgvCrypto::saveProject()
{
    if (currentProjectFilePath.isEmpty()) {
        // If no project file is open, ask for a new file path
        currentProjectFilePath = QFileDialog::getSaveFileName(this, "Save Project File", QDir::homePath(), "Project Files (*.sngvproject)");
    }

    if (!currentProjectFilePath.isEmpty()) {
        // Save the project file
        if (writeProjectFile(currentProjectFilePath)) {
            QMessageBox::information(this, "Save Project", "Project saved successfully.");
        } else {
            QMessageBox::warning(this, "Save Project", "Failed to save the project.");
        }
    }
}

void SgvCrypto::saveAsProject()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Save Project File", QDir::homePath(), "Project Files (*.sngvproject)");

    if (!filePath.isEmpty()) {
        // Save the project file
        if (writeProjectFile(filePath)) {
            currentProjectFilePath = filePath;
            QMessageBox::information(this, "Save Project", "Project saved successfully.");
        } else {
            QMessageBox::warning(this, "Save Project", "Failed to save the project.");
        }
    }
}
void SgvCrypto::exportProject()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, "Select Folder", QDir::homePath());
    if (!folderPath.isEmpty()) {
        // Export the project files and encrypt videos
        m_folderPath = folderPath;
        if (createProjectFile(folderPath)) {
            QMessageBox::information(this, "Export Project", "Project exported successfully.");
        } else {
            QMessageBox::warning(this, "Export Project", "Failed to export the project.");
        }
    }
}

bool SgvCrypto::readProjectFile(const QString& filePath)
{
    // Implement the logic to read and populate the UI from the project file
    // Example: Open the project file, parse the JSON, and populate the UI

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open project file: " << filePath;
        return false;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        qDebug() << "Invalid JSON format in the project file.";
        return false;
    }

    QJsonObject projectObject = jsonDoc.object();

    // Populate UI elements based on projectObject values
    ui->projectName_lineEdit->setText(projectObject.value("Pack_name").toString());

    // Clear the existing data in the table
    model->removeRows(0, model->rowCount());

    QJsonArray videosArray = projectObject.value("videos").toArray();
    for (int i = 0; i < videosArray.size(); ++i) {
        QJsonObject videoObject = videosArray.at(i).toObject();

        QString vbaseName = videoObject.value("vbaseName").toString();
        QString vName = videoObject.value("vName").toString();
        QString desc = videoObject.value("desc").toString();

        // Extract file name and path from vbaseName
        QFileInfo fileInfo(vbaseName);
        QString fileName = fileInfo.fileName();


        // Create QStandardItem for each column and set the data
        QStandardItem* nameItem = new QStandardItem(fileName);
        QStandardItem* pathItem = new QStandardItem(vbaseName);
        QStandardItem* videoNameItem = new QStandardItem(vName);
        QStandardItem* descriptionItem = new QStandardItem(desc);
        // Add the items to the model
        QList<QStandardItem*> rowItems;
        rowItems << nameItem << pathItem << videoNameItem << descriptionItem;
        model->appendRow(rowItems);
    }

    return true;
}

bool SgvCrypto::writeProjectFile(const QString& filePath)
{
    // Implement the logic to save the project file
    // Example: Create a JSON object, populate it from UI elements, and save to the file

    QJsonObject projectObject;
    projectObject["Pack_name"] = ui->projectName_lineEdit->text();

    QJsonArray videosArray;

    for (int row = 0; row < model->rowCount(); ++row) {
        QJsonObject videoObject;


        videoObject["vbaseName"] = model->data(model->index(row, 1)).toString();
        videoObject["vName"] = model->data(model->index(row, 2)).toString();
        videoObject["desc"] = model->data(model->index(row, 3)).toString();

        videosArray.append(videoObject);
    }

    projectObject["videos"] = videosArray;

    QJsonDocument jsonDoc(projectObject);
    QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Indented);
    QString nonConstFilePath = filePath;  // Create a non-const copy
    if (!nonConstFilePath.endsWith(".sngvproject", Qt::CaseInsensitive)) {
        nonConstFilePath += ".sngvproject";
    }
    QFile file(nonConstFilePath);  // Change this line
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to open project file for writing: " << filePath;
        return false;
    }

    file.write(jsonData);
    file.close();

    return true;
}
void SgvCrypto::onTableDoubleClicked(const QModelIndex &index)
{
    // Check if the double-clicked cell is in the "Description" column
    if (index.isValid() && index.column() == 3) {
        // Create a dialog
        QDialog dialog(this);
        dialog.setWindowTitle("Edit Description");

        // Get the size of the main window
        QSize mainWindowSize = size();
        int dialogWidth = mainWindowSize.width() / 2; // Set the width to 50% of the main window width
        int dialogHeight = mainWindowSize.height() / 2; // Set the height to 50% of the main window height

        // Set the size of the dialog
        dialog.resize(dialogWidth, dialogHeight);
        // Create a QPlainTextEdit widget
        QPlainTextEdit *plainTextEdit = new QPlainTextEdit(&dialog);
        plainTextEdit->setPlainText(model->data(index).toString());

        // Create a Save button
        QPushButton *saveButton = new QPushButton("Save", &dialog);

        // Layout for the dialog
        QVBoxLayout *layout = new QVBoxLayout(&dialog);
        layout->addWidget(plainTextEdit);
        layout->addWidget(saveButton);

        // Connect the Save button's clicked signal to a slot
        connect(saveButton, &QPushButton::clicked, [&]() {
            // Update the corresponding cell in the "Description" column with the text from QPlainTextEdit
            model->setData(index, plainTextEdit->toPlainText());
            dialog.close();
        });

        // Show the dialog
        dialog.exec();
    }
}
