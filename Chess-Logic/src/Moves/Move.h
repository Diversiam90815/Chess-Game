/*
  ==============================================================================

	Class:          Move

	Description:    Storing information classifying a move in chess

  ==============================================================================
*/


#pragma once

#include <string>
#include <functional>

#include "Parameters.h"
#include "MoveType.h"



struct Position
{
	int	 x = 0;
	int	 y = 0;

	bool operator==(const Position &other) const
	{
		return x == other.x && y == other.y;
	}
};

// Hash method so we can use it in unordered_map
namespace std
{
template <>
struct hash<Position>
{
	std::size_t operator()(const Position &pos) const noexcept
	{
		std::size_t h1 = std::hash<int>{}(pos.x);
		std::size_t h2 = std::hash<int>{}(pos.y);
		return h1 ^ (h2 << 1);
	}
};
} // namespace std



struct PossibleMove
{
	Position start;
	Position end;
	MoveType type = MoveType::Normal;

	bool	 operator==(const PossibleMove &other) const
	{
		return this->start == other.start && this->end == other.end;
	}
};


class Move
{
public:
	Move();

	Move(PossibleMove &possibleMove);

	Move(
		Position start, Position end, PieceType moved, PieceType captured = PieceType::DefaultType, MoveType type = MoveType::Normal, PieceType promotion = PieceType::DefaultType);

	Position	startingPosition;
	Position	endingPosition;

	PieceType	movedPiece	  = PieceType::DefaultType;
	PieceType	capturedPiece = PieceType::DefaultType;
	PieceType	promotionType = PieceType::DefaultType;
	PlayerColor player		  = PlayerColor::NoColor;

	MoveType	type		  = MoveType::Normal;

	std::string notation	  = "";

	int			number		  = 0; // Storing the current number of this move. Each move saved should increment this number!

	int			halfMoveClock = 0; // Incrememted with every move that is not a capture or pawn move (detect draw if halfMoveClock is 100)


	bool		operator<(const Move &other) const
	{
		return this->number < other.number;
	}

	bool operator==(const Move &other) const
	{
		return this->number == other.number;
	}
};