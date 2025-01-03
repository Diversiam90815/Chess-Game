﻿using Chess_UI.Configuration;
using Microsoft.UI.Dispatching;
using Microsoft.UI.Xaml.Media;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using static Chess_UI.Configuration.Images;
using static Chess_UI.Configuration.ChessLogicAPI;
using System.Collections.ObjectModel;
using System;
using Microsoft.UI.Composition.Interactions;


namespace Chess_UI.ViewModels
{
    public class ChessBoardViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private readonly DispatcherQueue DispatcherQueue;

        private const int MovesMaxColumns = 3;

        private Controller Controller;

        public ObservableCollection<ObservableCollection<string>> MoveHistoryColumns { get; } = [];

        public ObservableCollection<BoardSquare> Board { get; set; }



        public ChessBoardViewModel(DispatcherQueue dispatcherQueue, Controller controller)
        {
            this.DispatcherQueue = dispatcherQueue;
            this.Controller = controller;

            Controller.ExecutedMove += HandleExecutedMove;

            Board = new ObservableCollection<BoardSquare>();

            for (int i = 0; i < BOARD_SIZE * BOARD_SIZE; i++)
            {
                Board.Add(new(dispatcherQueue));
            }

            for (int i = 0; i < MovesMaxColumns; i++)
            {
                MoveHistoryColumns.Add([]);
            }

            LoadBoardFromNative();
        }


        public void LoadBoardFromNative()
        {
            var boardState = Controller.GetBoardStateFromNative();

            for (int i = 0; i < boardState.Length; i++)
            {
                int encoded = boardState[i];

                // Decode color and piece
                int colorVal = (encoded >> 4) & 0xF;    // top 8 bits
                int pieceVal = encoded & 0xF;          // bottom 8 bits

                // Compute x,y from the index
                //int x = i % BOARD_SIZE;
                //int y = i / BOARD_SIZE;

                int rowFromTop = i / 8;
                int col = i % 8;
                int rowFromBottom = 7 - rowFromTop;

                var square = new BoardSquare(
                    col,
                    rowFromBottom,
                    (PieceTypeInstance)pieceVal,
                    (PlayerColor)colorVal,
                    DispatcherQueue
                );

                Board[i] = square;
            }
        }


        public void AddMove(string move)
        {
            // Find the column with the least number of moves
            var minColumn = MoveHistoryColumns.OrderBy(col => col.Count).First();
            minColumn.Add(move);
        }


        public void ClearMoveHistory()
        {
            MoveHistoryColumns.Clear();
            MoveHistoryColumns.Add([]);
        }


        public void HandleSquareClick(BoardSquare square)
        {
            // The BoardSquare stores the UI-based (col, rowFromBottom).
            // We need to convert that back to engine coords, i.e. rowFromTop = 7 - rowFromBottom.
            int engineX = square.pos.x;
            int engineY = 7 - square.pos.y;  // flip it back

            Logger.LogInfo(string.Format("Square X{0}-Y{1} clicked!", square.pos.x, square.pos.y));

            switch (CurrentMoveState)
            {
                // The user is picking the start of a move
                case MoveState.NoMove:
                    {
                        Logger.LogInfo("Move State is NoMove and we start to initialize the move now!");

                        if (square.piece == PieceTypeInstance.DefaultType)
                            return;

                        CurrentPossibleMove = new PossibleMoveInstance
                        {
                            start = new PositionInstance(engineX, engineY)
                        };
                        CurrentMoveState = MoveState.InitiateMove;

                        ChessLogicAPI.HandleMoveStateChanged(CurrentPossibleMove.GetValueOrDefault());

                        // The engine will calculate possible moves 
                        // and eventually call back "delegateMessage::initiateMove"

                        break;
                    }

                // The user is picking the end of a move
                case MoveState.InitiateMove:
                    {
                        Logger.LogInfo("Move has already been initiated, and we start validating the move now!");

                        if (CurrentPossibleMove != null)
                        {
                            var move = CurrentPossibleMove.Value;
                            move.end = new PositionInstance(engineX, engineY);
                            move.type = CheckForMoveType();
                            CurrentPossibleMove = move;

                            if (CheckForValidMove())
                            {
                                Logger.LogInfo("The move has been validated, so we start the execution now!");

                                CurrentMoveState = MoveState.ExecuteMove;
                                ChessLogicAPI.HandleMoveStateChanged(CurrentPossibleMove.GetValueOrDefault());
                            }
                            else
                            {
                                Logger.LogWarning("Since the move could not been validated, we reset the move now!");

                                CurrentMoveState = MoveState.NoMove;
                                CurrentPossibleMove = null;
                                HandleMoveStateChanged(CurrentPossibleMove.GetValueOrDefault());
                            }

                            // The engine executes the move, calls delegate "moveExecuted",
                            // We'll get that event in the Controller.
                        }
                        else
                        {
                            Logger.LogError("CurrentPossible move is null!");
                        }
                        break;
                    }

                default:
                    // Possibly check for other states or do nothing
                    break;
            }
        }


        private bool CheckForValidMove()
        {
            if (!CurrentPossibleMove.HasValue)
            {
                Logger.LogError("CurrentPossibleMove.HasValue has returned false!");
                return false;
            }

            var move = CurrentPossibleMove.Value;

            // Check first if the move was aborted by selecting the same square again
            if (move.start == move.end)
            {
                Logger.LogInfo("Move has been cancelled since Start and End are the same square. The user has thus terminated the move!");
                return false;
            }

            // Check if it is a possible move
            foreach (var possibleMoves in Controller.PossibleMoves)
            {
                if (move == possibleMoves)
                {
                    Logger.LogInfo("The move seems to be valid!");
                    return true;
                }
            }
            Logger.LogWarning("The move could not be found within the PossibleMoves");
            Logger.LogWarning(string.Format("Move is from Start X{0}-Y{1} to End X{2}-Y{3}", move.start.x.ToString(), move.start.y.ToString(), move.end.x.ToString(), move.end.y.ToString()));
            return false;
        }


        private MoveTypeInstance CheckForMoveType()
        {
            return MoveTypeInstance.MoveType_Normal;

        }


        private void HandleExecutedMove(string moveNotation)
        {
            AddMove(moveNotation);
            LoadBoardFromNative();
            CurrentMoveState = MoveState.NoMove;
        }


        #region Current Move

        private MoveState currentMoveState = MoveState.NoMove;
        public MoveState CurrentMoveState
        {
            get => currentMoveState;
            set
            {
                if (value != currentMoveState)
                {
                    currentMoveState = value;
                    int state = (int)currentMoveState;
                    ChessLogicAPI.ChangeMoveState(state);
                }
            }
        }

        private PossibleMoveInstance? currentPossibleMove = null;
        public PossibleMoveInstance? CurrentPossibleMove
        {
            get => currentPossibleMove;
            set
            {
                currentPossibleMove = value;
            }
        }

        #endregion


        private ImageSource boardBackgroundImage = GetImage(BoardBackground.Wood);
        public ImageSource BoardBackgroundImage
        {
            get => boardBackgroundImage;
            set
            {
                if (boardBackgroundImage != value)
                {
                    boardBackgroundImage = value;
                    OnPropertyChanged();
                }
            }
        }


        #region Captured Pieces

        #region Images Captured Pieces

        private ImageSource capturedWhitePawnImage = GetCapturedPieceImage(PlayerColor.White, PieceTypeInstance.Pawn);
        public ImageSource CapturedWhitePawnImage
        {
            get => capturedWhitePawnImage;
            set
            {
                if (capturedWhitePawnImage != value)
                {
                    capturedWhitePawnImage = value;
                    OnPropertyChanged();
                }
            }
        }

        private ImageSource capturedWhiteBishopImage = GetCapturedPieceImage(PlayerColor.White, PieceTypeInstance.Bishop);
        public ImageSource CapturedWhiteBishopImage
        {
            get => capturedWhiteBishopImage;
            set
            {
                if (capturedWhiteBishopImage != value)
                {
                    capturedWhiteBishopImage = value;
                    OnPropertyChanged();
                }
            }
        }
        private ImageSource capturedWhiteRookImage = GetCapturedPieceImage(PlayerColor.White, PieceTypeInstance.Rook);
        public ImageSource CapturedWhiteRookImage
        {
            get => capturedWhiteRookImage;
            set
            {
                if (capturedWhiteRookImage != value)
                {
                    capturedWhiteRookImage = value;
                    OnPropertyChanged();
                }
            }
        }
        private ImageSource capturedWhiteQueenImage = GetCapturedPieceImage(PlayerColor.White, PieceTypeInstance.Queen);
        public ImageSource CapturedWhiteQueenImage
        {
            get => capturedWhiteQueenImage;
            set
            {
                if (capturedWhiteQueenImage != value)
                {
                    capturedWhiteQueenImage = value;
                    OnPropertyChanged();
                }
            }
        }
        private ImageSource capturedWhiteKnightImage = GetCapturedPieceImage(PlayerColor.White, PieceTypeInstance.Knight);
        public ImageSource CapturedWhiteKnightImage
        {
            get => capturedWhiteKnightImage;
            set
            {
                if (capturedWhiteKnightImage != value)
                {
                    capturedWhiteKnightImage = value;
                    OnPropertyChanged();
                }
            }
        }




        private ImageSource capturedBlackPawnImage = GetCapturedPieceImage(PlayerColor.Black, PieceTypeInstance.Pawn);
        public ImageSource CapturedBlackPawnImage
        {
            get => capturedBlackPawnImage;
            set
            {
                if (capturedBlackPawnImage != value)
                {
                    capturedBlackPawnImage = value;
                    OnPropertyChanged();
                }
            }
        }

        private ImageSource capturedBlackBishopImage = GetCapturedPieceImage(PlayerColor.Black, PieceTypeInstance.Bishop);
        public ImageSource CapturedBlackBishopImage
        {
            get => capturedBlackBishopImage;
            set
            {
                if (capturedBlackBishopImage != value)
                {
                    capturedBlackBishopImage = value;
                    OnPropertyChanged();
                }
            }
        }
        private ImageSource capturedBlackRookImage = GetCapturedPieceImage(PlayerColor.Black, PieceTypeInstance.Rook);
        public ImageSource CapturedBlackRookImage
        {
            get => capturedBlackRookImage;
            set
            {
                if (capturedBlackRookImage != value)
                {
                    capturedBlackRookImage = value;
                    OnPropertyChanged();
                }
            }
        }
        private ImageSource capturedBlackQueenImage = GetCapturedPieceImage(PlayerColor.Black, PieceTypeInstance.Queen);
        public ImageSource CapturedBlackQueenImage
        {
            get => capturedBlackQueenImage;
            set
            {
                if (capturedBlackQueenImage != value)
                {
                    capturedBlackQueenImage = value;
                    OnPropertyChanged();
                }
            }
        }
        private ImageSource capturedBlackKnightImage = GetCapturedPieceImage(PlayerColor.Black, PieceTypeInstance.Knight);
        public ImageSource CapturedBlackKnightImage
        {
            get => capturedBlackKnightImage;
            set
            {
                if (capturedBlackKnightImage != value)
                {
                    capturedBlackKnightImage = value;
                    OnPropertyChanged();
                }
            }
        }

        #endregion


        #region Num Captured Pieces 

        private int blackCapturedPawns = 0;
        public int BlackCapturedPawns
        {
            get => blackCapturedPawns;
            set
            {
                if (blackCapturedPawns != value)
                {
                    blackCapturedPawns = value;
                    OnPropertyChanged();
                }
            }
        }


        private int blackCapturedBishops = 0;
        public int BlackCapturedBishops
        {
            get => blackCapturedBishops;
            set
            {
                if (blackCapturedBishops != value)
                {
                    blackCapturedBishops = value;
                    OnPropertyChanged();
                }
            }
        }


        private int blackCapturedKnights = 0;
        public int BlackCapturedKnights
        {
            get => blackCapturedKnights;
            set
            {
                if (blackCapturedKnights != value)
                {
                    blackCapturedKnights = value;
                    OnPropertyChanged();
                }
            }
        }


        private int blackCapturedQueens = 0;
        public int BlackCapturedQueens
        {
            get => blackCapturedQueens;
            set
            {
                if (blackCapturedQueens != value)
                {
                    blackCapturedQueens = value;
                    OnPropertyChanged();
                }
            }
        }


        private int blackCapturedRooks = 0;
        public int BlackCapturedRooks
        {
            get => blackCapturedRooks;
            set
            {
                if (blackCapturedRooks != value)
                {
                    blackCapturedRooks = value;
                    OnPropertyChanged();
                }
            }
        }




        private int whiteCapturedPawns = 0;
        public int WhiteCapturedPawns
        {
            get => whiteCapturedPawns;
            set
            {
                if (whiteCapturedPawns != value)
                {
                    whiteCapturedPawns = value;
                    OnPropertyChanged();
                }
            }
        }


        private int whiteCapturedBishops = 0;
        public int WhiteCapturedBishops
        {
            get => whiteCapturedBishops;
            set
            {
                if (whiteCapturedBishops != value)
                {
                    whiteCapturedBishops = value;
                    OnPropertyChanged();
                }
            }
        }


        private int whiteCapturedKnights = 0;
        public int WhiteCapturedKnights
        {
            get => whiteCapturedKnights;
            set
            {
                if (whiteCapturedKnights != value)
                {
                    whiteCapturedKnights = value;
                    OnPropertyChanged();
                }
            }
        }


        private int whiteCapturedQueens = 0;
        public int WhiteCapturedQueens
        {
            get => whiteCapturedQueens;
            set
            {
                if (whiteCapturedQueens != value)
                {
                    whiteCapturedQueens = value;
                    OnPropertyChanged();
                }
            }
        }


        private int whiteCapturedRooks = 0;
        public int WhiteCapturedRooks
        {
            get => whiteCapturedRooks;
            set
            {
                if (whiteCapturedRooks != value)
                {
                    whiteCapturedRooks = value;
                    OnPropertyChanged();
                }
            }
        }

        #endregion


        #endregion


        protected void OnPropertyChanged([CallerMemberName] string name = null)
        {
            DispatcherQueue.TryEnqueue(() =>
            {
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
            });
        }
    }
}
