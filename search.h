#pragma once
#include "board.h"
#include "moveorder.h"
#include <iostream>

typedef struct SearchFlags {
    bool check_extend;
    bool transposition;
} SearchFlags;

class Search
{
    protected:
        // store best move
        Move best_move;

        // time control
        std::chrono::steady_clock::time_point start_time;
        int time_control;

        // flags
        MoveOrderFlags move_order_flags;
        SearchFlags search_flags;
    public:
        // getters
        virtual Move get_best_move() { return best_move; }

        // setters
        virtual void set_move_order_flags(MoveOrderFlags new_flags) { move_order_flags = new_flags; }
        virtual void set_search_flags(SearchFlags new_flags) { search_flags = new_flags; }
        virtual void set_time_control(int time) { time_control = time; }

        // time
        virtual void start_timer() { start_time = std::chrono::steady_clock::now(); }
        virtual bool time_exceeded() 
        {
            std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
            int time_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            if (time_elapsed >= time_control) return true;
            return false;
        }

        // search
        virtual int search(Board& board, int alpha, int beta, int depth, int ply) = 0;
        virtual Move deepening_search(Board& board)
        {   
            // start timer for search
            start_timer();
            Move best_move_so_far = {null, null, QUIET};

            // iteratively increase depth for seaerch
            for (int i = 1; i < 99; i++)
            {
                // search
                int score = search(board, -MAX_BOUND, MAX_BOUND, i, 0);

                // if we exceed our time limit, stop searching 
                if (time_exceeded()) 
                {
                    break;
                }

                // if we are checkmated or drawn, make move invalid and stop searching
                if (board.is_drawn() || board.is_lost()) 
                {
                    best_move_so_far = {null, null, QUIET};
                    break;
                }

                // otherwise, update best move
                best_move_so_far = get_best_move();
            }

            // return best move found
            return best_move_so_far;
        }
};