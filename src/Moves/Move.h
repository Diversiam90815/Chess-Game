/*
  ==============================================================================

	Class:          Move

	Description:    Storing information classifying a move in chess

  ==============================================================================
*/


#pragma once
#include "ChessPiece.h"

struct Move
{
	Move(
		int fx, int fy, int tx, int ty, PieceType moved, PieceType captured = PieceType::None, bool castling = false, bool enPassant = false, PieceType promotion = PieceType::None)
		: fromX(fx), fromY(fy), toX(tx), toY(ty), movedPiece(moved), capturedPiece(captured), isCastling(castling), isEnPassant(enPassant), promotionType(promotion)
	{
	}

	int		  fromX;
	int		  fromY;
	int		  toX;
	int		  toY;

	PieceType movedPiece;
	PieceType capturedPiece;
	PieceType promotionType;

	bool	  isCastling;
	bool	  isEnPassant;
};