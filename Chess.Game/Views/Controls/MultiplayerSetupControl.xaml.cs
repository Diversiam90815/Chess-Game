using Chess.UI.Services;
using Chess.UI.ViewModels;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.UI.Xaml;
using Microsoft.UI.Xaml.Controls;


namespace Chess.UI.Views.Controls
{
    public sealed partial class MultiplayerSetupControl : UserControl
    {
        private readonly MultiplayerViewModel _viewModel;


        public MultiplayerSetupControl()
        {
            this.InitializeComponent();

            _viewModel = App.Current.Services.GetService<MultiplayerViewModel>();
            this.Rootgrid.DataContext = _viewModel;
        }


        public void Initialize()
        {
            _viewModel.ResetViewState();
            _viewModel.StartMultiplayerSetup();
        }


        private void FindOpponentButton_Click(object sender, RoutedEventArgs e)
        {
            _viewModel.OnButtonClicked();
            _viewModel.FindOpponent();
        }


        private void CancelSearchButton_Click(object sender, RoutedEventArgs e)
        {
            _viewModel.OnButtonClicked();
            _viewModel.CancelSearch();
        }


        private void ConnectButton_Click(object sender, RoutedEventArgs e)
        {
            _viewModel.OnButtonClicked();
            _viewModel.ConnectToOpponent();
        }


        private void AcceptButton_Click(object sender, RoutedEventArgs e)
        {
            _viewModel.OnButtonClicked();
            _viewModel.AcceptConnectionRequest();
        }


        private void DeclineButton_Click(object sender, RoutedEventArgs e)
        {
            _viewModel.OnButtonClicked();
            _viewModel.DeclineConnectionRequest();
        }


        private void SelectWhiteButton_Click(object sender, RoutedEventArgs e)
        {
            _viewModel.OnButtonClicked();
            _viewModel.SelectPlayerColor(EngineAPI.Side.White);
        }


        private void SelectBlackButton_Click(object sender, RoutedEventArgs e)
        {
            _viewModel.OnButtonClicked();
            _viewModel.SelectPlayerColor(EngineAPI.Side.Black);
        }


        private void ReadyButton_Click(object sender, RoutedEventArgs e)
        {
            _viewModel.OnButtonClicked();
            _viewModel.SetPlayerReady();
        }
    }
}
