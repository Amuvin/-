#pragma once

#include <immintrin.h>

// ��Ŀ��������Ϊ��ͼ����ϵ����Ļ����ϵ����ֹ���õ��´������
enum GameVectorType
{
	MAP_VECTOR, SCREEN_VECTOR
};

// ����������͵���Ĵ洢�ṹ��ͨ�ýӿ�
class alignas(16) GameVectorBase
{
protected:
	GameVectorBase() {}

public:
	void setXY(double x, double y)
	{
		packed() = _mm_unpacklo_pd(_mm_load_sd(&x), _mm_load_sd(&y));
	}

	QString toString() const
	{
		return QString("(%1, %2)").arg(x).arg(y);
	}

	bool isZero() const
	{
		return x == 0.0 && y == 0.0;
	}

	// ���ظõ���������ĸ����ޣ������ԭ������������򷵻�0
	int quadrant() const
	{
		if (x > 0.0)
		{
			if (y > 0.0)
				return 1;
			else if (y < 0.0)
				return 2;
		}
		else if (x < 0.0)
		{
			if (y > 0.0)
				return 4;
			else if (y < 0.0)
				return 3;
		}
		return 0;
	}

	__m128d &packed() { return *(__m128d*)&x; }
	const __m128d &packed() const { return *(__m128d*)&x; }

public:
	double x, y;
};

// �����࣬���ڱ�ʾλ�ơ��ٶȡ����ٶȵȣ���������ʾ������
// MapVector��ʹ�õ�ͼ����ϵ��ScreenVector��ʹ����Ļ����ϵ����ʾ����������MapPoint��ScreenPoint
template<GameVectorType T>
class alignas(16) GameVector : public GameVectorBase
{
public:
	GameVector() {} // �����ʼ��Ϊ������

	GameVector(__m128d src)
	{
		packed() = src;
	}

	GameVector(double x, double y)
	{
		setXY(x, y);
	}

	GameVector(const GameVector &src)
	{
		packed() = src.packed();
	}

	// ����������
	static GameVector Zeros()
	{
		return { 0.0, 0.0 };
	}

	GameVector &operator=(const GameVector &src)
	{
		packed() = src.packed();
		return *this;
	}

	GameVector &operator+=(const GameVector &other)
	{
		packed() = _mm_add_pd(packed(), other.packed());
		return *this;
	}

	GameVector &operator-=(const GameVector &other)
	{
		packed() = _mm_sub_pd(packed(), other.packed());
		return *this;
	}

	GameVector &operator*=(double k)
	{
		packed() = _mm_mul_pd(packed(), _mm_broadcastsd_pd(_mm_load_sd(&k)));
		return *this;
	}

	GameVector &operator/=(double k)
	{
		packed() = _mm_div_pd(packed(), _mm_broadcastsd_pd(_mm_load_sd(&k)));
		return *this;
	}

	// ���������任�������������Ӧ����ˣ����ı�����
	GameVector stretch(const GameVector &other) const
	{
		return _mm_mul_pd(packed(), other.packed());
	}

	// ���ظ�������x��y����һ����x��y�Ķ��ٱ�������Ӧ����������ı�����
	GameVector times(const GameVector &other) const
	{
		return _mm_div_pd(packed(), other.packed());
	}

	// ģ����ƽ��
	double square() const
	{
		double result;
		__m128d tmp = _mm_mul_pd(packed(), packed());
		_mm_store_sd(&result, _mm_hadd_pd(tmp, tmp));
		return result;
	}

	// ��ģ��
	double length() const
	{
		double result;
		__m128d tmp = _mm_mul_pd(packed(), packed());
		_mm_store_sd(&result, _mm_sqrt_pd(_mm_hadd_pd(tmp, tmp)));
		return result;
	}

	// �����������ͬ����ĵ�λ���������ı�����
	GameVector normalize() const
	{
		__m128d tmp = _mm_mul_pd(packed(), packed());
		return _mm_div_pd(packed(), _mm_sqrt_pd(_mm_hadd_pd(tmp, tmp)));
	}

	// ���ش���������һ���������ϵ�ͶӰ�����ı�����
	double projection(const GameVector &other) const
	{
		double result;
		__m128d t1 = _mm_mul_pd(this->packed(), other.packed());
		__m128d t2 = _mm_mul_pd(other.packed(), other.packed());
		_mm_store_sd(&result, _mm_div_pd(_mm_hadd_pd(t1, t1), _mm_sqrt_pd(_mm_hadd_pd(t2, t2))));
		return result;
	}
};

template<GameVectorType T>
inline GameVector<T> operator+(const GameVector<T> &a, const GameVector<T> &b)
{
	return _mm_add_pd(a.packed(), b.packed());
}

template<GameVectorType T>
inline GameVector<T> operator-(const GameVector<T> &a, const GameVector<T> &b)
{
	return _mm_sub_pd(a.packed(), b.packed());
}

template<GameVectorType T>
inline GameVector<T> operator-(const GameVector<T> &a)
{
	return _mm_sub_pd({ 0.0, 0.0 }, a.packed());
}

template<GameVectorType T>
inline GameVector<T> operator*(const GameVector<T> &a, double k)
{
	return _mm_mul_pd(a.packed(), _mm_broadcastsd_pd(_mm_load_sd(&k)));
}

template<GameVectorType T>
inline GameVector<T> operator*(double k, const GameVector<T> &a)
{
	return a * k;
}

template<GameVectorType T>
inline GameVector<T> operator/(const GameVector<T> &a, double k)
{
	return _mm_div_pd(a.packed(), _mm_broadcastsd_pd(_mm_load_sd(&k)));
}

// �ֱ��x��y�����ֵ
template<GameVectorType T>
inline GameVector<T> vabs(const GameVector<T> &a)
{
	const quint64 mask[2] = { 0x7fffffffffffffff, 0x7fffffffffffffff };
	return _mm_and_pd(*(__m128d*)mask, a.packed());
}

// �ֱ��x��y�����ֵ���෴��
template<GameVectorType T>
inline GameVector<T> vnabs(const GameVector<T> &a)
{
	const quint64 mask[2] = { 0x8000000000000000, 0x8000000000000000 };
	return _mm_or_pd(*(__m128d*)mask, a.packed());
}

// �ֱ��ÿһάȡ���ֵ
template<GameVectorType T>
inline GameVector<T> vmax(const GameVector<T> &a, const GameVector<T> &b)
{
	return _mm_max_pd(a.packed(), b.packed());
}

// �ֱ��ÿһάȡС��ֵ
template<GameVectorType T>
inline GameVector<T> vmin(const GameVector<T> &a, const GameVector<T> &b)
{
	return _mm_min_pd(a.packed(), b.packed());
}

template<GameVectorType T>
inline bool operator==(const GameVector<T> &a, const GameVector<T> &b)
{
	return a.x == b.x && a.y == b.y;
}

template<GameVectorType T>
inline bool operator!=(const GameVector<T> &a, const GameVector<T> &b)
{
	return a.x != b.x || a.y != b.y;
}

// ���������ڻ�
template<GameVectorType T>
double dot(const GameVector<T> &a, const GameVector<T> &b)
{
	double result;
	__m128d tmp = _mm_mul_pd(a.packed(), b.packed());
	_mm_store_sd(&result, _mm_hadd_pd(tmp, tmp));
	return result;
}

// ���������н�����
template<GameVectorType T>
double cos_angle(const GameVector<T> &a, const GameVector<T> &b)
{
	return dot(a, b) / sqrt(a.square() * b.square());
}

// �����������
template<GameVectorType T>
double cross(const GameVector<T> &a, const GameVector<T> &b)
{
	return a.x * b.y - a.y * b.x;
}

// ���������н�����
template<GameVectorType T>
double sin_angle(const GameVector<T> &a, const GameVector<T> &b)
{
	return cross(a, b) / sqrt(a.square() * b.square());
}

// �������࣬ר��������ʾ�����꣬��������ӿ�������
template<GameVectorType T>
class alignas(16) GamePoint : public GameVectorBase
{
public:
	GamePoint() {} // �����ʼ��Ϊԭ��

	GamePoint(__m128d src)
	{
		packed() = src;
	}

	GamePoint(double x, double y)
	{
		packed() = _mm_unpacklo_pd(_mm_load_sd(&x), _mm_load_sd(&y));
	}

	GamePoint(const GamePoint &src)
	{
		packed() = src.packed();
	}

	// ����ԭ��
	static GamePoint Zeros()
	{
		return { 0.0, 0.0 };
	}

	QPointF toQPointF() const
	{
		return QPointF(x, y);
	}

	QPoint toQPoint() const
	{
		return QPoint(x, y);
	}

	GamePoint &operator=(const GamePoint &src)
	{
		packed() = src.packed();
		return *this;
	}

	GamePoint &operator+=(const GameVector<T> &offset)
	{
		packed() = _mm_add_pd(packed(), offset.packed());
		return *this;
	}

	GamePoint &operator-=(const GameVector<T> &offset)
	{
		packed() = _mm_sub_pd(packed(), offset.packed());
		return *this;
	}
};

template<GameVectorType T>
inline GamePoint<T> operator+(const GamePoint<T> &point, const GameVector<T> &offset)
{
	return _mm_add_pd(point.packed(), offset.packed());
}

template<GameVectorType T>
inline GamePoint<T> operator-(const GamePoint<T> &point, const GameVector<T> &offset)
{
	return _mm_sub_pd(point.packed(), offset.packed());
}

template<GameVectorType T>
inline GameVector<T> operator-(const GamePoint<T> &a, const GamePoint<T> &b)
{
	return _mm_sub_pd(a.packed(), b.packed());
}

template<GameVectorType T>
inline bool operator==(const GamePoint<T> &a, const GamePoint<T> &b)
{
	return a.x == b.x && a.y == b.y;
}

template<GameVectorType T>
inline bool operator!=(const GamePoint<T> &a, const GamePoint<T> &b)
{
	return a.x != b.x || a.y != b.y;
}

// �������ľ����ƽ��
template<GameVectorType T>
inline double distance_square(const GamePoint<T> &a, const GamePoint<T> &b)
{
	return (a - b).square();
}

// �������ľ���
template<GameVectorType T>
inline double distance(const GamePoint<T> &a, const GamePoint<T> &b)
{
	return (a - b).length();
}

// ��������е�
template<GameVectorType T>
inline GamePoint<T> midpoint(const GamePoint<T> &a, const GamePoint<T> &b)
{
	return _mm_mul_pd(_mm_add_pd(a.packed(), b.packed()), { 0.5, 0.5 });
}

// �󿿽���a�ĵ�i��n�ȷֵ�
template<GameVectorType T>
inline GamePoint<T> n_equal_point(const GamePoint<T> &a, const GamePoint<T> &b, qint64 i, qint64 n)
{
	double k = (double)i / (double)n;
	return _mm_add_pd(a.packed(), _mm_mul_pd(_mm_sub_pd(b.packed(), a.packed()), _mm_broadcastsd_pd(_mm_load_sd(&k))));
}

typedef GameVector<MAP_VECTOR> MapVector;
typedef GameVector<SCREEN_VECTOR> ScreenVector;
typedef GamePoint<MAP_VECTOR> MapPoint;
typedef GamePoint<SCREEN_VECTOR> ScreenPoint;

class alignas(16) GameMatrix
{
public:
	GameMatrix() {}

	GameMatrix(const __m256d &src)
	{
		packed() = src;
	}

	GameMatrix(const GameMatrix &src)
	{
		packed() = src.packed();
	}

	GameMatrix(double xx, double xy, double yx, double yy)
		: xx(xx), xy(xy), yx(yx), yy(yy) {}

	// ���������
	static GameMatrix Zeros()
	{
		return { 0.0, 0.0, 0.0, 0.0 };
	}

	// ���ص�λ����
	static GameMatrix Unit()
	{
		return { 1.0, 0.0, 0.0, 1.0 };
	}

	// ����������ƴ�ɾ���
	static GameMatrix RowStack(const GameVectorBase &rx, const GameVectorBase &ry)
	{
		return GameMatrix(rx.x, rx.y, ry.x, ry.y);
	}

	// ����������ƴ�ɾ���
	static GameMatrix ColumnStack(const GameVectorBase &cx, const GameVectorBase &cy)
	{
		return GameMatrix(cx.x, cy.x, cx.y, cy.y);
	}

	// ������ת�任����
	static GameMatrix Rotate(double angle)
	{
		double sin_angle = sin(angle), cos_angle = cos(angle);
		return GameMatrix(cos_angle, -sin_angle, sin_angle, cos_angle);
	}

	GameMatrix &operator=(const GameMatrix &src)
	{
		packed() = src.packed();
		return *this;
	}
	GameMatrix &operator+=(const GameMatrix &other)
	{
		packed() = _mm256_add_pd(packed(), other.packed());
		return *this;
	}

	GameMatrix &operator-=(const GameMatrix &other)
	{
		packed() = _mm256_sub_pd(packed(), other.packed());
		return *this;
	}

	GameMatrix &operator*=(double k)
	{
		packed() = _mm256_mul_pd(packed(), _mm256_broadcastsd_pd(_mm_load_sd(&k)));
		return *this;
	}

	GameMatrix &operator/=(double k)
	{
		packed() = _mm256_div_pd(packed(), _mm256_broadcastsd_pd(_mm_load_sd(&k)));
		return *this;
	}

	// ���ظþ����ת�ã����ı�����
	GameMatrix T() const
	{
		return GameMatrix(xx, yx, xy, yy);
	}

	// ��þ��������ʽ
	double det() const
	{
		return xx * yy - xy * yx;
	}

	// ���ظþ���İ�����󣬲��ı�����
	GameMatrix adjoint() const
	{
		return GameMatrix(yy, -xy, -yx, xx);
	}

	// ���ظþ��������󣬲��ı�����
	GameMatrix reverse() const
	{
		double k = det();
		return _mm256_div_pd({ yy, -xy, -yx, xx }, _mm256_broadcastsd_pd(_mm_load_sd(&k)));
	}

	__m256d &packed() { return *(__m256d*)&xx; }
	const __m256d &packed() const { return *(__m256d*)&xx; }

public:
	double xx, xy, yx, yy;
};

inline GameMatrix operator+(const GameMatrix &A, const GameMatrix &B)
{
	return _mm256_add_pd(A.packed(), B.packed());
}

inline GameMatrix operator-(const GameMatrix &A, const GameMatrix &B)
{
	return _mm256_sub_pd(A.packed(), B.packed());
}

inline GameMatrix operator-(const GameMatrix &A)
{
	return _mm256_sub_pd({ 0.0, 0.0, 0.0, 0.0 }, A.packed());
}

inline GameMatrix operator*(const GameMatrix &A, double k)
{
	return _mm256_mul_pd(A.packed(), _mm256_broadcastsd_pd(_mm_load_sd(&k)));
}

inline GameMatrix operator*(double k, const GameMatrix &A)
{
	return A * k;
}

inline __m128d operator*(const GameMatrix &A, __m128d b)
{
	return _mm_hadd_pd(_mm_mul_pd(*(__m128d*)&A.xx, b), _mm_mul_pd(*(__m128d*)&A.yx, b));
}

template<GameVectorType T>
inline GameVector<T> operator*(const GameMatrix &A, const GameVector<T> &b)
{
	return A * b.packed();
}

inline GameMatrix operator*(const GameMatrix &A, const GameMatrix &B)
{
	__m256d AxxBxx_AxyByx_AyxBxx_AyyByx = _mm256_mul_pd(A.packed(), { B.xx, B.yx, B.xx, B.yx });
	__m256d AxxBxy_AxyByy_AyxBxy_AyyByy = _mm256_mul_pd(A.packed(), { B.xy, B.yy, B.xy, B.yy });
	return _mm256_hadd_pd(AxxBxx_AxyByx_AyxBxx_AyyByx, AxxBxy_AxyByy_AyxBxy_AyyByy);
}

inline GameMatrix operator/(const GameMatrix &A, double k)
{
	return _mm256_div_pd(A.packed(), _mm256_broadcastsd_pd(_mm_load_sd(&k)));
}

inline bool operator==(const GameMatrix &A, const GameMatrix &B)
{
	return A.xx == B.xx && A.xy == B.xy && A.yx == B.yx && A.yy == B.yy;
}

inline bool operator!=(const GameMatrix &A, const GameMatrix &B)
{
	return A.xx != B.xx || A.xy != B.xy || A.yx != B.yx || A.yy != B.yy;
}

template<GameVectorType ST, GameVectorType DT>
class alignas(16) CoordinateTransformation
{
public:
	CoordinateTransformation(__m128d stretch_v, __m128d bias_v)
		: stretch_v(stretch_v), bias_v(bias_v) {}
	CoordinateTransformation(const GameVectorBase &stretch_v, const GameVectorBase &bias_v)
		: stretch_v(stretch_v.packed()), bias_v(bias_v.packed()) {}
	CoordinateTransformation(double stretch_x, double stretch_y, double bias_x, double bias_y)
		: stretch_v{ stretch_x, stretch_y }, bias_v{ bias_x, bias_y } {}

	// �����任��ֻ������ƽ��
	GameVector<DT> operator()(const GameVector<ST> &src) const
	{
		return _mm_mul_pd(src.packed(), stretch_v);
	}

	// ��任����������ƽ��
	GamePoint<DT> operator()(const GamePoint<ST> &src) const
	{
		return _mm_add_pd(_mm_mul_pd(src.packed(), stretch_v), bias_v);
	}

	// ����������任�����ı�����
	CoordinateTransformation<DT, ST> reverse() const
	{
		__m128d k_k = _mm_div_pd({ 1.0, 1.0 }, stretch_v);
		__m128d b_b = _mm_mul_pd(k_k, _mm_sub_pd({ 0.0, 0.0 }, bias_v));
		return CoordinateTransformation<DT, ST>(k_k, b_b);
	}

private:
	__m128d stretch_v;
	__m128d bias_v;
};

typedef CoordinateTransformation<MAP_VECTOR, SCREEN_VECTOR> M2S_Transformation;
typedef CoordinateTransformation<SCREEN_VECTOR, MAP_VECTOR> S2M_Transformation;

// ���Ա任
class alignas(16) LinearTransformation
{
public:
	LinearTransformation(const GameMatrix &A) : A(A) {}

	// ������ת�任����
	static LinearTransformation Rotate(double angle)
	{
		return GameMatrix::Rotate(angle);
	}

	template<GameVectorType T>
	GameVector<T> operator()(const GameVector<T> &src) const
	{
		return A * src;
	}

	LinearTransformation reverse() const
	{
		return A.reverse();
	}

public:
	GameMatrix A;
};

// �����Ƚ������Ա任f�ٽ������Ա任g�õ��ĸ��ϱ任
inline LinearTransformation compound(const LinearTransformation &f, const LinearTransformation &g)
{
	return LinearTransformation(g.A * f.A);
}

// ����任
class alignas(16) AffineTransformation
{
public:
	AffineTransformation(const GameMatrix &A, __m128d b) : A(A), b(b) {}

	// ������ĳ�����ת�任����
	template<GameVectorType T>
	static AffineTransformation Rotate(const GamePoint<T> &p0, double angle)
	{
		AffineTransformation result(GameMatrix::Rotate(angle), p0.packed());
		result.b = (p0 - result.A * GameVector<T>(p0.packed())).packed();
		return result;
	}

	template<GameVectorType T>
	GamePoint<T> operator()(const GamePoint<T> &src) const
	{
		return (A * src + GameVector<T>(b)).packed();
	}

	AffineTransformation reverse() const
	{
		AffineTransformation result(A.reverse(), b);
		result.b = (result.A * -MapVector(b)).packed();
		return result;
	}

public:
	GameMatrix A;
	__m128d b;
};

// �����Ƚ��з���任f�ٽ������Ա任g�õ��ĸ��ϱ任
inline AffineTransformation compound(const AffineTransformation &f, const LinearTransformation &g)
{
	return AffineTransformation(g.A * f.A, g.A * f.b);
}

// �����Ƚ������Ա任f�ٽ��з���任g�õ��ĸ��ϱ任
inline AffineTransformation compound(const LinearTransformation &f, const AffineTransformation &g)
{
	return AffineTransformation(g.A * f.A, g.b);
}

// �����Ƚ��б任f�ٽ��з���任g�õ��ĸ��ϱ任
inline AffineTransformation compound(const AffineTransformation &f, const AffineTransformation &g)
{
	return AffineTransformation(g.A * f.A, _mm_add_pd(g.A * f.b, f.b));
}
