/*
  ==============================================================================
	Module:			IObserver
	Description:    Interface observer class used by the Observer Pattern
  ==============================================================================
*/

#pragma once

#include <json.hpp>

#include "Move.h"
#include "Parameters.h"
#include "NetworkAdapter.h"
#include "Multiplayer/RemoteMessaging/MultiplayerMessageStruct.h"
#include "Discovery/DiscoveryEndpoint.h"

class IPlayerObserver
{
public:
	virtual ~IPlayerObserver() {};

	virtual void onScoreUpdate(PlayerColor player, int value)					   = 0;
	virtual void onAddCapturedPiece(PlayerColor player, PieceType captured)		   = 0;
	virtual void onRemoveLastCapturedPiece(PlayerColor player, PieceType captured) = 0;
};


class IMoveObserver
{
public:
	virtual ~IMoveObserver() {};

	virtual void onExecuteMove(const PossibleMove &move) = 0;
	virtual void onAddToMoveHistory(Move &move)			 = 0;
	virtual void onClearMoveHistory()					 = 0;
};


class IGameObserver
{
public:
	virtual ~IGameObserver() {};

	virtual void onEndGame(EndGameState state, PlayerColor winner) = 0;
	virtual void onChangeCurrentPlayer(PlayerColor player)		   = 0;
};


class IGameStateObserver
{
public:
	virtual ~IGameStateObserver() {};

	virtual void onGameStateChanged(GameState state) = 0;
};


class IRemoteReceiverObserver
{
public:
	virtual ~IRemoteReceiverObserver() {};

	virtual void onMessageReceived(MultiplayerMessageType type, std::vector<uint8_t> &message) = 0;
};


class INetworkObserver
{
public:
	virtual ~INetworkObserver() {};

	virtual void onNetworkAdapterChanged(const NetworkAdapter &adapter) = 0;
};


class IRemoteMessagesObserver
{
public:
	virtual ~IRemoteMessagesObserver() {};

	virtual void onRemoteMoveReceived(const PossibleMove &remoteMove)	= 0;
	virtual void onRemoteChatMessageReceived(const std::string &mesage) = 0;
};


class IRemoteSenderObserver
{
public:
	virtual ~IRemoteSenderObserver() {};

	virtual void onSendMessage(MultiplayerMessageType type, std::vector<uint8_t> &message) = 0;
};


class IDiscoveryObserver
{
public:
	virtual ~IDiscoveryObserver() {};

	virtual void onRemoteFound(const Endpoint &remote) = 0;
	// virtual void onRemoteSelected(const std::string &remoteName) = 0;
	// virtual void onRemoteRemoved(const std::string &remoteName)	 = 0;
};


class IConnectionStatusObserver
{
public:
	virtual ~IConnectionStatusObserver() {};

	virtual void onConnectionStateChanged(ConnectionState state, const std::string &errorMessage = "") = 0;
	virtual void onPendingHostApproval(const std::string &remotePlayerName)							   = 0;
};
