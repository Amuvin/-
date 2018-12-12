#include "stdafx.h"
#include "global_vars.h"
#include "GameDisplayer.h"
#include "MapWidget.h"
#include "DataManager.h"
#include "MapView.h"
#include "Enemy.h"

#define ROWS_PER_SCREEN 13.5
#define COLUMNS_PER_SCREEN 24.0
#define MAP_BLOCK_SIZE 100.0

GameDisplayer::GameDisplayer(DataReceiver *receiver)
	: dataReceiver(receiver), s2mt({ 1, -1 }, { 0, 0 })
	, pauseGame(false), bitmapPainted(false), actorCreated(false)
{
	name = u8"��ͼģ��";

	connect(this, &GameDisplayer::finishedPainting, mapWidget, &MapWidget::updateImage);
	connect(this, &GameDisplayer::actorDied, mapWidget, &MapWidget::actorDied);

	mapView = new MapView("../levels/view/2");

	ActorView::myActor = nullptr;

	defaultFPS = 60;
	emit started();
}

GameDisplayer::~GameDisplayer()
{
	waitAndQuit();
	for each (SpriteView *p in sprites)
		delete p;
	gameDisplayer = nullptr;
}

S2M_Transformation GameDisplayer::getScreenToMapTrans()
{
	s2mtMutex.lock();
	S2M_Transformation tmp = s2mt;
	s2mtMutex.unlock();
	return tmp;
}

void GameDisplayer::mainloop()
{
	// ���ղ����¾��������ͬʱ����¾�����������þ���
	dataReceiver->receiveSpriteState(sprites);

	// ��ȡ�������ڴ�С
	double width = mapWidget->width(), height = mapWidget->height();

	// �����ͣ���Ѿ��������򲻻�
	if (pauseGame)
	{
		if (bitmapPainted && lastWidth == width && lastHeight == height)
			return;
		bitmapPainted = true;
	}
	bitmapPainted = pauseGame;
	lastWidth = width;
	lastHeight = height;

	// �������û�������򲻻�
	if (ActorView::myActor == nullptr)
	{
		// ����������ˣ������ź�
		if (actorCreated)
		{
			pauseGame = true;
			emit actorDied();
		}
		return;
	}		
	actorCreated = true;

	// ����16:9�ı�������ͼƬ��С
	if (width * 9.0 > height * 16.0)
		width = height * (16.0 / 9.0);
	else if (width * 9.0 < height * 16.0)
		height = width * (9.0 / 16.0);
	
	// ��������任����
	double block_size = width / COLUMNS_PER_SCREEN;
	double stretch_x = block_size / MAP_BLOCK_SIZE, stretch_y = -stretch_x;
	double max_screen_x = mapView->getColumns() * block_size, max_screen_y = mapView->getRows() * block_size;
	double bias_x, bias_y;

	if (width >= max_screen_x)
		bias_x = (width - max_screen_x) * 0.5;
	else if (ActorView::myActor->centerPos.x * stretch_x <= width * 0.5)
		bias_x = 0;
	else if (ActorView::myActor->centerPos.x * stretch_x >= max_screen_x - width * 0.5)
		bias_x = width - max_screen_x;
	else
		bias_x = width * 0.5 - ActorView::myActor->centerPos.x * stretch_x;

	if (height >= max_screen_y)
		bias_y = (height - max_screen_y) * 0.5;
	else if (ActorView::myActor->centerPos.y * stretch_y < height * 0.5)
		bias_y = 0;
	else if (ActorView::myActor->centerPos.y * stretch_y >= max_screen_y - height * 0.5)
		bias_y = height - max_screen_y;
	else
		bias_y = height * 0.5 - ActorView::myActor->centerPos.y * stretch_y;

	// ��������任����
	M2S_Transformation m2st(stretch_x, stretch_y, bias_x, bias_y);
	setScreenToMapTrans(m2st.reverse());

	// ����ͼ��ͻ��Ҷ���
	imageMutex.lock();
	if (imageToShow.isNull() || imageToShow.width() != (int)width || imageToShow.height() != (int)height)
		imageToShow = QPixmap(width, height);
	imageToShow.fill(Qt::black);
	QPainter painter(&imageToShow);

	// ��ȡ���ָ��λ��
	ScreenPoint mouse_pos = mapWidget->getMousePos();

	// ����ͼ
	mapView->drawBody(painter, m2st(MapPoint::Zeros()), block_size, width, height);

	// ������
	for each (SpriteView *p in sprites)
		p->drawBody(painter, m2st);

	// ��Ѫ��
	for each (SpriteView *p in sprites)
		p->drawBloodBar(painter, m2st);
	ActorView::myActor->drawBloodBar(painter, width / 15, height / 15, width / 5, height / 5);
	SpriteView::DrawBloodBar(painter, QRect(width / 3, height / 50, width / 3, height / 30), EnemyView::bossHealth, EnemyView::bossMaxHealth, BOSS_BLOOD_BAR);

	// ������ɣ�֪ͨ�������ڸ���
	emit finishedPainting();
	imageMutex.unlock();
}

void GameDisplayer::setScreenToMapTrans(const S2M_Transformation &t)
{
	s2mtMutex.lock();
	s2mt = t;
	s2mtMutex.unlock();
}
