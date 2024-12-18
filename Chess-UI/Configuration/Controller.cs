﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using static Chess_UI.Configuration.ChessLogicAPI;

namespace Chess_UI.Configuration
{
    public class Controller
    {
        public Controller()
        {

        }

        static APIDelegate mDelegate = null;

        private void SetLogicAPIDelegate()
        {
            if (mDelegate == null)
            {
                mDelegate = new APIDelegate(DelegateHandler);
                SetDelegate(mDelegate);
            }


        }

        public void DelegateHandler(int message, nint data)
        {
            DelegateMessage delegateMessage = (DelegateMessage)message;
            switch (delegateMessage)
            {
                case DelegateMessage.PlayerHasWon:
                    {
                        HandleWinner(data);
                        break;
                    }

                case DelegateMessage.InitiateMove:
                    {
                        HandleInitiatedMove();
                        break;
                    }

                case DelegateMessage.PlayerScoreUpdate:
                    {
                        HandlePlayerScoreUpdate(data);
                        break;
                    }
                default: break;
            }
        }


        private void HandleWinner(nint data)
        {
            int player = Marshal.ReadInt32(data);
            PlayerColor winner = (PlayerColor)player;

            // set winner through event trigger
        }


        private void HandleInitiatedMove()
        {

        }


        private void HandlePlayerScoreUpdate(nint data)
        {
            Score score = (Score)Marshal.PtrToStructure(data, typeof(Score));

            PlayerColor player = score.player;
            int scoreValue = score.score;

            // trigger event for score change
        }

    }
}
