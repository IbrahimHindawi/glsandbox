#pragma once

#include "core.h"
#include "hkArray.h"
#include <string.h>

/*
 * TODO(Ibrahim): simplify rangeArenaInitalize & rangeArenaAppend API into
 * one coherent function call & eliminate that struct. Might require a HashMap.
 */

typedef char name[32];

typedef struct {
    i32 start;
    i32 end;
    i32 length;
    i32 index;
    name rname;
} Range;

typedef struct {
    i32 border;
    i32 last_index;
    usize maximum;
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

u32 rangeArenaAppend(RangeArena *range_arena, name new_range_name, u32 new_size) {
    if( range_arena->border == 0 ) {
        range_arena->border += new_size;
        ( (Range *)range_arena->ranges.data )[0] = (Range) { 
            .end = new_size, 
            .length = new_size,
        };
        strcpy(((Range *)range_arena->ranges.data)[0].rname, new_range_name);
        return 0;
    } else {
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
        strcpy(((Range *)range_arena->ranges.data)[range_arena->last_index].rname, new_range_name);
        return range_arena->last_index;
    }
};

// TODO(Ibrahim): optimize linear search to binary search
// TODO(Ibrahim): figure out signdness
i32 rangeArenaGetIndex(RangeArena *range_arena, name key) {
    for( i32 i = 0; i < range_arena->ranges.length; i++ ) {
        if( strcmp(((Range *)range_arena->ranges.data)[i].rname, key ) == 0 ) {
            return i;
        }
    }
    return -1;
}

void rangeArenaPrint(RangeArena *range_arena, name key) {
    i32 range_index = rangeArenaGetIndex(range_arena, key);
    Range range = ((Range *)range_arena->ranges.data)[range_index];
    printf("range = { rname: %s, start: %d, end: %d, length: %d, index: %d }\n", 
            range.rname, range.start, range.end, range.length, range.index);
    return;
}

#define rangeArenaIndexPrint(name, range_arena, range_index) { \
    printf("range: %s.\nstart: %d, end: %d, length: %d, index: %d.\n\n", \
            name, \
            ((Range *)range_arena->ranges.data)[range_index].start, \
            ((Range *)range_arena->ranges.data)[range_index].end, \
            ((Range *)range_arena->ranges.data)[range_index].length, \
            ((Range *)range_arena->ranges.data)[range_index].index); \
}
