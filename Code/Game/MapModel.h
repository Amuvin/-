#pragma once

#include "Sprite.h"

// ǽ�࣬��������ײ����
class WallModel : public SpriteModel
{
public:
	WallModel(const MapPoint &pos, double width, double height);

	bool collisionDetection(SpriteModel &other, double &delta_t) override;
	bool staticCollisionDetection(Collider &other, const MapPoint &p2, double &delta_t) override;
	bool dynamicCollisionDetection(Collider &other, const MapPoint &p2, const MapVector &v2, double &delta_t) override;

	double maxX() const override { return centerPos.x + collider.halfWH.x; }
	double minX() const override { return centerPos.x - collider.halfWH.x; }
	double maxY() const override { return centerPos.y + collider.halfWH.y; }
	double minY() const override { return centerPos.y - collider.halfWH.y; }

private:
	RectShapedCollider collider;
};

// ��ͼģ���࣬�����ж���ͼ�ϸ��ص��Ƿ��ͨ��
// �ڵ�ͼ����ϵ�У���ͼ�����Ͻ���(0, 0), ���½���(width(), -height())
class MapModel
{
public:
	MapModel(QString source_file_name, double block_size);
	~MapModel();

	double getWidth() const { return width; }
	double getHeight() const { return height; }
	QVector<SpriteModel*> getWalls() const { return walls; }

private:
	void addWall(int i, int j, int r, int c);

private:
	int rows, columns;
	double width, height;
	QVector<SpriteModel*> walls;
	double blockSize;
};

