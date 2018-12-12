#pragma once

class WorkerThread : public QObject
{
	Q_OBJECT

public:
	WorkerThread();
	virtual ~WorkerThread();

	bool isRunning() const { return running; }
	void waitAndQuit();

signals:
	void started();
	void stopped();

protected:
	virtual void mainloop() = 0;

private:
	void goToWork();

protected:
	QString name;
	double timePassed; // ��¼����ѭ����ʱ����
	double defaultFPS; // �趨֡��

private:
	QThread workThread;
	QTimer timer;
	time_t lastTime, currentTime;
	volatile bool running, toQuit;
};
