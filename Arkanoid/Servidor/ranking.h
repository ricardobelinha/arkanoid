#pragma once
#include "../DLL/structs.h"

#define MAX 256

int initializeRankingInRegistry(Ranking* rank);
void getRankingData(Ranking* rank);
void writeRankingData(Ranking* rank);
void showTopRanking(Ranking* rank);