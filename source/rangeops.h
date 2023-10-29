#include "core.h"

#define RangeIdsLength 3

typedef struct {
    i32 start;
    i32 end;
    i32 length;
    i32 index;
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

u32 rangeArenaInitialize(RangeArena *range_arena, i32 new_size) {
    // zero here is returned since this is the initial range
    range_arena->border += new_size;
    range_arena->ranges[0] = (Range){ 0, new_size, new_size , 0};
    return 0;
};

u32 rangeArenaAppend(RangeArena *range_arena, u32 new_size) {
    u32 old_size = range_arena->border;
    range_arena->border += new_size;
    range_arena->last_index += 1;
    range_arena->ranges[range_arena->last_index] = (Range){ old_size, old_size + new_size, (new_size + old_size) - old_size, range_arena->last_index };
    return range_arena->last_index;
};


#define rangeArenaIndexPrint(range_arena, range_index) { \
    printf("start: %d, end: %d, length: %d, index: %d.\n", \
            range_arena.ranges[range_index].start, \
            range_arena.ranges[range_index].end, \
            range_arena.ranges[range_index].length, \
            range_arena.ranges[range_index].index); \
}
