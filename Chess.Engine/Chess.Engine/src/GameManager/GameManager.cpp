/*
  ==============================================================================
	Module:         GameManager
	Description:    Manager for the Chess game
  ==============================================================================
*/

#include "GameManager.h"

#include "StateMachine.h"


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


bool GameManager::init()
{
	mLog.initLogging();
	mUserSettings.init();

	mUiCommunicationLayer = std::make_shared<UICommunication>();

	mChessBoard			  = std::make_shared<ChessBoard>();

	mMoveValidation		  = std::make_shared<MoveValidation>(mChessBoard);
	mMoveExecution		  = std::make_shared<MoveExecution>(mChessBoard, mMoveValidation);
	mMoveGeneration		  = std::make_shared<MoveGeneration>(mChessBoard, mMoveValidation, mMoveExecution);

	mWhitePlayer.setPlayerColor(PlayerColor::White);
	mBlackPlayer.setPlayerColor(PlayerColor::Black);

	initObservers();

	return true;
}


bool GameManager::startGame()
{
	clearState();

	mChessBoard->initializeBoard(); // Reset the board

	return true;
}


void GameManager::clearState()
{
	changeCurrentPlayer(PlayerColor::NoColor);

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
	mMovesGeneratedForCurrentTurn = false;			// Reset flag for the new turn

	if (getCurrentPlayer() == PlayerColor::NoColor) // We are in init state and set the first round's player : white
	{
		LOG_INFO("Since we setup the game now, we select the white player as the current player!");
		changeCurrentPlayer(PlayerColor::White);
		return;
	}

	if (getCurrentPlayer() == PlayerColor::White)
	{
		changeCurrentPlayer(PlayerColor::Black);
		LOG_INFO("Current player is {}", LoggingHelper::playerColourToString(getCurrentPlayer()).c_str());

		return;
	}

	changeCurrentPlayer(PlayerColor::White);

	LOG_INFO("Current player is {}", LoggingHelper::playerColourToString(getCurrentPlayer()).c_str());
}


bool GameManager::calculateAllMovesForPlayer()
{
	if (!mMovesGeneratedForCurrentTurn)
	{
		LOG_INFO("We start calculating this player's possible moves!");
		bool result					  = mMoveGeneration->calculateAllLegalBasicMoves(getCurrentPlayer());
		mMovesGeneratedForCurrentTurn = true;
		return result;
	}
	return false;
}


bool GameManager::initiateMove(const Position &startPosition)
{
	LOG_INFO("We started to initate a move with starting position {}", LoggingHelper::positionToString(startPosition).c_str());

	mAllMovesForPosition.clear();

	auto possibleMoves = mMoveGeneration->getMovesForPosition(startPosition);

	mAllMovesForPosition.reserve(possibleMoves.size());
	mAllMovesForPosition = possibleMoves;

	LOG_INFO("Number of possible moves for the current position is {}", mAllMovesForPosition.size());
	return true;
}


void GameManager::executeMove(PossibleMove &tmpMove)
{
	PossibleMove moveToExecute{};
	for (auto &move : mAllMovesForPosition)
	{
		if (move == tmpMove)
		{
			moveToExecute = move;

			// On pawn promotion, assign the promotion piece type
			if (moveToExecute.type == MoveType::PawnPromotion)
			{
				moveToExecute.promotionPiece = tmpMove.promotionPiece;
			}
			break;
		}
	}

	Move executedMove = mMoveExecution->executeMove(moveToExecute);

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
}


void GameManager::resetGame()
{
	mChessBoard->removeAllPiecesFromBoard();
	mChessBoard->initializeBoard();

	mMoveExecution->clearMoveHistory();

	mWhitePlayer.reset();
	mBlackPlayer.reset();

	clearState();
}


void GameManager::endGame(EndGameState state, PlayerColor player)
{
	for (auto &observer : mObservers)
	{
		auto obs = observer.lock();

		if (obs)
			obs->onEndGame(state, player);
	}
}


std::optional<PlayerColor> GameManager::getWinner() const
{
	return std::nullopt;
}


bool GameManager::checkForValidMoves(const PossibleMove &move)
{
	if (move.start == move.end)						// The user aborted the move by clicking the piece again
		return false;

	for (auto &possibleMove : mAllMovesForPosition) // Check if the move is a valid move
	{
		if (move == possibleMove)
			return true;
	}

	return false;
}


bool GameManager::checkForPawnPromotionMove(const PossibleMove &move)
{
	for (auto &possibleMove : mAllMovesForPosition)
	{
		if (move == possibleMove)
			return (possibleMove.type & MoveType::PawnPromotion) == MoveType::PawnPromotion;
	}
	return false;
}


std::vector<NetworkAdapter> GameManager::getNetworkAdapters()
{
	return mMultiplayerManager->getNetworkAdapters();
}


bool GameManager::changeCurrentNetworkAdapter(int ID)
{
	return mMultiplayerManager->changeCurrentNetworkAdapter(ID);
}


int GameManager::getCurrentNetworkAdapterID()
{
	return mMultiplayerManager->getCurrentNetworkAdapterID();
}


void GameManager::setLocalPlayerName(std::string name)
{
	mMultiplayerManager->setLocalPlayerName(name);
}


void GameManager::changeCurrentPlayer(PlayerColor player)
{
	if (mCurrentPlayer != player)
	{
		mCurrentPlayer = player;

		for (auto &observer : mObservers)
		{
			auto obs = observer.lock();

			if (obs)
				obs->onChangeCurrentPlayer(mCurrentPlayer);
		}
	}
}


PlayerColor GameManager::getCurrentPlayer() const
{
	return mCurrentPlayer;
}


EndGameState GameManager::checkForEndGameConditions()
{
	const Move *lastMove = mMoveExecution->getLastMove();

	if (lastMove)
	{
		bool isCheckMate = (lastMove->type & MoveType::Checkmate) == MoveType::Checkmate;
		if (isCheckMate)
		{
			LOG_INFO("Detected a Checkmate!");

			auto winner = getWinner();
			if (winner.has_value())
				endGame(EndGameState::Checkmate, winner.value());

			return EndGameState::Checkmate;
		}

		mMoveGeneration->calculateAllLegalBasicMoves(getCurrentPlayer()); // Calculate all legal moves to check if we have a stalemate (no valid moves left)
		bool isStaleMate = mMoveValidation->isStalemate(getCurrentPlayer());
		if (isStaleMate)
		{
			LOG_INFO("Detected a Stalemate");

			auto winner = getWinner();
			if (winner.has_value())
				endGame(EndGameState::StaleMate, winner.value());

			return EndGameState::StaleMate;
		}

		LOG_INFO("Game is still on-going. We switch player's turns!");
		return EndGameState::OnGoing;
	}

	LOG_WARNING("Couldn't find the last move! Game is still on-going");
	return EndGameState::OnGoing;
}


bool GameManager::startMultiplayerGame(bool isHost)
{
	mIsMultiplayerMode = true;
	mIsHost			   = isHost;

	// Initialize the game & board
	clearState();
	mChessBoard->initializeBoard();

	if (isHost)
	{
		changeCurrentPlayer(PlayerColor::White); // If we are the host, we are chosen to play as white -> TODO : Add UI selector in future
		mWhitePlayer.setIsLocalPlayer(true);
		mBlackPlayer.setIsLocalPlayer(false);
	}
	else
	{
		// Guests are black
		changeCurrentPlayer(PlayerColor::White); // Game still starts with white
		mWhitePlayer.setIsLocalPlayer(false);
		mBlackPlayer.setIsLocalPlayer(true);
	}

	// mMultiplayerManager->setInternalObservers(); // Set the internal multiplayer observers

	return true;
}



void GameManager::disconnectMultiplayerGame()
{
	if (mMultiplayerManager)
		mMultiplayerManager->disconnect();

	mIsMultiplayerMode = false;
	resetGame(); // Reset to single player
}


void GameManager::startedMultiplayer()
{
	// The player clicked on the Multiplayer menu button and started the multiplayer

	mMultiplayerManager = std::make_shared<MultiplayerManager>(); // Create the multiplayer manager

	mMultiplayerManager->setInternalObservers();				  // First set the observers, so
	mMultiplayerManager->init();								  // the modules already receive notifications on init
}


void GameManager::stoppedMultiplayer()
{
	mMultiplayerManager->reset();
}


bool GameManager::isMultiplayerActive() const
{
	return mIsMultiplayerMode;
}


bool GameManager::isLocalPlayerTurn()
{
	// If we're in single player mode, it is always our turn
	if (!isMultiplayerActive())
		return true;

	PlayerColor currentPlayer	   = getCurrentPlayer();

	bool		isWhitePlayerTurn  = currentPlayer == PlayerColor::White;
	bool		isBlackPlayerTurn  = currentPlayer == PlayerColor::Black;

	bool		isLocalPlayersTurn = (isWhitePlayerTurn && mWhitePlayer.isLocalPlayer() || isBlackPlayerTurn && mBlackPlayer.isLocalPlayer());
	return isLocalPlayersTurn;
}


void GameManager::startRemoteDiscovery(bool isHost)
{
	if (isHost)
	{
		LOG_INFO("Starting to host a session..");
		mMultiplayerManager->hostSession();
	}
	else
	{
		LOG_INFO("Starting to join a session..");
		mMultiplayerManager->startClientDiscovery();
	}
}


void GameManager::approveConnectionRequest()
{
	if (!mMultiplayerManager)
		return;

	mMultiplayerManager->approveConnectionRequest();
}


void GameManager::rejectConnectionRequest()
{
	if (!mMultiplayerManager)
		return;

	mMultiplayerManager->rejectConnectionRequest();
}


void GameManager::sendConnectionRequestToHost()
{
	if (!mMultiplayerManager)
		return;

	mMultiplayerManager->joinSession();
}


void GameManager::initObservers()
{
	this->attachObserver(mUiCommunicationLayer);

	mWhitePlayer.attachObserver(mUiCommunicationLayer);
	mBlackPlayer.attachObserver(mUiCommunicationLayer);

	mMoveExecution->attachObserver(mUiCommunicationLayer);

	StateMachine::GetInstance()->attachObserver(mUiCommunicationLayer);
}


void GameManager::initMultiplayerObservers()
{
	if (!mMultiplayerManager)
	{
		LOG_WARNING("Could not set up the observers, since the Multiplayer Manager is not set up yet!");
		return;
	}

	mMoveExecution->attachObserver(mMultiplayerManager->mRemoteSender);				   // Moves will be sent to the remote
	mMultiplayerManager->mRemoteReceiver->attachObserver(StateMachine::GetInstance()); // Received moves will be handled in the state machine
}


void GameManager::deinitObservers()
{
	this->detachObserver(mUiCommunicationLayer);

	mWhitePlayer.detachObserver(mUiCommunicationLayer);
	mBlackPlayer.detachObserver(mUiCommunicationLayer);

	mMoveExecution->detachObserver(mUiCommunicationLayer);

	StateMachine::GetInstance()->detachObserver(mUiCommunicationLayer);
}
