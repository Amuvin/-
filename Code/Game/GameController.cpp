#include "stdafx.h"
#include "global_vars.h"
#include "GameController.h"
#include "GameWindow.h"
#include "PhysicsEngine.h"
#include "GameDisplayer.h"

GameController::GameController()
{
	// ���״̬ת�ƺ�ת���¼�
	QObject::connect(mainMenuState.addTransition(chooseWidget, &ChooseWidget::singlePlayerClicked, &singlePlayerState),
		&QSignalTransition::triggered, gameWindow, &GameWindow::singlePlayerGame);
	mainMenuState.addTransition(chooseWidget, &ChooseWidget::multiplePlayerClicked, &createOrJoinState);

	createOrJoinState.addTransition(chooseWidget, &ChooseWidget::backToMainMenuClicked, &mainMenuState);
	QObject::connect(createOrJoinState.addTransition(chooseWidget, &ChooseWidget::createRoomClicked, &waitingForJoinState),
		&QSignalTransition::triggered, chooseWidget, &ChooseWidget::createRoom);
	createOrJoinState.addTransition(chooseWidget, &ChooseWidget::joinRoomClicked, &inputIPState);

	QObject::connect(waitingForJoinState.addTransition(chooseWidget, &ChooseWidget::closeRoomClicked, &createOrJoinState),
		&QSignalTransition::triggered, chooseWidget, &ChooseWidget::closeRoom);

	QObject::connect(waitingForJoinState.addTransition(chooseWidget, &ChooseWidget::hostStartClicked, &multiplePlayerState),
		&QSignalTransition::triggered, []() {
		chooseWidget->startGameServer();
		gameWindow->multiplePlayerGame();
	});

	inputIPState.addTransition(chooseWidget, &ChooseWidget::connectIPClicked, &connectIPState);
	inputIPState.addTransition(chooseWidget, &ChooseWidget::cancelSeekingClicked, &createOrJoinState);

	connectIPState.addTransition(chooseWidget, &ChooseWidget::connectSucceeded, &waitingForStartState);
	QObject::connect(connectIPState.addTransition(chooseWidget, &ChooseWidget::cancelSeekingClicked, &inputIPState),
		&QSignalTransition::triggered, chooseWidget, &ChooseWidget::cancelSeeking);
	QObject::connect(connectIPState.addTransition(chooseWidget, &ChooseWidget::connectFailed, &inputIPState),
		&QSignalTransition::triggered, chooseWidget, &ChooseWidget::notFoundRoom);

	QObject::connect(waitingForStartState.addTransition(chooseWidget, &ChooseWidget::hostStart, &multiplePlayerState),
		&QSignalTransition::triggered, []() {
		chooseWidget->startGameClient();
		gameWindow->multiplePlayerGame();
	});
	QObject::connect(waitingForStartState.addTransition(chooseWidget, &ChooseWidget::hostLeave, &inputIPState),
		&QSignalTransition::triggered, chooseWidget, &ChooseWidget::leaveRoomHost);
	QObject::connect(waitingForStartState.addTransition(chooseWidget, &ChooseWidget::leaveRoomClicked, &inputIPState),
		&QSignalTransition::triggered, chooseWidget, &ChooseWidget::leaveRoomClient);

	singlePlayerState.addTransition(mapWidget, &MapWidget::pauseGame, &pauseGameState);
	singlePlayerState.addTransition(mapWidget, &MapWidget::quitGame, &mainMenuState);
	singlePlayerState.addTransition(mapWidget, &MapWidget::actorDied, &gameOverState);

	pauseGameState.addTransition(mapWidget, &MapWidget::pauseGame, &singlePlayerState);
	pauseGameState.addTransition(mapWidget, &MapWidget::quitGame, &gameOverState);

	multiplePlayerState.addTransition(mapWidget, &MapWidget::quitGame, &mainMenuState);
	QObject::connect(multiplePlayerState.addTransition(mapWidget, &MapWidget::hostQuit, &mainMenuState),
		&QSignalTransition::triggered, []() { QMessageBox::information(mapWidget, u8"��Ϣ", u8"������ϻ����ر�������"); });
	multiplePlayerState.addTransition(mapWidget, &MapWidget::actorDied, &gameOverState);

	gameOverState.addTransition(mapWidget, &MapWidget::quitGame, &mainMenuState);

	// �༭״̬�����˳��¼�
	QObject::connect(&mainMenuState, &QState::entered, []() {
		chooseWidget->setStackWidgetIndex(INDEX_MAIN_MENU);
		gameWindow->setWindowTitle(u8"��ʼ����");
		gameWindow->setStackWidgetIndex(INDEX_CHOOSE_WINDOW);
		gameWindow->resetGame();
	});
	QObject::connect(&createOrJoinState, &QState::entered, []() {
		chooseWidget->setStackWidgetIndex(INDEX_CREATE_JOIN);
	});
	QObject::connect(&waitingForJoinState, &QState::entered, []() {
		chooseWidget->setStackWidgetIndex(INDEX_HOST_WAIT);
	});
	QObject::connect(&inputIPState, &QState::entered, []() {
		chooseWidget->setStackWidgetIndex(INDEX_CLIENT_CONNECT);
		chooseWidget->readyToInputIP();
	});
	QObject::connect(&connectIPState, &QState::entered, chooseWidget, &ChooseWidget::seekRoom);
	QObject::connect(&waitingForStartState, &QState::entered, []() {
		chooseWidget->setStackWidgetIndex(INDEX_CLIENT_WAIT);
		chooseWidget->joinedRoom();
	});
	QObject::connect(&pauseGameState, &QState::entered, []() {
		physicsEngine->pauseGame = true;
		gameDisplayer->pauseGame = true;
		gameWindow->setWindowTitle(u8"������Ϸ-����ͣ");
	});
	QObject::connect(&pauseGameState, &QState::exited, []() {
		physicsEngine->pauseGame = false;
		gameDisplayer->pauseGame = false;
		gameWindow->setWindowTitle(u8"������Ϸ");
	});
	QObject::connect(&gameOverState, &QState::entered, mapWidget, &MapWidget::gameOver);
	QObject::connect(&gameOverState, &QState::exited, mapWidget, &MapWidget::resetGame);

	// �����ж���õ�״̬����״̬��
	stateMachine.addState(&mainMenuState);
	stateMachine.addState(&singlePlayerState);
	stateMachine.addState(&createOrJoinState);
	stateMachine.addState(&waitingForJoinState);
	stateMachine.addState(&inputIPState);
	stateMachine.addState(&connectIPState);
	stateMachine.addState(&waitingForStartState);
	stateMachine.addState(&multiplePlayerState);
	stateMachine.addState(&pauseGameState);
	stateMachine.addState(&gameOverState);

	// �����˵�Ϊ��ʼ״̬������״̬��
	stateMachine.setInitialState(&mainMenuState);
	stateMachine.start();
}

void GameController::stop()
{
	stateMachine.stop();
}
