﻿using Chess.UI.Models;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Chess.UI.Multiplayer
{
    public interface IMultiplayerModel
    {
        void ResetToInit();

        void StartGameServer();

        void StartGameClient();

        void StartMultiplerGame(MultiplayerMode mode);

        void SetLocalPlayerName(string playerName);

        int GetSelectedNetworkAdapterID();

        void ChangeNetworkAdapter(int adapterID);

        List<NetworkAdapter> GetNetworkAdapters();

        void ConnectToHost();

        void AcceptConnectionRequest();

        void RejectConnectionRequest();

        event Action<string> OnConnectionErrorOccured;

        event Action<Services.EngineAPI.ConnectionState> OnConnectionStatusChanged;

        event Action<string> OnClientRequestedConnection;
    }
}
