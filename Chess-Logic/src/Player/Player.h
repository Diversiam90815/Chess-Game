/*
  ==============================================================================

	Class:          Player

	Description:    Stats and settings for the player

  ==============================================================================
*/


#pragma once

#include "ChessPiece.h"
#include "Parameters.h"
#include "Logging.h"

#include "ChessLogicAPIDefines.h"


struct Score
{
	Score(PlayerColor player, int value) : player(player), score(value){};

	PlayerColor player = PlayerColor::NoColor;
	int			score  = 0;

	bool		operator==(Score other)
	{
		return this->score == other.score && this->player == other.player;
	}
};


class Player
{
public:
	Player();

	Player(PlayerColor color);

	~Player();


	bool		  isOnTurn() const;
	void		  setOnTurn(bool value);

	Score		  getScore() const;
	void		  setScore(int value);

	PlayerColor	  getPlayerColor() const;
	void		  setPlayerColor(PlayerColor value);


	void		  addCapturedPiece(const PieceType piece);

	void		  updateScore();

	constexpr int getPieceValue(PieceType piece);

	void		  reset();

	void		  setDelegate(PFN_CALLBACK pDelegate);

private:
	PlayerColor			   mPlayerColor = PlayerColor::NoColor;

	std::vector<PieceType> mCapturedPieces;

	bool				   mIsCurrentTurn = false;

	Score				   mScore		  = Score(PlayerColor::NoColor, 0);

	PFN_CALLBACK		   mDelegate	  = nullptr;
};
