#ifndef ZZGO_BOARD_H
#define ZZGO_BOARD_H

#include <stdbool.h>
#include <stdint.h>

#include "stone.h"
#include "move.h"

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect((x), 0)


/* Note that "group" is only chain of stones that is solidly
 * connected for us. */

typedef uint16_t group_t;

struct group {
	uint16_t libs; /* Number of group liberties */
	/* Tried to experiment with tracing group start/end coordinates,
	 * however then we cannot use ro group cache anymore and it does
	 * not pay off. */
};

/* You should treat this struct as read-only. Always call functions below if
 * you want to change it. */

struct board {
	int size;
	int captures[S_MAX];
	float komi;

	int moves;
	struct move last_move;

	/* Stones played on the board */
	char *b; /* enum stone */
	/* Group id the stones are part of; 0 == no group */
	group_t *g;

	/* Cache of group info, indexed by gid */
	struct group *gi;

	/* --- private */
	int last_gid;
	bool use_alloca;
	struct move ko;
};

#define board_atxy(b_, x, y) ((b_)->b[(x) + (b_)->size * (y)])
#define board_at(b_, c) board_atxy(b_, coord_x(c), coord_y(c))

#define group_atxy(b_, x, y) ((b_)->g[x + (b_)->size * (y)])
#define group_at(b_, c) group_atxy(b_, coord_x(c), coord_y(c))

#define board_group(b_, g_) ((b_)->gi[(g_)])
#define board_group_libs(b_, g_) (board_group(b_, g_).libs)

struct board *board_init(void);
struct board *board_copy(struct board *board2, struct board *board1);
void board_done_noalloc(struct board *board);
void board_done(struct board *board);
void board_resize(struct board *board, int size);
void board_clear(struct board *board);

struct FILE;
void board_print(struct board *board, FILE *f);

/* Returns group id, 0 on error */
/* If you want to check if a move is valid, then play it, call this
 * right away; board_valid_move() actually just wraps this. */
int board_play(struct board *board, struct move *m);
/* Like above, but plays random move; the move coordinate is recorded
 * to *coord. This method will never fill your own eye. pass is played
 * when no move can be played. */
void board_play_random(struct board *b, enum stone color, coord_t *coord);

bool board_no_valid_moves(struct board *board, enum stone color);
bool board_valid_move(struct board *board, struct move *m, bool sensible);

bool board_is_liberty_of(struct board *board, coord_t *c, int group);
/* Returns S_NONE if not a 1pt eye, color of owner otherwise. */
enum stone board_is_one_point_eye(struct board *board, coord_t *c);

int board_group_capture(struct board *board, int group);

/* Positive: W wins */
/* board_official_score() is the scoring method for yielding score suitable
 * for external presentation. For fast scoring of two ZZGos playing,
 * use board_fast_score(). */
float board_official_score(struct board *board);
float board_fast_score(struct board *board);


/** Iterators */

#define foreach_point(board_) \
	do { \
		int x, y; \
		for (y = 0; y < board_->size; y++) { \
			for (x = 0; x < board_->size; x++) { \
				coord_t c = { x, y }; c = c; /* shut up gcc */
#define foreach_point_end \
			} \
		} \
	} while (0)

#define foreach_in_group(board_, group_) \
	do { \
		group_t *g__ = board_->g; \
		int group__ = group_; \
		foreach_point(board_) \
			if (unlikely(*g__++ == group__))
#define foreach_in_group_end \
		foreach_point_end; \
	} while (0)

#define foreach_neighbor(board_, coord_) \
	do { \
		coord_t q__[] = { { coord_x(coord_) - 1, coord_y(coord_) }, \
		                       { coord_x(coord_), coord_y(coord_) - 1 }, \
		                       { coord_x(coord_) + 1, coord_y(coord_) }, \
		                       { coord_y(coord_), coord_y(coord_) + 1 } }; \
		int fn__i; \
		for (fn__i = 0; fn__i < 4; fn__i++) { \
			int x = coord_x(q__[fn__i]), y = coord_y(q__[fn__i]); coord_t c = { x, y }; \
			if (unlikely(x < 0 || y < 0 || x >= board_->size || y >= board->size)) \
				continue;
#define foreach_neighbor_end \
		} \
	} while (0)


#endif
