#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QSplitter>

#include "fsmachine.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionExit_triggered();
    void on_actionMealy_machine_triggered();
    void on_actionSave_as_triggered();
    void on_actionMoore_Machine_triggered();
    void on_actionClose_triggered();

    void on_TransTable_itemChanged(QTableWidgetItem *ChangedItem);
    void on_OutputTable_itemChanged(QTableWidgetItem *ChangedItem);

    void on_ISequence_itemChanged(QTableWidgetItem *item);

    void on_AddState_clicked();
    void on_AddInput_clicked();
    void on_DeleteState_clicked();
    void on_DeleteInput_clicked();

    void on_Emulate_clicked();
    void on_Reset_clicked();

    void on_DoStep_clicked();
    void on_RunSeq_clicked();

    bool doStep();



private:
    void renameState(QString oldState, QString newState);
    void setMachineType(bool type);
    void clearMachine();
    void resetMachine();

    void saveMachine(QString file);
    void loadMachine(QString file);

    void updateStateName(QTableWidgetItem *ChangedItem);
    void updateInputName(QTableWidgetItem *ChangedItem);
    void checkTableItem(QTableWidgetItem *ChangedItem);

    bool MachineType; /// 0 - Мили, 1 - Мура
    FSMachine* Machine;

    Ui::MainWindow *ui;

    QStringList StateList;
    QStringList InputList;

    int CurrentStep;
    int LastStepState;
    QStringList InputSeq;
    QStringList OutputSeq;
};

#endif // MAINWINDOW_H
