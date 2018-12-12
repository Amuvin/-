#pragma once

#include "Sprite.h"

struct KeyState
{
	KeyState() : x(0), y(0) {}
	KeyState(qint8 x, qint8 y) : x(x), y(y) {}
	qint8 x, y;
};

struct MouseState
{
	MouseState() : x(0.0), y(0.0), pressed(false) {}
	MouseState(double x, double y, bool pressed = true) : x(x), y(y), pressed(pressed) {}

	double x, y;
	bool pressed;
};

struct InputState
{
	MouseState mouse;
	KeyState key;
};

class DataSender
{
public:
	virtual ~DataSender() {}

	static void EncodeSpriteState(QByteArray &dst, const QList<SpriteModel*> &src);
	static void EncodeNewSpriteInfo(QByteArray &dst, const QList<SpriteModel*> &src);

	virtual void sendKeyState(const KeyState &state) { qDebug() << u8"��ǰ��ɫ�ж�����(" << state.x << ", " << state.y << ")"; }
	virtual void sendMousePressInfo(const MapPoint &click_pos) { qDebug() << u8"�����(" << click_pos.x << ", " << click_pos.y << u8")������"; }
	virtual void sendMouseReleaseInfo() { qDebug() << u8"����ɿ�"; }
	virtual void sendSpriteState(const QList<SpriteModel*> &sprites) { qDebug() << u8"�����˾�������"; }
	virtual void sendSpriteState(QByteArray &data) { qDebug() << u8"�����˾�������"; }
	virtual void sendNewSpriteInfo(const QList<SpriteModel*> &sprites) { qDebug() << u8"�������¾�����Ϣ"; }
	virtual void sendNewSpriteInfo(QByteArray &data) { qDebug() << u8"�������¾�����Ϣ"; }
};

class DataReceiver
{
public:
	virtual ~DataReceiver() {}

	static void DecodeSpriteState(QList<SpriteView*> &dst, QByteArray &src);
	static void DecodeNewSpriteInfo(QList<SpriteView*> &dst, QByteArray &src);

	virtual void receiveInputState(InputState &state) { qDebug() << u8"���ܽ���"; }
	virtual void receiveInputState(QVector<InputState> &states) { qDebug() << u8"���ܽ���"; }
	virtual void receiveSpriteState(QList<SpriteView*> &sprites) { qDebug() << u8"���ܽ���"; }
};

class DataManager : public DataSender, public DataReceiver
{
public:
	DataManager() {}

	void sendKeyState(const KeyState &data) override;
	void sendMousePressInfo(const MapPoint &click_pos) override;
	void sendMouseReleaseInfo() override;	
	void sendSpriteState(const QList<SpriteModel*> &sprites) override;
	void sendSpriteState(QByteArray &data) override;
	void sendNewSpriteInfo(const QList<SpriteModel*> &sprites) override;
	void sendNewSpriteInfo(QByteArray &data) override;

	void receiveInputState(InputState &state) override;
	void receiveInputState(QVector<InputState> &states) override;
	void receiveSpriteState(QList<SpriteView*> &sprites) override;

private:
	QMutex inputStateMutex, spriteStateMutex, newSpriteInfoMutex;
	InputState inputStateData;
	QQueue<QByteArray> spriteStateData, newSpriteInfoData;
};

class DataBroadcaster : public DataSender
{
public:
	DataBroadcaster(const QVector<DataSender*> &senders) : senders(senders) {}

	void sendSpriteState(const QList<SpriteModel*> &sprites) override;
	void sendSpriteState(QByteArray &data) override;
	void sendNewSpriteInfo(const QList<SpriteModel*> &sprites) override;
	void sendNewSpriteInfo(QByteArray &data) override;

private:
	QVector<DataSender*> senders;
};

class DataCollector : public DataReceiver
{
public:
	DataCollector(const QVector<DataReceiver*> &receivers) : receivers(receivers) {}

	void receiveInputState(QVector<InputState> &states) override;

private:
	QVector<DataReceiver*> receivers;
};

// ������Ӧ�ò㱨�ĸ�ʽ�й�
#define KEY_STATE (quint8)0
#define MOUSE_PRESS (quint8)1
#define MOUSE_RELEASE (quint8)2
#define UPDATE_SPRITE (quint8)3
#define NEW_SPRITE (quint8)4
