#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStringList>
#include <QDebug>
#include <QColor>
#include <QPlainTextEdit>
#include <QFileDialog>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->MachineWidget->hide();
    ui->TransTable->item(0,0)->setFlags(Qt::NoItemFlags);
    ui->TransTable->setRowCount(1);
    ui->TransTable->setColumnCount(1);
    Machine = 0;
}

MainWindow::~MainWindow()
{
    clearMachine();
    qDebug()<<"delete ui";
    delete ui;
}

void MainWindow::setMachineType(bool type)
{
    MachineType = type;
    if (MachineType)
    {
        ui->splitter_2->widget(1)->setFixedWidth(64);
        ui->MachineWidget->show();
        ui->OutputTable->setColumnCount(2);
        ui->OutputTable->setItem(0,1,new QTableWidgetItem());
    }
    else
    {
        ui->splitter_2->widget(1)->setMinimumWidth(0);
        ui->splitter_2->widget(1)->setMaximumWidth(16777215);
        ui->MachineWidget->show();
    }
    qDebug()<<"MachineType"<<MachineType;
}

void MainWindow::clearMachine()
{
    qDebug()<<"clear memory";

    if (Machine)
    {
        resetMachine();
        Machine->clearMachine();
        delete Machine;
        Machine = 0;
    }

    for (int i = 0; i < ui->TransTable->rowCount(); i++)
        for (int j = 0; j < ui->TransTable->columnCount(); j++)
            if (i && j)
                delete ui->TransTable->item(i, j);

    ui->TransTable->setRowCount(1);
    ui->TransTable->setColumnCount(1);

    for (int i = 0; i < ui->OutputTable->rowCount(); i++)
        for (int j = 0; j < ui->OutputTable->columnCount(); j++)
            if (i && j)
                delete ui->OutputTable->item(i, j);

    ui->OutputTable->setRowCount(1);
    ui->OutputTable->setColumnCount(1);

    StateList.clear();
    ui->StartState->clear();
    InputList.clear();

    InputSeq.clear();
    InputSeq.append(QString());

    for (int i = 0; i < ui->ISequence->columnCount(); i++)
        delete ui->ISequence->item(0, i);
    ui->ISequence->setItem(0,0, new QTableWidgetItem());
    ui->ISequence->setColumnCount(1);
}

void MainWindow::resetMachine()
{
    qDebug()<<"reset machine";
    ui->TransTable->blockSignals(true);
    ui->OutputTable->blockSignals(true);

    ui->TransTable->item(LastStepState, 0)->setBackgroundColor(Qt::lightGray);
    ui->OutputTable->item(LastStepState, 0)->setBackgroundColor(Qt::lightGray);

    ui->CurrentState->clear();
    ui->InfiniteFlag->clear();
    ui->StateCount->clear();

    CurrentStep = 0;
    OutputSeq.clear();

    ui->Reset->setEnabled(false);
    ui->Emulate->setEnabled(true);

    for (int i = 0; i < ui->ISequence->columnCount()-1; i++)
        ui->ISequence->item(0, i)->setBackgroundColor(Qt::white);

    for (int i = 0; i < ui->OSequence->columnCount(); i++)
        delete ui->OSequence->item(0, i);
    ui->OSequence->setColumnCount(0);

    ui->TransTable->blockSignals(false);
    ui->OutputTable->blockSignals(false);

    ui->TransTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);
    ui->OutputTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);

}

void MainWindow::saveMachine(QString file)
{
    qDebug()<<"Saving machine...";
    QStringList tempOut;
    QStringList tempTrans;

    for (int i = 1; i < ui->TransTable->rowCount(); i++)
        for (int j = 1; j < ui->TransTable->columnCount(); j++)
            tempTrans.append(ui->TransTable->item(i, j)->text());


    if (MachineType)
    {
        qDebug()<<"Moore machine";

        for (int i = 1; i < ui->OutputTable->rowCount(); i++)
            tempOut.append(ui->OutputTable->item(i,1)->text());
    }
    else
    {
        qDebug()<<"Mealy machine";

        for (int i = 1; i < ui->OutputTable->rowCount(); i++)
            for (int j = 1; j < ui->OutputTable->columnCount(); j++)
                tempOut.append(ui->OutputTable->item(i, j)->text());
    }

    QFile outFile(file);
    if (outFile.open(QIODevice::WriteOnly))
    {
        QDataStream out (&outFile);
        out<<MachineType;
        out<<StateList;
        out<<InputList;
        out<<tempOut;
        out<<tempTrans;

        outFile.close();
    }
    else
    {
        QMessageBox::critical(this, tr("Error!"), tr("Can not open file!"), QMessageBox::Cancel);
    }

}

void MainWindow::loadMachine(QString file)
{
    if (Machine)
        clearMachine();

    qDebug()<<"Loading machine...";
    QFile inFile(file);
    if (inFile.open(QIODevice::ReadOnly))
    {
        QDataStream in (&inFile);
        QStringList tempOut;
        QStringList tempTrans;

        in>>MachineType;
        in>>StateList;
        in>>InputList;
        in>>tempOut;
        in>>tempTrans;

        qDebug()<<StateList;
        qDebug()<<InputList;
        qDebug()<<tempOut;
        qDebug()<<tempTrans;

        inFile.close();

        ui->TransTable->blockSignals(true);
        ui->OutputTable->blockSignals(true);

        setMachineType(MachineType);


        ui->TransTable->setRowCount(StateList.size()+1);
        ui->TransTable->setColumnCount(InputList.size() + 1);

        for (int i = 0; i < StateList.size(); i++)
        {
            ui->TransTable->setItem(i + 1, 0, new QTableWidgetItem(StateList[ i ]));
            ui->TransTable->item(i + 1, 0)->setBackgroundColor(Qt::lightGray);
            ui->StartState->addItem(StateList[ i ]);
        }

        for (int i = 0; i < InputList.size(); i++)
        {
            ui->TransTable->setItem(0, i + 1, new QTableWidgetItem(InputList[ i ]));
            ui->TransTable->item(0, i + 1)->setBackgroundColor(Qt::lightGray);
        }

        for (int i = 0; i < StateList.size(); i++)
            for (int j = 0; j < InputList.size(); j++)
                ui->TransTable->setItem(i + 1, j + 1, new QTableWidgetItem(tempTrans[ i*InputList.size() + j ]));


        ui->OutputTable->setRowCount(StateList.size()+1);

        if (MachineType)
        {
            qDebug()<<"Moore machine";

            ui->OutputTable->setColumnCount(2);
            for (int i = 0; i < StateList.size(); i++)
            {
                ui->OutputTable->setItem(i + 1, 0, new QTableWidgetItem(StateList[ i ]));
                ui->OutputTable->item(i + 1, 0)->setBackgroundColor(Qt::lightGray);
                ui->OutputTable->setItem(i + 1, 1, new QTableWidgetItem(tempOut[ i ]));
            }
        }
        else
        {
            qDebug()<<"Mealy machine";

            ui->OutputTable->setColumnCount(InputList.size() + 1);

            for (int i = 0; i < StateList.size(); i++)
            {
                ui->OutputTable->setItem(i + 1, 0, new QTableWidgetItem(StateList[ i ]));
                ui->OutputTable->item(i + 1, 0)->setBackgroundColor(Qt::lightGray);
            }

            for (int i = 0; i < InputList.size(); i++)
            {                
                ui->OutputTable->setItem(0, i + 1, new QTableWidgetItem(InputList[ i ]));
                ui->OutputTable->item(0, i + 1)->setBackgroundColor(Qt::lightGray);
            }

            for (int i = 0; i < StateList.size(); i++)
                for (int j = 0; j < InputList.size(); j++)
                    ui->OutputTable->setItem(i + 1, j + 1, new QTableWidgetItem(tempOut[ i*InputList.size() + j ]));
        }

        ui->OutputTable->blockSignals(false);
        ui->TransTable->blockSignals(false);
    }
    else
    {
        QMessageBox::critical(this, tr("Error!"), tr("Can not open file!"), QMessageBox::Cancel);
    }
}

void MainWindow::on_actionSave_triggered()
{
    if (ui->MachineWidget->isHidden())
        return;

    if (Machine)
        if (Machine->getMachineFile() != "")
        {
            saveMachine(Machine->getMachineFile());
            return;
        }

    on_actionSave_as_triggered();
}

void MainWindow::on_actionSave_as_triggered()
{
    if (ui->MachineWidget->isHidden())
        return;
    QString type = MachineType? "*.MooreFSM" : "*.MealyFSM" ;
    QString file = QFileDialog::getSaveFileName(0, tr("Save machine"), "", type);
    if (file == "")
        return;

    saveMachine(file);
}

void MainWindow::on_actionOpen_triggered()
{
    QString file = QFileDialog::getOpenFileName(0, tr("Open machine"), "", "*.MealyFSM , *.MooreFSM");
    if (file == "")
        return;

    clearMachine();
    loadMachine(file);
}


void MainWindow::on_actionMealy_machine_triggered()
{
    clearMachine();
    setMachineType(false);
}

void MainWindow::on_actionMoore_Machine_triggered()
{
    clearMachine();
    setMachineType(true);
}

void MainWindow::on_actionClose_triggered()
{
    clearMachine();
    ui->MachineWidget->hide();
}

void MainWindow::on_actionExit_triggered()
{
    qApp->exit(0);
}

void MainWindow::on_AddState_clicked()
{
    qDebug()<<"Add state";
    ui->TransTable->blockSignals(true);
    ui->OutputTable->blockSignals(true);

    int count = ui->TransTable->rowCount();

    ui->TransTable->insertRow(count);
    for (int i = 0; i < ui->TransTable->columnCount(); i++)
            ui->TransTable->setItem(count, i, new QTableWidgetItem());
    ui->TransTable->item(count, 0)->setBackgroundColor(Qt::lightGray);

    if (MachineType)
    {
        ui->OutputTable->insertRow(count);
        ui->OutputTable->setItem(count, 0, new QTableWidgetItem());
        ui->OutputTable->item(count, 0)->setBackgroundColor(Qt::lightGray);
        ui->OutputTable->setItem(count, 1, new QTableWidgetItem());
    }
    else
    {
        ui->OutputTable->insertRow(count);
        for (int i = 0; i < ui->OutputTable->columnCount(); i++)
                ui->OutputTable->setItem(count, i, new QTableWidgetItem());
        ui->OutputTable->item(count, 0)->setBackgroundColor(Qt::lightGray);
    }

    StateList.append(QString());
    ui->StartState->addItem(QString());


    ui->TransTable->blockSignals(false);
    ui->OutputTable->blockSignals(false);
}


void MainWindow::on_AddInput_clicked()
{
    qDebug()<<"Add input";
    ui->TransTable->blockSignals(true);
    ui->OutputTable->blockSignals(true);

    int count = ui->TransTable->columnCount();

    ui->TransTable->insertColumn(count);

    for (int i = 0; i <= ui->TransTable->rowCount(); i++)
        ui->TransTable->setItem(i, count, new QTableWidgetItem());
    ui->TransTable->item(0, count)->setBackgroundColor(Qt::lightGray);

    if (!MachineType)
    {
        ui->OutputTable->insertColumn(count);
        for (int i = 0; i <= ui->OutputTable->rowCount(); i++)
            ui->OutputTable->setItem(i, count, new QTableWidgetItem());
        ui->OutputTable->item(0, count)->setBackgroundColor(Qt::lightGray);
    }

    InputList.append(QString());

    ui->TransTable->blockSignals(false);
    ui->OutputTable->blockSignals(false);
}


void MainWindow::on_DeleteState_clicked()
{
    if (ui->TransTable->selectedItems().size() <= 0)
        return;

    int row = ui->TransTable->selectedItems().first()->row();
    if (row <= 0)
        return;

    ui->TransTable->blockSignals(true);
    ui->OutputTable->blockSignals(true);

    for (int i = 0; i < ui->TransTable->columnCount(); i++)
        delete ui->TransTable->item(row, i);
    ui->TransTable->removeRow(row);


    for (int i = 0; i < ui->OutputTable->columnCount(); i++)
        delete ui->OutputTable->item(row, i);
    ui->OutputTable->removeRow(row);

    StateList.removeAt(row - 1);
    ui->StartState->removeItem(row - 1);

    ui->TransTable->blockSignals(false);
    ui->OutputTable->blockSignals(false);
}


void MainWindow::on_DeleteInput_clicked()
{
    if (ui->TransTable->selectedItems().size() <= 0)
        return;

    int column = ui->TransTable->selectedItems().first()->column();
    if (column <= 0)
        return;

    ui->TransTable->blockSignals(true);
    ui->OutputTable->blockSignals(true);

    for (int i = 0; i < ui->TransTable->rowCount(); i++)
        delete ui->TransTable->item(i, column);
    ui->TransTable->removeColumn(column);

    if (!MachineType)
    {
        for (int i = 0; i < ui->OutputTable->rowCount(); i++)
            delete ui->OutputTable->item(i, column);
        ui->OutputTable->removeColumn(column);
    }

    InputList.removeAt(column - 1);

    ui->TransTable->blockSignals(false);
    ui->OutputTable->blockSignals(false);
}


void MainWindow::on_ISequence_itemChanged(QTableWidgetItem *item)
{
    ui->ISequence->blockSignals(true);

    if ((item->column() == ui->ISequence->columnCount() - 1) && (item->text() != ""))
    {
        int count = ui->ISequence->columnCount();
        ui->ISequence->insertColumn(count);
        ui->ISequence->setItem(0, count, new QTableWidgetItem());
        InputSeq.append(QString());
    }

    InputSeq[item->column()] = item->text();
    ui->ISequence->selectColumn(item->column() + 1);

    ui->ISequence->blockSignals(false);
}


void MainWindow::renameState(QString oldState, QString newState)
{
    qDebug()<<"rename state";

    for (int i = 1; i < ui->TransTable->columnCount(); i++)
        for (int j = 1; j < ui->TransTable->rowCount(); j++)
            if (ui->TransTable->item(j, i)->text() == oldState)
                ui->TransTable->item(j, i)->setText(newState);

    if (!MachineType)
    {
        for (int i = 1; i < ui->OutputTable->columnCount(); i++)
            for (int j = 1; j < ui->OutputTable->rowCount(); j++)
                if (ui->OutputTable->item(j, i)->text() == oldState)
                    ui->OutputTable->item(j, i)->setText(newState);
    }
}


void MainWindow::updateStateName(QTableWidgetItem *ChangedItem)
{
    qDebug()<<"update state";

    if (StateList.indexOf(ChangedItem->text()) != -1)
        ChangedItem->setText(StateList[ChangedItem->row() - 1]);
    else
    {
        ChangedItem->setBackgroundColor(Qt::lightGray);

        if (StateList[ChangedItem->row()-1] != QString(""))
            renameState(StateList[ChangedItem->row() - 1], ChangedItem->text());

        ui->OutputTable->item(ChangedItem->row(), 0)->setText(ChangedItem->text());
        ui->TransTable->item(ChangedItem->row(), 0)->setText(ChangedItem->text());

        StateList[ChangedItem->row()-1] = ChangedItem->text();
        ui->StartState->setItemText(ChangedItem->row()-1, ChangedItem->text());
    }
}


void MainWindow::updateInputName(QTableWidgetItem *ChangedItem)
{
    qDebug()<<"update input";

    if (InputList.indexOf(ChangedItem->text()) != -1)
        ChangedItem->setText(InputList[ChangedItem->column() - 1]);
    else
    {
        if (!MachineType)
        {
            ui->TransTable->item(0, ChangedItem->column())->setText(ChangedItem->text());
            ui->OutputTable->item(0, ChangedItem->column())->setText(ChangedItem->text());
        }

        InputList[ChangedItem->column() - 1] = ChangedItem->text();
    }
}


void MainWindow::checkTableItem(QTableWidgetItem *ChangedItem)
{
    qDebug()<<"check table";
    if (StateList.indexOf(ChangedItem->text()) == -1 && ChangedItem->text() != QString())
        ChangedItem->setBackgroundColor(Qt::red);
    else
        ChangedItem->setBackgroundColor(Qt::white);
}


void MainWindow::on_TransTable_itemChanged(QTableWidgetItem *ChangedItem)
{
    if (ChangedItem->column() == 0 && ChangedItem->row() == 0)
        return;

    ui->TransTable->blockSignals(true);
    ui->OutputTable->blockSignals(true);

    if (ChangedItem->column() == 0 && ChangedItem->row() != 0)
        updateStateName(ChangedItem);

    else
    if (ChangedItem->row() == 0 && ChangedItem->column() != 0)
        updateInputName(ChangedItem);

    else
            checkTableItem(ChangedItem);

    ui->TransTable->blockSignals(false);
    ui->OutputTable->blockSignals(false);

    qDebug()<<StateList<<InputList;
}


void MainWindow::on_OutputTable_itemChanged(QTableWidgetItem *ChangedItem)
{
    if (ChangedItem->column() == 0 && ChangedItem->row() == 0)
        return;

    ui->TransTable->blockSignals(true);
    ui->OutputTable->blockSignals(true);

    if (ChangedItem->column() == 0 && ChangedItem->row() != 0)
        updateStateName(ChangedItem);

    else
        if (!MachineType)
            if (ChangedItem->row() == 0 && ChangedItem->column() != 0)
                updateInputName(ChangedItem);

    ui->TransTable->blockSignals(false);
    ui->OutputTable->blockSignals(false);

    qDebug()<<StateList<<InputList;
}


void MainWindow::on_Emulate_clicked()
{
    qDebug()<<"Emulating...";
    if (ui->TransTable->rowCount() < 2 || ui->TransTable->columnCount() < 2)
        return;

    ui->TransTable->blockSignals(true);
    ui->OutputTable->blockSignals(true);

    ui->TransTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->OutputTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QStringList tempOut;
    QStringList tempTrans;


    for (int i = 1; i < ui->TransTable->rowCount(); i++)
        for (int j = 1; j < ui->TransTable->columnCount(); j++)
            tempTrans.append(ui->TransTable->item(i, j)->text());


    if (MachineType)
    {
        qDebug()<<"Moore machine";
        Machine = new MooreMachine();

        for (int i = 1; i < ui->OutputTable->rowCount(); i++)
            tempOut.append(ui->OutputTable->item(i, 1)->text());
    }
    else
    {
        qDebug()<<"Mealy machine";
        Machine =  new MealyMachine();

        for (int i = 1; i < ui->OutputTable->rowCount(); i++)
            for (int j = 1; j < ui->OutputTable->columnCount(); j++)
                tempOut.append(ui->OutputTable->item(i, j)->text());
    }

    Machine->updateMachine(StateList, InputList, tempOut, tempTrans);
    Machine->setStartState(ui->StartState->currentIndex());
    qDebug()<<"Machine updated";

    ui->StateCount->setText(QString::number(Machine->getStateCount()));
    ui->CurrentState->setText(ui->StartState->currentText());

    LastStepState = Machine->getCurrentState() + 1;
    CurrentStep = 0;
    ui->TransTable->item(LastStepState, 0)->setBackgroundColor(Qt::green);
    ui->OutputTable->item(LastStepState, 0)->setBackgroundColor(Qt::green);

    bool res = Machine->checkGrafAcyclic();
    ui->InfiniteFlag->setText((res)?tr("Yes"):tr("No"));

    ui->Reset->setEnabled(true);
    ui->Emulate->setEnabled(false);
}




void MainWindow::on_Reset_clicked()
{
    resetMachine();
}

void MainWindow::on_DoStep_clicked()
{
    doStep();
}

bool MainWindow::doStep()
{
    if (!Machine)
        return false;

    if (CurrentStep >= ui->ISequence->columnCount())
        return false;

    if (ui->ISequence->item(0, CurrentStep)->text() == "")
    {
        if (CurrentStep < ui->ISequence->columnCount()-2)
        {
            CurrentStep++;
            on_DoStep_clicked();
        }
        return true;
    }


    QString output;

    try
    {
        output = Machine->doStep(ui->ISequence->item(0, CurrentStep)->text());
    }
    catch (int error)
    {
        if (error == 1)
        {
            ui->ErrorLabel->setText(tr("No such input: ") + ui->ISequence->item(0, CurrentStep)->text());
            return false;
        }
        if (error == 0)
        {
            ui->ErrorLabel->setText(tr("No such transition"));
            return false;
        }
    }


    ui->ErrorLabel->setText("Well done!");

    OutputSeq.append(output);
    ui->OSequence->insertColumn(ui->OSequence->columnCount());
    ui->OSequence->setItem(0, ui->OSequence->columnCount() - 1, new QTableWidgetItem(OutputSeq.last()));

    int curState = Machine->getCurrentState();
    ui->CurrentState->setText(StateList[curState]);

    ui->TransTable->item(LastStepState, 0)->setBackgroundColor(Qt::lightGray);
    ui->TransTable->item(curState + 1, 0)->setBackgroundColor(Qt::green);

    ui->OutputTable->item(LastStepState, 0)->setBackgroundColor(Qt::lightGray);
    ui->OutputTable->item(curState + 1, 0)->setBackgroundColor(Qt::green);

    LastStepState = curState + 1;

    ui->ISequence->item(0, CurrentStep)->setBackgroundColor(Qt::green);

    CurrentStep++;
    return true;
}

void MainWindow::on_RunSeq_clicked()
{
    bool flag = true;
    do
        flag = doStep();
    while (flag);
}
