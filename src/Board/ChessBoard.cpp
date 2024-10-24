/*
  ==============================================================================

	Class:          ChessBoard

	Description:    Class charecterizing a virtual chess board

  ==============================================================================
*/


#include "ChessBoard.h"
#include "ChessPiece.h"


ChessBoard::ChessBoard()
{
	squares.resize(8);
	for (int y = 0; y < 8; ++y)
	{
		for (int x = 0; x < 8; ++x)
		{
			squares[y].emplace_back(x, y);
		}
	}
}


ChessBoard::~ChessBoard()
{
}


Square & ChessBoard::getSquare(int x, int y)
{
	return squares[y][x];
}


void ChessBoard::setPiece(int x, int y, std::shared_ptr<ChessPiece> piece)
{
	squares[y][x].piece = piece;
}



std::shared_ptr<ChessPiece> ChessBoard::getPiece(int x, int y)
{
	return squares[y][y].piece;
}


void ChessBoard::removePiece(int x, int y)
{
	squares[y][x].piece = nullptr;
}


bool ChessBoard::movePiece(int fromX, int fromY, int toX, int toY)
{
	auto piece = getPiece(fromX, fromY);
	if (!piece)
		return false;

	removePiece(fromX, fromY);
	setPiece(toX, toY, piece);
	piece->setHasMoved(true);
	return true;
}


bool ChessBoard::isEmpty(int x, int y) const
{
	return squares[y][x].piece == nullptr;
}


const Move *ChessBoard::getLastMove()
{
	if (moveHistory.empty())
		return nullptr;
	return &moveHistory.back();
}


void ChessBoard::addMoveToHistory(const Move &move)
{
	moveHistory.push_back(move);
}


void ChessBoard::initializeBoard()
{
	// Place pieces for White
	setPiece(0, 0, std::make_shared<Rook>(PieceColor::White));
	setPiece(1, 0, std::make_shared<Knight>(PieceColor::White));
	setPiece(2, 0, std::make_shared<Bishop>(PieceColor::White));
	setPiece(3, 0, std::make_shared<Queen>(PieceColor::White));
	setPiece(4, 0, std::make_shared<King>(PieceColor::White));
	setPiece(5, 0, std::make_shared<Bishop>(PieceColor::White));
	setPiece(6, 0, std::make_shared<Knight>(PieceColor::White));
	setPiece(7, 0, std::make_shared<Rook>(PieceColor::White));
	for (int x = 0; x < 8; ++x)
	{
		setPiece(x, 1, std::make_shared<Pawn>(PieceColor::White));
	}

	// Place pieces for Black
	setPiece(0, 7, std::make_shared<Rook>(PieceColor::Black));
	setPiece(1, 7, std::make_shared<Knight>(PieceColor::Black));
	setPiece(2, 7, std::make_shared<Bishop>(PieceColor::Black));
	setPiece(3, 7, std::make_shared<Queen>(PieceColor::Black));
	setPiece(4, 7, std::make_shared<King>(PieceColor::Black));
	setPiece(5, 7, std::make_shared<Bishop>(PieceColor::Black));
	setPiece(6, 7, std::make_shared<Knight>(PieceColor::Black));
	setPiece(7, 7, std::make_shared<Rook>(PieceColor::Black));
	for (int x = 0; x < 8; ++x)
	{
		setPiece(x, 6, std::make_shared<Pawn>(PieceColor::Black));
	}
}
