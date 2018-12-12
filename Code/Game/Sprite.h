#pragma once

#include "Collider.h"

class PhysicsEngine;

// ������������������͵�ö�٣�������һһ��Ӧ
enum SpriteType
{
	SPRITE_ACTOR, SPRITE_BULLET, SPRITE_OBSTACLE, SPRITE_ENEMY, SPRITE_WALL
};

// Ѫ����ʽö��
enum BloodBarType
{
	RED_BLOOD_BAR, BLUE_MAGIC_BAR, GRAY_SHELLED_BAR, BOSS_BLOOD_BAR, MOB_BLOOD_BAR
};

// ���ڼ�¼Ѫ���Ľṹ��
struct HealthData
{
	HealthData(int health)
		: health(health), maxHealth(health) {}

	// �������������˺���������˷���true��û������false
	bool injured(int damage);
	
	int health, maxHealth;
};

// Ѫ���ṹ��Ĵ����װ汾
struct HealthDataWithShield : public HealthData
{
	HealthDataWithShield(int health, int shield)
		: HealthData(health), shield(shield), maxShield(shield) {}

	// �������������˺����ȵ��׺��Ѫ������ͬ��
	bool injured(int damage);

	int shield, maxShield;
};

// ���ڼ�¼ħ��ֵ�Ľṹ��
struct MagicData
{
	MagicData(int magic)
		: magic(magic), maxMagic(magic) {}

	// �������������ħ�������ʣ��ħ�������򷵻�false�����ĳɹ�����true
	bool consume(int cost);

	int magic, maxMagic;
};

// ���ڼ�¼������ȴʱ��Ľṹ��
template<size_t n>
struct CooldownData
{
	CooldownData() : remainTime(0), totalTime{0} {}

	double remainTime, totalTime[n];
};

// ������ͼ�Ļ��࣬��ͼģ���õ�
class SpriteView
{
public:
	virtual ~SpriteView() {}

protected:
	SpriteView() {}

public:
	static void DrawBloodBar(QPainter &painter, QRect rect, int health, int maxhp, BloodBarType type);
	static void DrawBloodBackground(QPainter &painter, QRect rect);

	virtual void readStream(QDataStream &spriteStateData) {}
	virtual void drawBody(QPainter &painter, const M2S_Transformation &f) {}
	virtual void drawBloodBar(QPainter &painter, const M2S_Transformation &f) {}

public:
	MapPoint centerPos;
};

// ����ģ�͵Ļ��࣬��������ģ���õ�
class SpriteModel
{
public:
	virtual ~SpriteModel() {}

protected:
	SpriteModel(SpriteType type, int kind) : type(type), kind(kind), toDelete(false), isNew(true) {}

public:
	static bool SketchyCollisionDetection(const SpriteModel &src1, const SpriteModel &src2);

	virtual void writeStream(QDataStream &spriteStateData) const {}
	virtual void action() {}
	virtual bool collisionDetection(SpriteModel &other, double &delta_t) { return false; }
	virtual bool staticCollisionDetection(Collider &other, const MapPoint &p2, double &delta_t) { return false; }
	virtual bool dynamicCollisionDetection(Collider &other, const MapPoint &p2, const MapVector &v2, double &delta_t) { return false; }
	virtual void dealCollision(SpriteModel &other, double t) {}
	virtual void updatePos(double delta_t) {}
	virtual void borderCheck(double map_width, double map_height) {}
	virtual void updateTime(double delta_t) {}

	virtual double maxX() const = 0;
	virtual double minX() const = 0;
	virtual double maxY() const = 0;
	virtual double minY() const = 0;

	SpriteType getType() const { return type; }
	int getCollisionKind() const { return kind; }
	ClosedRegion2D wrappedRect() const { return ClosedRegion2D(minX(), maxX(), minY(), maxY()); }

	
public:
	MapPoint centerPos;
	bool toDelete, isNew;

private:
	SpriteType type;
	int kind;
};

