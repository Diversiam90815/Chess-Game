﻿using Microsoft.UI.Dispatching;
using Microsoft.UI.Xaml.Media;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Text;
using System.Threading.Tasks;
using Windows.System;
using static Chess_UI.Services.ChessLogicAPI;

namespace Chess_UI.Services
{
    public class BoardSquare : INotifyPropertyChanged
    {
        private readonly Microsoft.UI.Dispatching.DispatcherQueue DispatcherQueue;

        public event PropertyChangedEventHandler PropertyChanged;

        public Images.PieceTheme PieceTheme { get; private set; }

        private readonly ThemeManager ThemeManager;


        public BoardSquare(Microsoft.UI.Dispatching.DispatcherQueue dispatcher, ThemeManager themeManager)
        {
            this.pos = new PositionInstance(0, 0);
            this.piece = PieceTypeInstance.DefaultType;
            this.colour = PlayerColor.NoColor;

            this.DispatcherQueue = dispatcher;
            this.ThemeManager = themeManager;
            this.ThemeManager.PropertyChanged += OnThemeManagerPropertyChanged;

            this.PieceTheme = themeManager.CurrentPieceTheme;

        }

        public BoardSquare(int x, int y, PieceTypeInstance pieceTypeInstance, PlayerColor color, Microsoft.UI.Dispatching.DispatcherQueue dispatcher, ThemeManager themeManager)
        {
            this.pos = new PositionInstance(x, y);
            this.piece = pieceTypeInstance;
            this.colour = color;

            this.DispatcherQueue = dispatcher;
            this.ThemeManager = themeManager;
            this.ThemeManager.PropertyChanged += OnThemeManagerPropertyChanged;

            this.PieceTheme = themeManager.CurrentPieceTheme;
        }


        private void OnThemeManagerPropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (e.PropertyName == nameof(ThemeManager.CurrentPieceTheme))
            {
                UpdatePieceTheme(ThemeManager.CurrentPieceTheme);
            }
        }


        private void UpdatePieceTheme(Images.PieceTheme pieceTheme)
        {
            this.PieceTheme = pieceTheme;
        }


        private PositionInstance _pos;
        public PositionInstance pos
        {
            get => _pos;
            set
            {
                if (_pos.x != value.x || _pos.y != value.y)
                {
                    _pos = value;
                    OnPropertyChanged();
                }
            }
        }

        private PieceTypeInstance _piece;
        public PieceTypeInstance piece
        {
            get => _piece;
            set
            {
                if (_piece != value)
                {
                    _piece = value;
                    OnPropertyChanged();
                }
            }
        }

        private PlayerColor _colour;
        public PlayerColor colour
        {
            get => _colour;
            set
            {
                if (_colour != value)
                {
                    _colour = value;
                    OnPropertyChanged();
                }
            }
        }

        private bool isHighlighted;
        public bool IsHighlighted
        {
            get => isHighlighted;
            set
            {
                if (isHighlighted != value)
                {
                    isHighlighted = value;
                    OnPropertyChanged();
                    OnPropertyChanged(nameof(BackgroundBrush)); // if you choose a computed brush
                }
            }
        }

        public Brush BackgroundBrush
        {
            get
            {
                // Return a special color if IsHighlighted, otherwise transparent
                return IsHighlighted
                    ? new SolidColorBrush(Windows.UI.Color.FromArgb(128, 173, 216, 230)) // a light blue-ish
                    : new SolidColorBrush(Windows.UI.Color.FromArgb(0, 0, 0, 0));
            }
        }

        public ImageSource PieceImage
        {
            get
            {
                if (piece == PieceTypeInstance.DefaultType || colour == PlayerColor.NoColor)
                    return null;

                return Images.GetPieceImage(PieceTheme, colour, piece);
            }
        }

        private void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            DispatcherQueue?.TryEnqueue(() =>
            {
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
            });
        }
    }
}
