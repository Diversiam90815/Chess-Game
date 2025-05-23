/*
  ==============================================================================
	Module:			Test Helper
	Description:    Helper methods for the testing modules
  ==============================================================================
*/

#pragma once
#include <memory>

#include "ChessBoard.h"


// Helper to create a default board
std::shared_ptr<ChessBoard> CreateDefaultBoard();
