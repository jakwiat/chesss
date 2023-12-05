#include <stdio.h>
#include <postgres.h>
#include <stdlib.h>
#include <fmgr.h>

#include "utils/builtins.h"
#include "libpq/pqformat.h"
#include "smallchesslib.h"
#include <stdbool.h>
#include <string.h>


PG_MODULE_MAGIC;

typedef struct {
    char fen[128]; // FEN representation of chessboard
} chessboard;

typedef struct {
    char san[4096]; // SAN representation of chess game
} chessgame;

#define DatumGetChessgameP(X)  ((chessgame *) DatumGetPointer(X))
#define ChessgamePGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_CHESSGAME_P(n) DatumGetChessgameP(PG_GETARG_DATUM(n))
#define PG_RETURN_CHESSGAME_P(x) return ChessgamePGetDatum(x)

#define DatumGetChessboardP(X)  ((chessboard *) DatumGetPointer(X))
#define ChessboardPGetDatum(X)  PointerGetDatum(X)
#define PG_GETARG_CHESSBOARD_P(n) DatumGetChessboardP(PG_GETARG_DATUM(n))

/*___________________________________________________________________________
* Function declarations */

PG_FUNCTION_INFO_V1(chessgame_in);
PG_FUNCTION_INFO_V1(chessgame_out);
PG_FUNCTION_INFO_V1(chessboard_in);
PG_FUNCTION_INFO_V1(chessboard_out);

// Function to convert external representation to chessgame type
Datum chessgame_in(PG_FUNCTION_ARGS) {
    char *str = PG_GETARG_CSTRING(0);
    chessgame *result;
    char sanitized_moves[4096] = ""; // Adjust the size as needed
    int index = 0;

    while (*str != '\0') {
        if (*str == ' ' && index > 0) {
            sanitized_moves[index++] = ' ';
        } else if (isalnum(*str) || *str == '-') {
            sanitized_moves[index++] = *str;
        }
        str++;
    }
    sanitized_moves[index] = '\0';

    result = (chessgame *) palloc(sizeof(chessgame));
    strncpy(result->san, sanitized_moves, sizeof(result->san));
    PG_RETURN_POINTER(result);
}

Datum chessgame_out(PG_FUNCTION_ARGS) {
    chessgame *chess = PG_GETARG_CHESSGAME_P(0);
    char *result;

    result = psprintf("%s", chess->san);
    PG_RETURN_CSTRING(result);
}

Datum chessboard_in(PG_FUNCTION_ARGS) {
    char *fen_str = PG_GETARG_CSTRING(0);

    chessboard *result = (chessboard *) palloc(sizeof(chessboard));
    strncpy(result->fen, fen_str, sizeof(result->fen));

    PG_RETURN_POINTER(result);
}

Datum chessboard_out(PG_FUNCTION_ARGS) {
    chessboard *board = (chessboard *) PG_GETARG_POINTER(0);
    char *result = pstrdup(board->fen);

    PG_RETURN_CSTRING(result);
}

// Chess functions

PG_FUNCTION_INFO_V1(getBoard);

Datum getBoard(PG_FUNCTION_ARGS) {
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                 errmsg("Invalid arguments for getBoard")));
        PG_RETURN_NULL();
    }

    chessgame *game = (chessgame *)PG_GETARG_POINTER(0);
    int halfMoves = PG_GETARG_INT32(1);
	char *game_str = pstrdup(game->san);
	
	SCL_Record chesslib_record;
	SCL_recordFromPGN(chesslib_record, game_str);

	SCL_Board chesslib_board;
	SCL_boardInit(chesslib_board);
	if (halfMoves > 0) {
        SCL_recordApply(chesslib_record, chesslib_board, halfMoves);
    }
	char board_str[128];
    SCL_boardToFEN(chesslib_board, board_str);

    chessboard *tempBoard = (chessboard *)palloc(sizeof(chessboard));
    strncpy(tempBoard->fen, board_str, sizeof(tempBoard->fen));

    PG_RETURN_POINTER(tempBoard);
}

int cGetFirstMoves(SCL_Record record, int moves) {

    uint16_t l = SCL_recordLength(record);

    if (moves == l) {
        return 0;
    }
    else if (moves == 0) {
        SCL_recordInit(record);
    }
    else {
        moves = (moves - 1) * 2;
        record[moves] = (record[moves] & 0x3f) | SCL_RECORD_END;
    }

    return 0;
}

PG_FUNCTION_INFO_V1(getFirstMoves);

Datum getFirstMoves(PG_FUNCTION_ARGS) {
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                 errmsg("Invalid arguments for getFirstMoves")));
        PG_RETURN_NULL();
    }
	chessgame *game = (chessgame *)PG_GETARG_POINTER(0);
    int halfMoveCount = PG_GETARG_INT32(1);

    if (halfMoveCount < 0) {
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                 errmsg("Half-move count should be non-negative")));
    }

    char *game_str = pstrdup(game->san);
	SCL_Record chesslib_record;
	SCL_recordFromPGN(chesslib_record, game_str);
	
    cGetFirstMoves(chesslib_record, halfMoveCount);
	
	char result_str[4096];
	SCL_printPGN(chesslib_record, 0, result_str);
	
	chessgame *result = (chessgame *) palloc(sizeof(chessgame));
    strncpy(result->san, result_str, sizeof(result->san));
	
    PG_RETURN_POINTER(result);
}

int cHasBoard(SCL_Record record, SCL_Board board, int moves) {

    SCL_Board gameboard;
    SCL_boardInit(gameboard);
    if (SCL_boardsDiffer(gameboard, board) == 0) {
        return 1; //TRUE
    }

    for (int i = 0; i < moves; ++i)
    {
        uint8_t s0, s1;
        char p;

        SCL_recordGetMove(record, i, &s0, &s1, &p);
        SCL_boardMakeMove(gameboard, s0, s1, p);
        if (SCL_boardsDiffer(gameboard, board) == 0) {
            return 1; //TRUE
        }
    }
    return 0; //FALSE
}

PG_FUNCTION_INFO_V1(hasBoard);

Datum hasBoard(PG_FUNCTION_ARGS) {
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1) || PG_ARGISNULL(2)) {
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                 errmsg("Invalid arguments for hasBoard")));
        PG_RETURN_NULL();
    }

    chessgame *game = (chessgame *)PG_GETARG_POINTER(0);
	chessboard *board = (chessboard *)PG_GETARG_POINTER(1);
    int halfMoves = PG_GETARG_INT32(2);

    char *game_str = pstrdup(game->san);
    char *board_str = pstrdup(board->fen);
	
	SCL_Record chesslib_record;
	SCL_recordFromPGN(chesslib_record, game_str);
	
	SCL_Board chesslib_board;
	SCL_boardInit(chesslib_board);
    SCL_boardFromFEN(chesslib_board, board_str);
	
    int res;
	res = cHasBoard(chesslib_record, chesslib_board, halfMoves);

    PG_RETURN_BOOL(res == 1);
}

int cHasOpening(SCL_Record record, SCL_Record opening) {

    int openingLength = SCL_recordLength(opening);
    int recordLength = SCL_recordLength(record);

    if (openingLength > recordLength) {
        return 0; //FALSE
    }

    cGetFirstMoves(record, openingLength);
	
    for (int i = 0; i < openingLength * 2; i++) {
        if (record[i] != opening[i]) {
            return 0; //FALSE
        }
    }
    return 1; //TRUE
}

PG_FUNCTION_INFO_V1(hasOpening);

Datum hasOpening(PG_FUNCTION_ARGS) {
    if (PG_ARGISNULL(0) || PG_ARGISNULL(1)) {
        ereport(ERROR,
                (errcode(ERRCODE_INVALID_PARAMETER_VALUE),
                 errmsg("Invalid arguments for hasOpening")));
        PG_RETURN_NULL();
    }

    chessgame *game = (chessgame *)PG_GETARG_POINTER(0);
	chessgame *opening = (chessgame *)PG_GETARG_POINTER(1);

    char *game_str = pstrdup(game->san);
    char *opening_str = pstrdup(opening->san);
	
	SCL_Record chesslib_record;
	SCL_recordFromPGN(chesslib_record, game_str);
	
	SCL_Record chesslib_opening;
	SCL_recordFromPGN(chesslib_opening, opening_str);
	
    int res;
	res = cHasOpening(chesslib_record, chesslib_opening);

    PG_RETURN_BOOL(res == 1);
}