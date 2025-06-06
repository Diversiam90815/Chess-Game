﻿using Microsoft.UI.Xaml.Media;
using Microsoft.UI.Xaml.Media.Imaging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static Chess.UI.Services.EngineAPI;


namespace Chess.UI.Images
{
    public class ImageServices : IImageService
    {
        public enum MainMenuButton
        {
            StartGame = 1,
            Settings,
            Multiplayer,
            EndGame
        }

        public enum BoardTheme
        {
            Wood = 1,
            Wood2 = 2,
            Plain,
            Plastic,
            Marble,
            Marble2,
            Glass
        }

        public enum PieceTheme
        {
            Basic = 1,
            Standard
        }


        public Dictionary<BoardTheme, ImageSource> BoardBackgroundImages;

        public Dictionary<MainMenuButton, ImageSource> MainMenutImages;

        public Dictionary<PieceTypeInstance, ImageSource> CapturedWhitePiecesImages;

        public Dictionary<PieceTypeInstance, ImageSource> CapturedBlackPiecesImages;


        public ImageServices()
        {
            Init();
        }



        private void Init()
        {
            BoardBackgroundImages = new()
        {
            { BoardTheme.Wood, LoadImage("/Assets/Board/Wood.png") },
            { BoardTheme.Wood2, LoadImage("/Assets/Board/Wood2.png") },
            { BoardTheme.Plain, LoadImage("/Assets/Board/Plain.png") },
            { BoardTheme.Plastic, LoadImage("/Assets/Board/Plastic.png") },
            { BoardTheme.Marble, LoadImage("/Assets/Board/Marble.png") },
            { BoardTheme.Marble2, LoadImage("/Assets/Board/Marble2.png") },
            { BoardTheme.Glass, LoadImage("/Assets/Board/Glass.png") }
        };

            MainMenutImages = new()
        {
        {MainMenuButton.StartGame, LoadImage("/Assets/Buttons/pawn.png") },
        {MainMenuButton.Settings, LoadImage("/Assets/Buttons/rook.png") },
        {MainMenuButton.Multiplayer, LoadImage("/Assets/Buttons/queen.png") },
        {MainMenuButton.EndGame, LoadImage("/Assets/Buttons/horse.png") }
        };

            CapturedWhitePiecesImages = new()
        {
        {PieceTypeInstance.Pawn, LoadImage("/Assets/Pieces/Standard/PawnW.png") },
        {PieceTypeInstance.Bishop, LoadImage("/Assets/Pieces/Standard/BishopW.png") },
        {PieceTypeInstance.Queen, LoadImage("/Assets/Pieces/Standard/QueenW.png") },
        {PieceTypeInstance.Rook, LoadImage("/Assets/Pieces/Standard/RookW.png") },
        {PieceTypeInstance.Knight, LoadImage("/Assets/Pieces/Standard/KnightW.png") },
        {PieceTypeInstance.King, LoadImage("/Assets/Pieces/Standard/KingW.png") }
        };

            CapturedBlackPiecesImages = new()
        {
        {PieceTypeInstance.Pawn, LoadImage("/Assets/Pieces/Standard/PawnB.png") },
        {PieceTypeInstance.Bishop, LoadImage("/Assets/Pieces/Standard/BishopB.png") },
        {PieceTypeInstance.Queen, LoadImage("/Assets/Pieces/Standard/QueenB.png") },
        {PieceTypeInstance.Rook, LoadImage("/Assets/Pieces/Standard/RookB.png") },
        {PieceTypeInstance.Knight, LoadImage("/Assets/Pieces/Standard/KnightB.png") },
        {PieceTypeInstance.King, LoadImage("/Assets/Pieces/Standard/KingB.png") }
        };
        }


        // Dynamic piece image loading based on theme, color, and type
        public ImageSource GetPieceImage(PieceTheme theme, PlayerColor color, PieceTypeInstance pieceType)
        {
            // Convert enum values to strings that match folder and file naming conventions
            string themeName = theme.ToString();
            string colorSuffix = color == PlayerColor.White ? "W" : "B";
            string pieceName = pieceType.ToString();

            // Construct the relative file path. Assumes folder structure: Assets/Pieces/{Theme}/
            string relativePath = $"/Assets/Pieces/{themeName}/{pieceName}{colorSuffix}.png";

            return LoadImage(relativePath);
        }


        public ImageSource LoadImage(string relativeFilePath)
        {
            return new BitmapImage(new Uri($"ms-appx:///{relativeFilePath.TrimStart('/')}"));
        }


        public ImageSource GetImage(MainMenuButton button) => MainMenutImages[button];

        public ImageSource GetImage(BoardTheme background) => BoardBackgroundImages[background];


        public ImageSource GetCapturedPieceImage(PlayerColor player, PieceTypeInstance pieceTypeInstance)
        {
            return player switch
            {
                PlayerColor.White => CapturedWhitePiecesImages[pieceTypeInstance],
                PlayerColor.Black => CapturedBlackPiecesImages[pieceTypeInstance],
                _ => null
            };
        }


    }
}
