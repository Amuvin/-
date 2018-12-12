#include "stdafx.h"
#include "Collider.h"

bool CircleShapedCollider::staticCollisionDetection(CircleShapedCollider &other, const MapPoint &p1, const MapPoint &p2)
{
	return distance_square(p1, p2) < square(this->radius + other.radius);
}

bool CircleShapedCollider::staticCollisionDetection(RectShapedCollider &other, const MapPoint &p1, const MapPoint &p2)
{
	// ��Բ��Ϊ��׼��������ε����λ��
	MapVector v0 = p2 - p1;

	// �ҳ������Ͼ���Բ������ĵ��λ��
	MapVector v = vmin(vmax(MapVector::Zeros(), v0 - other.halfWH), v0 + other.halfWH);
	
	// �ж������Բ�ĵľ����Ƿ�С�ڰ뾶
	return v.square() < square(radius);
}

bool CircleShapedCollider::dynamicCollisionDetection(CircleShapedCollider &other, const MapPoint &p1, const MapPoint &p2, const MapVector &v1, const MapVector &v2, double &delta_t)
{
	// ������Ծ�ֹ���������
	if (v1 == v2)
	{
		bool collision = CircleShapedCollider::staticCollisionDetection(other, p1, p2);
		delta_t = collision ? 0 : delta_t;
		return collision;
	}		 

	// �������λ�ƺ��ٶ�
	MapVector r = p1 - p2, v = v1 - v2;

	// �������t�Ķ��κ�����Բ�ľ���ƽ����ȥ�뾶�͵�ƽ������ϵ��
	double a = v.square(), half_b = dot(r, v), c = r.square() - square(this->radius + other.radius);

	// �жϸö��κ����Ƿ�����㣬�������һ��������ײ����������ͬ�����Ϊ���ߣ���Ϊ����ײ
	double delta_4 = square(half_b) - a * c;
	if (delta_4 <= 0)
		return false;

	// ����ٽ�ʱ��
	double sqrt_delta_4 = sqrt(delta_4);
	double t_begin = (-half_b - sqrt_delta_4) / a, t_end = (-half_b + sqrt_delta_4) / a;

	// �����ײʱ�䵱ǰ֡����ǰ���ҷ���ʱ�䵱ǰ֡��ʼ����ᷢ����ײ������¼������ײʱ��
	if (t_begin <= delta_t && t_end > 0)
	{
		delta_t = t_begin > 0 ? t_begin * 0.99 : 0;
		return true;
	}

	// ����������ж�Ϊ����ײ
	return false;
}

bool CircleShapedCollider::dynamicCollisionDetection(RectShapedCollider &other, const MapPoint &p1, const MapPoint &p2, const MapVector &v1, const MapVector &v2, double &delta_t)
{
	// ������Ծ�ֹ���������
	if (v1 == v2)
	{
		bool collision = CircleShapedCollider::staticCollisionDetection(other, p1, p2);
		delta_t = collision ? 0 : delta_t;
		return collision;
	}

	// ��ȡ���ƴ���
	double t1 = delta_t, t2 = delta_t;
	RectShapedCollider tmp1(radius * 2, radius * 2);
	if (tmp1.RectShapedCollider::dynamicCollisionDetection(other, p1, p2, v1, v2, t1))
	{
		CircleShapedCollider tmp2(other.halfWH.length());
		if (tmp2.CircleShapedCollider::dynamicCollisionDetection(*this, p1, p2, v1, v2, t2))
		{
			delta_t = t1 > t2 ? t1 : t2;
			return true;
		}
	}
	return false;
}

bool RectShapedCollider::staticCollisionDetection(RectShapedCollider &other, const MapPoint &p1, const MapPoint &p2)
{
	ClosedRegion2D region1(p1 - this->halfWH, p1 + this->halfWH), region2(p2 - other.halfWH, p2 + other.halfWH);
	return intersect(region1, region2);
}

bool RectShapedCollider::dynamicCollisionDetection(RectShapedCollider &other, const MapPoint &p1, const MapPoint &p2, const MapVector &v1, const MapVector &v2, double &delta_t)
{
	// ������Ծ�ֹ���������
	if (v1 == v2)
	{
		bool collision = RectShapedCollider::staticCollisionDetection(other, p1, p2);
		delta_t = collision ? 0 : delta_t;
		return collision;
	}

	// �ֱ���������ε����½Ǻ����Ͻǵ�����
	MapPoint left1 = p1 - this->halfWH, right1 = p1 + this->halfWH, left2 = p2 - other.halfWH, right2 = p2 + other.halfWH;

	// ��������ٶ�
	MapVector v = v1 - v2;

	// �ֱ����x��y�������ཻ��ʱ��
	MapVector t1xy = (left2 - right1).times(v), t2xy = (right2 - left1).times(v);
	MapVector tminxy = vmin(t1xy, t2xy), tmaxxy = vmax(t1xy, t2xy);
	ClosedInterval ix(tminxy.x, tmaxxy.x), iy(tminxy.y, tmaxxy.y);

	// ����������޽������򲻻���ײ����v.x��v.yΪ0����ĳһά������Ч
	if (v.x == 0)
	{
		if (left1.x >= right2.x || left2.x >= right1.x)
			return false;
		else
			ix = ClosedInterval(-INFINITY, INFINITY);
	}
	else if (v.y == 0)
	{
		if (left1.y >= right2.y || left2.y >= right1.y)
			return false;
		else
			iy = ClosedInterval(-INFINITY, INFINITY);
	}
	else
	{
		if (!intersect(ix, iy))
			return false;
	}

	// ���������ϵĽ�����Ϊ��ײ��ʱ��
	double t_begin = ix.left > iy.left ? ix.left : iy.left;
	double t_end = ix.right < iy.right ? ix.right : iy.right;

	// �����ײʱ�䵱ǰ֡����ǰ���ҷ���ʱ�䵱ǰ֡��ʼ����ᷢ����ײ������¼������ײʱ��
	if (t_begin <= delta_t && t_end > 0)
	{
		delta_t = t_begin > 0 ? t_begin * 0.99 : 0;
		return true;
	}

	// ����������ж�Ϊ����ײ
	return false;
}
