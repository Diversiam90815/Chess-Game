/*
  ==============================================================================

	Class:          GameManager

	Description:    Manager for the Chess game

  ==============================================================================
*/


#pragma once
#include "MovementManager.h"
#include "Player.h"


class GameManager
{
public:
	GameManager();
	~GameManager();

	void init();

	void switchTurns();

private:

	Player whitePlayer;
	Player blackPlayer;
};