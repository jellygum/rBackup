/*
        Copyright Jonathan Manly 2020

        This file is part of rBackup.

        rBackup is free software: you can redistribute it and/or modify
        it under the terms of the GNU Lesser General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        rBackup is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public License
        along with rBackup.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::MainWindow), manager(new Manager()),
          commandGenerated(false), isUpdating(false)
{
        ui->setupUi(this);
        add_jobs_to_list();
        create_checkbox_array();
}

MainWindow::~MainWindow()
{
        delete ui;
        delete manager;
}

void MainWindow::on_browseDest_clicked()
{
        QString fileName = QFileDialog::getExistingDirectory(
                this, tr("Select Directory"), "/home",
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        ui->destination->setText(fileName);
}

void MainWindow::on_browseSrc_clicked()
{
        QString fileName = QFileDialog::getExistingDirectory(
                this, tr("Select Directory"), "/home",
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

        ui->destination->setText(fileName);
}

void MainWindow::on_generateButton_clicked()
{
        ui->command->setPlainText(generate());
        commandGenerated = true;
}

void MainWindow::on_finish_clicked()
{
        ui->tabs->setCurrentIndex(JOBS);
        int status = 0;
        if (!isUpdating) {
                status = manager->add_new_job(create_job());
                if (!status) {
                        ui->jobNamesList->addItem(ui->jobName->text());
                        clear_form();
                }

        } else {
                status = manager->update_job(create_job());
                if (!status)
                        clear_form();
                ui->jobName->setEnabled(true);
                isUpdating = false;
        }
        manager->save_jobs();
}

void MainWindow::on_newButton_clicked()
{
        ui->tabs->setCurrentIndex(SETTINGS);
}

QString MainWindow::generate() const
{
        QString tmp = "", out = selectBackupType();

        if (ui->transferCompression->isChecked())
                out += TRANSFER_COMPRESSION;

        out += selectDeleteType();
        out += ui->source->text() + " ";
        out += ui->destination->text() + " ";
        if (ui->backupCompression->currentIndex() != 0)
                out += " && ";
        out += selectCompressionType();

        return out;
}

QString MainWindow::selectBackupType() const
{
        QString out = "";

        switch (ui->backupType->currentIndex()) {
                void on_actionServicePath_triggered();
        case 0:
                out += INCREMENTAL_OPTIONS;
                break;
        case 1:
                out += INCREMENTAL_OPTIONS;
                out += NO_DELTA;
                break;
        case 2:
                out += FULL_OPTIONS;
                break;
        case 3:
                out += INCREMENTAL_OPTIONS;
                out += NO_DELTA;
                break;
        default:
                throw std::out_of_range("Invalid Backup Type Index");
        }

        return out;
}

QString MainWindow::selectDeleteType() const
{
        switch (ui->deleteWhen->currentIndex()) {
        case 0:
                return DELETE_DURING;
        case 1:
                return DELETE_AFTER;
        case 2:
                return DELETE_BEFORE;
        default:
                throw std::out_of_range("Invalid Delete Type Index");
        }
}

QString MainWindow::selectCompressionType() const
{
        QString out = "";
        QString dest = ui->destination->text();
        switch (ui->backupCompression->currentIndex()) {
        case 0:
                break;
        case 1:
                out += TAR;
                out += dest + ".tar " + dest;
                break;
        case 2:
                out += TAR_GZ;
                out += dest + ".tar.gz " + dest;
                break;
        case 3:
                out += TAR_BZ;
                out += dest + ".tar.bz2 " + dest;
                break;
        case 4:
                out += TAR_XZ;
                out += dest + ".tar.xz " + dest;
                break;
        default:
                throw std::out_of_range("Invalid Compression Type Index");
        }
        return out;
}

BackupJob MainWindow::create_job() const
{
        QString command = "";
        if (commandGenerated)
                command = ui->command->toPlainText();
        else
                command = generate();
        return BackupJob(ui->jobName->text(), ui->destination->text(), ui->source->text(), command,
                         create_days(), create_flags(), create_time());
}

JobFlags MainWindow::create_flags() const
{
        JobFlags flags;
        int index = ui->backupType->currentIndex();
        flags.delta = index == 1 || index == 3;

        flags.compType = (CompressionType)ui->backupCompression->currentIndex();
        flags.deleteType = (DeleteType)ui->deleteWhen->currentIndex();
        flags.recurring = ui->recurring->isChecked();
        flags.backupCompression = flags.compType != 0;
        flags.transferCompression = ui->transferCompression->isChecked();
        flags.backupType = (BackupType)ui->backupType->currentIndex();

        return flags;
}

Days MainWindow::create_days() const
{
        Days days;
        for (size_t i = 0; i < days.size(); i++) {
                days[i] = checkboxes[i]->isChecked();
        }
        return days;
}

QString MainWindow::create_time() const
{
        return ui->timeEdit->time().toString();
}

void MainWindow::add_jobs_to_list()
{
        std::list<std::string> list = manager->get_job_names();
        for (const auto &name : list) {
                ui->jobNamesList->addItem(QString::fromStdString(name));
        }
        ui->jobNamesList->setCurrentRow(0);
}

void MainWindow::edit_job(const BackupJob &job)
{
        isUpdating = true;
        JobFlags tmp = job.get_flags();
        ui->jobName->setText(job.get_name());
        ui->jobName->setEnabled(false); // prevent change of jobname.
        set_days_from_array(job.get_days());
        ui->source->setText(job.get_src());
        ui->destination->setText(job.get_dest());
        ui->recurring->setChecked(tmp.recurring);
        ui->timeEdit->setTime(QTime::fromString(job.get_time()));
        ui->command->setPlainText(job.get_command());
        ui->deleteWhen->setCurrentIndex(tmp.deleteType);
        ui->backupCompression->setCurrentIndex(tmp.compType);
        ui->transferCompression->setChecked(tmp.transferCompression);
        ui->backupType->setCurrentIndex(tmp.backupType);
}

void MainWindow::set_days_from_array(const Days &days)
{
        for (size_t i = 0; i < days.size(); i++) {
                checkboxes[i]->setChecked(days[i]);
        }
}

void MainWindow::clear_form()
{
        ui->jobName->setText("");
        ui->source->setText("");
        ui->destination->setText("");
        ui->recurring->setChecked(true);
        ui->timeEdit->setTime(QTime::currentTime());
        ui->command->setPlainText("");
        ui->deleteWhen->setCurrentIndex(0);
        ui->backupCompression->setCurrentIndex(0);
        ui->backupType->setCurrentIndex(0);
        ui->transferCompression->setChecked(0);

        for (size_t i = 0; i < checkboxes.size(); i++) {
                checkboxes[i]->setChecked(false);
        }
}

void MainWindow::on_jobNamesList_itemSelectionChanged()
{
        QString jobname = ui->jobNamesList->selectedItems().first()->text();
        QString jobText = manager->get_job_text(jobname);
        if (jobText == "") {
                show_error_dialog("Job not found!");
                return;
        }
        ui->jobInfo->setPlainText(jobText);
}

void MainWindow::on_editButton_clicked()
{
        ui->tabs->setCurrentIndex(SETTINGS);
        try {
                edit_job(manager->get_job(ui->jobNamesList->currentItem()->text().toStdString()));
        } catch (std::exception &e) {
                show_error_dialog(e.what());
        }
}

void MainWindow::create_checkbox_array()
{
        checkboxes[0] = ui->monday;
        checkboxes[1] = ui->tuesday;
        checkboxes[2] = ui->wednesday;
        checkboxes[3] = ui->thursday;
        checkboxes[4] = ui->friday;
        checkboxes[5] = ui->saturday;
        checkboxes[6] = ui->sunday;
}

void MainWindow::on_enableButton_clicked()
{
        int status = manager->enable_job(ui->jobNamesList->currentItem()->text());
        if (status)
                show_error_dialog("Unable to enable job.");
        else
                ui->jobInfo->setPlainText(
                        manager->get_job_text(ui->jobNamesList->currentItem()->text()));
}

void MainWindow::on_actionExit_triggered()
{
        close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
        QMessageBox::StandardButton save;
        save = QMessageBox::question(this, "Exit", "Save before exiting?",
                                     QMessageBox::Yes | QMessageBox::No);
        if (save == QMessageBox::Yes)
                manager->save_jobs();
}

void MainWindow::on_runButton_clicked()
{
        manager->run_job(ui->jobNamesList->currentItem()->text());
}

void MainWindow::on_disableButton_clicked()
{
        int status = manager->disable_job(ui->jobNamesList->currentItem()->text());
        if (status)
                show_error_dialog("Unable to disable job.");
        else
                ui->jobInfo->setPlainText(
                        manager->get_job_text(ui->jobNamesList->currentItem()->text()));
}
