#include "transposition.h"

void TranspositionTable::clear_table()
{
    // zero out everything
    for (int i = 0; i < TT_ENTRIES; i++)
    {
        entries[i].hash = 0;
        entries[i].best_move = {null, null, QUIET};
        entries[i].node_type = EXACT;
        entries[i].score = 0;
        entries[i].depth = -1;
    }
}

void TranspositionTable::add(u64 hash, Move best_move, TTFlag node_type, int score, int depth, int ply)
{
    u64 index = hash % TT_ENTRIES;
    TTEntry& entry = entries[index];

    // if entry has matching hash, only replace info if depth is greater; otherwise, follow always-replace scheme
    if ((hash == entry.hash && depth >= entry.depth) || hash != entry.hash)
    {
        entry.hash = hash;
        entry.best_move = best_move;
        entry.node_type = node_type;

        // apply special logic for mating scores
        if (is_mate_score(score))
        {
            if (score < 0) entry.score = score - ply;
            else entry.score = score + ply;
        }
        else entry.score = score;
        
        entry.depth = depth;
    }
}

TTEntry TranspositionTable::probe(u64 hash, int ply)
{
    TTEntry entry = entries[hash % TT_ENTRIES];

    // check if hash matches
    if (entry.hash != hash) return {0, {null, null, QUIET}, EXACT, 0, -1};

    // normalize mating scores
    if (is_mate_score(entry.score))
    {
        if (entry.score < 0) entry.score += ply;
        else entry.score -= ply;
    }

    // return entry
    return entry;
}