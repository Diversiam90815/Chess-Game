﻿using Chess.UI.MoveHistory;
using Chess.UI.Services;
using Chess.UI.Wrappers;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.UI.Dispatching;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;


namespace Chess.UI.ViewModels
{
    public class MoveHistoryViewModel
    {
        private const int MovesMaxColumns = 3;

        public ObservableCollection<ObservableCollection<string>> MoveHistoryColumns { get; } = [];

        private IMoveHistoryModel _model { get; }

        private readonly IDispatcherQueueWrapper _dispatcherQueue;


        public MoveHistoryViewModel(IDispatcherQueueWrapper dispatcher, IMoveHistoryModel model)
        {
            _dispatcherQueue = dispatcher;
            _model = model;

            for (int i = 0; i < MovesMaxColumns; i++)
            {
                MoveHistoryColumns.Add(new ObservableCollection<string>());
            }
            _model.MoveHistoryUpdated += OnHandleMoveHistoryUpdated;
        }


        public void AddMove(string move)
        {
            // Find the column with the least number of moves
            var minColumn = MoveHistoryColumns.OrderBy(col => col.Count).First();

            minColumn.Add(move);
        }


        public void ClearMoveHistory()
        {
            foreach (var column in MoveHistoryColumns)
            {
                column.Clear();
            }
            // TODO: Clear move history in backend
        }


        public void RemoveLastMove()
        {
            _model.RemoveLastMove();
            OnHandleMoveHistoryUpdated();
        }


        private void OnHandleMoveHistoryUpdated()
        {
            _dispatcherQueue.TryEnqueue(() =>
            {
                ClearMoveHistory();

                foreach (var moveNotation in _model.MoveHistory)
                {
                    AddMove(moveNotation);
                }
            });
        }
    }
}
