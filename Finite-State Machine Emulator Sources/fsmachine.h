#ifndef FSMACHINE_H
#define FSMACHINE_H

#include <QObject>
#include <QHash>
#include <QStringList>
#include <QVector>

class FSMachine: public QObject
{
    Q_OBJECT

public:
    FSMachine(){}
    ~FSMachine(){}

    virtual QString doStep(QString input) = 0;
    virtual void updateMachine(QStringList States, QStringList Input, QStringList Output, QStringList Transitions) = 0;

    virtual bool getMachineType() = 0;
    virtual void clearMachine() = 0;

    void setStartState(int state);
    int getStartState();

    int getStateCount();
    int getCurrentState();

    QString getMachineFile();

    bool checkGrafAcyclic();

signals:
    void machineError(int type);
    void saveFileMissed();

protected:
    QString MachineFile;

    int StartState;
    int CurrentState;

    QHash <QString, int> StateNames;
    QHash <QString, int> InputNames;
    QVector <QVector<QString>> TransitionsTable;
};


class MealyMachine : virtual public FSMachine
{
    Q_OBJECT
public:
    MealyMachine();

    QString doStep(QString input);
    void updateMachine(QStringList States, QStringList Input, QStringList Output, QStringList Transitions);

    bool getMachineType();
    void clearMachine();

private:
    QVector <QVector<QString>> OutputTable;
};


class MooreMachine : virtual public FSMachine
{
    Q_OBJECT
public:
    MooreMachine();

    QString doStep(QString input);
    void updateMachine(QStringList States, QStringList Input, QStringList Output, QStringList Transitions);

    bool getMachineType();
    void clearMachine();

private:
    QVector<QString> OutputTable;
};

#endif // FSMACHINE_H
