#pragma once
#include "move.h"

typedef enum TTFlag {
    EXACT,
    LOWER_BOUND,
    UPPER_BOUND
} TTFlag;

typedef struct TTEntry {
    u64 hash;
    Move best_move;
    TTFlag node_type;
    int score;
    int depth;

} TTEntry;

class TranspositionTable
{
    private:
        TTEntry* entries;
    public:
        TranspositionTable() { entries = new TTEntry[TT_ENTRIES]; }
        ~TranspositionTable() { delete[] entries; }
        void clear_table();
        void add(u64 hash, Move best_move, TTFlag node_type, int score, int depth, int ply);
        TTEntry probe(u64 hash, int ply);
};