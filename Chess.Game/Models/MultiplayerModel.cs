using Chess.UI.Services;
using System;
using static Chess.UI.Services.EngineAPI;


namespace Chess.UI.Models
{
    public interface IMultiplayerModel
    {
        void StartMultiplayer();

        void StopMultiplayer();

        void FindOpponent();

        void ConnectToOpponent(int index);

        void RespondToConnectionRequest(bool accept);

        void CancelSearch();

        void SetLocalPlayerColor(EngineAPI.Side color);

        void SetPlayerReady(bool ready);

        void HandleLocalPlayerChosenByRemote(Side local);


        event Action<string> OnConnectionErrorOccured;

        event Action<MultiplayerState, string> OnMultiplayerStateChanged;

        event Action<string> OnOpponentDiscovered;

        event Action<Side> OnPlayerChanged;

        event Action<Side> OnMultiplayerPlayerSetFromRemote;
    }


    public class MultiplayerModel : IMultiplayerModel
    {
        private readonly ICommunicationLayer _backendCommunication;


        public MultiplayerModel(ICommunicationLayer backendCommunication)
        {
            _backendCommunication = backendCommunication;
            _backendCommunication.MultiplayerStateChanged += HandleMultiplayerStateUpdates;
            _backendCommunication.OpponentDiscoveredEvent += HandleOpponentDiscovered;
            _backendCommunication.PlayerChanged += HandlePlayerChanged;
            _backendCommunication.MultiPlayerChosenByRemote += HandleLocalPlayerChosenByRemote;
        }


        public void StartMultiplayer()
        {
            EngineAPI.StartMultiplayer();
        }


        public void StopMultiplayer()
        {
            EngineAPI.StopMultiplayer();
        }


        public void FindOpponent()
        {
            EngineAPI.FindOpponent();
        }


        public void ConnectToOpponent(int index)
        {
            EngineAPI.ConnectToOpponent(index);
        }


        public void RespondToConnectionRequest(bool accept)
        {
            EngineAPI.RespondToConnectionRequest(accept);
        }


        public void CancelSearch()
        {
            EngineAPI.StopMultiplayer();
        }


        public void SetLocalPlayerColor(EngineAPI.Side color)
        {
            EngineAPI.SetLocalPlayer((int)color);
        }


        public void SetPlayerReady(bool flag)
        {
            EngineAPI.SetLocalPlayerReady(flag);
        }


        public void HandlePlayerChanged(Side player)
        {
            OnPlayerChanged?.Invoke(player);
        }


        public void HandleLocalPlayerChosenByRemote(Side local)
        {
            OnMultiplayerPlayerSetFromRemote?.Invoke(local);
        }


        private void HandleMultiplayerStateUpdates(MultiplayerStateEvent stateEvent)
        {
            MultiplayerState state = stateEvent.State;

            if (state == MultiplayerState.Error)
            {
                OnConnectionErrorOccured?.Invoke(stateEvent.errorMessage);
                return;
            }

            OnMultiplayerStateChanged?.Invoke(state, stateEvent.remoteName);
        }


        private void HandleOpponentDiscovered(MultiplayerStateEvent opponentEvent)
        {
            OnOpponentDiscovered?.Invoke(opponentEvent.remoteName);
        }


        public event Action<string> OnConnectionErrorOccured;
        public event Action<MultiplayerState, string> OnMultiplayerStateChanged;
        public event Action<string> OnOpponentDiscovered;
        public event Action<Side> OnPlayerChanged;
        public event Action<Side> OnMultiplayerPlayerSetFromRemote;
    }
}
