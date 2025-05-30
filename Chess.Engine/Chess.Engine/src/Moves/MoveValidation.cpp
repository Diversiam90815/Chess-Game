/*
  ==============================================================================
	Module:			MoveValidation
	Description:    Manages the validation of moves in chess
  ==============================================================================
*/


#include "MoveValidation.h"


MoveValidation::MoveValidation(std::shared_ptr<ChessBoard> board) : mChessBoard(board) {}


MoveValidation::~MoveValidation() {}


void MoveValidation::init(std::shared_ptr<ChessBoard> board)
{
	this->mChessBoard = board;
}


bool MoveValidation::validateMove(Move &move, PlayerColor playerColor)
{
	// 1. Verify the piece exists and belongs to the player
	auto &piece = mChessBoard->getPiece(move.startingPosition);
	if (!piece || piece->getColor() != playerColor)
		return false;

	// 2. Verify the move is valid for the piece type
	bool moveIsValid = false;
	auto validMoves	 = piece->getPossibleMoves(move.startingPosition, *mChessBoard);
	for (const auto &validMove : validMoves)
	{
		if (validMove.end == move.endingPosition)
		{
			moveIsValid = true;
			break;
		}
	}

	if (!moveIsValid)
		return false;

	// 3. Check if king would be in check after move
	if (wouldKingBeInCheckAfterMove(move, playerColor))
	{
		LOG_INFO("Move could not be validated, since the king would be in check after this move!");
		return false;
	}

	return true;
}


bool MoveValidation::isKingInCheck(const Position &ourKing, PlayerColor playerColor)
{
	PlayerColor opponentColor = playerColor == PlayerColor::White ? PlayerColor::Black : PlayerColor::White;
	return isSquareAttacked(ourKing, opponentColor);
}


bool MoveValidation::isCheckmate(PlayerColor player)
{
	Position kingPosition = mChessBoard->getKingsPosition(player);

	if (!isKingInCheck(kingPosition, player))
		return false; // If the king is not in check, it's not checkmate

	// If the king is in check, check if there are any moves that can escape check
	auto playerPieces = mChessBoard->getPiecesFromPlayer(player);
	for (const auto &[startPosition, piece] : playerPieces)
	{
		auto possibleMoves = piece->getPossibleMoves(startPosition, *mChessBoard);

		for (const auto &move : possibleMoves)
		{
			Move testMove(startPosition, move.end, piece->getType());

			if (!wouldKingBeInCheckAfterMove(testMove, player))
			{
				return false; // If there's a legal move that stops check, it's not checkmate
			}
		}
	}

	// No legal move can stop the check; it's checkmate
	return true;
}


bool MoveValidation::isStalemate(PlayerColor player)
{
	Position kingPosition = mChessBoard->getKingsPosition(player);
	if (isKingInCheck(kingPosition, player))
		return false;


	// Assume calculateAllLegalMoves() from MoveGeneration has been called before
	auto playerPieces = mChessBoard->getPiecesFromPlayer(player);
	for (const auto &[startPosition, piece] : playerPieces)
	{
		auto moves = piece->getPossibleMoves(startPosition, *mChessBoard);

		for (const auto &moveCandidate : moves)
		{
			Move testMove(startPosition, moveCandidate.end, piece->getType());
			if (!wouldKingBeInCheckAfterMove(testMove, player))
				return false;
		}
	}
	return true;
}


bool MoveValidation::wouldKingBeInCheckAfterMove(Move &move, PlayerColor playerColor)
{
	bool		kingInCheck	   = false;

	// Make a local copy of the board
	ChessBoard	boardCopy	   = *mChessBoard; // copies the chessboard to a local copy

	// Save the current state
	auto	   &movingPiece	   = boardCopy.getPiece(move.startingPosition);
	auto	   &capturingPiece = boardCopy.getPiece(move.endingPosition); // If there is no piece being captured in this move, this will be nullptr
	bool		isKing		   = movingPiece->getType() == PieceType::King;
	PlayerColor opponentColour = playerColor == PlayerColor::White ? PlayerColor::Black : PlayerColor::White;

	LOG_DEBUG("Simulating move: {} -> {} with piece {}", LoggingHelper::positionToString(move.startingPosition).c_str(),
			  LoggingHelper::positionToString(move.endingPosition).c_str(), LoggingHelper::pieceTypeToString(movingPiece->getType()).c_str());

	if (capturingPiece)
	{
		auto type = capturingPiece->getType();
		boardCopy.removePiece(move.endingPosition);
		LOG_DEBUG("After placing, occupant of endSquare = {}", LoggingHelper::pieceTypeToString(type).c_str());
	}

	// Simulate the move
	boardCopy.movePiece(move.startingPosition, move.endingPosition);

	// Update King's position if it's the king
	Position kingPosition;
	if (isKing)
	{
		kingPosition = move.endingPosition;
		boardCopy.updateKingsPosition(kingPosition, playerColor); // Ensure board copy knows where the king is
	}
	else
	{
		kingPosition = boardCopy.getKingsPosition(playerColor);
	}

	// Check if King is under attack (isSquareUnderAttack)
	PlayerColor opponentColor = (playerColor == PlayerColor::White) ? PlayerColor::Black : PlayerColor::White;
	kingInCheck				  = isSquareAttacked(kingPosition, opponentColor, boardCopy);

	LOG_DEBUG("King is at {}", LoggingHelper::positionToString(kingPosition).c_str());
	LOG_DEBUG("isSquareAttacked(...) = {}", kingInCheck ? "true" : "false");

	return kingInCheck;
}


bool MoveValidation::isSquareAttacked(const Position &square, PlayerColor attackerColor)
{
	// Iterate over all opponent pieces
	auto opponentPieces = mChessBoard->getPiecesFromPlayer(attackerColor);

	for (const auto &[pos, piece] : opponentPieces)
	{
		// Get possible moves for the opponent's piece
		auto moves = piece->getPossibleMoves(pos, *mChessBoard, true); // Attack only since we need to differentiate attack and just move moves from pawns

		for (const auto &move : moves)
		{
			if (move.end == square)
			{
				return true;
			}
		}
	}

	return false;
}


bool MoveValidation::isSquareAttacked(const Position &square, PlayerColor attackerColor, ChessBoard &chessboard)
{
	// Iterate over all opponent pieces
	auto opponentPieces = chessboard.getPiecesFromPlayer(attackerColor);

	for (const auto &[pos, piece] : opponentPieces)
	{
		// Get possible moves for the opponent's piece
		auto moves = piece->getPossibleMoves(pos, chessboard, true); // Attack only since we need to differentiate attack and just move moves from pawns

		for (const auto &move : moves)
		{
			if (move.end == square)
			{
				LOG_DEBUG("Square ({}, {}) is attacked by {} at ({}, {})", square.x, square.y, LoggingHelper::pieceTypeToString(piece->getType()).c_str(), pos.x, pos.y);
				return true;
			}
		}
	}

	return false;
}
