###########################################
#   Chess
#       by Jens W. Langenberg
###########################################

cmake_minimum_required(VERSION 3.25)

set(CHESS_VERSION 1.0.0.255)

project(Chess-Logic VERSION ${CHESS_VERSION} LANGUAGES CXX)


###########################################
#   CPP AND PROJECT STANDARDS
###########################################

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


###########################################
#   PROJECT DIRECTORIES
###########################################

set (SOURCE_DIR                 ${PROJECT_SOURCE_DIR}/src)
set (SUBMODULES_DIR             ${PROJECT_SOURCE_DIR}/submodules)
    
set (PIECE_DIR                  ${SOURCE_DIR}/Piece)
set (MOVE_DIR                   ${SOURCE_DIR}/Moves)
set (GAME_MANAGER_DIR           ${SOURCE_DIR}/GameManager)
set (BOARD_DIR                  ${SOURCE_DIR}/Board)
set (PLAYER_DIR                 ${SOURCE_DIR}/Player)
set (API_DIR                    ${SOURCE_DIR}/API)
set (MISC_DIR                   ${SOURCE_DIR}/Miscellaneous)
set (FILEMANAGER_DIR            ${SOURCE_DIR}/FileManager)
set (LOGGER_DIR                 ${SOURCE_DIR}/Logger)
set (USERSETTINGS_DIR           ${SOURCE_DIR}/UserSettings)
set (OBSERVER_DIR               ${SOURCE_DIR}/Observer)
set (COMMUNICATION_DIR          ${SOURCE_DIR}/Communication)


###########################################
#   INCLUDE DIRECTORIES
###########################################

set(ALL_PROJECT_DIRS 
            ${PIECE_DIR}
            ${MOVE_DIR}
            ${GAME_MANAGER_DIR}
            ${BOARD_DIR}
            ${PLAYER_DIR}
            ${API_DIR}
            ${MISC_DIR}
            ${LOGGER_DIR}
            ${FILEMANAGER_DIR}
            ${SOURCE_DIR}
            ${USERSETTINGS_DIR}
            ${OBSERVER_DIR}
            ${COMMUNICATION_DIR}
)

include_directories(${ALL_PROJECT_DIRS})


###########################################
#   SET THE FILES
###########################################

set(API_FILES
    ${API_DIR}/ChessLogicAPI.h    ${API_DIR}/ChessLogicAPI.cpp
    ${API_DIR}/ChessLogicAPIDefines.h
)

set(PIECE_FILES
    ${PIECE_DIR}/ChessPiece.h   ${PIECE_DIR}/ChessPiece.cpp
    ${PIECE_DIR}/Pawn.h         ${PIECE_DIR}/Pawn.cpp
    ${PIECE_DIR}/Knight.h       ${PIECE_DIR}/Knight.cpp
    ${PIECE_DIR}/Bishop.h       ${PIECE_DIR}/Bishop.cpp
    ${PIECE_DIR}/Rook.h         ${PIECE_DIR}/Rook.cpp
    ${PIECE_DIR}/Queen.h        ${PIECE_DIR}/Queen.cpp
    ${PIECE_DIR}/King.h         ${PIECE_DIR}/King.cpp
)

set(BOARD_FILES
    ${BOARD_DIR}/ChessBoard.h    ${BOARD_DIR}/ChessBoard.cpp
)
    
set(MOVE_FILES
    ${MOVE_DIR}/MoveType.h
    ${MOVE_DIR}/Move.h
    ${MOVE_DIR}/MoveHelper.h            ${MOVE_DIR}/MoveHelper.cpp
    ${MOVE_DIR}/MoveGeneration.h        ${MOVE_DIR}/MoveGeneration.cpp
    ${MOVE_DIR}/MoveValidation.h        ${MOVE_DIR}/MoveValidation.cpp
    ${MOVE_DIR}/MoveExecution.h         ${MOVE_DIR}/MoveExecution.cpp
    ${MOVE_DIR}/MoveNotationHelper.h    ${MOVE_DIR}/MoveNotationHelper.cpp
)

set(GAME_MANAGER_FILES
    ${GAME_MANAGER_DIR}/GameManager.h    ${GAME_MANAGER_DIR}/GameManager.cpp
)

set(PLAYER_FILES
    ${PLAYER_DIR}/Player.h      ${PLAYER_DIR}/Player.cpp
)

set(MISC_FILES
    ${MISC_DIR}/Parameters.h
)

set(LOGGER_FILES
    ${LOGGER_DIR}/Logging.h             ${LOGGER_DIR}/Logging.cpp
    ${LOGGER_DIR}/LoggingHelper.h       ${LOGGER_DIR}/LoggingHelper.cpp
)

set(FILEMANAGER_FILES
    ${FILEMANAGER_DIR}/FileManager.h    ${FILEMANAGER_DIR}/FileManager.cpp
)

set(USERSETTINGS_FILES
    ${USERSETTINGS_DIR}/UserSettings.h    ${USERSETTINGS_DIR}/UserSettings.cpp
)

set(OBSERVER_FILES
    ${OBSERVER_DIR}/IObserver.h
    ${OBSERVER_DIR}/IObservable.h
)

set(COMMUNICATION_FILES
    ${COMMUNICATION_DIR}/UICommunication.h    ${COMMUNICATION_DIR}/UICommunication.cpp
)


set(ALL_FILES
    ${API_FILES}
    ${PIECE_FILES}
    ${BOARD_FILES}
    ${MOVE_FILES}
    ${GAME_MANAGER_FILES}
    ${PLAYER_FILES}
    ${MISC_FILES}
    ${LOGGER_FILES}
    ${FILEMANAGER_FILES}
    ${USERSETTINGS_FILES}
    ${OBSERVER_FILES}
    ${COMMUNICATION_FILES}
)


###########################################
#   SOURCE GROUPS
###########################################

source_group("API"              FILES ${API_FILES})
source_group("Pieces"           FILES ${PIECE_FILES})
source_group("Board"            FILES ${BOARD_FILES})
source_group("Moves"            FILES ${MOVE_FILES})
source_group("GameManager"      FILES ${GAME_MANAGER_FILES})
source_group("Player"           FILES ${PLAYER_FILES})
source_group("Miscellaneous"    FILES ${MISC_FILES})
source_group("Logging"          FILES ${LOGGER_FILES})
source_group("FileManager"      FILES ${FILEMANAGER_FILES})
source_group("UserSettings"     FILES ${USERSETTINGS_FILES})
source_group("Observer"         FILES ${OBSERVER_FILES})
source_group("Communication"    FILES ${COMMUNICATION_FILES})


###########################################
#   SUBMODULES
###########################################

# Logger

add_subdirectory(${SUBMODULES_DIR}/Logger)
set(LOGGER_INCLUDE_FILE   ${SUBMODULES_DIR}/Logger/include)


# NLOHMANN JSON

set(JSON_DIR submodules/json/)
set(JSON_SINGLE_FILE ${JSON_DIR}/single_include/nlohmann)
add_subdirectory(${SUBMODULES_DIR}/json)


###########################################
#   CREATE TARGET
###########################################

add_library(${PROJECT_NAME} SHARED
    ${ALL_FILES}
)


###########################################
#   OUTPUT PROPERTIES
###########################################

set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "${PROJECT_NAME}"
)


###########################################
#   COMPILE DEFINITIONS
###########################################

target_compile_definitions(${PROJECT_NAME} PUBLIC
    -DENV_DEVELOPMENT
    _CRT_SECURE_NO_WARNINGS
)


###########################################
#   COMPILE OPTIONS
###########################################

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Release>:
        /Oi
        /Gy
    >
    $<$<CONFIG:Debug>:
        /Od
        /Z7
    >
    /sdl
    /MP
    /WX-        # Don't treat warnings as errors
)


###########################################
#   TARGET INCLUDE DIRECTORIES
###########################################

target_include_directories(${PROJECT_NAME} PUBLIC
        ${PROJECT_BINARY_DIR}
        ${ALL_PROJECT_DIRS}
        ${LOGGER_INCLUDE_FILE}
        ${JSON_SINGLE_FILE}

)


###########################################
#   LINK LIBRARIES
###########################################

set(PROJECT_LIBS Logger)

target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_LIBS})

