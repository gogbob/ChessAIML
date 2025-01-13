#pragma once
#include <vector>
#include <math.h>
#include <math_functions.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <cstdio>
#include <omp.h>

using namespace std;

class ChessPosition
{
public:

    bool SIDEMOVE;//determines if it is black or whites move
    int Pieces[8][8];//gives the piece location, 0 being that there are no pieces
    vector<int> PreviousMove; //only for en passants
    // 1 = pawn (white pieces)
    // 2 = bishop
    // 3 = knight
    // 4 = rook that can castle
    // 5 = rook that cant castle
    // 6 = king
    // 7 = queen
    // 8 to 14, the same but with the other color (in this case black)



    void RemovePiece(int x, int y)
    {
        Pieces[x][y] = 0;
    }

    int getPieceID(int x, int y)
    {
        return Pieces[x][y];
    }

    void AddPiece(int x, int y, int PieceId)
    {
        Pieces[x][y] = PieceId;
    }

    //setting default value
    void setDefault()
    {
        SIDEMOVE = 0;
        PreviousMove.push_back(-1);
        PreviousMove.push_back(-1);
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                //white
                //i = 8 because white starts at the bottom of the board
                if (j == 7)
                {
                    if (i % 7 == 0)
                    {
                        Pieces[i][j] = 4;
                    }
                    if (i % 5 == 1)
                    {
                        Pieces[i][j] = 3;
                    }
                    if (i % 3 == 2)
                    {
                        Pieces[i][j] = 2;

                    }
                    if (i == 3)
                    {
                        Pieces[i][j] = 7;
                    }
                    if (i == 4)
                    {
                        Pieces[i][j] = 6;
                    }
                }
                else if (j == 0)
                {
                    if (i % 7 == 0)
                    {
                        Pieces[i][j] = 11;
                    }
                    if (i % 5 == 1)
                    {
                        Pieces[i][j] = 10;
                    }
                    if (i % 3 == 2)
                    {
                        Pieces[i][j] = 9;

                    }
                    if (i == 3)
                    {
                        Pieces[i][j] = 14;
                    }
                    if (i == 4)
                    {
                        Pieces[i][j] = 13;
                    }
                }
                else if (j == 6)
                {

                    Pieces[i][j] = 1;
                }
                else if (j == 1)
                {

                    Pieces[i][j] = 8;
                }
                else Pieces[i][j] = 0;
            }
        }
    }

    void AddPrevMove(int x, int y)
    {
        PreviousMove[0] = x;
        PreviousMove[1] = y;
    }

    void CopyPosition(ChessPosition pos)
    {
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                Pieces[i][j] = pos.Pieces[i][j];



            }
        }
        SIDEMOVE = pos.SIDEMOVE;
        if (PreviousMove.size() >= 2)
        {
            PreviousMove[0] = pos.PreviousMove[0];
            PreviousMove[1] = pos.PreviousMove[1];
        }
        else
        {
            PreviousMove.push_back(pos.PreviousMove[0]);
            PreviousMove.push_back(pos.PreviousMove[1]);
        }
    }


    vector<vector<vector<int>>> AllPieceMoves()
    {
        //hi


        vector<vector<vector<int>>> PieceMoves;


        vector<vector<vector<int>>> PinLines;
        vector<vector<vector<int>>> CheckLines;
        vector<vector<int>> PinnedPieces;
        vector<vector<int>> CheckingPiece;
        int numChecks = 0;
        vector<vector<int>> NoKingZone;

        vector<int> NoCastleZone;

        int kingLocation[2];

        for (int pieceindexx = 0; pieceindexx < 8; pieceindexx++)
        {
            for (int pieceindexy = 0; pieceindexy < 8; pieceindexy++)
            {
                if (Pieces[pieceindexx][pieceindexy] - SIDEMOVE * 7 == 6)
                {
                    kingLocation[0] = pieceindexx;
                    kingLocation[1] = pieceindexy;
                }
            }
        }

        ////check if black checks or pins pieces
        for (int pieceindexx = 0; pieceindexx < 8; pieceindexx++)
        {
            for (int pieceindexy = 0; pieceindexy < 8; pieceindexy++)
            {
                if (Pieces[pieceindexx][pieceindexy] != 0)
                {
                    //check for other piece pins and checks
                    if (floor((float)Pieces[pieceindexx][pieceindexy] / 8.0f) != SIDEMOVE)
                    {


                        //check for pins
                        if (Pieces[pieceindexx][pieceindexy] - 7 * (!SIDEMOVE) == 2)
                        {
                            vector<vector<int>> temporaryMoveMemory;
                            temporaryMoveMemory.push_back({ pieceindexx , pieceindexy });
                            vector<int> temporaryPinPieceData = { -1, -1 };
                            //bishops
                            bool end = false;
                            bool searchKing = false;
                            for (int i = 1; i <= 7 - max(pieceindexx, pieceindexy) && !end; i++)
                            {
                                if (Pieces[pieceindexx + i][pieceindexy + i] == 0)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy + i });
                                    if (searchKing == false)
                                    {
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + i + posX == kingLocation[0] && pieceindexy + i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx + i , pieceindexy + i });
                                                }
                                            }
                                        }
                                        if (pieceindexy == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexx + i);
                                    }
                                }
                                else if (pieceindexx + i == kingLocation[0] && pieceindexy + i == kingLocation[1])
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy + i });
                                    if (searchKing == false)
                                    {
                                        if (pieceindexx + i + 1 < 8 && pieceindexy + i + 1 < 8)
                                        {
                                            NoKingZone.push_back({ pieceindexx + i + 1 , pieceindexy + i + 1 });
                                        }
                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy });
                                        CheckLines.push_back(temporaryMoveMemory);



                                        CheckingPiece.push_back({ pieceindexx , pieceindexy });
                                        end = true;
                                    }
                                    else
                                    {
                                        end = true;
                                        PinLines.push_back(temporaryMoveMemory);
                                        PinnedPieces.push_back({ temporaryPinPieceData[0], temporaryPinPieceData[1] });
                                    }
                                }
                                else if (floor((float)Pieces[pieceindexx + i][pieceindexy + i] / 8.0f) == SIDEMOVE)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy + i });
                                    if (searchKing == false)
                                    {
                                        temporaryPinPieceData[0] = pieceindexx + i;
                                        temporaryPinPieceData[1] = pieceindexy + i;
                                        searchKing = true;
                                    }
                                    else end = true;
                                }
                                else
                                {
                                    end = true;
                                    if (searchKing == false)
                                    {
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + i + posX == kingLocation[0] && pieceindexy + i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx + i , pieceindexy + i });
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            end = false;
                            searchKing = false;
                            temporaryMoveMemory.erase(temporaryMoveMemory.begin(), temporaryMoveMemory.end());
                            temporaryPinPieceData[0] = 0;
                            temporaryPinPieceData[1] = 0;

                            for (int i = 1; i <= min(pieceindexx, pieceindexy) && !end; i++)
                            {
                                if (Pieces[pieceindexx - i][pieceindexy - i] == 0)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexy - i });
                                    if (searchKing == false)
                                    {
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx - i + posX == kingLocation[0] && pieceindexy - i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx - i , pieceindexy - i });
                                                }
                                            }
                                        }
                                        if (pieceindexy == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexy - i);
                                    }
                                }
                                else if (pieceindexx - i == kingLocation[0] && pieceindexy - i == kingLocation[1])
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexy - i });
                                    if (searchKing == false)
                                    {
                                        if (pieceindexx - i - 1 >= 0 && pieceindexy - i - 1 >= 0)
                                        {
                                            NoKingZone.push_back({ pieceindexx - i - 1 , pieceindexy - i - 1 });
                                        }

                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy });
                                        CheckingPiece.push_back({ pieceindexx , pieceindexy });
                                        numChecks++;
                                        CheckLines.push_back(temporaryMoveMemory);
                                        end = true;
                                    }
                                    else
                                    {
                                        end = true;
                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy });
                                        PinLines.push_back(temporaryMoveMemory);
                                        PinnedPieces.push_back({ temporaryPinPieceData[0], temporaryPinPieceData[1] });
                                    }
                                }
                                else if (floor((float)Pieces[pieceindexx - i][pieceindexy - i] / 8.0f) == SIDEMOVE)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexy - i });
                                    if (searchKing == false)
                                    {
                                        searchKing = true;
                                        temporaryPinPieceData[0] = pieceindexx - i;
                                        temporaryPinPieceData[1] = pieceindexy - i;
                                    }
                                    else end = true;
                                }
                                else
                                {
                                    end = true;
                                    if (searchKing == false)
                                    {
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx - i + posX == kingLocation[0] && pieceindexy - i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx - i , pieceindexy - i });
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            end = false;
                            searchKing = false;
                            temporaryMoveMemory.erase(temporaryMoveMemory.begin(), temporaryMoveMemory.end());
                            temporaryPinPieceData[0] = 0;
                            temporaryPinPieceData[1] = 0;

                            for (int i = 1; i <= min(7 - pieceindexx, pieceindexy) && !end; i++)
                            {
                                if (Pieces[pieceindexx + i][pieceindexy - i] == 0)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy - i });
                                    if (searchKing == false)
                                    {
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + i + posX == kingLocation[0] && pieceindexy - i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx + i , pieceindexy - i });
                                                }
                                            }
                                        }
                                        if (pieceindexy == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexx + i);
                                    }
                                }
                                else if (pieceindexx + i == kingLocation[0] && pieceindexy - i == kingLocation[1])
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy - i });
                                    if (searchKing == false)
                                    {
                                        if (pieceindexx + i + 1 < 8 && pieceindexy - i - 1 >= 0)
                                        {
                                            NoKingZone.push_back({ pieceindexx + i + 1 , pieceindexy - i - 1 });
                                        }

                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy });
                                        CheckLines.push_back(temporaryMoveMemory);
                                        CheckingPiece.push_back({ pieceindexx , pieceindexy });
                                        numChecks++;
                                        end = true;
                                    }
                                    else
                                    {
                                        end = true;
                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy });
                                        PinLines.push_back(temporaryMoveMemory);
                                        PinnedPieces.push_back({ temporaryPinPieceData[0], temporaryPinPieceData[1] });
                                    }
                                }
                                else if (floor((float)Pieces[pieceindexx + i][pieceindexy - i] / 8.0f) == SIDEMOVE)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy - i });
                                    if (searchKing == false)
                                    {
                                        searchKing = true;
                                        temporaryPinPieceData[0] = pieceindexx + i;
                                        temporaryPinPieceData[1] = pieceindexy - i;
                                    }
                                    else end = true;
                                }
                                else
                                {
                                    end = true;
                                    if (searchKing == false)
                                    {
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + i + posX == kingLocation[0] && pieceindexy - i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx + i , pieceindexy - i });
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            end = false;
                            searchKing = false;
                            temporaryMoveMemory.erase(temporaryMoveMemory.begin(), temporaryMoveMemory.end());
                            temporaryPinPieceData[0] = 0;
                            temporaryPinPieceData[1] = 0;

                            for (int i = 1; i <= min(pieceindexx, 7 - pieceindexy) && !end; i++)
                            {
                                if (Pieces[pieceindexx - i][pieceindexy + i] == 0)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexy + i });
                                    if (searchKing == false)
                                    {
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx - i + posX == kingLocation[0] && pieceindexy + i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx - i , pieceindexy + i });
                                                }
                                            }
                                        }
                                        if (pieceindexy == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexx - i);
                                    }
                                }
                                else if (pieceindexx - i == kingLocation[0] && pieceindexy + i == kingLocation[1])
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexy + i });
                                    if (searchKing == false)
                                    {
                                        if (pieceindexx - i - 1 >= 0 && pieceindexy + i + 1 < 8)
                                        {
                                            NoKingZone.push_back({ pieceindexx - i - 1 , pieceindexy + i + 1 });
                                        }


                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy });
                                        CheckLines.push_back(temporaryMoveMemory);
                                        CheckingPiece.push_back({ pieceindexx , pieceindexy });
                                        numChecks++;
                                        end = true;
                                    }
                                    else
                                    {
                                        end = true;
                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy });
                                        PinLines.push_back(temporaryMoveMemory);
                                        PinnedPieces.push_back({ temporaryPinPieceData[0], temporaryPinPieceData[1] });
                                    }
                                }
                                else if (floor((float)Pieces[pieceindexx - i][pieceindexy + i] / 8.0f) == SIDEMOVE)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexx + i });
                                    if (searchKing == false)
                                    {
                                        searchKing = true;
                                        temporaryPinPieceData[0] = pieceindexx - i;
                                        temporaryPinPieceData[1] = pieceindexy + i;
                                    }
                                    else end = true;
                                }
                                else
                                {
                                    end = true;
                                    if (searchKing == false)
                                    {
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx - i + posX == kingLocation[0] && pieceindexy + i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx - i , pieceindexy + i });
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if (Pieces[pieceindexx][pieceindexy] - 7 * (!SIDEMOVE) == 3)
                        {
                            int CheckPos[2];
                            for (int dir = 0; dir < 2; dir++)
                            {
                                for (int i = -1; i <= 1; i += 2)
                                {
                                    for (int j = -1; j <= 1; j += 2)
                                    {
                                        if (dir == 0)
                                        {
                                            if ((pieceindexx + i * 2) >= 0 && (pieceindexx + i * 2) < 8 && (pieceindexy + j) < 8 && (pieceindexy + j) >= 0)
                                            {
                                                if (Pieces[pieceindexx + i * 2][pieceindexy + j] == 0 || (Pieces[pieceindexx + i * 2][pieceindexy + j] != 0 && floor((float)Pieces[pieceindexx + i * 2][pieceindexy + j] / 8.0f) == (!SIDEMOVE)))
                                                {
                                                    for (int posX = -1; posX <= 1; posX++)
                                                    {
                                                        for (int posY = -1; posY <= 1; posY++)
                                                        {
                                                            if (pieceindexx + i * 2 + posX == kingLocation[0] && pieceindexy + j + posY == kingLocation[1])
                                                            {
                                                                NoKingZone.push_back({ pieceindexx + i * 2 , pieceindexy + j });
                                                            }
                                                        }
                                                    }
                                                }
                                                else if (Pieces[pieceindexx + i * 2][pieceindexy + j] - 7 * SIDEMOVE == 6)
                                                {
                                                    numChecks++;
                                                    CheckingPiece.push_back({ pieceindexx, pieceindexy });
                                                    CheckLines.push_back({ { pieceindexx, pieceindexy } });
                                                }
                                            }
                                        }
                                        else
                                        {
                                            if ((pieceindexx + j) >= 0 && (pieceindexx + j) < 8 && (pieceindexy + i * 2) < 8 && (pieceindexy + i * 2) >= 0)
                                            {
                                                if (Pieces[pieceindexx + j][pieceindexy + i * 2] == 0 || (Pieces[pieceindexx + j][pieceindexy + i * 2] != 0 && floor((float)Pieces[pieceindexx + j][pieceindexy + i * 2] / 8.0f) == (!SIDEMOVE)))
                                                {
                                                    for (int posX = -1; posX <= 1; posX++)
                                                    {
                                                        for (int posY = -1; posY <= 1; posY++)
                                                        {
                                                            if (pieceindexx + j + posX == kingLocation[0] && pieceindexy + i * 2 + posY == kingLocation[1])
                                                            {
                                                                NoKingZone.push_back({ pieceindexx + j , pieceindexy + i * 2 });
                                                            }
                                                        }
                                                    }
                                                }
                                                if (Pieces[pieceindexx + j][pieceindexy + i * 2] - 7 * SIDEMOVE == 6)
                                                {
                                                    numChecks++;
                                                    CheckingPiece.push_back({ pieceindexx, pieceindexy });
                                                    CheckLines.push_back({ { pieceindexx, pieceindexy } });
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if (Pieces[pieceindexx][pieceindexy] - 7 * (!SIDEMOVE) == 4 || Pieces[pieceindexx][pieceindexy] - 7 * (!SIDEMOVE) == 5)
                        {

                            vector<vector<int>> temporaryMoveMemory;
                            vector<int> temporaryPinPieceData = { -1, -1 };
                            bool searchKing = false;
                            bool end = false;

                            temporaryMoveMemory.push_back({ pieceindexx, pieceindexy });

                            for (int i = 1; i < 8 - pieceindexx && !end; i++)
                            {
                                if (Pieces[pieceindexx + i][pieceindexy] == 0)
                                {
                                    if (searchKing == false)
                                    {
                                        temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy });
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + i + posX == kingLocation[0] && pieceindexy + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx + i , pieceindexy });
                                                }
                                            }
                                        }

                                        if (pieceindexy == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexx + i);
                                    }
                                }
                                else if (pieceindexx + i == kingLocation[0] && pieceindexy == kingLocation[1])
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy });
                                    if (searchKing == false)
                                    {
                                        if (pieceindexx + i + 1 < 8)
                                        {
                                            NoKingZone.push_back({ pieceindexx + i + 1 , pieceindexy });
                                        }
                                        CheckingPiece.push_back({ pieceindexx , pieceindexy });
                                        CheckLines.push_back(temporaryMoveMemory);
                                        numChecks++;
                                        end = true;
                                    }
                                    else
                                    {
                                        end = true;
                                        temporaryMoveMemory.push_back({ pieceindexx, pieceindexy });
                                        PinLines.push_back(temporaryMoveMemory);
                                        PinnedPieces.push_back({ temporaryPinPieceData[0], temporaryPinPieceData[1] });
                                    }
                                }
                                else if (floor((float)Pieces[pieceindexx + i][pieceindexy] / 8.0f) == SIDEMOVE)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy });
                                    if (searchKing == false)
                                    {
                                        temporaryPinPieceData[0] = pieceindexx + i;
                                        temporaryPinPieceData[1] = pieceindexy;
                                        searchKing = true;
                                    }
                                    else end = true;
                                }
                                else
                                {
                                    end = true;
                                    if (searchKing == false)
                                    {
                                        temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy });
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + i + posX == kingLocation[0] && pieceindexy + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx + i , pieceindexy });
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            temporaryMoveMemory.erase(temporaryMoveMemory.begin(), temporaryMoveMemory.end());
                            temporaryMoveMemory.push_back({ pieceindexx, pieceindexy });
                            temporaryPinPieceData[0] = 0;
                            temporaryPinPieceData[1] = 0;
                            searchKing = false;
                            end = false;
                            for (int i = 1; i < 8 - pieceindexy && !end; i++)
                            {
                                if (Pieces[pieceindexx][pieceindexy + i] == 0)
                                {
                                    if (searchKing == false)
                                    {
                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy + i });
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + posX == kingLocation[0] && pieceindexy + i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx , pieceindexy + i });
                                                }
                                            }
                                        }
                                        if (pieceindexy + i == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexx);
                                    }
                                }
                                else if (pieceindexx == kingLocation[0] && pieceindexy + i == kingLocation[1])
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx , pieceindexy + i });
                                    if (searchKing == false)
                                    {

                                        if (pieceindexy + i + 1 < 8)
                                        {
                                            NoKingZone.push_back({ pieceindexx , pieceindexy + i + 1 });
                                        }


                                        CheckingPiece.push_back({ pieceindexx, pieceindexy });
                                        CheckLines.push_back(temporaryMoveMemory);
                                        numChecks++;
                                        end = true;
                                    }
                                    else
                                    {
                                        end = true;

                                        PinLines.push_back(temporaryMoveMemory);
                                        PinnedPieces.push_back({ temporaryPinPieceData[0], temporaryPinPieceData[1] });
                                    }
                                }
                                else if (floor((float)Pieces[pieceindexx][pieceindexy + i] / 8.0f) == SIDEMOVE)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx , pieceindexy + i });
                                    if (searchKing == false)
                                    {
                                        temporaryPinPieceData[0] = pieceindexx;
                                        temporaryPinPieceData[1] = pieceindexy + i;
                                        searchKing = true;
                                    }
                                    else end = true;
                                }
                                else
                                {
                                    end = true;
                                    if (searchKing == false)
                                    {
                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy + i });
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + posX == kingLocation[0] && pieceindexy + i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx , pieceindexy + i });
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            temporaryMoveMemory.erase(temporaryMoveMemory.begin(), temporaryMoveMemory.end());
                            temporaryMoveMemory.push_back({ pieceindexx, pieceindexy });
                            temporaryPinPieceData[0] = 0;
                            temporaryPinPieceData[1] = 0;
                            searchKing = false;
                            end = false;
                            for (int i = 1; i <= pieceindexy && !end; i++)
                            {
                                if (Pieces[pieceindexx][pieceindexy - i] == 0)
                                {
                                    if (searchKing == false)
                                    {
                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy - i });
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + posX == kingLocation[0] && pieceindexy - i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx , pieceindexy - i });
                                                }
                                            }
                                        }
                                        if (pieceindexy - i == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexx);
                                    }
                                }
                                else if (pieceindexx == kingLocation[0] && pieceindexy - i == kingLocation[1])
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx , pieceindexy - i });
                                    if (searchKing == false)
                                    {
                                        if (pieceindexy - i - 1 >= 0)
                                        {
                                            NoKingZone.push_back({ pieceindexx , pieceindexy - i - 1 });
                                        }
                                        CheckingPiece.push_back({ pieceindexx, pieceindexy });
                                        CheckLines.push_back(temporaryMoveMemory);
                                        numChecks++;
                                        end = true;
                                    }
                                    else
                                    {
                                        end = true;
                                        PinLines.push_back(temporaryMoveMemory);
                                        PinnedPieces.push_back({ temporaryPinPieceData[0], temporaryPinPieceData[1] });
                                    }
                                }
                                else if (floor((float)Pieces[pieceindexx][pieceindexy - i] / 8.0f) == SIDEMOVE)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx , pieceindexy - i });
                                    if (searchKing == false)
                                    {
                                        temporaryPinPieceData[0] = pieceindexx;
                                        temporaryPinPieceData[1] = pieceindexy - i;
                                        searchKing = true;
                                    }
                                    else
                                    {
                                        end = true;

                                    }
                                }
                                else
                                {
                                    end = true;
                                    if (searchKing == false)
                                    {

                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy - i });
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + posX == kingLocation[0] && pieceindexy - i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx , pieceindexy - i });
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            temporaryMoveMemory.erase(temporaryMoveMemory.begin(), temporaryMoveMemory.end());
                            temporaryMoveMemory.push_back({ pieceindexx, pieceindexy });
                            temporaryPinPieceData[0] = 0;
                            temporaryPinPieceData[1] = 0;
                            searchKing = false;
                            end = false;
                            for (int i = 1; i <= pieceindexx && !end; i++)
                            {
                                if (Pieces[pieceindexx - i][pieceindexy] == 0)
                                {
                                    if (searchKing == false)
                                    {
                                        temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexy });
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx - i + posX == kingLocation[0] && pieceindexy + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx - i , pieceindexy });
                                                }
                                            }
                                        }
                                    }

                                    if (pieceindexy == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexx - i);
                                }
                                else if (pieceindexx - i == kingLocation[0] && pieceindexy == kingLocation[1])
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx - i, pieceindexy });
                                    if (searchKing == false)
                                    {
                                        if (pieceindexx - i - 1 >= 0)
                                        {
                                            NoKingZone.push_back({ pieceindexx - i - 1 , pieceindexy });
                                        }

                                        temporaryMoveMemory.push_back({ pieceindexx, pieceindexy });
                                        CheckingPiece.push_back({ pieceindexx, pieceindexy });
                                        CheckLines.push_back(temporaryMoveMemory);
                                        numChecks++;
                                        end = true;
                                    }
                                    else
                                    {
                                        end = true;
                                        PinLines.push_back(temporaryMoveMemory);
                                        PinnedPieces.push_back({ temporaryPinPieceData[0], temporaryPinPieceData[1] });
                                    }
                                }
                                else if (floor((float)Pieces[pieceindexx - i][pieceindexy] / 8.0f) == SIDEMOVE)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexy });
                                    if (searchKing == false)
                                    {
                                        temporaryPinPieceData[0] = pieceindexx - i;
                                        temporaryPinPieceData[1] = pieceindexy;
                                        searchKing = true;
                                    }
                                    else end = true;
                                }
                                else
                                {
                                    end = true;
                                    if (searchKing == false)
                                    {
                                        temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexy });
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx - i + posX == kingLocation[0] && pieceindexy + posY == kingLocation[1])
                                                {

                                                    NoKingZone.push_back({ pieceindexx - i , pieceindexy });
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if (Pieces[pieceindexx][pieceindexy] - 7 * (!SIDEMOVE) == 1)
                        {
                            if (pieceindexx - 1 >= 0)
                            {
                                if (Pieces[pieceindexx - 1][pieceindexy - 1 + 2 * (!SIDEMOVE)] - 7 * SIDEMOVE == 6)
                                {
                                    CheckLines.push_back({ { pieceindexx , pieceindexy } });
                                    CheckingPiece.push_back({ pieceindexx , pieceindexy });
                                }
                                else
                                {
                                    for (int posX = -1; posX <= 1; posX++)
                                    {
                                        for (int posY = -1; posY <= 1; posY++)
                                        {
                                            if (pieceindexx - 1 + posX == kingLocation[0] && pieceindexy - 1 + 2 * (!SIDEMOVE) + posY == kingLocation[1])
                                            {
                                                NoKingZone.push_back({ pieceindexx - 1 , pieceindexy - 1 + 2 * (!SIDEMOVE) });
                                            }
                                        }
                                    }

                                    if (pieceindexy == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexx - 1);
                                }
                            }

                            if (pieceindexx + 1 < 8)
                            {
                                if (Pieces[pieceindexx + 1][pieceindexy - 1 + 2 * (!SIDEMOVE)] - 7 * SIDEMOVE == 6)
                                {
                                    CheckLines.push_back({ { pieceindexx , pieceindexy } });
                                    CheckingPiece.push_back({ pieceindexx , pieceindexy });
                                }
                                else
                                {
                                    for (int posX = -1; posX <= 1; posX++)
                                    {
                                        for (int posY = -1; posY <= 1; posY++)
                                        {
                                            if (pieceindexx + 1 + posX == kingLocation[0] && pieceindexy - 1 + 2 * (!SIDEMOVE) + posY == kingLocation[1])
                                            {
                                                NoKingZone.push_back({ pieceindexx + 1 , pieceindexy - 1 + 2 * (!SIDEMOVE) });
                                            }
                                        }
                                    }

                                    if (pieceindexy - 1 + 2 * (!SIDEMOVE) == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexx + 1);
                                }
                            }
                        }
                        else if (Pieces[pieceindexx][pieceindexy] - 7 * (!SIDEMOVE) == 7)
                        {
                            vector<vector<int>> temporaryMoveMemory;
                            temporaryMoveMemory.push_back({ pieceindexx , pieceindexy });
                            vector<int> temporaryPinPieceData = { -1, -1 };
                            //bishops
                            bool end = false;
                            bool searchKing = false;
                            for (int i = 1; i <= 7 - max(pieceindexx, pieceindexy) && !end; i++)
                            {
                                if (Pieces[pieceindexx + i][pieceindexy + i] == 0)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy + i });
                                    if (searchKing == false)
                                    {
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + i + posX == kingLocation[0] && pieceindexy + i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx + i , pieceindexy + i });
                                                }
                                            }
                                        }
                                        if (pieceindexy == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexx + i);
                                    }
                                }
                                else if (pieceindexx + i == kingLocation[0] && pieceindexy + i == kingLocation[1])
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy + i });
                                    if (searchKing == false)
                                    {
                                        if (pieceindexx + i + 1 < 8 && pieceindexy + i + 1 < 8)
                                        {
                                            NoKingZone.push_back({ pieceindexx + i + 1 , pieceindexy + i + 1 });
                                        }
                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy });
                                        CheckLines.push_back(temporaryMoveMemory);



                                        CheckingPiece.push_back({ pieceindexx , pieceindexy });
                                        end = true;
                                    }
                                    else
                                    {

                                        end = true;
                                        PinLines.push_back(temporaryMoveMemory);
                                        PinnedPieces.push_back({ temporaryPinPieceData[0], temporaryPinPieceData[1] });
                                    }
                                }
                                else if (floor((float)Pieces[pieceindexx + i][pieceindexy + i] / 8.0f) == SIDEMOVE)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy + i });
                                    if (searchKing == false)
                                    {
                                        temporaryPinPieceData[0] = pieceindexx + i;
                                        temporaryPinPieceData[1] = pieceindexy + i;
                                        searchKing = true;
                                    }
                                    else end = true;
                                }
                                else
                                {
                                    end = true;
                                    if (searchKing == false)
                                    {
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + i + posX == kingLocation[0] && pieceindexy + i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx + i , pieceindexy + i });
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            end = false;
                            searchKing = false;
                            temporaryMoveMemory.erase(temporaryMoveMemory.begin(), temporaryMoveMemory.end());
                            temporaryPinPieceData[0] = 0;
                            temporaryPinPieceData[1] = 0;

                            for (int i = 1; i <= min(pieceindexx, pieceindexy) && !end; i++)
                            {
                                if (Pieces[pieceindexx - i][pieceindexy - i] == 0)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexy - i });
                                    if (searchKing == false)
                                    {
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx - i + posX == kingLocation[0] && pieceindexy - i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx - i , pieceindexy - i });
                                                }
                                            }
                                        }
                                        if (pieceindexy == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexy - i);
                                    }
                                }
                                else if (pieceindexx - i == kingLocation[0] && pieceindexy - i == kingLocation[1])
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexy - i });
                                    if (searchKing == false)
                                    {
                                        if (pieceindexx - i - 1 >= 0 && pieceindexy - i - 1 >= 0)
                                        {
                                            NoKingZone.push_back({ pieceindexx - i - 1 , pieceindexy - i - 1 });
                                        }

                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy });
                                        CheckingPiece.push_back({ pieceindexx , pieceindexy });
                                        numChecks++;
                                        CheckLines.push_back(temporaryMoveMemory);
                                        end = true;
                                    }
                                    else
                                    {
                                        end = true;
                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy });
                                        PinLines.push_back(temporaryMoveMemory);
                                        PinnedPieces.push_back({ temporaryPinPieceData[0], temporaryPinPieceData[1] });
                                    }
                                }
                                else if (floor((float)Pieces[pieceindexx - i][pieceindexy - i] / 8.0f) == SIDEMOVE)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexy - i });
                                    if (searchKing == false)
                                    {
                                        searchKing = true;
                                        temporaryPinPieceData[0] = pieceindexx - i;
                                        temporaryPinPieceData[1] = pieceindexy - i;
                                    }
                                    else end = true;
                                }
                                else
                                {
                                    end = true;
                                    if (searchKing == false)
                                    {
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx - i + posX == kingLocation[0] && pieceindexy - i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx - i , pieceindexy - i });
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            end = false;
                            searchKing = false;
                            temporaryMoveMemory.erase(temporaryMoveMemory.begin(), temporaryMoveMemory.end());
                            temporaryPinPieceData[0] = 0;
                            temporaryPinPieceData[1] = 0;

                            for (int i = 1; i <= min(7 - pieceindexx, pieceindexy) && !end; i++)
                            {
                                if (Pieces[pieceindexx + i][pieceindexy - i] == 0)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy - i });
                                    if (searchKing == false)
                                    {
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + i + posX == kingLocation[0] && pieceindexy - i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx + i , pieceindexy - i });
                                                }
                                            }
                                        }
                                        if (pieceindexy == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexx + i);
                                    }
                                }
                                else if (pieceindexx + i == kingLocation[0] && pieceindexy - i == kingLocation[1])
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy - i });
                                    if (searchKing == false)
                                    {
                                        if (pieceindexx + i + 1 < 8 && pieceindexy - i - 1 >= 0)
                                        {
                                            NoKingZone.push_back({ pieceindexx + i + 1 , pieceindexy - i - 1 });
                                        }

                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy });
                                        CheckLines.push_back(temporaryMoveMemory);
                                        CheckingPiece.push_back({ pieceindexx , pieceindexy });
                                        numChecks++;
                                        end = true;
                                    }
                                    else
                                    {
                                        end = true;
                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy });
                                        PinLines.push_back(temporaryMoveMemory);
                                        PinnedPieces.push_back({ temporaryPinPieceData[0], temporaryPinPieceData[1] });
                                    }
                                }
                                else if (floor((float)Pieces[pieceindexx + i][pieceindexy - i] / 8.0f) == SIDEMOVE)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy - i });
                                    if (searchKing == false)
                                    {
                                        searchKing = true;
                                        temporaryPinPieceData[0] = pieceindexx + i;
                                        temporaryPinPieceData[1] = pieceindexy - i;
                                    }
                                    else end = true;
                                }
                                else
                                {
                                    end = true;
                                    if (searchKing == false)
                                    {
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + i + posX == kingLocation[0] && pieceindexy - i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx + i , pieceindexy - i });
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            end = false;
                            searchKing = false;
                            temporaryMoveMemory.erase(temporaryMoveMemory.begin(), temporaryMoveMemory.end());
                            temporaryPinPieceData[0] = 0;
                            temporaryPinPieceData[1] = 0;

                            for (int i = 1; i <= min(pieceindexx, 7 - pieceindexy) && !end; i++)
                            {
                                if (Pieces[pieceindexx - i][pieceindexy + i] == 0)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexy + i });
                                    if (searchKing == false)
                                    {
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx - i + posX == kingLocation[0] && pieceindexy + i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx - i , pieceindexy + i });
                                                }
                                            }
                                        }
                                        if (pieceindexy == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexx - i);
                                    }
                                }
                                else if (pieceindexx - i == kingLocation[0] && pieceindexy + i == kingLocation[1])
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexy + i });
                                    if (searchKing == false)
                                    {
                                        if (pieceindexx - i - 1 >= 0 && pieceindexy + i + 1 < 8)
                                        {
                                            NoKingZone.push_back({ pieceindexx - i - 1 , pieceindexy + i + 1 });
                                        }


                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy });
                                        CheckLines.push_back(temporaryMoveMemory);
                                        CheckingPiece.push_back({ pieceindexx , pieceindexy });
                                        numChecks++;
                                        end = true;
                                    }
                                    else
                                    {
                                        end = true;
                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy });
                                        PinLines.push_back(temporaryMoveMemory);
                                        PinnedPieces.push_back({ temporaryPinPieceData[0], temporaryPinPieceData[1] });
                                    }
                                }
                                else if (floor((float)Pieces[pieceindexx - i][pieceindexy + i] / 8.0f) == SIDEMOVE)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexx + i });
                                    if (searchKing == false)
                                    {
                                        searchKing = true;
                                        temporaryPinPieceData[0] = pieceindexx - i;
                                        temporaryPinPieceData[1] = pieceindexy + i;
                                    }
                                    else end = true;
                                }
                                else
                                {
                                    end = true;
                                    if (searchKing == false)
                                    {
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx - i + posX == kingLocation[0] && pieceindexy + i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx - i , pieceindexy + i });
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            end = false;
                            searchKing = false;
                            temporaryMoveMemory.erase(temporaryMoveMemory.begin(), temporaryMoveMemory.end());
                            temporaryMoveMemory.push_back({ pieceindexx , pieceindexy });
                            temporaryPinPieceData[0] = 0;
                            temporaryPinPieceData[1] = 0;

                            //rook part of the queen
                            //
                            ///
                            //


                            for (int i = 1; i < 8 - pieceindexx && !end; i++)
                            {
                                if (Pieces[pieceindexx + i][pieceindexy] == 0)
                                {
                                    if (searchKing == false)
                                    {
                                        temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy });
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + i + posX == kingLocation[0] && pieceindexy + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx + i , pieceindexy });
                                                }
                                            }
                                        }

                                        if (pieceindexy == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexx + i);
                                    }
                                }
                                else if (pieceindexx + i == kingLocation[0] && pieceindexy == kingLocation[1])
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy });
                                    if (searchKing == false)
                                    {
                                        if (pieceindexx + i + 1 < 8)
                                        {
                                            NoKingZone.push_back({ pieceindexx + i + 1 , pieceindexy });
                                        }
                                        CheckingPiece.push_back({ pieceindexx , pieceindexy });
                                        CheckLines.push_back(temporaryMoveMemory);
                                        numChecks++;
                                        end = true;
                                    }
                                    else
                                    {
                                        end = true;
                                        temporaryMoveMemory.push_back({ pieceindexx, pieceindexy });
                                        PinLines.push_back(temporaryMoveMemory);
                                        PinnedPieces.push_back({ temporaryPinPieceData[0], temporaryPinPieceData[1] });
                                    }
                                }
                                else if (floor((float)Pieces[pieceindexx + i][pieceindexy] / 8.0f) == SIDEMOVE)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy });
                                    if (searchKing == false)
                                    {
                                        temporaryPinPieceData[0] = pieceindexx + i;
                                        temporaryPinPieceData[1] = pieceindexy;
                                        searchKing = true;
                                    }
                                    else end = true;
                                }
                                else
                                {
                                    end = true;
                                    if (searchKing == false)
                                    {
                                        temporaryMoveMemory.push_back({ pieceindexx + i , pieceindexy });
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + i + posX == kingLocation[0] && pieceindexy + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx + i , pieceindexy });
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            temporaryMoveMemory.erase(temporaryMoveMemory.begin(), temporaryMoveMemory.end());
                            temporaryMoveMemory.push_back({ pieceindexx, pieceindexy });
                            temporaryPinPieceData[0] = 0;
                            temporaryPinPieceData[1] = 0;
                            searchKing = false;
                            end = false;
                            for (int i = 1; i < 8 - pieceindexy && !end; i++)
                            {
                                if (Pieces[pieceindexx][pieceindexy + i] == 0)
                                {
                                    if (searchKing == false)
                                    {
                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy + i });
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + posX == kingLocation[0] && pieceindexy + i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx , pieceindexy + i });
                                                }
                                            }
                                        }
                                        if (pieceindexy + i == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexx);
                                    }
                                }
                                else if (pieceindexx == kingLocation[0] && pieceindexy + i == kingLocation[1])
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx , pieceindexy + i });
                                    if (searchKing == false)
                                    {

                                        if (pieceindexy + i + 1 < 8)
                                        {
                                            NoKingZone.push_back({ pieceindexx , pieceindexy + i + 1 });
                                        }


                                        CheckingPiece.push_back({ pieceindexx, pieceindexy });
                                        CheckLines.push_back(temporaryMoveMemory);
                                        numChecks++;
                                        end = true;
                                    }
                                    else
                                    {
                                        end = true;

                                        PinLines.push_back(temporaryMoveMemory);
                                        PinnedPieces.push_back({ temporaryPinPieceData[0], temporaryPinPieceData[1] });
                                    }
                                }
                                else if (floor((float)Pieces[pieceindexx][pieceindexy + i] / 8.0f) == SIDEMOVE)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx , pieceindexy + i });
                                    if (searchKing == false)
                                    {
                                        temporaryPinPieceData[0] = pieceindexx;
                                        temporaryPinPieceData[1] = pieceindexy + i;
                                        searchKing = true;
                                    }
                                    else end = true;
                                }
                                else
                                {
                                    end = true;
                                    if (searchKing == false)
                                    {
                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy + i });
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + posX == kingLocation[0] && pieceindexy + i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx , pieceindexy + i });
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            temporaryMoveMemory.erase(temporaryMoveMemory.begin(), temporaryMoveMemory.end());
                            temporaryMoveMemory.push_back({ pieceindexx, pieceindexy });
                            temporaryPinPieceData[0] = 0;
                            temporaryPinPieceData[1] = 0;
                            searchKing = false;
                            end = false;
                            for (int i = 1; i <= pieceindexy && !end; i++)
                            {
                                if (Pieces[pieceindexx][pieceindexy - i] == 0)
                                {
                                    if (searchKing == false)
                                    {
                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy - i });
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + posX == kingLocation[0] && pieceindexy - i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx , pieceindexy - i });
                                                }
                                            }
                                        }
                                        if (pieceindexy - i == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexx);
                                    }
                                }
                                else if (pieceindexx == kingLocation[0] && pieceindexy - i == kingLocation[1])
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx , pieceindexy - i });
                                    if (searchKing == false)
                                    {
                                        if (pieceindexy - i - 1 >= 0)
                                        {
                                            NoKingZone.push_back({ pieceindexx , pieceindexy - i - 1 });
                                        }
                                        CheckingPiece.push_back({ pieceindexx, pieceindexy });
                                        CheckLines.push_back(temporaryMoveMemory);
                                        numChecks++;
                                        end = true;
                                    }
                                    else
                                    {
                                        end = true;
                                        PinLines.push_back(temporaryMoveMemory);
                                        PinnedPieces.push_back({ temporaryPinPieceData[0], temporaryPinPieceData[1] });
                                    }
                                }
                                else if (floor((float)Pieces[pieceindexx][pieceindexy - i] / 8.0f) == SIDEMOVE)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx , pieceindexy - i });
                                    if (searchKing == false)
                                    {
                                        temporaryPinPieceData[0] = pieceindexx;
                                        temporaryPinPieceData[1] = pieceindexy - i;
                                        searchKing = true;
                                    }
                                    else
                                    {
                                        end = true;

                                    }
                                }
                                else
                                {
                                    end = true;
                                    if (searchKing == false)
                                    {

                                        temporaryMoveMemory.push_back({ pieceindexx , pieceindexy - i });
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx + posX == kingLocation[0] && pieceindexy - i + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx , pieceindexy - i });
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            temporaryMoveMemory.erase(temporaryMoveMemory.begin(), temporaryMoveMemory.end());
                            temporaryMoveMemory.push_back({ pieceindexx, pieceindexy });
                            temporaryPinPieceData[0] = 0;
                            temporaryPinPieceData[1] = 0;
                            searchKing = false;
                            end = false;
                            for (int i = 1; i <= pieceindexx && !end; i++)
                            {
                                if (Pieces[pieceindexx - i][pieceindexy] == 0)
                                {
                                    if (searchKing == false)
                                    {
                                        temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexy });
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx - i + posX == kingLocation[0] && pieceindexy + posY == kingLocation[1])
                                                {
                                                    NoKingZone.push_back({ pieceindexx - i , pieceindexy });
                                                }
                                            }
                                        }
                                    }

                                    if (pieceindexy == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexx - i);
                                }
                                else if (pieceindexx - i == kingLocation[0] && pieceindexy == kingLocation[1])
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx - i, pieceindexy });
                                    if (searchKing == false)
                                    {
                                        if (pieceindexx - i - 1 >= 0)
                                        {
                                            NoKingZone.push_back({ pieceindexx - i - 1 , pieceindexy });
                                        }

                                        temporaryMoveMemory.push_back({ pieceindexx, pieceindexy });
                                        CheckingPiece.push_back({ pieceindexx, pieceindexy });
                                        CheckLines.push_back(temporaryMoveMemory);
                                        numChecks++;
                                        end = true;
                                    }
                                    else
                                    {
                                        end = true;
                                        PinLines.push_back(temporaryMoveMemory);
                                        PinnedPieces.push_back({ temporaryPinPieceData[0], temporaryPinPieceData[1] });
                                    }
                                }
                                else if (floor((float)Pieces[pieceindexx - i][pieceindexy] / 8.0f) == SIDEMOVE)
                                {
                                    temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexy });
                                    if (searchKing == false)
                                    {
                                        temporaryPinPieceData[0] = pieceindexx - i;
                                        temporaryPinPieceData[1] = pieceindexy;
                                        searchKing = true;
                                    }
                                    else end = true;
                                }
                                else
                                {
                                    end = true;
                                    if (searchKing == false)
                                    {
                                        temporaryMoveMemory.push_back({ pieceindexx - i , pieceindexy });
                                        for (int posX = -1; posX <= 1; posX++)
                                        {
                                            for (int posY = -1; posY <= 1; posY++)
                                            {
                                                if (pieceindexx - i + posX == kingLocation[0] && pieceindexy + posY == kingLocation[1])
                                                {

                                                    NoKingZone.push_back({ pieceindexx - i , pieceindexy });
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else if (Pieces[pieceindexx][pieceindexy] - 7 * (!SIDEMOVE) == 6)
                        {
                            for (int posX = -1; posX <= 1; posX++)
                            {
                                for (int posY = -1; posY <= 1; posY++)
                                {
                                    for (int posotherkX = -1; posotherkX <= 1; posotherkX++)
                                    {
                                        for (int posotherY = -1; posotherY <= 1; posotherY++)
                                        {
                                            if (posX != 0 || posY != 0)
                                            {
                                                for (int posotherkX = -1; posotherkX <= 1; posotherkX++)
                                                {
                                                    for (int posotherY = -1; posotherY <= 1; posotherY++)
                                                    {
                                                        if (pieceindexx + posX + posotherkX == kingLocation[0] && pieceindexy + posY + posotherY == kingLocation[1])
                                                        {
                                                            NoKingZone.push_back({ pieceindexx + posX , pieceindexy + posY });
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    if (pieceindexy + posY == (!SIDEMOVE) * 7) NoCastleZone.push_back(pieceindexx + posX);
                                }
                            }
                        }
                    }
                }
            }
        }



        //if there is a double check, the only move you can make is moving the king

        if (CheckingPiece.size() >= 2)
        {
            vector<vector<int>> moves;


            moves.push_back({ kingLocation[0], kingLocation[1] });
            for (int posX = -1; posX <= 1; posX++)
            {
                for (int posY = -1; posY <= 1; posY++)
                {
                    if (posX != 0 || posY != 0)
                    {
                        bool cangoThere = true;
                        if (NoKingZone.size() > 0)
                        {
                            for (int idx = 0; idx < NoKingZone.size(); idx++)
                            {
                                if (kingLocation[0] + posX >= 0 && kingLocation[0] + posX < 8 && kingLocation[1] + posY >= 0 && kingLocation[1] + posY < 8)
                                {
                                    if ((kingLocation[0] + posX == NoKingZone.at(idx)[0] && kingLocation[1] + posY == NoKingZone.at(idx)[1]))
                                    {
                                        cangoThere = false;
                                    }
                                }
                            }
                        }
                        if (CheckingPiece.size() == 1)
                        {
                            for (int idx = 0; idx < CheckLines[0].size(); idx++)
                            {
                                if (kingLocation[0] + posX >= 0 && kingLocation[0] + posX < 8 && kingLocation[1] + posY >= 0 && kingLocation[1] + posY < 8)
                                {
                                    if ((kingLocation[0] + posX == CheckLines[0].at(idx)[0] && kingLocation[1] + posY == CheckLines[0].at(idx)[1]) && !(Pieces[kingLocation[0] + posX][kingLocation[1] + posY] != 0 && floor((float)Pieces[kingLocation[0] + posX][kingLocation[1] + posY] / 8.0f) != SIDEMOVE))
                                    {

                                        cangoThere = false;
                                    }

                                }
                            }
                        }
                        if (cangoThere && kingLocation[0] + posX >= 0 && kingLocation[0] + posX < 8 && kingLocation[1] + posY >= 0 && kingLocation[1] + posY < 8)
                        {
                            if (!(Pieces[kingLocation[0] + posX][kingLocation[1] + posY] != 0 && floor((float)Pieces[kingLocation[0] + posX][kingLocation[1] + posY] / 8.0f) == SIDEMOVE))
                            {
                                moves.push_back({ kingLocation[0] + posX, kingLocation[1] + posY });
                            }

                        }
                        cangoThere = true;
                    }
                }
            }
            if (moves.size() > 1)
            {
                PieceMoves.push_back(moves);
            }
        }
        else
        {
            for (int pieceindexx = 0; pieceindexx < 8; pieceindexx++)
            {
                for (int pieceindexy = 0; pieceindexy < 8; pieceindexy++)
                {
                    int piece = Pieces[pieceindexx][pieceindexy];
                    if (Pieces[pieceindexx][pieceindexy] != 0)
                    {
                        //check for other piece pins and checks
                        if (floor((float)Pieces[pieceindexx][pieceindexy] / 8.0f) == SIDEMOVE)
                        {
                            bool isPinned = false;

                            vector<vector<int>> PinLine;
                            for (int i = 0; i < 8 && !isPinned; i++)
                            {
                                for (int j = 0; j < 8 && !isPinned; j++)
                                {
                                    for (int idx = 0; idx < PinnedPieces.size() && !isPinned; idx++)
                                    {
                                        isPinned = ((pieceindexx == PinnedPieces.at(idx)[0]) && pieceindexy == PinnedPieces.at(idx)[1]);
                                        if (isPinned) PinLine = PinLines.at(idx);
                                    }
                                }
                            }

                            vector<vector<int>> moves;


                            moves.push_back({ pieceindexx, pieceindexy });

                            bool end = false;
                            //do moves

                            if (!isPinned || CheckingPiece.size() != 1)
                            {
                                if (Pieces[pieceindexx][pieceindexy] - 7 * (SIDEMOVE) == 2)
                                {

                                    for (int i = 1; i <= 7 - max(pieceindexx, pieceindexy) && !end; i++)
                                    {
                                        if (Pieces[pieceindexx + i][pieceindexy + i] == 0)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx + i == PinLine.at(idx)[0] && pieceindexy + i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx + i, pieceindexy + i });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx + i == (CheckLines.at(0)[idx])[0] && pieceindexy + i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx + i, pieceindexy + i });
                                                }
                                            }
                                            else
                                                moves.push_back({ pieceindexx + i, pieceindexy + i });
                                        }
                                        else if (floor((float)Pieces[pieceindexx + i][pieceindexy + i] / 8.0f) != SIDEMOVE)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx + i == PinLine.at(idx)[0] && pieceindexy + i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx + i, pieceindexy + i });
                                                }
                                                end = true;
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx + i == (CheckLines.at(0)[idx])[0] && pieceindexy + i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx + i, pieceindexy + i });
                                                }
                                            }
                                            else
                                            {
                                                moves.push_back({ pieceindexx + i, pieceindexy + i });

                                            }
                                            end = true;

                                        }
                                        else end = true;
                                    }

                                    end = false;

                                    for (int i = 1; i <= min(pieceindexx, pieceindexy) && !end; i++)
                                    {
                                        if (Pieces[pieceindexx - i][pieceindexy - i] == 0)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx - i == PinLine.at(idx)[0] && pieceindexy - i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx - i, pieceindexy - i });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx - i == (CheckLines.at(0)[idx])[0] && pieceindexy - i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx - i, pieceindexy - i });
                                                }
                                            }
                                            else
                                                moves.push_back({ pieceindexx - i, pieceindexy - i });
                                        }
                                        else if (floor((float)Pieces[pieceindexx - i][pieceindexy - i] / 8.0f) != SIDEMOVE)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx - i == PinLine.at(idx)[0] && pieceindexy - i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx - i, pieceindexy - i });
                                                }
                                                end = true;
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx - i == (CheckLines.at(0)[idx])[0] && pieceindexy - i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx - i, pieceindexy - i });
                                                }
                                            }
                                            else
                                            {
                                                moves.push_back({ pieceindexx - i, pieceindexy - i });

                                            }
                                            end = true;
                                        }
                                        else end = true;
                                    }
                                    end = false;

                                    for (int i = 1; i <= min(7 - pieceindexx, pieceindexy) && !end; i++)
                                    {
                                        if (Pieces[pieceindexx + i][pieceindexy - i] == 0)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx + i == PinLine.at(idx)[0] && pieceindexy - i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx + i, pieceindexy - i });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx + i == (CheckLines.at(0)[idx])[0] && pieceindexy - i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx + i, pieceindexy - i });
                                                }
                                            }
                                            else
                                                moves.push_back({ pieceindexx + i, pieceindexy - i });
                                        }
                                        else if (floor((float)Pieces[pieceindexx + i][pieceindexy - i] / 8.0f) != SIDEMOVE)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx + i == PinLine.at(idx)[0] && pieceindexy - i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx + i, pieceindexy - i });
                                                }
                                                end = true;
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx + i == (CheckLines.at(0)[idx])[0] && pieceindexy - i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx + i, pieceindexy - i });
                                                }
                                            }
                                            else
                                            {
                                                moves.push_back({ pieceindexx + i, pieceindexy - i });

                                            }
                                            end = true;
                                        }
                                        else end = true;
                                    }

                                    end = false;
                                    for (int i = 1; i <= min(pieceindexx, 7 - pieceindexy) && !end; i++)
                                    {
                                        if (Pieces[pieceindexx - i][pieceindexy + i] == 0)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx - i == PinLine.at(idx)[0] && pieceindexy + i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx - i, pieceindexy + i });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx - i == (CheckLines.at(0)[idx])[0] && pieceindexy + i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx - i, pieceindexy + i });
                                                }
                                            }
                                            else
                                                moves.push_back({ pieceindexx - i, pieceindexy + i });
                                        }
                                        else if (floor((float)Pieces[pieceindexx - i][pieceindexy + i] / 8.0f) != SIDEMOVE)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx - i == PinLine.at(idx)[0] && pieceindexy + i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx - i, pieceindexy + i });
                                                }
                                                end = true;
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx - i == (CheckLines.at(0)[idx])[0] && pieceindexy + i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx - i, pieceindexy + i });
                                                }
                                            }
                                            else
                                            {
                                                moves.push_back({ pieceindexx - i, pieceindexy + i });

                                            }
                                            end = true;
                                        }
                                        else end = true;
                                    }

                                }
                                else if (Pieces[pieceindexx][pieceindexy] - 7 * (SIDEMOVE) == 3)
                                {
                                    int CheckPos[2];
                                    for (int dir = 0; dir < 2; dir++)
                                    {
                                        for (int i = -1; i <= 1; i += 2)
                                        {
                                            for (int j = -1; j <= 1; j += 2)
                                            {
                                                if (!isPinned)
                                                {
                                                    if (dir == 0)
                                                    {

                                                        if ((pieceindexx + i * 2) >= 0 && (pieceindexx + i * 2) < 8 && (pieceindexy + j) < 8 && (pieceindexy + j) >= 0)
                                                        {
                                                            //if it is an empty space or an opponents piece, it counts as a move (since we know it isn't pinned and that the king is not in check)
                                                            if (floor((float)Pieces[pieceindexx + i * 2][pieceindexy + j] / 8.0f) != SIDEMOVE || Pieces[pieceindexx + i * 2][pieceindexy + j] == 0)
                                                            {
                                                                if (CheckingPiece.size() == 1)
                                                                {
                                                                    for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                                    {
                                                                        if (pieceindexx + i * 2 == (CheckLines.at(0)[idx])[0] && pieceindexy + j == (CheckLines.at(0)[idx])[1])
                                                                            moves.push_back({ pieceindexx + i * 2, pieceindexy + j });
                                                                    }
                                                                }
                                                                else moves.push_back({ pieceindexx + i * 2, pieceindexy + j });
                                                            }
                                                        }

                                                    }
                                                    else
                                                    {
                                                        if ((pieceindexx + j) >= 0 && (pieceindexx + j) < 8 && (pieceindexy + i * 2) < 8 && (pieceindexy + i * 2) >= 0)
                                                        {
                                                            //if it is an empty space or an opponents piece, it counts as a move (since we know it isn't pinned and that the king is not in check)
                                                            if (floor((float)Pieces[pieceindexx + j][pieceindexy + i * 2] / 8.0f) != SIDEMOVE || Pieces[pieceindexx + j][pieceindexy + i * 2] == 0)
                                                            {
                                                                if (CheckingPiece.size() == 1)
                                                                {
                                                                    for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                                    {
                                                                        if (pieceindexx + j == (CheckLines.at(0)[idx])[0] && pieceindexy + i * 2 == (CheckLines.at(0)[idx])[1])
                                                                            moves.push_back({ pieceindexx + j, pieceindexy + i * 2 });
                                                                    }
                                                                }
                                                                else moves.push_back({ pieceindexx + j, pieceindexy + i * 2 });
                                                            }
                                                        }
                                                    }
                                                }
                                                else end = true;
                                            }
                                        }
                                    }
                                }
                                else if (Pieces[pieceindexx][pieceindexy] - 7 * (SIDEMOVE) == 4 || Pieces[pieceindexx][pieceindexy] - 7 * (SIDEMOVE) == 5)
                                {

                                    bool end = false;
                                    for (int i = 1; i < 8 - pieceindexx && !end; i++)
                                    {
                                        if (Pieces[pieceindexx + i][pieceindexy] == 0)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx + i == PinLine.at(idx)[0] && pieceindexy == PinLine.at(idx)[1]) moves.push_back({ pieceindexx + i, pieceindexy });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx + i == (CheckLines.at(0)[idx])[0] && pieceindexy == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx + i, pieceindexy });
                                                }
                                            }
                                            else
                                                moves.push_back({ pieceindexx + i, pieceindexy });
                                        }
                                        else if (floor((float)Pieces[pieceindexx + i][pieceindexy] / 8.0f) != SIDEMOVE)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx + i == PinLine.at(idx)[0] && pieceindexy == PinLine.at(idx)[1]) moves.push_back({ pieceindexx + i, pieceindexy });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx + i == (CheckLines.at(0)[idx])[0] && pieceindexy == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx + i, pieceindexy });
                                                }
                                            }
                                            else
                                            {
                                                moves.push_back({ pieceindexx + i, pieceindexy });
                                            }
                                            end = true;
                                        }
                                        else end = true;
                                    }
                                    end = false;
                                    for (int i = 1; i < 8 - pieceindexy && !end; i++)
                                    {
                                        int value[2] = { pieceindexx, pieceindexy + i };
                                        if (Pieces[pieceindexx][pieceindexy + i] == 0)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx == PinLine.at(idx)[0] && pieceindexy + i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx, pieceindexy + i });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx == (CheckLines.at(0)[idx])[0] && pieceindexy + i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx, pieceindexy + i });
                                                }
                                            }
                                            else
                                                moves.push_back({ pieceindexx, pieceindexy + i });
                                        }
                                        else if (floor((float)Pieces[pieceindexx][pieceindexy + i] / 8.0f) != SIDEMOVE)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx == PinLine.at(idx)[0] && pieceindexy + i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx, pieceindexy + i });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx == (CheckLines.at(0)[idx])[0] && pieceindexy + i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx, pieceindexy + i });
                                                }
                                            }
                                            else
                                            {
                                                moves.push_back({ pieceindexx, pieceindexy + i });

                                            }
                                            end = true;
                                        }
                                        else end = true;
                                    }
                                    end = false;
                                    for (int i = 1; i <= pieceindexy && !end; i++)
                                    {
                                        if (Pieces[pieceindexx][pieceindexy - i] == 0)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx == PinLine.at(idx)[0] && pieceindexy - i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx, pieceindexy - i });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx == (CheckLines.at(0)[idx])[0] && pieceindexy - i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx, pieceindexy - i });
                                                }
                                            }
                                            else
                                                moves.push_back({ pieceindexx, pieceindexy - i });
                                        }
                                        else if (floor((float)Pieces[pieceindexx][pieceindexy - i] / 8.0f) != SIDEMOVE)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx == PinLine.at(idx)[0] && pieceindexy - i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx, pieceindexy - i });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx == (CheckLines.at(0)[idx])[0] && pieceindexy - i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx, pieceindexy - i });
                                                }
                                            }
                                            else
                                            {
                                                moves.push_back({ pieceindexx, pieceindexy - i });

                                            }
                                            end = true;
                                        }
                                        else end = true;
                                    }
                                    end = false;
                                    for (int i = 1; i <= pieceindexx && !end; i++)
                                    {
                                        int value[2] = { pieceindexx - i, pieceindexy };
                                        if (Pieces[pieceindexx - i][pieceindexy] == 0)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx - i == PinLine.at(idx)[0] && pieceindexy == PinLine.at(idx)[1]) moves.push_back({ pieceindexx - i, pieceindexy });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx - i == (CheckLines.at(0)[idx])[0] && pieceindexy == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx - i, pieceindexy });
                                                }
                                            }
                                            else
                                                moves.push_back({ pieceindexx - i, pieceindexy });
                                        }
                                        else if (floor((float)Pieces[pieceindexx - i][pieceindexy] / 8.0f) != SIDEMOVE)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx - i == PinLine.at(idx)[0] && pieceindexy == PinLine.at(idx)[1]) moves.push_back({ pieceindexx - i, pieceindexy });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx - i == (CheckLines.at(0)[idx])[0] && pieceindexy == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx - i, pieceindexy });
                                                }
                                            }
                                            else
                                            {
                                                moves.push_back({ pieceindexx - i, pieceindexy });

                                            }
                                            end = true;
                                        }
                                        else end = true;
                                    }
                                    end = false;
                                }
                                else if (Pieces[pieceindexx][pieceindexy] - 7 * (SIDEMOVE) == 1)
                                {
                                    if (!isPinned || !(CheckingPiece.size() == 1))
                                    {

                                        if (pieceindexy == (!SIDEMOVE) * 5 + 1)
                                        {

                                            if (Pieces[pieceindexx][pieceindexy - 2 + 4 * SIDEMOVE] == 0 && Pieces[pieceindexx][pieceindexy - 1 + 2 * SIDEMOVE] == 0)
                                            {
                                                if (isPinned)
                                                {
                                                    for (int idx = 0; idx < PinLine.size(); idx++)
                                                    {
                                                        if (pieceindexx == PinLine.at(idx)[0] && pieceindexy - 2 + 4 * SIDEMOVE == PinLine.at(idx)[1]) moves.push_back({ pieceindexx , pieceindexy - 2 + 4 * SIDEMOVE });
                                                    }
                                                }
                                                else if (CheckingPiece.size() == 1)
                                                {
                                                    for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                    {
                                                        if (pieceindexx == (CheckLines.at(0)[idx])[0] && pieceindexy - 2 + 4 * SIDEMOVE == (CheckLines.at(0)[idx])[1])
                                                            moves.push_back({ pieceindexx , pieceindexy - 2 + 4 * SIDEMOVE });
                                                    }
                                                }
                                                else
                                                    moves.push_back({ pieceindexx , pieceindexy - 2 + 4 * SIDEMOVE });
                                            }
                                        }

                                        if (PreviousMove[0] != -1)
                                        {
                                            if (PreviousMove[0] - pieceindexx == 1 && PreviousMove[1] == pieceindexy)
                                            {
                                                if (isPinned && CheckingPiece.size() != 1)
                                                {
                                                    for (int idx = 0; idx < PinLine.size(); idx++)
                                                    {
                                                        if (pieceindexx + 1 == PinLine.at(idx)[0] && pieceindexy - 1 + 2 * SIDEMOVE == PinLine.at(idx)[1]) moves.push_back({ pieceindexx + 1 , pieceindexy - 1 + 2 * SIDEMOVE });
                                                    }
                                                }
                                                else if (CheckingPiece.size() == 1)
                                                {
                                                    for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                    {
                                                        if (pieceindexx + 1 == (CheckLines.at(0)[idx])[0] && pieceindexy - 1 + 2 * SIDEMOVE == (CheckLines.at(0)[idx])[1])
                                                            moves.push_back({ pieceindexx + 1 , pieceindexy - 1 + 2 * SIDEMOVE });
                                                    }
                                                }
                                                else moves.push_back({ pieceindexx + 1 , pieceindexy - 1 + 2 * SIDEMOVE });
                                            }
                                            if (PreviousMove[0] - pieceindexx == -1 && PreviousMove[1] == pieceindexy)
                                            {
                                                if (isPinned && CheckingPiece.size() != 1)
                                                {
                                                    for (int idx = 0; idx < PinLine.size(); idx++)
                                                    {
                                                        if (pieceindexx - 1 == PinLine.at(idx)[0] && pieceindexy - 1 + 2 * SIDEMOVE == PinLine.at(idx)[1]) moves.push_back({ pieceindexx - 1 , pieceindexy - 1 + 2 * SIDEMOVE });
                                                    }
                                                }
                                                else if (CheckingPiece.size() == 1)
                                                {
                                                    for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                    {
                                                        if (pieceindexx - 1 == (CheckLines.at(0)[idx])[0] && pieceindexy - 1 + 2 * SIDEMOVE == (CheckLines.at(0)[idx])[1])
                                                            moves.push_back({ pieceindexx - 1 , pieceindexy - 1 + 2 * SIDEMOVE });
                                                    }
                                                }
                                                else moves.push_back({ pieceindexx - 1 , pieceindexy - 1 + 2 * SIDEMOVE });
                                            }
                                        }



                                        if (Pieces[pieceindexx][pieceindexy - 1 + 2 * SIDEMOVE] == 0)
                                        {
                                            if (isPinned)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx == PinLine.at(idx)[0] && pieceindexy - 1 + 2 * SIDEMOVE == PinLine.at(idx)[1]) moves.push_back({ pieceindexx , pieceindexy - 1 + 2 * SIDEMOVE });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx == (CheckLines.at(0)[idx])[0] && pieceindexy - 1 + 2 * SIDEMOVE == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx , pieceindexy - 1 + 2 * SIDEMOVE });
                                                }
                                            }
                                            else
                                                moves.push_back({ pieceindexx , pieceindexy - 1 + 2 * SIDEMOVE });
                                        }

                                        if (pieceindexx - 1 >= 0)
                                        {
                                            if (floor((float)Pieces[pieceindexx - 1][pieceindexy - 1 + 2 * SIDEMOVE] / 8.0f) != SIDEMOVE && Pieces[pieceindexx - 1][pieceindexy - 1 + 2 * SIDEMOVE] != 0)
                                            {
                                                if (isPinned && CheckingPiece.size() != 1)
                                                {
                                                    for (int idx = 0; idx < PinLine.size(); idx++)
                                                    {
                                                        if (pieceindexx - 1 == PinLine.at(idx)[0] && pieceindexy - 1 + 2 * SIDEMOVE == PinLine.at(idx)[1]) moves.push_back({ pieceindexx - 1 , pieceindexy - 1 + 2 * SIDEMOVE });
                                                    }
                                                }
                                                else if (CheckingPiece.size() == 1)
                                                {
                                                    for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                    {
                                                        if (pieceindexx - 1 == (CheckLines.at(0)[idx])[0] && pieceindexy - 1 + 2 * SIDEMOVE == (CheckLines.at(0)[idx])[1])
                                                            moves.push_back({ pieceindexx - 1 , pieceindexy - 1 + 2 * SIDEMOVE });
                                                    }
                                                }
                                                else moves.push_back({ pieceindexx - 1 , pieceindexy - 1 + 2 * SIDEMOVE });
                                            }
                                        }

                                        if (pieceindexx + 1 < 8)
                                        {
                                            if (floor((float)Pieces[pieceindexx + 1][pieceindexy - 1 + 2 * SIDEMOVE] / 8.0f) != SIDEMOVE && Pieces[pieceindexx + 1][pieceindexy - 1 + 2 * SIDEMOVE] != 0)
                                            {
                                                if (isPinned)
                                                {
                                                    for (int idx = 0; idx < PinLine.size(); idx++)
                                                    {
                                                        if (pieceindexx + 1 == PinLine.at(idx)[0] && pieceindexy - 1 + 2 * SIDEMOVE == PinLine.at(idx)[1]) moves.push_back({ pieceindexx + 1 , pieceindexy - 1 + 2 * SIDEMOVE });
                                                    }
                                                }
                                                else if (CheckingPiece.size() == 1)
                                                {
                                                    for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                    {
                                                        if (pieceindexx + 1 == (CheckLines.at(0)[idx])[0] && pieceindexy - 1 + 2 * SIDEMOVE == (CheckLines.at(0)[idx])[1])
                                                            moves.push_back({ pieceindexx + 1 , pieceindexy - 1 + 2 * SIDEMOVE });
                                                    }
                                                }
                                                else moves.push_back({ pieceindexx + 1 , pieceindexy - 1 + 2 * SIDEMOVE });
                                            }
                                        }
                                    }
                                }
                                else if (Pieces[pieceindexx][pieceindexy] - 7 * (SIDEMOVE) == 7)
                                {
                                    end = false;
                                    for (int i = 1; i <= 7 - max(pieceindexx, pieceindexy) && !end; i++)
                                    {
                                        if (Pieces[pieceindexx + i][pieceindexy + i] == 0)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx + i == PinLine.at(idx)[0] && pieceindexy + i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx + i, pieceindexy + i });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx + i == (CheckLines.at(0)[idx])[0] && pieceindexy + i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx + i, pieceindexy + i });
                                                }
                                            }
                                            else
                                                moves.push_back({ pieceindexx + i, pieceindexy + i });
                                        }
                                        else if (floor((float)Pieces[pieceindexx + i][pieceindexy + i] / 8.0f) != SIDEMOVE)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx + i == PinLine.at(idx)[0] && pieceindexy + i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx + i, pieceindexy + i });
                                                }
                                                end = true;
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx + i == (CheckLines.at(0)[idx])[0] && pieceindexy + i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx + i, pieceindexy + i });
                                                }
                                            }
                                            else
                                            {
                                                moves.push_back({ pieceindexx + i, pieceindexy + i });

                                            }
                                            end = true;

                                        }
                                        else end = true;
                                    }

                                    end = false;

                                    for (int i = 1; i <= min(pieceindexx, pieceindexy) && !end; i++)
                                    {
                                        if (Pieces[pieceindexx - i][pieceindexy - i] == 0)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx - i == PinLine.at(idx)[0] && pieceindexy - i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx - i, pieceindexy - i });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx - i == (CheckLines.at(0)[idx])[0] && pieceindexy - i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx - i, pieceindexy - i });
                                                }
                                            }
                                            else
                                                moves.push_back({ pieceindexx - i, pieceindexy - i });
                                        }
                                        else if (floor((float)Pieces[pieceindexx - i][pieceindexy - i] / 8.0f) != SIDEMOVE)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx - i == PinLine.at(idx)[0] && pieceindexy - i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx - i, pieceindexy - i });
                                                }
                                                end = true;
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx - i == (CheckLines.at(0)[idx])[0] && pieceindexy - i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx - i, pieceindexy - i });
                                                }
                                            }
                                            else
                                            {
                                                moves.push_back({ pieceindexx - i, pieceindexy - i });

                                            }
                                            end = true;
                                        }
                                        else end = true;
                                    }
                                    end = false;

                                    for (int i = 1; i <= min(7 - pieceindexx, pieceindexy) && !end; i++)
                                    {
                                        if (Pieces[pieceindexx + i][pieceindexy - i] == 0)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx + i == PinLine.at(idx)[0] && pieceindexy - i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx + i, pieceindexy - i });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx + i == (CheckLines.at(0)[idx])[0] && pieceindexy - i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx + i, pieceindexy - i });
                                                }
                                            }
                                            else
                                                moves.push_back({ pieceindexx + i, pieceindexy - i });
                                        }
                                        else if (floor((float)Pieces[pieceindexx + i][pieceindexy - i] / 8.0f) != SIDEMOVE)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx + i == PinLine.at(idx)[0] && pieceindexy - i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx + i, pieceindexy - i });
                                                }
                                                end = true;
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx + i == (CheckLines.at(0)[idx])[0] && pieceindexy - i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx + i, pieceindexy - i });
                                                }
                                            }
                                            else
                                            {
                                                moves.push_back({ pieceindexx + i, pieceindexy - i });

                                            }
                                            end = true;
                                        }
                                        else end = true;
                                    }

                                    end = false;
                                    for (int i = 1; i <= min(pieceindexx, 7 - pieceindexy) && !end; i++)
                                    {
                                        if (Pieces[pieceindexx - i][pieceindexy + i] == 0)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx - i == PinLine.at(idx)[0] && pieceindexy + i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx - i, pieceindexy + i });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx - i == (CheckLines.at(0)[idx])[0] && pieceindexy + i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx - i, pieceindexy + i });
                                                }
                                            }
                                            else
                                                moves.push_back({ pieceindexx - i, pieceindexy + i });
                                        }
                                        else if (floor((float)Pieces[pieceindexx - i][pieceindexy + i] / 8.0f) != SIDEMOVE)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx - i == PinLine.at(idx)[0] && pieceindexy + i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx - i, pieceindexy + i });
                                                }
                                                end = true;
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx - i == (CheckLines.at(0)[idx])[0] && pieceindexy + i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx - i, pieceindexy + i });
                                                }
                                            }
                                            else
                                            {
                                                moves.push_back({ pieceindexx - i, pieceindexy + i });

                                            }
                                            end = true;
                                        }
                                        else end = true;
                                    }

                                    //rook part of queen movement
                                    end = false;
                                    for (int i = 1; i < 8 - pieceindexx && !end; i++)
                                    {
                                        if (Pieces[pieceindexx + i][pieceindexy] == 0)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx + i == PinLine.at(idx)[0] && pieceindexy == PinLine.at(idx)[1]) moves.push_back({ pieceindexx + i, pieceindexy });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx + i == (CheckLines.at(0)[idx])[0] && pieceindexy == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx + i, pieceindexy });
                                                }
                                            }
                                            else
                                                moves.push_back({ pieceindexx + i, pieceindexy });
                                        }
                                        else if (floor((float)Pieces[pieceindexx + i][pieceindexy] / 8.0f) != SIDEMOVE)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx + i == PinLine.at(idx)[0] && pieceindexy == PinLine.at(idx)[1]) moves.push_back({ pieceindexx + i, pieceindexy });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx + i == (CheckLines.at(0)[idx])[0] && pieceindexy == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx + i, pieceindexy });
                                                }
                                            }
                                            else
                                            {
                                                moves.push_back({ pieceindexx + i, pieceindexy });
                                                end = true;
                                            }
                                        }
                                        else end = true;
                                    }
                                    end = false;
                                    for (int i = 1; i < 8 - pieceindexy && !end; i++)
                                    {
                                        int value[2] = { pieceindexx, pieceindexy + i };
                                        if (Pieces[pieceindexx][pieceindexy + i] == 0)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx == PinLine.at(idx)[0] && pieceindexy + i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx, pieceindexy + i });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx == (CheckLines.at(0)[idx])[0] && pieceindexy + i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx, pieceindexy + i });
                                                }
                                            }
                                            else
                                                moves.push_back({ pieceindexx, pieceindexy + i });
                                        }
                                        else if (floor((float)Pieces[pieceindexx][pieceindexy + i] / 8.0f) != SIDEMOVE)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx == PinLine.at(idx)[0] && pieceindexy + i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx, pieceindexy + i });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx == (CheckLines.at(0)[idx])[0] && pieceindexy + i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx, pieceindexy + i });
                                                }
                                            }
                                            else
                                            {
                                                moves.push_back({ pieceindexx, pieceindexy + i });

                                            }
                                            end = true;
                                        }
                                        else end = true;
                                    }
                                    end = false;
                                    for (int i = 1; i <= pieceindexy && !end; i++)
                                    {
                                        if (Pieces[pieceindexx][pieceindexy - i] == 0)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx == PinLine.at(idx)[0] && pieceindexy - i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx, pieceindexy - i });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx == (CheckLines.at(0)[idx])[0] && pieceindexy - i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx, pieceindexy - i });
                                                }
                                            }
                                            else
                                                moves.push_back({ pieceindexx, pieceindexy - i });
                                        }
                                        else if (floor((float)Pieces[pieceindexx][pieceindexy - i] / 8.0f) != SIDEMOVE)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx == PinLine.at(idx)[0] && pieceindexy - i == PinLine.at(idx)[1]) moves.push_back({ pieceindexx, pieceindexy - i });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx == (CheckLines.at(0)[idx])[0] && pieceindexy - i == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx, pieceindexy - i });
                                                }
                                            }
                                            else
                                            {
                                                moves.push_back({ pieceindexx, pieceindexy - i });

                                            }
                                            end = true;
                                        }
                                        else end = true;
                                    }
                                    end = false;
                                    for (int i = 1; i <= pieceindexx && !end; i++)
                                    {
                                        int value[2] = { pieceindexx - i, pieceindexy };
                                        if (Pieces[pieceindexx - i][pieceindexy] == 0)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx - i == PinLine.at(idx)[0] && pieceindexy == PinLine.at(idx)[1]) moves.push_back({ pieceindexx - i, pieceindexy });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx - i == (CheckLines.at(0)[idx])[0] && pieceindexy == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx - i, pieceindexy });
                                                }
                                            }
                                            else
                                                moves.push_back({ pieceindexx - i, pieceindexy });
                                        }
                                        else if (floor((float)Pieces[pieceindexx - i][pieceindexy] / 8.0f) != SIDEMOVE)
                                        {
                                            if (isPinned && CheckingPiece.size() != 1)
                                            {
                                                for (int idx = 0; idx < PinLine.size(); idx++)
                                                {
                                                    if (pieceindexx - i == PinLine.at(idx)[0] && pieceindexy == PinLine.at(idx)[1]) moves.push_back({ pieceindexx - i, pieceindexy });
                                                }
                                            }
                                            else if (CheckingPiece.size() == 1)
                                            {
                                                for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                {
                                                    if (pieceindexx - i == (CheckLines.at(0)[idx])[0] && pieceindexy == (CheckLines.at(0)[idx])[1])
                                                        moves.push_back({ pieceindexx - i, pieceindexy });
                                                }
                                            }
                                            else
                                            {
                                                moves.push_back({ pieceindexx - i, pieceindexy });

                                            }
                                            end = true;
                                        }
                                        else end = true;
                                    }
                                    end = false;
                                }
                                else if (Pieces[pieceindexx][pieceindexy] - 7 * (SIDEMOVE) == 6)
                                {
                                    for (int posX = -1; posX <= 1; posX++)
                                    {
                                        for (int posY = -1; posY <= 1; posY++)
                                        {
                                            if (posX != 0 || posY != 0)
                                            {
                                                bool cangoThere = true;
                                                if (NoKingZone.size() > 0)
                                                {
                                                    for (int idx = 0; idx < NoKingZone.size(); idx++)
                                                    {
                                                        if (pieceindexx + posX >= 0 && pieceindexx + posX < 8 && pieceindexy + posY >= 0 && pieceindexy + posY < 8)
                                                        {
                                                            if ((pieceindexx + posX == NoKingZone.at(idx)[0] && pieceindexy + posY == NoKingZone.at(idx)[1]))
                                                            {
                                                                cangoThere = false;
                                                            }
                                                        }
                                                    }
                                                }
                                                if (CheckingPiece.size() == 1)
                                                {
                                                    for (int idx = 0; idx < CheckLines[0].size(); idx++)
                                                    {
                                                        if (pieceindexx + posX >= 0 && pieceindexx + posX < 8 && pieceindexy + posY >= 0 && pieceindexy + posY < 8)
                                                        {
                                                            if ((pieceindexx + posX == CheckLines[0].at(idx)[0] && pieceindexy + posY == CheckLines[0].at(idx)[1]) && !(Pieces[pieceindexx + posX][pieceindexy + posY] != 0 && floor((float)Pieces[pieceindexx + posX][pieceindexy + posY] / 8.0f) != SIDEMOVE))
                                                            {

                                                                cangoThere = false;
                                                            }

                                                        }
                                                    }
                                                }
                                                if (cangoThere && pieceindexx + posX >= 0 && pieceindexx + posX < 8 && pieceindexy + posY >= 0 && pieceindexy + posY < 8)
                                                {
                                                    if (!(Pieces[pieceindexx + posX][pieceindexy + posY] != 0 && floor((float)Pieces[pieceindexx + posX][pieceindexy + posY] / 8.0f) == SIDEMOVE))
                                                    {
                                                        moves.push_back({ pieceindexx + posX, pieceindexy + posY });
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    if (CheckingPiece.size() == 0)
                                    {
                                        if (pieceindexx == 4 && pieceindexy == (!SIDEMOVE) * 7 && Pieces[0][(!SIDEMOVE) * 7] == 4 + 7 * SIDEMOVE)
                                        {
                                            bool cancastleLong = true;
                                            for (int posX = 1; posX <= 3; posX++)
                                            {
                                                if (Pieces[pieceindexx - posX][pieceindexy] == 0)
                                                {
                                                    for (int idxCastle = 0; idxCastle < NoCastleZone.size() && cancastleLong; idxCastle++)
                                                    {
                                                        if (pieceindexx - posX == NoCastleZone[idxCastle]) cancastleLong = false;
                                                    }
                                                }
                                                else cancastleLong = false;
                                            }
                                            if (cancastleLong)
                                            {
                                                moves.push_back({ pieceindexx - 2, pieceindexy });
                                            }

                                        }
                                        if (pieceindexx == 4 && pieceindexy == (!SIDEMOVE) * 7 && Pieces[7][(!SIDEMOVE) * 7] == 4 + 7 * SIDEMOVE)
                                        {
                                            bool cancastleShort = true;
                                            for (int posX = 1; posX <= 2; posX++)
                                            {
                                                if (Pieces[pieceindexx + posX][pieceindexy] == 0)
                                                {
                                                    for (int idxCastle = 0; idxCastle < NoCastleZone.size() && cancastleShort; idxCastle++)
                                                    {
                                                        if (pieceindexx + posX == NoCastleZone[idxCastle]) cancastleShort = false;
                                                    }
                                                }
                                                else cancastleShort = false;
                                            }
                                            if (cancastleShort)
                                            {
                                                moves.push_back({ pieceindexx + 2, pieceindexy });
                                            }
                                        }
                                    }
                                }
                            }
                            if (moves.size() > 1)
                            {
                                PieceMoves.push_back(moves);
                            }
                        }
                    }
                }
            }
        }

        if (PieceMoves.size() == 0 && numChecks == 0)
        {
            PieceMoves.push_back({ { kingLocation[0], kingLocation[1] } });
        }

        return PieceMoves;
    }

    float giveSimpleEval()
    {
        vector<vector<vector<int>>> moves = AllPieceMoves();
        
        float finalEval, prevFinalEval = -10000;

        vector<float> evals;


        bool isCheckmate = false;

        #pragma omp parallel for
        for (int i = 0; i < moves.size(); i++)
        {
            float eval = 0.0f;
            float previousEval = -10000;
            int piecex = moves[i][0][0];
            int piecey = moves[i][0][1];
            for (int i1 = 1; i1 < moves[i].size(); i1++)
            {

                vector<vector<vector<int>>> newPieceMoves;
                int postpiecex = moves[i][i1][0];
                int postpiecey = moves[i][i1][1];
                ChessPosition NewPos1;

                NewPos1.CopyPosition(*this);

                int Piecetaken = NewPos1.getPieceID(postpiecex, postpiecey);
                int Piecetomove = NewPos1.getPieceID(piecex, piecey);
                bool isPromotion = false;
                float promotioneval = 0.0f;

                NewPos1.RemovePiece(piecex, piecey);
                if (Piecetomove % 7 == 6)
                {
                    NewPos1.AddPiece(postpiecex, postpiecey, Piecetomove);
                    //if king
                    if (postpiecex - piecex >= 2)
                    {
                        //short Castle
                        NewPos1.AddPiece(5, 7 * (!NewPos1.SIDEMOVE), 5 + NewPos1.SIDEMOVE * 7);
                        NewPos1.RemovePiece(7, 7 * (!NewPos1.SIDEMOVE));
                        if (NewPos1.getPieceID(0, 7 * (!NewPos1.SIDEMOVE)) == 4 + NewPos1.SIDEMOVE * 7) NewPos1.AddPiece(0, 7 * (!NewPos1.SIDEMOVE), 5 + NewPos1.SIDEMOVE * 7);
                    }
                    if (postpiecex - piecex <= -2)
                    {
                        //long Castle
                        NewPos1.AddPiece(3, 7 * (!NewPos1.SIDEMOVE), 5 + NewPos1.SIDEMOVE * 7);
                        NewPos1.RemovePiece(0, 7 * (!NewPos1.SIDEMOVE));
                        if (NewPos1.getPieceID(7, 7 * (!NewPos1.SIDEMOVE)) == 4 + NewPos1.SIDEMOVE * 7) NewPos1.AddPiece(7, 7 * (!NewPos1.SIDEMOVE), 5 + NewPos1.SIDEMOVE * 7);
                    }
                    else
                    {
                        if (NewPos1.getPieceID(0, 7 * (!NewPos1.SIDEMOVE)) == 4 + NewPos1.SIDEMOVE * 7) NewPos1.AddPiece(0, 7 * (!NewPos1.SIDEMOVE), 5 + NewPos1.SIDEMOVE * 7);
                        if (NewPos1.getPieceID(7, 7 * (!NewPos1.SIDEMOVE)) == 4 + NewPos1.SIDEMOVE * 7) NewPos1.AddPiece(7, 7 * (!NewPos1.SIDEMOVE), 5 + NewPos1.SIDEMOVE * 7);
                    }
                    NewPos1.SIDEMOVE = !NewPos1.SIDEMOVE;
                    newPieceMoves = NewPos1.AllPieceMoves();
                    NewPos1.AddPrevMove(-1, -1);
                }
                else if (Piecetomove % 7 == 1)
                {
                    if (postpiecey == NewPos1.SIDEMOVE * 7)
                    {
                        //promotion

                        float prevpromotioneval = -10000;

                        isPromotion = true;
                        //promotion
                        for (int i = 0; i < 4; i++)
                        {
                            ChessPosition NewPosProm;
                            NewPosProm.CopyPosition(NewPos1);
                            if (i == 0)
                            {
                                //promote to queen
                                NewPosProm.AddPiece(postpiecex, postpiecey, 7 + (NewPos1.SIDEMOVE * 7));
                            }
                            else if (i == 1)
                            {
                                //promote to bishop
                                NewPosProm.AddPiece(postpiecex, postpiecey, 2 + (NewPos1.SIDEMOVE * 7));
                            }
                            else if (i == 2)
                            {
                                //promote to knight
                                NewPosProm.AddPiece(postpiecex, postpiecey, 3 + (NewPos1.SIDEMOVE * 7));
                            }
                            else if (i == 3)
                            {
                                //promote to rook
                                NewPosProm.AddPiece(postpiecex, postpiecey, 5 + (NewPos1.SIDEMOVE * 7));

                            }
                            NewPosProm.SIDEMOVE = !NewPosProm.SIDEMOVE;

                            float promotioneval1 = 0.0f;

                            NewPosProm.AddPrevMove(-1, -1);

                            vector<vector<vector<int>>> newPieceMovesProm = NewPosProm.AllPieceMoves();

                            if (newPieceMovesProm.size() == 0)
                            {
                                promotioneval1 = -40 + 80 * NewPosProm.SIDEMOVE;
                            }
                            else if (newPieceMovesProm[0].size() == 1)
                            {
                                promotioneval = 0;
                            }
                            else
                            {
                                promotioneval1 = NewPosProm.DoubleLayer(NewPosProm, newPieceMovesProm, false);
                            }

                            if (prevpromotioneval == -10000)
                            {
                                promotioneval = promotioneval1;
                                prevpromotioneval = promotioneval;
                            }
                            else
                            {

                                if (NewPosProm.SIDEMOVE == 0) promotioneval = fminf(promotioneval1, prevpromotioneval);
                                else promotioneval = fmaxf(promotioneval1, prevpromotioneval);
                                prevpromotioneval = promotioneval;
                            }

                            
                        }
                        NewPos1.SIDEMOVE = !NewPos1.SIDEMOVE;
                    }
                    else
                    {
                        NewPos1.AddPiece(postpiecex, postpiecey, Piecetomove);
                        if (Piecetaken == 0 && postpiecex - piecex != 0)
                        {
                            //en passant
                            NewPos1.RemovePiece(postpiecex, postpiecey - 1 + 2 * (!NewPos1.SIDEMOVE));
                        }

                        if (abs(postpiecey - piecey) >= 2)
                        {
                            //flying pawn to put in previous move
                            NewPos1.AddPrevMove(postpiecex, postpiecey);
                        }
                        else
                        {
                            NewPos1.AddPrevMove(-1, -1);
                        }

                        NewPos1.SIDEMOVE = !NewPos1.SIDEMOVE;
                        NewPos1.RemovePiece(piecex, piecey);
                        newPieceMoves = NewPos1.AllPieceMoves();
                    }
                }
                else
                {
                    NewPos1.AddPiece(postpiecex, postpiecey, Piecetomove);
                    NewPos1.AddPrevMove(-1, -1);
                    NewPos1.SIDEMOVE = !NewPos1.SIDEMOVE;
                    newPieceMoves = NewPos1.AllPieceMoves();
                }


                float tempEval = 0.0f;
                if (!isPromotion)
                {
                    if (newPieceMoves.size() == 0)
                    {

                        tempEval = -40.0f + 80.0f * NewPos1.SIDEMOVE;
                        isCheckmate = true;
                    }
                    else if(newPieceMoves[0].size() == 1)
                    {
                        tempEval = 0;
                    }
                    else
                    {
                        tempEval = NewPos1.DoubleLayer(NewPos1, newPieceMoves, false);
                    }

                    if (previousEval == -10000)
                    {
                        eval = tempEval;
                        previousEval = eval;

                    }
                    else
                    {
                        if (NewPos1.SIDEMOVE == 0) eval = fminf(tempEval, previousEval);
                        else eval = fmaxf(tempEval, previousEval);
                        previousEval = eval;

                    }
                }
                else
                {
                    if (previousEval == -10000)
                    {
                        eval = promotioneval;
                        previousEval = eval;
                    }
                    else
                    {
                        if (NewPos1.SIDEMOVE == 0)
                        {
                            eval = fminf(promotioneval, previousEval);
                        }
                        else eval = fmaxf(promotioneval, previousEval);
                        previousEval = eval;

                    }

                }

            }
            evals.push_back(eval);
        }

        finalEval = evals[0];
        prevFinalEval = evals[0];
        for (int i = 0; i < evals.size(); i++)
        {
            if ((*this).SIDEMOVE == 1) finalEval = fminf(evals[i], prevFinalEval);
            else finalEval = fmaxf(evals[i], prevFinalEval);

            prevFinalEval = finalEval;
        }

        return finalEval;
    }

    float SingleLayerEval(ChessPosition Pos, vector<vector<vector<int>>> PieceMoves, bool print)
    {
        float PrevEval = -10000;
        float eval = 0.0f;

        vector<vector<int>> move;

        move.push_back({ -1, -1 });
        move.push_back({ -1, -1 });

        for (int h = 0; h < PieceMoves.size(); h++)
        {

            int piecex = PieceMoves[h][0][0];
            int piecey = PieceMoves[h][0][1];
            for (int h1 = 1; h1 < PieceMoves[h].size(); h1++)
            {
                int postpiecex = PieceMoves[h][h1][0];
                int postpiecey = PieceMoves[h][h1][1];
                ChessPosition NewPos;

                bool isPromotion = false;
                float promotioneval = 0.0f;


                NewPos.CopyPosition(Pos);

                int Piecetaken = NewPos.getPieceID(postpiecex, postpiecey);
                int Piecetomove = NewPos.getPieceID(piecex, piecey);
                NewPos.RemovePiece(piecex, piecey);
                if (Piecetomove - 7 * NewPos.SIDEMOVE == 6)
                {
                    NewPos.AddPiece(postpiecex, postpiecey, Piecetomove);
                    //if king
                    if (postpiecex - piecex >= 2)
                    {
                        //short Castle
                        NewPos.AddPiece(5, 7 * (!NewPos.SIDEMOVE), 5 + NewPos.SIDEMOVE * 7);
                        NewPos.RemovePiece(7, 7 * (!NewPos.SIDEMOVE));
                        if (NewPos.getPieceID(0, 7 * (!NewPos.SIDEMOVE)) == 4 + NewPos.SIDEMOVE * 7) NewPos.AddPiece(0, 7 * (!NewPos.SIDEMOVE), 5 + NewPos.SIDEMOVE * 7);
                    }
                    if (postpiecex - piecex <= -2)
                    {
                        //long Castle
                        NewPos.AddPiece(3, 7 * (!NewPos.SIDEMOVE), 5 + NewPos.SIDEMOVE * 7);
                        NewPos.RemovePiece(0, 7 * (!NewPos.SIDEMOVE));
                        if (NewPos.getPieceID(7, 7 * (!NewPos.SIDEMOVE)) == 4 + NewPos.SIDEMOVE * 7) NewPos.AddPiece(7, 7 * (!NewPos.SIDEMOVE), 5 + NewPos.SIDEMOVE * 7);
                    }
                    else
                    {
                        if (NewPos.getPieceID(0, 7 * (!NewPos.SIDEMOVE)) == 4 + NewPos.SIDEMOVE * 7) NewPos.AddPiece(0, 7 * (!NewPos.SIDEMOVE), 5 + NewPos.SIDEMOVE * 7);
                        if (NewPos.getPieceID(7, 7 * (!NewPos.SIDEMOVE)) == 4 + NewPos.SIDEMOVE * 7) NewPos.AddPiece(7, 7 * (!NewPos.SIDEMOVE), 5 + NewPos.SIDEMOVE * 7);
                    }
                    NewPos.SIDEMOVE = !NewPos.SIDEMOVE;
                    NewPos.AddPrevMove(-1, -1);
                }
                else if (Piecetomove - 7 * NewPos.SIDEMOVE == 1)
                {
                    if (postpiecey == NewPos.SIDEMOVE * 7)
                    {
                        ChessPosition NewPos2;
                        NewPos2.CopyPosition(NewPos);

                        float prevpromotioneval = -10000;

                        isPromotion = true;
                        //promotion
                        for (int k = 0; k < 4; k++)
                        {

                            if (k == 0)
                            {
                                //promote to queen
                                NewPos2.AddPiece(postpiecex, postpiecey, 7 + (NewPos.SIDEMOVE * 7));
                            }
                            else if (k == 1)
                            {
                                //promote to bishop
                                NewPos2.AddPiece(postpiecex, postpiecey, 2 + (NewPos.SIDEMOVE * 7));
                            }
                            else if (k == 2)
                            {
                                //promote to knight
                                NewPos2.AddPiece(postpiecex, postpiecey, 3 + (NewPos.SIDEMOVE * 7));
                            }
                            else if (k == 3)
                            {
                                //promote to rook
                                NewPos2.AddPiece(postpiecex, postpiecey, 5 + (NewPos.SIDEMOVE * 7));
                            }
                            NewPos2.SIDEMOVE = !NewPos2.SIDEMOVE;

                            if (prevpromotioneval == -10000)
                            {
                                promotioneval = NewPos2.givePositionEval(postpiecex, postpiecey, Piecetaken != 0, false);
                                prevpromotioneval = promotioneval;
                            }
                            else
                            {
                                if (NewPos2.SIDEMOVE == 0) promotioneval = fminf(NewPos2.givePositionEval(postpiecex, postpiecey, Piecetaken != 0, false), prevpromotioneval);
                                else promotioneval = fmaxf(NewPos2.givePositionEval(postpiecex, postpiecey, Piecetaken != 0, false), prevpromotioneval);
                                prevpromotioneval = promotioneval;
                            }
                        }

                        NewPos.SIDEMOVE = !NewPos.SIDEMOVE;
                    }
                    else
                    {
                        NewPos.AddPiece(postpiecex, postpiecey, Piecetomove);
                        if (Piecetaken == 0 && postpiecex - piecex != 0)
                        {
                            //en passant
                            NewPos.RemovePiece(postpiecex, postpiecey - 1 + 2 * (!NewPos.SIDEMOVE));
                        }

                        if (abs(postpiecey - piecey) >= 2)
                        {
                            //flying pawn to put in previous move
                            NewPos.AddPrevMove(postpiecex, postpiecey);
                        }
                        else
                        {
                            NewPos.AddPrevMove(-1, -1);
                        }

                        NewPos.SIDEMOVE = !NewPos.SIDEMOVE;
                        NewPos.RemovePiece(piecex, piecey);
                    }
                }
                else
                {
                    NewPos.AddPiece(postpiecex, postpiecey, Piecetomove);
                    NewPos.AddPrevMove(-1, -1);
                    NewPos.SIDEMOVE = !NewPos.SIDEMOVE;
                }


                if (!isPromotion)
                {

                    if (PrevEval == -10000)
                    {
                        eval = NewPos.givePositionEval(postpiecex, postpiecey, Piecetaken != 0, false);
                        PrevEval = eval;
                        move[0] = { piecex, piecey };
                        move[1] = { postpiecex, postpiecey };
                    }
                    else
                    {
                        if (NewPos.SIDEMOVE == 0) eval = fminf(NewPos.givePositionEval(postpiecex, postpiecey, Piecetaken != 0, false), PrevEval);
                        else eval = fmaxf(NewPos.givePositionEval(postpiecex, postpiecey, Piecetaken != 0, false), PrevEval);
                        PrevEval = eval;
                        if (eval == NewPos.givePositionEval(postpiecex, postpiecey, Piecetaken != 0, false))
                        {
                            move[0] = { piecex, piecey };
                            move[1] = { postpiecex, postpiecey };
                        }
                    }
                }
                else
                {
                    if (PrevEval == -10000)
                    {
                        eval = promotioneval;
                        PrevEval = eval;
                    }
                    else
                    {
                        if (NewPos.SIDEMOVE == 0) eval = fminf(promotioneval, PrevEval);
                        else eval = fmaxf(promotioneval, PrevEval);
                        PrevEval = eval;
                    }

                }

            }
        }


        return eval;
    }

    float DoubleLayer(ChessPosition Pos, vector<vector<vector<int>>> PieceMoves, bool print)
    {
        float PrevEval = -10000;
        float eval = 0.0f;


        for (int h = 0; h < PieceMoves.size(); h++)
        {

            int piecex = PieceMoves[h][0][0];
            int piecey = PieceMoves[h][0][1];
            for (int h1 = 1; h1 < PieceMoves[h].size(); h1++)
            {
                int postpiecex = PieceMoves[h][h1][0];
                int postpiecey = PieceMoves[h][h1][1];
                ChessPosition NewPos;

                bool isPromotion = false;
                float promotioneval = 0.0f;


                NewPos.CopyPosition(Pos);

                int Piecetaken = NewPos.getPieceID(postpiecex, postpiecex);
                int Piecetomove = NewPos.getPieceID(piecex, piecey);
                NewPos.RemovePiece(piecex, piecey);
                if (Piecetomove % 7 == 6)
                {
                    NewPos.AddPiece(postpiecex, postpiecey, Piecetomove);
                    //if king
                    if (postpiecex - piecex >= 2)
                    {
                        //short Castle
                        NewPos.AddPiece(5, 7 * (!NewPos.SIDEMOVE), 5 + NewPos.SIDEMOVE * 7);
                        NewPos.RemovePiece(7, 7 * (!NewPos.SIDEMOVE));
                        if (NewPos.getPieceID(0, 7 * (!NewPos.SIDEMOVE)) == 4 + NewPos.SIDEMOVE * 7) NewPos.AddPiece(0, 7 * (!NewPos.SIDEMOVE), 5 + NewPos.SIDEMOVE * 7);
                    }
                    if (postpiecex - piecex <= -2)
                    {
                        //long Castle
                        NewPos.AddPiece(3, 7 * (!NewPos.SIDEMOVE), 5 + NewPos.SIDEMOVE * 7);
                        NewPos.RemovePiece(0, 7 * (!NewPos.SIDEMOVE));
                        if (NewPos.getPieceID(7, 7 * (!NewPos.SIDEMOVE)) == 4 + NewPos.SIDEMOVE * 7) NewPos.AddPiece(7, 7 * (!NewPos.SIDEMOVE), 5 + NewPos.SIDEMOVE * 7);
                    }
                    else
                    {
                        if (NewPos.getPieceID(0, 7 * (!NewPos.SIDEMOVE)) == 4 + NewPos.SIDEMOVE * 7) NewPos.AddPiece(0, 7 * (!NewPos.SIDEMOVE), 5 + NewPos.SIDEMOVE * 7);
                        if (NewPos.getPieceID(7, 7 * (!NewPos.SIDEMOVE)) == 4 + NewPos.SIDEMOVE * 7) NewPos.AddPiece(7, 7 * (!NewPos.SIDEMOVE), 5 + NewPos.SIDEMOVE * 7);
                    }
                    NewPos.SIDEMOVE = !NewPos.SIDEMOVE;
                    NewPos.AddPrevMove(-1, -1);
                }
                else if (Piecetomove % 7 == 1)
                {
                    if (postpiecey == NewPos.SIDEMOVE * 7)
                    {
                        ChessPosition NewPos2;
                        NewPos2.CopyPosition(NewPos);

                        float prevpromotioneval = -10000;

                        isPromotion = true;
                        //promotion
                        for (int k = 0; k < 4; k++)
                        {

                            if (k == 0)
                            {
                                //promote to queen
                                NewPos2.AddPiece(postpiecex, postpiecey, 7 + (NewPos.SIDEMOVE * 7));
                            }
                            else if (k == 1)
                            {
                                //promote to bishop
                                NewPos2.AddPiece(postpiecex, postpiecey, 2 + (NewPos.SIDEMOVE * 7));
                            }
                            else if (k == 2)
                            {
                                //promote to knight
                                NewPos2.AddPiece(postpiecex, postpiecey, 3 + (NewPos.SIDEMOVE * 7));
                            }
                            else if (k == 3)
                            {
                                //promote to rook
                                NewPos2.AddPiece(postpiecex, postpiecey, 5 + (NewPos.SIDEMOVE * 7));
                            }
                            NewPos2.SIDEMOVE = !NewPos2.SIDEMOVE;

                            vector<vector<vector<int>>> AllPromotionmoves = NewPos2.AllPieceMoves();

                            
                            float tempEval = NewPos2.SingleLayerEval(NewPos2, AllPromotionmoves, false);

                            if (AllPromotionmoves.size() == 0)
                            {
                                tempEval = -30 + 60 * NewPos2.SIDEMOVE;
                            }
                            else if (AllPromotionmoves[0].size() == 1)
                            {
                                tempEval = 0;
                            }
                            else
                            {
                                tempEval = NewPos2.SingleLayerEval(NewPos2, AllPromotionmoves, false);
                            }

                            if (prevpromotioneval == -10000)
                            {
                                promotioneval = tempEval;
                                prevpromotioneval = promotioneval;
                            }
                            else
                            {
                                if (NewPos2.SIDEMOVE == 0) promotioneval = fminf(tempEval, prevpromotioneval);
                                else promotioneval = fmaxf(tempEval, prevpromotioneval);
                                prevpromotioneval = promotioneval;
                            }
                        }

                        NewPos.SIDEMOVE = !NewPos.SIDEMOVE;
                    }
                    else
                    {
                        NewPos.AddPiece(postpiecex, postpiecey, Piecetomove);
                        if (Piecetaken == 0 && postpiecex - piecex != 0)
                        {
                            //en passant
                            NewPos.RemovePiece(postpiecex, postpiecey - 1 + 2 * (!NewPos.SIDEMOVE));
                        }

                        if (abs(postpiecey - piecey) >= 2)
                        {
                            //flying pawn to put in previous move
                            NewPos.AddPrevMove(postpiecex, postpiecey);
                        }
                        else
                        {
                            NewPos.AddPrevMove(-1, -1);
                        }

                        NewPos.AddPiece(postpiecex, postpiecey, Piecetomove);
                        NewPos.SIDEMOVE = !NewPos.SIDEMOVE;
                    }
                }
                else
                {
                    NewPos.AddPiece(postpiecex, postpiecey, Piecetomove);
                    NewPos.AddPrevMove(-1, -1);
                    NewPos.SIDEMOVE = !NewPos.SIDEMOVE;
                }

                vector<vector<vector<int>>> Moves1 = NewPos.AllPieceMoves();


                if (!isPromotion)
                {
                    float tempEval = 0.0f;

                    if (Moves1.size() == 0)
                    {
                        tempEval = -30 + 60 + NewPos.SIDEMOVE;
                    }
                    else if (Moves1[0].size() == 1)
                    {
                        tempEval = 0;
                    }
                    else
                    {
                        tempEval = NewPos.SingleLayerEval(NewPos, Moves1, print);
                    }

                    if (PrevEval == -10000)
                    {
                        eval = tempEval;
                        PrevEval = eval;

                    }
                    else
                    {
                        if (NewPos.SIDEMOVE == 0) eval = fminf(tempEval, PrevEval);
                        else eval = fmaxf(tempEval, PrevEval);
                        PrevEval = eval;
                    }
                }
                else
                {
                    if (PrevEval == -10000)
                    {
                        eval = promotioneval;
                        PrevEval = PrevEval;
                    }
                    else
                    {
                        if (NewPos.SIDEMOVE == 0) eval = fminf(promotioneval, PrevEval);
                        else eval = fmaxf(promotioneval, PrevEval);
                        PrevEval = eval;
                    }

                }
            }
        }




        return eval;
    }

    float givePositionEval(int pieceX, int pieceY, bool PieceTaken, bool print)
    {
        float eval = 0.0f;

        if (PieceTaken)
        {
            float pieceval = 0.0f;
            if (getPieceID(pieceX, pieceY) - 7 * (!SIDEMOVE) == 1) pieceval = 1.0f;
            else if (getPieceID(pieceX, pieceY) - 7 * (!SIDEMOVE) == 2) pieceval = 3.0f;
            else if (getPieceID(pieceX, pieceY) - 7 * (!SIDEMOVE) == 3) pieceval = 3.0f;
            else if (getPieceID(pieceX, pieceY) - 7 * (!SIDEMOVE) == 4) pieceval = 5.5f;
            else if (getPieceID(pieceX, pieceY) - 7 * (!SIDEMOVE) == 5) pieceval = 5.0f;
            else if (getPieceID(pieceX, pieceY) - 7 * (!SIDEMOVE) == 6) pieceval = -1.0f;
            else if (getPieceID(pieceX, pieceY) - 7 * (!SIDEMOVE) == 7)
            {
                pieceval = 9.0f;
            }
            

            bool foundPiecetoTakeBack = false;

            if (PieceTaken && pieceval != -1.0f)
            {
                if (pieceX + 1 < 8)
                {
                    if (getPieceID(pieceX + 1, pieceY + 1 - 2 * SIDEMOVE) == 1 + 7 * SIDEMOVE)
                    {
                        eval -= pieceval * (SIDEMOVE * 2 - 1);
                        foundPiecetoTakeBack = true;
                    }

                }

                if (pieceX - 1 >= 0 && !foundPiecetoTakeBack)
                {
                    if (getPieceID(pieceX - 1, pieceY + 1 - 2 * SIDEMOVE) == 1 + 7 * SIDEMOVE)
                    {
                        eval -= pieceval * (SIDEMOVE * 2 - 1);
                        foundPiecetoTakeBack = true;
                    }
                    else
                    {
                    }
                }
                if (!foundPiecetoTakeBack)
                {
                    bool cangoSS = true;
                    bool cangoDD = true;
                    bool cangoDS = true;
                    bool cangoSD = true;
                    for (int i = 1; i <= max(max(pieceX, pieceY), max(7 - pieceX, 7 - pieceY)); i++)
                    {
                        bool xAdd = pieceX + i < 8, yAdd = pieceY + i < 8, ySub = pieceY - i >= 0, xSub = pieceX - i >= 0;
                        if (!foundPiecetoTakeBack)
                        {

                            if (xAdd && yAdd && !foundPiecetoTakeBack && cangoDD)
                            {
                                if (getPieceID(pieceX + i, pieceY + i) == 2 + 7 * SIDEMOVE || getPieceID(pieceX + i, pieceY + i) == 7 + 7 * SIDEMOVE)
                                {
                                    eval -= pieceval * (SIDEMOVE * 2 - 1);
                                    foundPiecetoTakeBack = true;
                                }
                                else if (getPieceID(pieceX + i, pieceY + i) != 0) cangoDD = false;
                            }



                            if (xSub && ySub && !foundPiecetoTakeBack && cangoSS)
                            {
                                if (getPieceID(pieceX - i, pieceY - i) == 2 + 7 * SIDEMOVE || getPieceID(pieceX - i, pieceY - i) == 7 + 7 * SIDEMOVE)
                                {
                                    eval -= pieceval * (SIDEMOVE * 2 - 1);
                                    foundPiecetoTakeBack = true;
                                }
                                else if (getPieceID(pieceX - i, pieceY - i) != 0) cangoSS = false;
                            }

                            if (xAdd && ySub && !foundPiecetoTakeBack && cangoDS)
                            {
                                if (getPieceID(pieceX + i, pieceY - i) == 2 + 7 * SIDEMOVE || getPieceID(pieceX + i, pieceY - i) == 7 + 7 * SIDEMOVE)
                                {
                                    eval -= pieceval * (SIDEMOVE * 2 - 1);
                                    foundPiecetoTakeBack = true;
                                }
                                else if (getPieceID(pieceX + i, pieceY - i) != 0) cangoDS = false;
                            }
                            if (xSub && yAdd && !foundPiecetoTakeBack && cangoSD)
                            {
                                if (getPieceID(pieceX - i, pieceY + i) == 2 + 7 * SIDEMOVE || getPieceID(pieceX - i, pieceY + i) == 7 + 7 * SIDEMOVE)
                                {
                                    eval -= pieceval * (SIDEMOVE * 2 - 1);
                                    foundPiecetoTakeBack = true;
                                }
                                else if (getPieceID(pieceX - i, pieceY + i) != 0) cangoSD = false;
                            }

                        }
                    }
                }
                if (!foundPiecetoTakeBack)
                {
                    bool cangoSY = true;
                    bool cangoDY = true;
                    bool cangoDX = true;
                    bool cangoSX = true;
                    for (int i = 1; i <= max(max(pieceX, pieceY), max(7 - pieceX, 7 - pieceY)); i++)
                    {
                        if (!foundPiecetoTakeBack)
                        {
                            if (pieceX + i < 8 && !foundPiecetoTakeBack && cangoDX)
                            {
                                if (getPieceID(pieceX + i, pieceY) == 5 + 7 * SIDEMOVE || getPieceID(pieceX + i, pieceY) == 4 + 7 * SIDEMOVE || getPieceID(pieceX + i, pieceY) == 7 + 7 * SIDEMOVE)
                                {
                                    eval -= pieceval * (SIDEMOVE * 2 - 1);
                                    foundPiecetoTakeBack = true;
                                }
                                else if (getPieceID(pieceX + i, pieceY) != 0) cangoDX = false;
                            }
                            if (pieceY + i < 8 && !foundPiecetoTakeBack && cangoDY)
                            {
                                if (getPieceID(pieceX, pieceY + i) == 5 + 7 * SIDEMOVE || getPieceID(pieceX, pieceY + i) == 4 + 7 * SIDEMOVE || getPieceID(pieceX, pieceY + i) == 7 + 7 * SIDEMOVE)
                                {
                                    eval -= pieceval * (SIDEMOVE * 2 - 1);
                                    foundPiecetoTakeBack = true;
                                }
                                else if (getPieceID(pieceX, pieceY + i) != 0) cangoDY = false;
                            }

                            if (pieceY - i >= 0 && !foundPiecetoTakeBack && cangoSY)
                            {
                                if (getPieceID(pieceX, pieceY - i) == 5 + 7 * SIDEMOVE || getPieceID(pieceX, pieceY - i) == 4 + 7 * SIDEMOVE || getPieceID(pieceX, pieceY - i) == 7 + 7 * SIDEMOVE)
                                {

                                    eval -= pieceval * (SIDEMOVE * 2 - 1);
                                    foundPiecetoTakeBack = true;
                                }
                                else if (getPieceID(pieceX, pieceY - i) != 0) cangoSY = false;
                            }
                            if (pieceX - i >= 0 && !foundPiecetoTakeBack && cangoSX)
                            {
                                if (getPieceID(pieceX - i, pieceY) == 5 + 7 * SIDEMOVE || getPieceID(pieceX - i, pieceY) == 4 + 7 * SIDEMOVE || getPieceID(pieceX - i, pieceY) == 7 + 7 * SIDEMOVE)
                                {
                                    eval -= pieceval * (SIDEMOVE * 2 - 1);
                                    foundPiecetoTakeBack = true;
                                }
                                else if (getPieceID(pieceX - i, pieceY) != 0) cangoSX = false;
                            }
                        }
                    }

                }

                if (!foundPiecetoTakeBack)
                {
                    for (int i = -1; i <= 1; i += 2)
                    {
                        for (int j = -1; j <= 1; j += 2)
                        {
                            if (!foundPiecetoTakeBack)
                            {
                                if (pieceX + i * 2 >= 0 && pieceX + i * 2 < 8 && pieceY + j >= 0 && pieceY + j < 8 && !foundPiecetoTakeBack)
                                {
                                    if (Pieces[pieceX + i * 2][pieceY + j] == SIDEMOVE * 7 + 3)
                                    {
                                        eval -= pieceval * (SIDEMOVE * 2 - 1);
                                        foundPiecetoTakeBack = true;
                                    }
                                }
                                if (pieceX + i >= 0 && pieceX + i < 8 && pieceY + j * 2 >= 0 && pieceY + j * 2 < 8 && !foundPiecetoTakeBack)
                                {
                                    if (Pieces[pieceX + i][pieceY + j * 2] == SIDEMOVE * 7 + 3)
                                    {
                                        eval -= pieceval * (SIDEMOVE * 2 - 1);
                                        foundPiecetoTakeBack = true;
                                    }
                                }
                            }
                        }
                    }
                }
                if (!foundPiecetoTakeBack)
                {
                    for (int posX = -1; posX <= 1; posX++)
                    {
                        for (int posY = -1; posY <= 1; posY++)
                        {
                            if (!foundPiecetoTakeBack)
                            {
                                if (posX != 0 || posY != 0)
                                    if (pieceX + posX >= 0 && pieceX + posX < 8 && pieceY + posY >= 0 && pieceY + posY < 8 && !foundPiecetoTakeBack)
                                    {
                                        if (Pieces[pieceX + posX][pieceY + posY] == SIDEMOVE * 7 + 6)
                                        {
                                            eval -= pieceval * (SIDEMOVE * 2 - 1);
                                            foundPiecetoTakeBack = true;
                                        }
                                    }
                            }
                        }
                    }
                }
            }
        }



        float pawnPositionalEval[8][8] = { {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {2.6f, 2.5f, 2.5f, 2.5f,2.5f, 2.5f, 2.5f, 2.6f},
            {2.3f, 1.8f, 1.7f, 1.4f,1.4f, 1.7f, 1.8f, 2.3f},
            {2.0f, 1.3f, 1.0f, 1.7f, 1.8f, 1.0f, 1.3f, 2.0f},
            {1.6f, 0.9f, 0.6f, 1.6f, 1.6f, 0.6f, 0.9f, 1.6f},
            {1.3f, 0.9f, 0.7f, 1.2f, 1.2f, 0.7f, 0.9f, 1.3f},
            {0.8f, 1.0f, 1.3f, 1.0f, 1.0f, 1.3f, 1.0f, 0.8f},
        {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f} };

        float kingPositionalEval[8][8] = { {-4.0f, -4.0f, -4.0f, -5.0f,-5.0f, -4.0f, -4.0f, -4.0f},
            {-4.0f, -4.0f, -4.0f, -5.0f,-5.0f, -4.0f, -4.0f, -4.0f},
            {-4.0f, -4.0f, -4.0f, -5.0f,-5.0f, -4.0f, -4.0f,  -4.0f},
            {-1.8f, -2.3f,  -2.8f,  -2.8f,  -2.8f, -2.8f, -2.3f, -1.8f},
            {-0.9f, -1.3f, -1.6f, -1.8f, -1.8f, -1.6f, -1.3f, -0.9f},
            {0.8f, 0.3f, 0.1f, -0.3f, -0.3f, 0.1f, 0.3f, 0.8f},
            {1.2f, 0.8f, 0.8f, 1.0f, 1.0f, 0.8f, 0.8f, 1.2f},
        {2.0f, 3.0f, 4.0f, 1.0f, 1.0f, 1.0f, 4.0f, 3.0f} };

        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 8; j++)
            {
                if (Pieces[i][j] != 0)
                {
                    if (floor((float)Pieces[i][j] / 8.0f) == 0)
                    {
                        //calculate for white pieces
                        if (Pieces[i][j] == 6)
                        {
                            //calculate for king
                            //eval += kingPositionalEval[j][i];
                        }
                        else if (Pieces[i][j] == 1)
                        {
                            //calculate for pawn
                            //eval += pawnPositionalEval[j][i];
                            eval += 1.0f;
                        }
                        else
                        {
                            //calculate for everything else (nothing special)
                            if (Pieces[i][j] == 2) eval += 3.0f;
                            if (Pieces[i][j] == 3) eval += 3.0f;
                            if (Pieces[i][j] == 4) eval += 5.5f;
                            if (Pieces[i][j] == 5) eval += 5.0f;
                            if (Pieces[i][j] == 7) eval += 9.0f;
                        }
                    }
                    if (floor((float)Pieces[i][j] / 8.0f) == 1)
                    {
                        //calculate for black pieces
                        if (Pieces[i][j] - 7 == 6)
                        {
                            //calculate for king
                            //eval -= kingPositionalEval[7 - j][i];
                        }
                        else if (Pieces[i][j] - 7 == 1)
                        {
                            //calculate for pawn
                            //eval -= pawnPositionalEval[7 - j][i];
                            eval -= 1.0f;
                        }
                        else
                        {
                            //calculate for everything else (nothing special)
                            if (Pieces[i][j] - 7 == 2) eval -= 3.0f;
                            if (Pieces[i][j] - 7 == 3) eval -= 3.0f;
                            if (Pieces[i][j] - 7 == 4) eval -= 5.5f;
                            if (Pieces[i][j] - 7 == 5) eval -= 5.0f;
                            if (Pieces[i][j] - 7 == 7) eval -= 9.0f;
                        }
                    }
                }
            }
        }

        return eval;
    }

    void DoMoveByString(string movestr)
    {

        vector<vector<int>> move;
        int xFound = movestr.find('x');
        if (xFound > 0)
        {

            movestr.erase(xFound, 1);
        }
        int plusFound = movestr.find('+');
        if (plusFound > 0)
        {
            movestr.erase(plusFound, 1);
        }

        int equFound = movestr.find('=');

        int hashFound = movestr.find('#');
        if (hashFound > 0)
        {
            movestr.erase(hashFound, 1);
        }

        if (movestr[0] == 'O')
        {
            //castle move
            if (movestr[3] == '-')
            {
                //long castle
                AddPiece(3, 7 * (!SIDEMOVE), 5 + SIDEMOVE * 7);
                RemovePiece(0, 7 * (!SIDEMOVE));
                AddPiece(2, 7 * (!SIDEMOVE), 6 + SIDEMOVE * 7);
                RemovePiece(4, 7 * (!SIDEMOVE));
                if (getPieceID(7, 7 * (!SIDEMOVE)) == 4 + SIDEMOVE * 7) AddPiece(7, 7 * (!SIDEMOVE), 5 + SIDEMOVE * 7);
            }
            else
            {
                //short castle
                AddPiece(5, 7 * (!SIDEMOVE), 5 + SIDEMOVE * 7);
                RemovePiece(7, 7 * (!SIDEMOVE));
                AddPiece(6, 7 * (!SIDEMOVE), 6 + SIDEMOVE * 7);
                RemovePiece(4, 7 * (!SIDEMOVE));
                if (getPieceID(0, 7 * (!SIDEMOVE)) == 4 + SIDEMOVE * 7) AddPiece(0, 7 * (!SIDEMOVE), 5 + SIDEMOVE * 7);
            }
        }
        else if (equFound > 0)
        {
            //promotion of pawn

            int postY = 7 - (movestr[movestr.size() - 3] - 49);
            int postX = movestr[movestr.size() - 4] - 97;

            int pieceidx;

            if (movestr[movestr.size() - 1] == 'Q') pieceidx = 7;
            else if (movestr[movestr.size() - 1] == 'B') pieceidx = 2;
            else if (movestr[movestr.size() - 1] == 'N') pieceidx = 3;
            else if (movestr[movestr.size() - 1] == 'R') pieceidx = 5;

            int initX;
            int initY = postY + 1 - 2 * SIDEMOVE;
            if (movestr[0] >= 97)
            {
                //specification
                initX = movestr[0] - 97;
            }
            else
            {
                for (int i = -1; i <= 1; i += 2)
                {
                    if (postX + i < 8 && postX + i >= 0) if (Pieces[postX + i][postY + 1 - 2 * SIDEMOVE])
                    {
                        initX = postX + i;
                    }
                }
                if (Pieces[postX][postY + 1 - 2 * SIDEMOVE] == 1 + 7 * SIDEMOVE)
                {
                    initX = postX;
                }
            }


            AddPiece(postX, postY, pieceidx + 7 * SIDEMOVE);
            RemovePiece(initX, initY);

        }
        else
        {
            //normal move
            int postY = 7 - (movestr[movestr.size() - 1] - 49);
            int postX = movestr[movestr.size() - 2] - 97;

            int pieceTaken = getPieceID(postX, postY);

            int initY, initX;
            if (movestr.size() > 2)
            {
                if (movestr[0] >= 97)
                {
                    initY = postY + 1 - 2 * SIDEMOVE;
                    initX = movestr[movestr.size() - 3] - 97;
                    //pawn specifies x axis

                    if (Pieces[postX][postY] == 0)
                    {

                        RemovePiece(postX, initY);
                    }
                        

                    AddPiece(postX, postY, Pieces[initX][initY]);
                    RemovePiece(initX, initY);

                    

                    PreviousMove[0] = -1;
                    PreviousMove[1] = -1;

                }
                else if (movestr[0] >= 66)
                {
                    //specified piece


                    if (movestr.size() - 3 > 0)
                    {
                        //specifies from where
                        if (movestr[movestr.size() - 3] >= 97)
                        {
                            //specifies X axis
                            initX = movestr[movestr.size() - 3] - 97;
                            if (movestr[0] == 'N')
                            {
                                //knight move
                                for (int i = -1; i <= 1; i += 2)
                                {
                                    int addvalue = 3 - abs(postX - initX);
                                    if (postY - (i * addvalue) >= 0 && postY - (i * addvalue) < 8) 
                                        if (Pieces[initX][postY - (i * addvalue)] == SIDEMOVE * 7 + 3 && !CanMoveisPinned({ initX, postY - (i * addvalue) }, { postX, postY })) initY = postY - (i * addvalue);
                                }
                            }
                            else if (movestr[0] == 'B')
                            {
                                //knight move
                                for (int i = -1; i <= 1; i += 2)
                                {
                                    int addvalue = abs(postX - initX);
                                    if (postY - (i * addvalue) >= 0 && postY - (i * addvalue) < 8) 
                                        if (Pieces[initX][postY - (i * addvalue)] == SIDEMOVE * 7 + 2 && !CanMoveisPinned({ initX, postY - (i * addvalue) }, { postX, postY })) initY = postY - (i * addvalue);
                                }
                            }
                            else if (movestr[0] == 'Q')
                            {
                                //knight move
                                for (int i = -1; i <= 1; i += 2)
                                {
                                    int addvalue = abs(postX - initX);
                                    if (postY - (i * addvalue) >= 0 && postY - (i * addvalue) < 8) 
                                        if (Pieces[initX][postY - (i * addvalue)] == SIDEMOVE * 7 + 7 && !CanMoveisPinned({ initX, postY - (i * addvalue) }, { postX, postY })) initY = postY - (i * addvalue);
                                }


                                if (Pieces[initX][postY] == SIDEMOVE * 7 + 7 && !CanMoveisPinned({ initX, postY }, { postX, postY })) initY = postY;


                                if (initX == postX)
                                {
                                    bool foundPiece = false;
                                    bool cangoSY = true;
                                    bool cangoDY = true;
                                    for (int i = 1; i <= max(7 - postY, postY) && !foundPiece; i++)
                                    {
                                        if (postY + i < 8 && cangoDY)
                                        {
                                            if (getPieceID(postX, postY + i) == 7 + 7 * SIDEMOVE && !CanMoveisPinned({ postX, postY + i }, { postX, postY }))
                                            {
                                                initY = postY + i;
                                                foundPiece = true;
                                            }
                                            else if (getPieceID(postX, postY + i) != 0) cangoDY = false;
                                        }
                                        if (postY - i >= 0 && cangoSY)
                                        {
                                            if (getPieceID(postX, postY - i) == 7 + 7 * SIDEMOVE && !CanMoveisPinned({ postX, postY - i }, { postX, postY }))
                                            {
                                                initY = postY - i;
                                                foundPiece = true;
                                            }
                                            else if (getPieceID(postX, postY - i) != 0) cangoSY = false;
                                        }
                                    }
                                }
                            }
                            else if (movestr[0] == 'R')
                            {

                                if ((Pieces[initX][postY] == SIDEMOVE * 7 + 5 || Pieces[initX][postY] == SIDEMOVE * 7 + 4) && !CanMoveisPinned({ initX, postY }, { postX, postY })) initY = postY;

                                if (initX == postX)
                                {
                                    bool foundPiece = false;
                                    bool cangoSY = true;
                                    bool cangoDY = true;
                                    for (int i = 1; i <= max(7 - postY, postY) && !foundPiece; i++)
                                    {
                                        if (postY + i < 8 && cangoDY)
                                        {
                                            if ((getPieceID(postX, postY + i) == 5 + 7 * SIDEMOVE || getPieceID(postX, postY + i) == 4 + 7 * SIDEMOVE) && !CanMoveisPinned({ postX, postY + i }, { postX, postY }))
                                            {
                                                initY = postY + i;
                                                foundPiece = true;
                                            }
                                            else if (getPieceID(postX, postY + i) != 0) cangoDY = false;
                                        }
                                        if (postY - i >= 0 && cangoSY)
                                        {
                                            if ((getPieceID(postX, postY - i) == 5 + 7 * SIDEMOVE || getPieceID(postX, postY - i) == 4 + 7 * SIDEMOVE) && !CanMoveisPinned({ postX, postY - i }, { postX, postY }))
                                            {
                                                initY = postY - i;
                                                foundPiece = true;
                                            }
                                            else if (getPieceID(postX, postY - i) != 0) cangoSY = false;
                                        }
                                    }
                                }
                            }
                        }
                        else if (movestr[movestr.size() - 3] >= 49 && movestr[movestr.size() - 3] < 66 && movestr.size() != 5)
                        {
                            if (movestr.size() == 5)
                            {
                                initX = movestr[1] - 97;
                                initY = movestr[2] - 49;
                            }
                            else
                            {

                                //specifies X axis
                                initY = 7 - (movestr[movestr.size() - 3] - 49);
                                if (movestr[0] == 'N')
                                {
                                    //knight move
                                    for (int i = -1; i <= 1; i += 2)
                                    {
                                        int addvalue = 3 - abs(postY - initY);
                                        if (postX - (i * addvalue) >= 0 && postX - (i * addvalue) < 8) 
                                            if (Pieces[postX - (i * addvalue)][initY] == SIDEMOVE * 7 + 3 && !CanMoveisPinned({ postX - (i * addvalue), initY }, { postX, postY })) initX = postX - (i * addvalue);
                                    }
                                }
                                else if (movestr[0] == 'B')
                                {
                                    //knight move
                                    for (int i = -1; i <= 1; i += 2)
                                    {
                                        int addvalue = abs(postY - initY);
                                        if (postX - (i * addvalue) >= 0 && postX - (i * addvalue) < 8) 
                                            if (Pieces[postX - (i * addvalue)][initY] == SIDEMOVE * 7 + 2 && !CanMoveisPinned({ postX - (i * addvalue), initY }, { postX, postY })) initX = postX - (i * addvalue);
                                    }
                                }
                                else if (movestr[0] == 'Q')
                                {
                                    //knight move
                                    for (int i = -1; i <= 1; i += 2)
                                    {
                                        int addvalue = abs(postY - initY);
                                        if (postX - (i * addvalue) >= 0 && postX - (i * addvalue) < 8) 
                                            if (Pieces[postX - (i * addvalue)][initY] == SIDEMOVE * 7 + 7 && !CanMoveisPinned({ postX - (i * addvalue), initY }, { postX, postY })) initX = postX - (i * addvalue);
                                    }


                                    if (Pieces[postX][initY] == SIDEMOVE * 7 + 7 && !CanMoveisPinned({ postX, initY }, { postX, postY })) initX = postX;

                                    if (initY == postY)
                                    {
                                        bool foundPiece = false;
                                        bool cangoSX = true;
                                        bool cangoDX = true;
                                        for (int i = 1; i <= max(7 - postX, postX) && !foundPiece; i++)
                                        {
                                            if (postX + i < 8 && cangoDX)
                                            {
                                                if (getPieceID(postX + i, postY) == 7 + 7 * SIDEMOVE && !CanMoveisPinned({ postX + i, postY }, { postX, postY }))
                                                {
                                                    initX = postX + i;
                                                    foundPiece = true;
                                                }
                                                else if (getPieceID(postX + i, postY) != 0) cangoDX = false;
                                            }
                                            if (postX - i >= 0 && cangoSX)
                                            {
                                                if (getPieceID(postX - i, postY) == 7 + 7 * SIDEMOVE && !CanMoveisPinned({ postX - i, postY }, { postX, postY }))
                                                {
                                                    initX = postX - i;
                                                    foundPiece = true;
                                                }
                                                else if (getPieceID(postX - i, postY) != 0) cangoSX = false;
                                            }
                                        }
                                    }

                                }
                                else if (movestr[0] == 'R')
                                {

                                    if ((Pieces[postX][initY] == SIDEMOVE * 7 + 5 || Pieces[postX][initY] == SIDEMOVE * 7 + 4) && !CanMoveisPinned({ postX, initY }, { postX, postY })) initX = postX;

                                }
                            }
                        }
                    }
                    else
                    {
                        if (movestr[0] == 'N')
                        {
                            for (int i = -1; i <= 1; i += 2)
                            {
                                for (int j = -1; j <= 1; j += 2)
                                {
                                    if (postX + i * 2 >= 0 && postX + i * 2 < 8 && postY + j >= 0 && postY + j < 8)
                                    {
                                        if (Pieces[postX + i * 2][postY + j] == SIDEMOVE * 7 + 3 && !CanMoveisPinned({ postX + i * 2, postY + j }, { postX, postY }))
                                        {
                                            initX = postX + i * 2;
                                            initY = postY + j;
                                        }
                                    }
                                    if (postX + i >= 0 && postX + i < 8 && postY + j * 2 >= 0 && postY + j * 2 < 8)
                                    {
                                        if (Pieces[postX + i][postY + j * 2] == SIDEMOVE * 7 + 3 && !CanMoveisPinned({ postX + i, postY + j * 2 }, { postX, postY }))
                                        {
                                            initX = postX + i;
                                            initY = postY + j * 2;
                                        }
                                    }
                                }
                            }
                        }
                        else if (movestr[0] == 'B')
                        {
                            bool foundPiece = false;
                            bool cangoSS = true;
                            bool cangoDD = true;
                            bool cangoDS = true;
                            bool cangoSD = true;
                            for (int i = 1; i <= max(max(postX, postY), max(7 - postX, 7 - postY)) && !foundPiece; i++)
                            {
                                bool xAdd = postX + i < 8, yAdd = postY + i < 8, ySub = postY - i >= 0, xSub = postX - i >= 0;

                                if (xAdd && yAdd && cangoDD)
                                {
                                    if (getPieceID(postX + i, postY + i) == 2 + 7 * SIDEMOVE && !CanMoveisPinned({ postX + i, postY + i }, { postX, postY }))
                                    {
                                        initX = postX + i;
                                        initY = postY + i;
                                        foundPiece = true;
                                    }
                                    else if (getPieceID(postX + i, postY + i) != 0) cangoDD = false;
                                }



                                if (xSub && ySub && cangoSS)
                                {
                                    if (getPieceID(postX - i, postY - i) == 2 + 7 * SIDEMOVE && !CanMoveisPinned({ postX - i, postY - i }, { postX, postY }))
                                    {
                                        initX = postX - i;
                                        initY = postY - i;
                                        foundPiece = true;
                                    }
                                    else if (getPieceID(postX - i, postY - i) != 0) cangoSS = false;
                                }

                                if (xAdd && ySub && cangoDS)
                                {
                                    if (getPieceID(postX + i, postY - i) == 2 + 7 * SIDEMOVE && !CanMoveisPinned({ postX + i, postY - i }, { postX, postY }))
                                    {
                                        initX = postX + i;
                                        initY = postY - i;
                                        foundPiece = true;
                                    }
                                    else if (getPieceID(postX + i, postY - i) != 0) cangoDS = false;
                                }
                                if (xSub && yAdd && cangoSD)
                                {
                                    if (getPieceID(postX - i, postY + i) == 2 + 7 * SIDEMOVE && !CanMoveisPinned({ postX - i, postY + i }, { postX, postY }))
                                    {
                                        initX = postX - i;
                                        initY = postY + i;
                                        foundPiece = true;
                                    }
                                    else if (getPieceID(postX - i, postY + i) != 0) cangoSD = false;
                                }
                            }
                        }
                        else if (movestr[0] == 'R')
                        {
                            bool foundPiece = false;
                            bool cangoSY = true;
                            bool cangoDY = true;
                            bool cangoDX = true;
                            bool cangoSX = true;
                            for (int i = 1; i <= max(postX, 7 - postX) && !foundPiece; i++)
                            {
                                if (postX + i < 8 && cangoDX)
                                {
                                    if ((getPieceID(postX + i, postY) == 5 + 7 * SIDEMOVE || getPieceID(postX + i, postY) == 4 + 7 * SIDEMOVE) && !CanMoveisPinned({ postX + i, postY }, { postX, postY }))
                                    {
                                        initX = postX + i;
                                        initY = postY;
                                        foundPiece = true;
                                    }
                                    else if (getPieceID(postX + i, postY) != 0) cangoDX = false;
                                }
                                if (postX - i >= 0 && cangoSX)
                                {
                                    if ((getPieceID(postX - i, postY) == 5 + 7 * SIDEMOVE || getPieceID(postX - i, postY) == 4 + 7 * SIDEMOVE) && !CanMoveisPinned({ postX - i, postY }, { postX, postY }))
                                    {
                                        initX = postX - i;
                                        initY = postY;
                                        foundPiece = true;
                                    }
                                    else if (getPieceID(postX - i, postY) != 0) cangoSX = false;
                                }
                            }

                            for (int i = 1; i <= max(postY, 7 - postY) && !foundPiece; i++)
                            {
                                if (postY + i < 8 && cangoDY)
                                {
                                    if ((getPieceID(postX, postY + i) == 5 + 7 * SIDEMOVE || getPieceID(postX, postY + i) == 4 + 7 * SIDEMOVE) && !CanMoveisPinned({ postX, postY + i }, { postX, postY }))
                                    {
                                        initX = postX;
                                        initY = postY + i;
                                        foundPiece = true;
                                    }
                                    else if (getPieceID(postX, postY + i) != 0) cangoDY = false;
                                }
                                if (postY - i >= 0 && cangoSY)
                                {
                                    if ((getPieceID(postX, postY - i) == 5 + 7 * SIDEMOVE || getPieceID(postX, postY - i) == 4 + 7 * SIDEMOVE) && !CanMoveisPinned({ postX, postY - i }, { postX, postY }))
                                    {
                                        initX = postX;
                                        initY = postY - i;
                                        foundPiece = true;
                                    }
                                    else if (getPieceID(postX, postY - i) != 0) cangoSY = false;
                                }
                            }

                        }
                        else if (movestr[0] == 'Q')
                        {
                            bool foundPiece = false;
                            bool cangoSS = true;
                            bool cangoDD = true;
                            bool cangoDS = true;
                            bool cangoSD = true;
                            for (int i = 1; i <= max(max(postX, postY), max(7 - postX, 7 - postY)) && !foundPiece; i++)
                            {
                                bool xAdd = postX + i < 8, yAdd = postY + i < 8, ySub = postY - i >= 0, xSub = postX - i >= 0;

                                if (xAdd && yAdd && cangoDD)
                                {
                                    if (getPieceID(postX + i, postY + i) == 7 + 7 * SIDEMOVE && !CanMoveisPinned({ postX + i, postY + i }, { postX, postY }))
                                    {
                                        initX = postX + i;
                                        initY = postY + i;
                                        foundPiece = true;
                                    }
                                    else if (getPieceID(postX + i, postY + i) != 0) cangoDD = false;
                                }



                                if (xSub && ySub && cangoSS)
                                {
                                    if (getPieceID(postX - i, postY - i) == 7 + 7 * SIDEMOVE && !CanMoveisPinned({ postX - i, postY - i }, { postX, postY }))
                                    {
                                        initX = postX - i;
                                        initY = postY - i;
                                        foundPiece = true;
                                    }
                                    else if (getPieceID(postX - i, postY - i) != 0) cangoSS = false;
                                }

                                if (xAdd && ySub && cangoDS)
                                {
                                    if (getPieceID(postX + i, postY - i) == 7 + 7 * SIDEMOVE && !CanMoveisPinned({ postX + i, postY - i }, { postX, postY }))
                                    {
                                        initX = postX + i;
                                        initY = postY - i;
                                        foundPiece = true;
                                    }
                                    else if (getPieceID(postX + i, postY - i) != 0) cangoDS = false;
                                }
                                if (xSub && yAdd && cangoSD)
                                {
                                    if (getPieceID(postX - i, postY + i) == 7 + 7 * SIDEMOVE && !CanMoveisPinned({ postX - i, postY + i }, { postX, postY }))
                                    {
                                        initX = postX - i;
                                        initY = postY + i;
                                        foundPiece = true;
                                    }
                                    else if (getPieceID(postX - i, postY + i) != 0) cangoSD = false;
                                }
                            }

                            foundPiece = false;

                            bool cangoSY = true;
                            bool cangoDY = true;
                            bool cangoDX = true;
                            bool cangoSX = true;
                            for (int i = 1; i <= max(postX, 7 - postX) && !foundPiece; i++)
                            {
                                if (postX + i < 8 && cangoDX)
                                {
                                    if (getPieceID(postX + i, postY) == 7 + 7 * SIDEMOVE && !CanMoveisPinned({ postX + i, postY }, { postX, postY }))
                                    {
                                        initX = postX + i;
                                        initY = postY;
                                        foundPiece = true;
                                    }
                                    else if (getPieceID(postX + i, postY) != 0) cangoDX = false;
                                }
                                
                                if (postX - i >= 0 && cangoSX)
                                {
                                    if (getPieceID(postX - i, postY) == 7 + 7 * SIDEMOVE && !CanMoveisPinned({ postX - i, postY }, { postX, postY }))
                                    {
                                        initX = postX - i;
                                        initY = postY;
                                        foundPiece = true;
                                    }
                                    else if (getPieceID(postX - i, postY) != 0) cangoSX = false;
                                }
                            }
                            for (int i = 1; i <= max(postY, 7 - postY) && !foundPiece; i++)
                            {
                                if (postY + i < 8 && cangoDY)
                                {
                                    if (getPieceID(postX, postY + i) == 7 + 7 * SIDEMOVE && !CanMoveisPinned({ postX, postY + i }, { postX, postY }))
                                    {
                                        initX = postX;
                                        initY = postY + i;
                                        foundPiece = true;
                                    }
                                    else if (getPieceID(postX, postY + i) != 0) cangoDY = false;
                                }
                                if (postY - i >= 0 && cangoSY)
                                {
                                    if (getPieceID(postX, postY - i) == 7 + 7 * SIDEMOVE && !CanMoveisPinned({ postX, postY - i }, { postX, postY }))
                                    {
                                        initX = postX;
                                        initY = postY - i;
                                        foundPiece = true;
                                    }
                                    else if (getPieceID(postX, postY - i) != 0) cangoSY = false;
                                }
                            }
                        }
                        else if (movestr[0] == 'K')
                        {
                            for (int posX = -1; posX <= 1; posX++)
                            {
                                for (int posY = -1; posY <= 1; posY++)
                                {
                                    if (posX != 0 || posY != 0)
                                    {

                                        if (postX + posX >= 0 && postX + posX < 8 && postY + posY >= 0 && postY + posY < 8)
                                        {
                                            if (Pieces[postX + posX][postY + posY] == SIDEMOVE * 7 + 6)
                                            {
                                                initX = postX + posX;
                                                initY = postY + posY;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    AddPiece(postX, postY, getPieceID(initX, initY));
                    RemovePiece(initX, initY);

                    PreviousMove[0] = -1;
                    PreviousMove[1] = -1;

                }
            }
            else
            {
                for (int i = -1; i <= 1; i += 2)
                {
                    if (postX + i < 8 && postX + i >= 0) if (Pieces[postX + i][postY + 1 - 2 * SIDEMOVE] == 1 + 7 * SIDEMOVE)
                    {
                        initX = postX + i;
                        initY = postY + 1 - 2 * SIDEMOVE;
                        //if (pieceTaken == 0) RemovePiece(postX, initY);

                        PreviousMove[0] = -1;
                        PreviousMove[1] = -1;
                    }
                }

                if (Pieces[postX][postY + 1 - 2 * SIDEMOVE] == 1 + 7 * SIDEMOVE)
                {
                    initX = postX;
                    initY = postY + 1 - 2 * SIDEMOVE;
                    PreviousMove[0] = -1;
                    PreviousMove[1] = -1;
                }
                else if (Pieces[postX][postY + 2 - 4 * SIDEMOVE] == 1 + 7 * SIDEMOVE)
                {
                    initX = postX;
                    initY = postY + 2 - 4 * SIDEMOVE;
                    PreviousMove[0] = postX;
                    PreviousMove[1] = postY;

                }
                AddPiece(postX, postY, Pieces[initX][initY]);
                RemovePiece(initX, initY);
            }


        }

    }

    bool CanMoveisPinned(vector<int> initPiece, vector<int> postPiece)
    {
        int initX = initPiece[0];
        int initY = initPiece[1];
        int postX = postPiece[0];
        int postY = postPiece[1];

        bool foundpostPieceSS = false;
        bool foundpostPieceSD = false;
        bool foundpostPieceDS = false;
        bool foundpostPieceDD = false;
        bool foundpostPieceSX = false;
        bool foundpostPieceSY = false;
        bool foundpostPieceDX = false;
        bool foundpostPieceDY = false;

        if (abs(postX - initX) != abs(postY - initY) && abs(postX - initX) * abs(postY - initY) != 0)
        {
            //kniht move so nothing
        }
        else if (postX - initX < 0)
        {
            if (postY - initY < 0)
            {
                foundpostPieceDD = true;
            }
            else if (postY - initY == 0)
            {
                foundpostPieceDX = true;
            }
            else
            {
                foundpostPieceDS = true;
            }
        }
        else if (postX - initX == 0)
        {
            if (postY - initY < 0)
            {
                foundpostPieceDY = true;
            }
            else
            {
                foundpostPieceSY = true;
            }
        }
        else
        {
            if (postY - initY < 0)
            {
                foundpostPieceSD = true;
            }
            else if (postY - initY == 0)
            {
                foundpostPieceSX = true;
            }
            else
            {
                foundpostPieceSS = true;
            }
        }

        bool foundKing = false;
        bool foundKingDD = false;
        bool foundKingDS = false;
        bool foundKingSD = false;
        bool foundKingSS = false;
        bool foundAttackPieceSS = false;
        bool foundAttackPieceSD = false;
        bool foundAttackPieceDS = false;
        bool foundAttackPieceDD = false;
        bool cangoSS = true;
        bool cangoDD = true;
        bool cangoDS = true;
        bool cangoSD = true;


        for (int i = 1; i <= max(max(initX, initY), max(7 - initX, 7 - initY)); i++)
        {
            bool xAdd = initX + i < 8, yAdd = initY + i < 8, ySub = initY - i >= 0, xSub = initX - i >= 0;

            if (xAdd && yAdd && cangoDD)
            {
                if (getPieceID(initX + i, initY + i) == 7 + 7 * (!SIDEMOVE) || getPieceID(initX + i, initY + i) == 2 + 7 * (!SIDEMOVE))
                {
                    foundAttackPieceDD = true;
                    cangoDD = false;
                }
                else if(getPieceID(initX + i, initY + i) == 6 + 7 * SIDEMOVE)
                {
                    foundKingDD = true;
                    cangoDD = false;
                }
                else if (getPieceID(initX + i, initY + i) != 0) cangoDD = false;
            }



            if (xSub && ySub && cangoSS)
            {
                if (getPieceID(initX - i, initY - i) == 7 + 7 * (!SIDEMOVE) || getPieceID(initX - i, initY - i) == 2 + 7 * (!SIDEMOVE))
                {
                    foundAttackPieceSS = true;
                    cangoSS = false;
                }
                else if (getPieceID(initX - i, initY - i) == 6 + 7 * SIDEMOVE)
                {
                    foundKingSS = true;
                    cangoSS = false;
                }
                else if (getPieceID(initX - i, initY - i) != 0) cangoSS = false;
            }

            if (xAdd && ySub && cangoDS)
            {
                if (getPieceID(initX + i, initY - i) == 7 + 7 * (!SIDEMOVE) || getPieceID(initX + i, initY - i) == 2 + 7 * (!SIDEMOVE))
                {
                    foundAttackPieceDS = true;
                    cangoDS = false;
                }
                else if (getPieceID(initX + i, initY - i) == 6 + 7 * SIDEMOVE)
                {
                    foundKingDS = true;
                    cangoDS = false;
                }
                else if (getPieceID(initX + i, initY - i) != 0) cangoDS = false;
            }
            if (xSub && yAdd && cangoSD)
            {
                if (getPieceID(initX - i, initY + i) == 7 + 7 * (!SIDEMOVE) || getPieceID(initX - i, initY + i) == 2 + 7 * (!SIDEMOVE))
                {
                    foundAttackPieceSD = true;
                    cangoSD = false;
                }
                else if (getPieceID(initX - i, initY + i) == 6 + 7 * SIDEMOVE)
                {
                    foundKingSD = true;
                    cangoSD = false;
                }
                else if (getPieceID(initX - i, initY + i) != 0) cangoSD = false;
            }
        }

        bool foundKingSX = false;
        bool foundKingSY = false;
        bool foundKingDX = false;
        bool foundKingDY = false;

        bool foundAttackPieceSX = false;
        bool foundAttackPieceSY = false;
        bool foundAttackPieceDX = false;
        bool foundAttackPieceDY = false;

        bool cangoSY = true;
        bool cangoDY = true;
        bool cangoDX = true;
        bool cangoSX = true;
        for (int i = 1; i <= max(max(initX, initY), max(7 - initX, 7 - initY)); i++)
        {
            if (initX + i < 8 && cangoDX)
            {
                if (getPieceID(initX + i, initY) == 7 + 7 * (!SIDEMOVE) || getPieceID(initX + i, initY) == 5 + 7 * (!SIDEMOVE) || getPieceID(initX + i, initY) == 4 + 7 * (!SIDEMOVE))
                {
                    foundAttackPieceDX = true;
                    cangoDX = false;
                }
                else if (getPieceID(initX + i, initY) == 6 + 7 * SIDEMOVE)
                {
                    foundKingDX = true;
                    cangoDX = false;
                }
                else if (getPieceID(initX + i, initY) != 0) cangoDX = false;
            }

            if (initX - i >= 0 && cangoSX)
            {
                if (getPieceID(initX - i, initY) == 7 + 7 * (!SIDEMOVE) || getPieceID(initX - i, initY) == 5 + 7 * (!SIDEMOVE) || getPieceID(initX - i, initY) == 4 + 7 * (!SIDEMOVE))
                {
                    foundAttackPieceSX = true;
                    cangoSX = false;
                }
                else if (getPieceID(initX - i, initY) == 6 + 7 * SIDEMOVE)
                {
                    foundKingSX = true;
                    cangoSX = false;
                }
                else if (getPieceID(initX - i, initY) != 0) cangoSX = false;
            }
            if (initY + i < 8 && cangoDY)
            {
                if (getPieceID(initX, initY + i) == 7 + 7 * (!SIDEMOVE) || getPieceID(initX, initY + i) == 5 + 7 * (!SIDEMOVE) || getPieceID(initX, initY + i) == 4 + 7 * (!SIDEMOVE))
                {
                    foundAttackPieceDY = true;
                    cangoDY = false;
                }
                else if (getPieceID(initX, initY + i) == 6 + 7 * SIDEMOVE)
                {
                    foundKingDY = true;
                    cangoDY = false;
                }
                else if (getPieceID(initX, initY + i) != 0) cangoDY = false;
            }
            if (initY - i >= 0 && cangoSY)
            {
                if (getPieceID(initX, initY - i) == 7 + 7 * (!SIDEMOVE) || getPieceID(initX, initY - i) == 5 + 7 * (!SIDEMOVE) || getPieceID(initX, initY - i) == 4 + 7 * (!SIDEMOVE))
                {
                    foundAttackPieceSY = true;
                    cangoSY = false;
                }
                else if (getPieceID(initX, initY - i) == 6 + 7 * SIDEMOVE)
                {
                    foundKingSY = true;
                    cangoSY = false;
                }
                else if (getPieceID(initX, initY - i) != 0) cangoSY = false;
            }
        }


        //bishop attack
        if (foundAttackPieceSS && foundKingDD)
        {
            if (foundpostPieceSS || foundpostPieceDD)
            {
                return false;
            }
            else return true;
        }
        else if (foundAttackPieceDD && foundKingSS)
        {
            if (foundpostPieceSS || foundpostPieceDD)
            {
                return false;
            }
            else return true;
        }
        else if (foundAttackPieceSD && foundKingDS)
        {
            if (foundpostPieceSD || foundpostPieceDS)
            {
                return false;
            }
            else return true;
        }
        else if (foundAttackPieceDS && foundKingSD)
        {
            if (foundpostPieceSD || foundpostPieceDS)
            {
                return false;
            }
            else return true;
        }
        else if (foundAttackPieceSX && foundKingDX) //rook attack
        {
            if (foundpostPieceSX || foundpostPieceDX)
            {
                return false;
            }
            else return true;
        }
        else if (foundAttackPieceDX && foundKingSX)
        {
            if (foundpostPieceSX || foundpostPieceDX)
            {
                return false;
            }
            else return true;
        }
        else if (foundAttackPieceDY && foundKingSY)
        {
            if (foundpostPieceSY || foundpostPieceDY)
            {
                return false;
            }
            else return true;
        }
        else if (foundAttackPieceSY && foundKingDY)
        {
            if (foundpostPieceSY || foundpostPieceDY)
            {
                return false;
            }
            else return true;
        }
        else
        {
            return false;
        }
    }
};




