// C Program to show main with no return type and no
// arguments
#include <stdio.h>
#include "smallchesslib.h"

#define ERR_CODE1    400
#define ERR_CODE2    401

/*___________________________________________________________________________
* Function declarations */

int getBoard(SCL_Record record, int moves);
int getFirstMoves(SCL_Record r, int moves);
int hasOpening(SCL_Record record, SCL_Record opening);
int hasBoard(SCL_Record record, SCL_Board board, int moves);

/*___________________________________________________________________________
* HAS BOARD function. IN: chessgame/record, chessboard, move count OUT: integer
* 
* The function checks if during the first x moves of the chessgame, a certain state
* of the chessboard happens. The function executes the moves one by one and compares
* the temporary board gameboard to the given board. Execution is based on the 
* SCL_applyRecord function from the lib. The function instantly returns 1 if the
* given board is the starting board.
* 
* Returns 1 when a board match is found, 0 if it is not, and the error code 1 if 
* the number of desired moves is higher than the length of the provided chessgame.
*/

int hasBoard(SCL_Record record, SCL_Board board, int moves) {

    SCL_Board gameboard;

    SCL_boardInit(gameboard);
    if (SCL_boardsDiffer(gameboard, board) == 0) {
        return 1;
    }
    
    uint16_t recordLength = SCL_recordLength(record);

    if (moves > recordLength) {
        return ERR_CODE1;
    }

    for (int i = 0; i < moves; ++i)
    {
        uint8_t s0, s1;
        char p;

        SCL_recordGetMove(record, i, &s0, &s1, &p);
        SCL_boardMakeMove(gameboard, s0, s1, p);
        if (SCL_boardsDiffer(gameboard, board) == 0) {
            return 1;
        }
    }
    return 0;
}

/*___________________________________________________________________________
* HAS OPENING function. IN: record/chessgame, record/chessgame, OUT: integer
* 
* The function checks if chessgame 1 has an opening (shorter chessgame 2).
* 
* Returns 1 for true, 0 for false, error code 1 for empty chessgames, 
* error code 2 for opening longer than the game. 
*/

int hasOpening(SCL_Record record, SCL_Record opening) {

    int openingLength = SCL_recordLength(opening);
    int recordLength = SCL_recordLength(record);

    if (recordLength == 0) {
        return ERR_CODE1;
    }
    if (openingLength == 0) {
        return ERR_CODE1;
    }
    if (openingLength > recordLength) {
        return ERR_CODE2;
    }

    getFirstMoves(record, openingLength);
    for (int i = 0; i < openingLength * 2; i++) {
        if (record[i] != opening[i]) {
            return 0; //FALSE
        }
    }
    return 1; //TRUE
}

/*___________________________________________________________________________
* GET BOARD function. IN: record/chessgame, moves count, OUT: board (in theory :>)
* 
* The function creates a SCL_Board structure based on the provided chessgame
* and the number of moves considered. The board local variable is a product of 
* this operation and it should be returned. Here only the pointer to the array 
* can be returned like that, so instead a result code is returned and the actual 
* returning of board is to be resolved. The commented section allows printing 
* the result board in FEN format. 
* 
* Returns 0 after succesful board creation, error code 1 if the number of desired 
* moves is higher than the length of the provided chessgame. 
*/

int getBoard(SCL_Record record, int moves) {

    SCL_Board board;

    if (moves == 0) {
        SCL_boardInit(board);
    }
    else if (moves > SCL_recordLength(record)) {
        return ERR_CODE1;
    }
    else {
        SCL_recordApply(record, board, moves);
    }

    //char str2[4096];
    //SCL_boardToFEN(board, str2);
    //printf(str2);
    return 0;
}

/*___________________________________________________________________________
* GET FIRST MOVES function. IN: record/chessgame, moves count, OUT: chessgame (also in theory)
* 
* Here the function creates a shorter version of the chessgame/record. It changes 
* the chessgame - analogously to the SCL_recordRemoveLast function from the library
* - by inserting a mark indicating draw or end of game for other reasons after 
* the number of moves specified in the input. The record is changed in place so the
* record variable is also the output chessgame. When the desired number of moves is
* equal to game length, to record is not modified. If needed, the result record can 
* be copied to a new record by SCL_recordCopy function from the lib.
* 
* Returns 0 after succesful record modification, error code 1 for empty record, and 
* error code 2 if the number of desired moves is higher than the length of the 
* provided chessgame.
*/

 int getFirstMoves(SCL_Record record, int moves) {

    uint16_t l = SCL_recordLength(record);

    if (l == 0) {
        return ERR_CODE1;
    }
    if (moves > l) {
        return ERR_CODE2;
    }
    if (moves == l) {
        return 0;
    }
    if (moves == 0) {
        SCL_recordInit(record);
    }
    else {
        moves = (moves - 1) * 2;
        record[moves] = (record[moves] & 0x3f) | SCL_RECORD_END;
    }

    //char strr[4096];
    //SCL_printPGN(firstMoves, putCharStr, 0, strr);
    //printf(strr);
    return 0;
}

 //___________________________________________________________________________




// Defining a main function
void main()
{
    //___________ SCL_Board ___________     
    // create SCL_Board from FEN
    SCL_Board board;
    SCL_boardFromFEN(board, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    // get FEN string from SCL_Board
    char str2[4096];
    SCL_boardToFEN(board, str2);
    //printf(str2);


    //___________ SCL_Record ___________     
    // create SCL_Record from PGN like notation
    SCL_Record record;
    SCL_recordFromPGN(record, "1. h4 g5 2. hxg5 Nf6 3. Nf3 Bg7+ 4. e3 O-O 5. Nc3 c5");
    //SCL_recordFromPGN(record, "1. Nf3 Nf6 2. c4 g6 3. Nc3 Bg7 4. d4 O-O 5. Bf4 d5 6. Qb3 dxc4 7. Qxc4 c6 8. e4 Nbd7 9. Rd1 Nb6 10. Qc5 Bg4 11. Bg5 Na4 12. Qa3 Nxc3 13. bxc3 Nxe4 14. Bxe7 Qb6 15. Bc4 Nxc3 16. Bc5 Rfe8+ 17. Kf1 Be6 18. Bxb6 Bxc4+ 19. Kg1 Ne2+ 20. Kf1 Nxd4+ 21. Kg1 Ne2+ 22. Kf1 Nc3+ 23. Kg1 axb6 24. Qb4 Ra4 25. Qxb6 Nxd1 26. h3 Rxa2 27. Kh2 Nxf2 28. Re1 Rxe1 29. Qd8+ Bf8 30. Nxe1 Bd5 31. Nf3 Ne4 32. Qb8 b5 33. h4 h5 34. Ne5 Kg7 35. Kg1 Bc5+ 36. Kf1 Ng3+ 37. Ke1 Bb4+ 38. Kd1 Bb3+ 39. Kc1 Ne2+ 40. Kb1 Nc3+ 41. Kc1 Rc2");

    // get SCL_Record length (number of halfmoves)
    int a = SCL_recordLength(record);
    
    // get PGN string from SCL_Record
    char str3[4096];
    SCL_printPGN(record, 0, str3);
    printf(str3);


    //___________ getBoard ___________     
    getBoard(record, 5);


    //___________ getFirstMoves ___________     
    getFirstMoves(record, 3);

    //___________ hasOpening ___________
    SCL_Record opening1;
    SCL_recordFromPGN(opening1, "1. Nf3 Nf6 2. c4 g6");
    SCL_Record record1;
    SCL_recordFromPGN(record1, "1. Nf3 Nf6 2. c4 g6 3. Nc3 Bg7 4. d4 O-O 5. Bf4 d5 6. Qb3 dxc4 7. Qxc4 c6 8. e4 Nbd7 9. Rd1 Nb6 10. Qc5 Bg4 11. Bg5 Na4 12. Qa3 Nxc3 13. bxc3 Nxe4 14. Bxe7 Qb6 15. Bc4 Nxc3 16. Bc5 Rfe8+ 17. Kf1 Be6 18. Bxb6 Bxc4+ 19. Kg1 Ne2+ 20. Kf1 Nxd4+ 21. Kg1 Ne2+ 22. Kf1 Nc3+ 23. Kg1 axb6 24. Qb4 Ra4 25. Qxb6 Nxd1 26. h3 Rxa2 27. Kh2 Nxf2 28. Re1 Rxe1 29. Qd8+ Bf8 30. Nxe1 Bd5 31. Nf3 Ne4 32. Qb8 b5 33. h4 h5 34. Ne5 Kg7 35. Kg1 Bc5+ 36. Kf1 Ng3+ 37. Ke1 Bb4+ 38. Kd1 Bb3+ 39. Kc1 Ne2+ 40. Kb1 Nc3+ 41. Kc1 Rc2");

    hasOpening(record1, opening1);

    //___________ hasBoard ___________  
    SCL_Record record2;
    SCL_recordFromPGN(record2, "1. Nf3 Nf6 2. c4 g6 3. Nc3 Bg7 4. d4 O-O 5. Bf4 d5 6. Qb3 dxc4 7. Qxc4 c6 8. e4 Nbd7 9. Rd1 Nb6 10. Qc5 Bg4 11. Bg5 Na4 12. Qa3 Nxc3 13. bxc3 Nxe4 14. Bxe7 Qb6 15. Bc4 Nxc3 16. Bc5 Rfe8+ 17. Kf1 Be6 18. Bxb6 Bxc4+ 19. Kg1 Ne2+ 20. Kf1 Nxd4+ 21. Kg1 Ne2+ 22. Kf1 Nc3+ 23. Kg1 axb6 24. Qb4 Ra4 25. Qxb6 Nxd1 26. h3 Rxa2 27. Kh2 Nxf2 28. Re1 Rxe1 29. Qd8+ Bf8 30. Nxe1 Bd5 31. Nf3 Ne4 32. Qb8 b5 33. h4 h5 34. Ne5 Kg7 35. Kg1 Bc5+ 36. Kf1 Ng3+ 37. Ke1 Bb4+ 38. Kd1 Bb3+ 39. Kc1 Ne2+ 40. Kb1 Nc3+ 41. Kc1 Rc2");
    SCL_Board board2;
    SCL_boardFromFEN(board2, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR");

    hasBoard(record2, board2, 8);


    //SCL_recordRemoveLast(record);
    //SCL_recordApply(record, board, SCL_recordLength(record));
}