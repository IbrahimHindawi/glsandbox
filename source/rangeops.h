#pragma once

#include "core.h"
#include "hkArray.h"

/*
 * TODO(Ibrahim): simplify rangeArenaInitalize & rangeArenaAppend API into
 * one coherent function call & eliminate that struct. Might require a HashMap.
 */

typedef struct {
    i32 start;
    i32 end;
    i32 length;
    i32 index;
} Range;

typedef struct {
    i32 border;
    i32 last_index;
    i32 maximum;
    hkArray ranges;
} RangeArena;

// RangeArena range_arena;
// RangeArena range_arena_box;

RangeArena *rangeArenaAllocate(u64 count) {
    RangeArena *range_arena = malloc(sizeof(RangeArena));
    // check(range_arena);
    // range_arena->border = count;
    range_arena->border = 0;
    range_arena->last_index = 0;
    range_arena->maximum = count;
    range_arena->ranges = hkArrayCreate(sizeof(Range), count);
    // range_arena->ranges = NULL;
    return range_arena;
}

u32 rangeArenaInitalize(RangeArena *range_arena, u32 new_size) {
    range_arena->border += new_size;
    ((Range *)range_arena->ranges.data)[0] = (Range) { 
        .end = new_size, 
        .length = new_size 
    };
    return 0;
}

u32 rangeArenaAppend(RangeArena *range_arena, u32 new_size) {
    u32 old_size = range_arena->border;
    range_arena->border += new_size;
    assert(range_arena->border < range_arena->maximum);
    range_arena->last_index += 1;
    ((Range *)range_arena->ranges.data)[range_arena->last_index] = (Range){ 
        .start = old_size, 
        .end = old_size + new_size, 
        .length = (new_size + old_size) - old_size, 
        .index = range_arena->last_index 
    };
    return range_arena->last_index;
};

#define rangeArenaIndexPrint(name, range_arena, range_index) { \
    printf("range: %s.\nstart: %d, end: %d, length: %d, index: %d.\n\n", \
            name, \
            ((Range *)range_arena->ranges.data)[range_index].start, \
            ((Range *)range_arena->ranges.data)[range_index].end, \
            ((Range *)range_arena->ranges.data)[range_index].length, \
            ((Range *)range_arena->ranges.data)[range_index].index); \
}
