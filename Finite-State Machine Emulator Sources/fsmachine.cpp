#include "fsmachine.h"
#include <QFile>
#include <QDataStream>
#include <QDebug>

int FSMachine::getStateCount()
{
    return StateNames.size();
}

int FSMachine::getCurrentState()
{
    return CurrentState;
}

int FSMachine::getStartState()
{
    return StartState;
}

void FSMachine::setStartState(int state)
{
    StartState = CurrentState = state;
}

QString FSMachine::getMachineFile()
{
    return MachineFile;
}

bool FSMachine::checkGrafAcyclic()
{
    qDebug()<<"acyclic check";

    QVector <QVector<bool>> IdentTable;

    IdentTable.resize(TransitionsTable.size());
    for (int i = 0; i < IdentTable.size(); i++)
        IdentTable[i].resize(TransitionsTable.size());

    for (int i = 0; i < IdentTable.size(); i++)
        for (int j = 0; j < IdentTable[i].size(); j++)
            IdentTable[i][j] = false;

    for (int i = 0; i < TransitionsTable.size(); i++)
        for (int j = 0; j < TransitionsTable[i].size(); j++)
            if (StateNames.contains(TransitionsTable[i][j]))
                IdentTable[i][StateNames[TransitionsTable[i][j]]] = true;


    bool stateWasDeleted = true;

    while (stateWasDeleted)
    {
        stateWasDeleted = false;
        for (int i = 0; i < IdentTable.size();i++)
        {
            bool stateType = false;
            for (int j = 0; j < IdentTable.size(); j++)
                if (IdentTable[i][j] == true)
                    stateType = true;

            if (stateType == false)
            {
                for (int k = 0; k < IdentTable.size(); k++)
                    IdentTable[k].removeAt(i);
                IdentTable.removeAt(i);
                stateWasDeleted = true;
            }
        }
    }

    if (IdentTable.size() != 0)
    {
        qDebug()<<"cyclic";
        return true;
    }
    else
    {
        qDebug()<<"acyclic";
        return false;
    }
}


///////////////////////////////////Mealy/////////////////////////////////////////

MealyMachine::MealyMachine()
{
    CurrentState = StartState = 0;
}



void MealyMachine::updateMachine(QStringList States, QStringList Input, QStringList Output, QStringList Transitions)
{
    CurrentState = 0;
    QVector <QString> tempTrans;
    QVector <QString> tempOut;
    for (int i = 0; i < States.size(); i++)
    {
        StateNames.insert(States[i], i);
        for (int j = 0; j < Input.size(); j++)
        {
            tempTrans.append(Transitions[i *Input.size() + j]);
            tempOut.append(Output[i*Input.size() + j]);
        }

        TransitionsTable.append(tempTrans);
        OutputTable.append(tempOut);
        tempTrans.clear();
        tempOut.clear();
    }

    for (int i = 0; i < Input.size(); i++)
        InputNames.insert(Input[i], i);

    qDebug()<<TransitionsTable;
    qDebug()<<OutputTable;
}

void MealyMachine::clearMachine()
{
    StateNames.clear();
    InputNames.clear();
    TransitionsTable.clear();
    OutputTable.clear();
    CurrentState = StartState;
}

QString MealyMachine::doStep(QString input)
{
    if (!InputNames.contains(input))
    {
        throw 1;
        return QString();
    }

    int index = InputNames[input];
    QString output = OutputTable[CurrentState][index];

    if (StateNames.contains(TransitionsTable [CurrentState][index]))
        CurrentState = StateNames[TransitionsTable [CurrentState][index]];
    else
        throw 2;

    return output;
}

bool MealyMachine::getMachineType()
{
    return false;
}


///////////////////////////////////Moore/////////////////////////////////////////

MooreMachine::MooreMachine()
{
    StartState = CurrentState = 0;
}

void MooreMachine::updateMachine(QStringList States, QStringList Input, QStringList Output, QStringList Transitions)
{
    CurrentState = 0;
    QVector <QString> tempTrans;
    for (int i = 0; i < States.size(); i++)
    {
        StateNames.insert(States[i], i);
        for (int j = 0; j < Input.size(); j++)
            tempTrans.append(Transitions[i*Input.size() + j]);

        TransitionsTable.append(tempTrans);
        tempTrans.clear();
    }

    for (int i = 0; i < Input.size(); i++)
        InputNames.insert(Input[i], i);

    for (int i = 0; i < Output.size(); i++)
        OutputTable.append(Output[i]);
}

void MooreMachine::clearMachine()
{
    StateNames.clear();
    InputNames.clear();
    TransitionsTable.clear();
    OutputTable.clear();
    CurrentState = StartState;
}

QString MooreMachine::doStep(QString input)
{
    if (!InputNames.contains(input))
    {
        throw 1;
        return QString();
    }

    int index = InputNames[input];
    QString output = OutputTable[CurrentState];
    if (StateNames.contains(TransitionsTable[CurrentState][index]))
        CurrentState = StateNames[TransitionsTable[CurrentState][index]];
    else
        throw 2;

    return output;
}

bool MooreMachine::getMachineType()
{
    return true;
}

