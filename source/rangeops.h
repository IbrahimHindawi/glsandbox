#include "core.h"

enum {
    RangeIdEnemy,
    RangeIdHero,
    RangeIdProjectiles,
    RangeIdsLength,
} RangeIds;

typedef struct {
    i32 start;
    i32 end;
    i32 length;
} Range;

Range rangeInitialize(Range *range, i32 start, i32 end) {
    Range result = {start, end, end - start};
    return result;
}

typedef struct {
    i32 border;
    i32 last_index;
    Range ranges[RangeIdsLength];
} RangeArena;

RangeArena range_arena;

void rangeArenaInitialize(RangeArena *range_arena, i32 new_size) {
    range_arena->border += new_size;
    range_arena->ranges[0] = (Range){ 0, new_size, new_size };
    range_arena->last_index += 1;
};

void rangeArenaAppend(RangeArena *range_arena, u32 new_size) {
    u32 old_size = range_arena->border;
    range_arena->border += new_size;
    range_arena->ranges[range_arena->last_index] = (Range){ old_size, old_size + new_size, (new_size + old_size) - old_size};
    range_arena->last_index += 1;
};

