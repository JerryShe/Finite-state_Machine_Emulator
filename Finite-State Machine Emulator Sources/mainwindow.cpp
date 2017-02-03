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
    ui->StateTable->item(0,0)->setFlags(Qt::NoItemFlags);
    ui->StateTable->setRowCount(1);
    ui->StateTable->setColumnCount(1);
    Machine = 0;
}

MainWindow::~MainWindow()
{
    clearMachine();
    qDebug()<<"delete ui";
    delete ui;
}

void MainWindow::clearMachine()
{
    qDebug()<<"clear memory";
    resetMachine();

    if (Machine)
    {
        Machine->clearMachine();
        delete Machine;
        Machine = 0;
    }

    for (int i = 0; i < ui->StateTable->rowCount(); i++)
        for (int j = 0; j < ui->StateTable->columnCount(); j++)
            if (i && j)
                delete ui->StateTable->item(i, j);


    ui->StateTable->setRowCount(1);
    ui->StateTable->setColumnCount(1);

    for (int i = 0; i < ui->StateOutputTable->columnCount(); i++)
    {
        delete ui->StateOutputTable->item(0, i);
        delete ui->StateOutputTable->item(1, i);
    }
    ui->StateOutputTable->setColumnCount(0);

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
    ui->StateTable->blockSignals(true);
    ui->StateOutputTable->blockSignals(true);

    ui->StateTable->item(LastStepState, 0)->setBackgroundColor(Qt::lightGray);

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

    ui->StateTable->blockSignals(true);
    ui->StateOutputTable->blockSignals(true);
    ui->StateTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::AnyKeyPressed);

}

void MainWindow::saveMachine(QString file)
{
    qDebug()<<"Saving machine...";
    QStringList tempOut;
    QStringList tempTrans;
    if (MachineType)
    {
        qDebug()<<"Moore machine";

        for (int i = 0; i < ui->StateOutputTable->columnCount(); i++)
            tempOut.append(ui->StateOutputTable->item(1,i)->text());

        for (int i = 1; i < ui->StateTable->rowCount(); i++)
            for (int j = 1; j < ui->StateTable->columnCount(); j++)
                tempTrans.append(ui->StateTable->item(i, j)->text());
    }
    else
    {
        qDebug()<<"Mealy machine";

        for (int i = 1; i < ui->StateTable->rowCount(); i++)
            for (int j = 1; j <= (ui->StateTable->columnCount() - 1)/2; j++)
                tempOut.append(ui->StateTable->item(i, j)->text());

        for (int i = 1; i < ui->StateTable->rowCount(); i++)
            for (int j = (ui->StateTable->columnCount() - 1)/2 + 1; j < ui->StateTable->columnCount(); j++)
                tempTrans.append(ui->StateTable->item(i, j)->text());
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

        inFile.close();

        ui->StateTable->blockSignals(true);

        setMachineType(MachineType);
        ui->StateTable->setRowCount(StateList.size()+1);

        for (int i = 0; i < StateList.size(); i++)
        {
            ui->StateTable->setItem(i + 1, 0, new QTableWidgetItem(StateList[ i ]));
            ui->StateTable->item(i + 1, 0)->setBackgroundColor(Qt::lightGray);
            ui->StartState->addItem(StateList[ i ]);
        }

        if (MachineType)
        {
            qDebug()<<"Moore machine";

            ui->StateOutputTable->blockSignals(true);

            ui->StateTable->setColumnCount(InputList.size()+1);

            for (int i = 0; i < InputList.size(); i++)
            {
                ui->StateTable->setItem(0, i + 1, new QTableWidgetItem(InputList[ i ]));
                ui->StateTable->item(0, i + 1)->setBackgroundColor(Qt::lightGray);
            }

            ui->StateOutputTable->setColumnCount(StateList.size());
            for (int i = 0; i < StateList.size(); i++)
            {
                ui->StateOutputTable->setItem(0, i, new QTableWidgetItem(StateList[ i ]));
                ui->StateOutputTable->item(0, i)->setBackgroundColor(Qt::lightGray);
                ui->StateOutputTable->setItem(1, i, new QTableWidgetItem(tempOut[ i ]));
            }

            for (int i = 0; i < StateList.size(); i++)
                for (int j = 0; j < InputList.size(); j++)
                {
                    ui->StateTable->setItem(i + 1, j + 1, new QTableWidgetItem(tempTrans[ i*InputList.size() + j ]));
                }

            ui->StateOutputTable->blockSignals(false);
        }
        else
        {
            qDebug()<<"Mealy machine";

            ui->StateTable->setColumnCount(InputList.size()*2+1);

            for (int i = 1; i <= InputList.size(); i++)
            {
                ui->StateTable->setItem(0, i, new QTableWidgetItem(InputList[ i - 1 ]));
                ui->StateTable->setItem(0, i + InputList.size(), new QTableWidgetItem(InputList[ i - 1 ]));
                ui->StateTable->item(0, i)->setBackgroundColor(Qt::lightGray);
                ui->StateTable->item(0, i + InputList.size())->setBackgroundColor(Qt::lightGray);
            }

            for (int i = 0; i < StateList.size(); i++)
                for (int j = 0; j < InputList.size(); j++)
                {
                    ui->StateTable->setItem(i + 1, j + 1, new QTableWidgetItem(tempOut[ i*InputList.size() + j ]));
                    ui->StateTable->setItem(i + 1, j + 1 + InputList.size(), new QTableWidgetItem(tempTrans[ i*InputList.size() + j ]));
                }
        }

        ui->StateTable->blockSignals(false);
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

    loadMachine(file);
}

void MainWindow::setMachineType(bool type)
{
    if (Machine != 0)
        clearMachine();

    MachineType = type;
    if (MachineType)
    {
        ui->MachineWidget->show();
        ui->StateOutputLabel->show();
        ui->StateOutputTable->show();
        ui->MachineTableLabel->setText(tr("State transition table"));
    }
    else
    {
        ui->MachineWidget->show();
        ui->StateOutputLabel->hide();
        ui->StateOutputTable->hide();
        ui->MachineTableLabel->setText(tr("State transition/output table"));
    }
    qDebug()<<"MachineType"<<MachineType;
}


void MainWindow::on_actionMealy_machine_triggered()
{
    setMachineType(false);
}

void MainWindow::on_actionMoore_Machine_triggered()
{
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
    ui->StateTable->blockSignals(true);
    ui->StateOutputTable->blockSignals(true);

    int count = ui->StateTable->rowCount();

    ui->StateTable->insertRow(count);

    for (int i = 0; i < ui->StateTable->columnCount(); i++)
            ui->StateTable->setItem(count, i, new QTableWidgetItem());

    ui->StateTable->item(count, 0)->setBackgroundColor(Qt::lightGray);

    if (MachineType)
    {
        count--;
        ui->StateOutputTable->insertColumn(count);
        ui->StateOutputTable->setItem(0, count, new QTableWidgetItem());
        ui->StateOutputTable->item(0, count)->setBackgroundColor(Qt::lightGray);
        ui->StateOutputTable->setItem(1, count, new QTableWidgetItem());
    }

    StateList.append(QString());
    ui->StartState->addItem(QString());


    ui->StateTable->blockSignals(false);
    ui->StateOutputTable->blockSignals(false);
}

void MainWindow::on_AddInput_clicked()
{
    ui->StateTable->blockSignals(true);
    ui->StateOutputTable->blockSignals(true);

    int count;
    if (MachineType)
        count = ui->StateTable->columnCount();
    else
        count = (ui->StateTable->columnCount() - 1)/2 + 1;

    ui->StateTable->insertColumn(count);
    for (int i = 0; i <= ui->StateTable->rowCount(); i++)
        ui->StateTable->setItem(i, count, new QTableWidgetItem());
    ui->StateTable->item(0, count)->setBackgroundColor(Qt::lightGray);

    if (!MachineType)
    {
        ui->StateTable->insertColumn(count*2);
        for (int i = 0; i <= ui->StateTable->rowCount(); i++)
            ui->StateTable->setItem(i, count*2, new QTableWidgetItem());
        ui->StateTable->item(0, count*2)->setBackgroundColor(Qt::lightGray);
    }

    InputList.append(QString());

    ui->StateTable->blockSignals(false);
    ui->StateOutputTable->blockSignals(false);
}

void MainWindow::renameState(QString oldState, QString newState)
{
    qDebug()<<"rename state";
    ui->StateTable->blockSignals(true);
    ui->StateOutputTable->blockSignals(true);

    int i;
    if (MachineType)
        i = 1;
    else
        i = (ui->StateTable->columnCount()-1)/2 + 1;

    for (; i < ui->StateTable->columnCount(); i++)
        for (int j = 1; j < ui->StateTable->rowCount(); j++)
        {
            if (ui->StateTable->item(j, i))
            {
                if (ui->StateTable->item(j, i)->text() == oldState)
                    ui->StateTable->item(j, i)->setText(newState);
            }
            else
                    qDebug()<<"wtf"<<j<<i;
        }

    ui->StateOutputTable->blockSignals(false);
    ui->StateTable->blockSignals(false);
}

void MainWindow::on_DeleteState_clicked()
{
    ui->StateTable->blockSignals(true);
    ui->StateOutputTable->blockSignals(true);

    int row = ui->StateTable->selectedItems().first()->row();

    for (int i = 0; i < ui->StateTable->columnCount(); i++)
        delete ui->StateTable->item(row, i);
    ui->StateTable->removeRow(row);

    if (MachineType)
    {
        delete ui->StateOutputTable->item(0, row - 1);
        delete ui->StateOutputTable->item(1, row - 1);
        ui->StateOutputTable->removeColumn(row - 1);
    }

    StateList.removeAt(row - 1);
    ui->StartState->removeItem(row - 1);

    ui->StateOutputTable->blockSignals(false);
    ui->StateTable->blockSignals(false);
}

void MainWindow::on_DeleteInput_clicked()
{
    if (ui->StateTable->selectedItems().size() <= 0)
        return;

    ui->StateTable->blockSignals(true);
    ui->StateOutputTable->blockSignals(true);

    int column = ui->StateTable->selectedItems().first()->column();

    if (!MachineType)
    {
        if (column > (ui->StateTable->columnCount()-1)/2)
        {
            for (int i = 0; i < ui->StateTable->rowCount(); i++)
                delete ui->StateTable->item(i, column);
            ui->StateTable->removeColumn(column);
            column = column - (ui->StateTable->columnCount()-1)/2 - 1;
        }
        else
        {
            for (int i = 0; i < ui->StateTable->rowCount(); i++)
                delete ui->StateTable->item(i, (ui->StateTable->columnCount()-1)/2 + column);
            ui->StateTable->removeColumn((ui->StateTable->columnCount()-1)/2 + column);
        }
    }

    for (int i = 0; i < ui->StateTable->rowCount(); i++)
        delete ui->StateTable->item(i, column);
    ui->StateTable->removeColumn(column);

    InputList.removeAt(column - 1);

    ui->StateTable->blockSignals(false);
    ui->StateOutputTable->blockSignals(false);
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
        if (MachineType)
            ui->StateOutputTable->item(0, ChangedItem->row() - 1)->setText(ChangedItem->text());

        StateList[ChangedItem->row()-1] = ChangedItem->text();
        ui->StartState->setItemText(ChangedItem->row()-1, ChangedItem->text());
    }
}

void MainWindow::updateInputName(QTableWidgetItem *ChangedItem)
{
    qDebug()<<"update input";
    if (!MachineType)
    {
        if (ChangedItem->column() > (ui->StateTable->columnCount()-1)/2)
        {
            if (InputList.indexOf(ChangedItem->text()) != -1)
                ChangedItem->setText(InputList[ChangedItem->column()-(ui->StateTable->columnCount()-1)/2 - 1]);
            else
            {
                ui->StateTable->item(0, ChangedItem->column()-(ui->StateTable->columnCount()-1)/2)->setText(ChangedItem->text());
                InputList[ChangedItem->column() - (ui->StateTable->columnCount()-1)/2 - 1] = ChangedItem->text();
            }
        }
        else
        {
            if (InputList.indexOf(ChangedItem->text()) != -1)
                ChangedItem->setText(InputList[ChangedItem->column() - 1]);
            else
            {
                ui->StateTable->item(0, (ui->StateTable->columnCount() - 1)/2 + ChangedItem->column())->setText(ChangedItem->text());
                InputList[ChangedItem->column() - 1] = ChangedItem->text();
            }
        }
    }
    else
    {
        if (InputList.indexOf(ChangedItem->text()) != -1)
            ChangedItem->setText(InputList[ChangedItem->column() - 1]);
        else
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

void MainWindow::on_StateTable_itemChanged(QTableWidgetItem *ChangedItem)
{
    if (ChangedItem->column() == 0 && ChangedItem->row() == 0)
        return;

    ui->StateTable->blockSignals(true);
    ui->StateOutputTable->blockSignals(true);

    if (ChangedItem->column() == 0 && ChangedItem->row() != 0)
        updateStateName(ChangedItem);

    else
    if (ChangedItem->row() == 0 && ChangedItem->column() != 0)
        updateInputName(ChangedItem);

    else
    {
        if (MachineType)
            checkTableItem(ChangedItem);
        else
            if (ChangedItem->column() > (ui->StateTable->columnCount()-1)/2)
                checkTableItem(ChangedItem);
    }

    ui->StateTable->blockSignals(false);
    ui->StateOutputTable->blockSignals(false);
    qDebug()<<StateList<<InputList;
}

void MainWindow::on_StateOutputTable_itemChanged(QTableWidgetItem *ChangedItem)
{
    ui->StateTable->blockSignals(true);
    ui->StateOutputTable->blockSignals(true);

    if (MachineType)
    {
        if (ChangedItem->row() == 0)
        {
            if (StateList.indexOf(ChangedItem->text()) != -1)
            {
                ui->StateTable->item(ChangedItem->column() + 1, 0)->setText(ChangedItem->text());
                StateList[ChangedItem->column()] = ChangedItem->text();
                ui->StartState->setItemText(ChangedItem->column(), ChangedItem->text());
            }
            else
                ChangedItem->setText(StateList[ChangedItem->column()]);
        }
    }

    ui->StateTable->blockSignals(false);
    ui->StateOutputTable->blockSignals(false);
}



void MainWindow::on_Emulate_clicked()
{
    qDebug()<<"Emulating...";
    ui->StateTable->blockSignals(true);
    ui->StateOutputTable->blockSignals(true);
    ui->StateTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QStringList tempOut;
    QStringList tempTrans;
    if (MachineType)
    {
        qDebug()<<"Moore machine";
        Machine = new MooreMachine();

        for (int i = 0; i < ui->StateOutputTable->columnCount(); i++)
            tempOut.append(ui->StateOutputTable->item(1,i)->text());

        for (int i = 1; i < ui->StateTable->rowCount(); i++)
            for (int j = 1; j < ui->StateTable->columnCount(); j++)
                tempTrans.append(ui->StateTable->item(i, j)->text());
    }
    else
    {
        qDebug()<<"Mealy machine";
        Machine =  new MealyMachine();

        for (int i = 1; i < ui->StateTable->rowCount(); i++)
            for (int j = 1; j <= (ui->StateTable->columnCount() - 1)/2; j++)
                tempOut.append(ui->StateTable->item(i, j)->text());

        for (int i = 1; i < ui->StateTable->rowCount(); i++)
            for (int j = (ui->StateTable->columnCount() - 1)/2 + 1; j < ui->StateTable->columnCount(); j++)
                tempTrans.append(ui->StateTable->item(i, j)->text());
    }

    Machine->updateMachine(StateList, InputList, tempOut, tempTrans);
    Machine->setStartState(ui->StartState->currentIndex());
    qDebug()<<"Machine updated";

    ui->StateCount->setText(QString::number(Machine->getStateCount()));
    ui->CurrentState->setText(ui->StartState->currentText());

    LastStepState = Machine->getCurrentState() + 1;
    ui->StateTable->item(LastStepState, 0)->setBackgroundColor(Qt::green);

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
        CurrentStep++;
        on_DoStep_clicked();
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

    ui->StateTable->item(LastStepState, 0)->setBackgroundColor(Qt::lightGray);
    ui->StateTable->item(curState + 1, 0)->setBackgroundColor(Qt::green);
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
