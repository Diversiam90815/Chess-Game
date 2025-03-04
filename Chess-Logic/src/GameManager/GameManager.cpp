/*
  ==============================================================================
	Module:         GameManager
	Description:    Manager for the Chess game
  ==============================================================================
*/

#include "GameManager.h"


GameManager::GameManager() {}


GameManager::~GameManager()
{
	deinitObservers();
}


GameManager *GameManager::GetInstance()
{
	static GameManager *sInstance = nullptr;
	if (nullptr == sInstance)
	{
		sInstance = new GameManager();
	}
	return sInstance;
}


void GameManager::ReleaseInstance()
{
	GameManager *sInstance = GetInstance();
	if (sInstance)
	{
		delete sInstance;
	}
}


void GameManager::init()
{
	mLog.initLogging();
	mUserSettings.init();

	mUiCommunicationLayer = std::make_unique<UICommunication>();

	mChessBoard			  = std::make_shared<ChessBoard>();

	mMoveValidation		  = std::make_shared<MoveValidation>(mChessBoard);
	mMoveExecution		  = std::make_shared<MoveExecution>(mChessBoard, mMoveValidation);
	mMoveGeneration		  = std::make_shared<MoveGeneration>(mChessBoard, mMoveValidation, mMoveExecution);

	mWhitePlayer.setPlayerColor(PlayerColor::White);
	mBlackPlayer.setPlayerColor(PlayerColor::Black);

	initObservers();
}


void GameManager::startGame()
{
	clearState();

	mChessBoard->initializeBoard();			 // Reset the board
	changeCurrentPlayer(PlayerColor::White); // Setting the player at the end, since this will trigger the move calculation
}


void GameManager::clearState()
{
	changeCurrentPlayer(PlayerColor::NoColor);

	gameStateChanged(GameState::Init);
	setCurrentMoveState(MoveState::NoMove);
	mAllMovesForPosition.clear();
	mMovesGeneratedForCurrentTurn = false; // Reset flag
}


void GameManager::setDelegate(PFN_CALLBACK pDelegate)
{
	mUiCommunicationLayer->setDelegate(pDelegate);
}


PieceType GameManager::getCurrentPieceTypeAtPosition(const Position position)
{
	if (!mChessBoard)
		return PieceType::DefaultType;

	auto &chessPiece = mChessBoard->getPiece(position);

	if (chessPiece)
	{
		return chessPiece->getType();
	}
	return PieceType::DefaultType;
}


std::vector<PossibleMove> GameManager::getPossibleMoveForPosition()
{
	return mAllMovesForPosition;
}


bool GameManager::getBoardState(int boardState[BOARD_SIZE][BOARD_SIZE])
{
	if (!mChessBoard)
		return false;

	for (int y = 0; y < BOARD_SIZE; ++y)
	{
		for (int x = 0; x < BOARD_SIZE; ++x)
		{
			Position pos	  = {x, y};
			auto	&piece	  = mChessBoard->getPiece(pos);

			int		 colorVal = 0; // 0 = no color
			int		 typeVal  = 0; // 0 = PieceType::DefaultType

			if (piece)
			{
				colorVal = static_cast<int>(piece->getColor()) & 0xF;
				typeVal	 = static_cast<int>(piece->getType()) & 0xF;
			}

			// Pack color in high nibble, type in low nibble:
			// bits [4..7] = color, bits [0..3] = piece type
			int encoded		 = (colorVal << 4) | (typeVal & 0xF);

			boardState[y][x] = encoded;
		}
	}
	return true;
}


void GameManager::switchTurns()
{
	setCurrentMoveState(MoveState::NoMove);
	mMovesGeneratedForCurrentTurn = false; // Reset flag for the new turn

	if (getCurrentPlayer() == PlayerColor::White)
	{
		changeCurrentPlayer(PlayerColor::Black);
		LOG_INFO("Current player is {}", LoggingHelper::playerColourToString(getCurrentPlayer()).c_str());

		return;
	}

	changeCurrentPlayer(PlayerColor::White);

	LOG_INFO("Current player is {}", LoggingHelper::playerColourToString(getCurrentPlayer()).c_str());
}


void GameManager::executeMove(PossibleMove &move)
{
	Move executedMove = mMoveExecution->executeMove(move);

	LoggingHelper::logMove(executedMove);

	if (executedMove.capturedPiece != PieceType::DefaultType)
	{
		if (getCurrentPlayer() == PlayerColor::White)
		{
			mWhitePlayer.addCapturedPiece(executedMove.capturedPiece);
			mWhitePlayer.updateScore();
		}
		else
		{
			mBlackPlayer.addCapturedPiece(executedMove.capturedPiece);
			mBlackPlayer.updateScore();
		}
	}

	checkForEndGameConditions();
}


void GameManager::undoMove()
{
	const Move *lastMove = mMoveExecution->getLastMove();

	if (!lastMove)
	{
		LOG_WARNING("No moves found to undo!");
		return;
	}

	mChessBoard->movePiece(lastMove->endingPosition, lastMove->startingPosition);

	if (lastMove->capturedPiece != PieceType::DefaultType)
	{
		PlayerColor capturedColor  = (lastMove->player == PlayerColor::White) ? PlayerColor::Black : PlayerColor::White;
		auto		pieceToRestore = ChessPiece::CreatePiece(lastMove->capturedPiece, capturedColor);

		if (pieceToRestore)
		{
			mChessBoard->setPiece(lastMove->endingPosition, pieceToRestore);
		}

		if (lastMove->player == PlayerColor::White)
		{
			mWhitePlayer.removeLastCapturedPiece();
		}
		else if (lastMove->player == PlayerColor::Black)
		{
			mBlackPlayer.removeLastCapturedPiece();
		}
	}

	auto &piece = mChessBoard->getPiece(lastMove->startingPosition);
	piece->decreaseMoveCounter();

	mMoveExecution->removeLastMove();
	switchTurns();
}


void GameManager::gameStateChanged(GameState state)
{
	if (mCurrentState == state)
		return;

	mCurrentState = state;

	for (auto observer : mObservers)
	{
		if (observer)
		{
			observer->onGameStateChanged(state);
		}
	}
}


GameState GameManager::getCurrentGameState() const
{
	return mCurrentState;
}


void GameManager::setCurrentMoveState(MoveState state)
{
	if (mCurrentMoveState != state)
	{
		mCurrentMoveState = state;
	}
}


MoveState GameManager::getCurrentMoveState() const
{
	return mCurrentMoveState;
}


void GameManager::resetGame()
{
	mChessBoard->removeAllPiecesFromBoard();
	mChessBoard->initializeBoard();

	mWhitePlayer.reset();
	mBlackPlayer.reset();

	clearState();
}


void GameManager::endGame(PlayerColor player)
{
	for (auto observer : mObservers)
	{
		if (observer)
		{
			observer->onEndGame(player);
		}
	}
}


std::optional<PlayerColor> GameManager::getWinner() const
{
	if (mCurrentState == GameState::Checkmate)
		return getCurrentPlayer() == PlayerColor::White ? PlayerColor::White : PlayerColor::Black;

	else if (mCurrentState == GameState::Stalemate)
		return std::nullopt; // Draw in case of stalemate

	return std::nullopt;
}


void GameManager::handleMoveStateChanges(PossibleMove &move)
{
	switch (mCurrentMoveState)
	{
	case (MoveState::NoMove):
	{
		if (!mMovesGeneratedForCurrentTurn)
		{
			LOG_INFO("Move State is NoMove -> We start calculating this player's possible moves!");
			mMoveGeneration->calculateAllLegalBasicMoves(getCurrentPlayer());
			mMovesGeneratedForCurrentTurn = true;
		}
		break;
	}

	case (MoveState::InitiateMove):
	{
		LOG_INFO("We started to initate a move with starting position {}", LoggingHelper::positionToString(move.start).c_str());

		mAllMovesForPosition.clear();

		auto possibleMoves = mMoveGeneration->getMovesForPosition(move.start);

		mAllMovesForPosition.reserve(possibleMoves.size());
		mAllMovesForPosition = possibleMoves;

		moveStateInitiated(); // Let the UI know the moves for current round are ready -> handling need to be refactored later!

		LOG_INFO("Number of possible moves for the current position is {}", mAllMovesForPosition.size());

		break;
	}

	case (MoveState::ExecuteMove):
	{
		LOG_INFO("Executing the move now!");
		executeMove(move);
		break;
	}
	default: break;
	}
}


void GameManager::moveStateInitiated()
{
	for (auto observer : mObservers)
	{
		if (observer)
		{
			observer->onMoveStateInitiated();
		}
	}
}


void GameManager::changeCurrentPlayer(PlayerColor player)
{
	if (mCurrentPlayer != player)
	{
		mCurrentPlayer = player;

		for (auto observer : mObservers)
		{
			if (observer)
			{
				observer->onChangeCurrentPlayer(mCurrentPlayer);
			}
		}
	}
}


PlayerColor GameManager::getCurrentPlayer() const
{
	return mCurrentPlayer;
}



void GameManager::checkForEndGameConditions()
{
	const Move *lastMove = mMoveExecution->getLastMove();

	if (lastMove)
	{
		bool isCheckMate = (lastMove->type & MoveType::Checkmate) == MoveType::Checkmate;
		if (isCheckMate)
		{
			LOG_INFO("Detected a Checkmate!");
			gameStateChanged(GameState::Checkmate);

			auto winner = getWinner();
			if (winner.has_value())
				endGame(winner.value());

			return;
		}

		mMoveGeneration->calculateAllLegalBasicMoves(getCurrentPlayer()); // Calculate all legal moves to check if we have a stalemate (no valid moves left)
		bool isStaleMate = mMoveValidation->isStalemate(getCurrentPlayer());
		if (isStaleMate)
		{
			LOG_INFO("Detected a Stalemate");
			gameStateChanged(GameState::Stalemate);

			auto winner = getWinner();
			if (winner.has_value())
				endGame(winner.value());

			return;
		}

		LOG_INFO("Game is still on-going. We switch player's turns!");
		gameStateChanged(GameState::OnGoing);
		switchTurns();
		return;
	}

	LOG_WARNING("Couldn't find the last move! Game is still on-going");
	gameStateChanged(GameState::OnGoing);
}


void GameManager::initObservers()
{
	this->attachObserver(mUiCommunicationLayer.get());

	mWhitePlayer.attachObserver(mUiCommunicationLayer.get());
	mBlackPlayer.attachObserver(mUiCommunicationLayer.get());

	mMoveExecution->attachObserver(mUiCommunicationLayer.get());
}


void GameManager::deinitObservers()
{
	this->detachObserver(mUiCommunicationLayer.get());

	mWhitePlayer.detachObserver(mUiCommunicationLayer.get());
	mBlackPlayer.detachObserver(mUiCommunicationLayer.get());

	mMoveExecution->detachObserver(mUiCommunicationLayer.get());
}


void GameManager::attachObserver(IGameObserver *observer)
{
	mObservers.push_back(observer);
}


void GameManager::detachObserver(IGameObserver *observer)
{
	mObservers.erase(std::remove(mObservers.begin(), mObservers.end(), observer), mObservers.end());
}
