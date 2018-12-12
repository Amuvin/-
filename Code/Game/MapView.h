#pragma once

// ��ͼ��ͼ�࣬���ڻ��Ƶ�ͼ
// ��ͼ����ϵy�����ϣ���Ļ����ϵy�����£�x�ᶼ����
class MapView
{
public:
	MapView(QString source_file_name);

	static void LoadResource();

	void drawBody(QPainter &painter, const ScreenPoint &map_zero_on_screen, double block_size, double screen_width, double screen_height);

	int getRows() const { return rows; }
	int getColumns() const { return columns; }

private:
	static QVector<QImage> sourcePictures;
	static QVector<QPixmap> scaledPictures;

	int rows, columns;
	QVector<quint8> data;	
};
