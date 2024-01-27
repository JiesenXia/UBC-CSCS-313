#include "cache.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/*
 * Initialize a new cache set with the given associativity and index of the first cache line.
 */
static void cache_set_init(cache_set_t *cache_set, unsigned int associativity,
                           cache_line_t *lines, int first_index) {
    cache_set->size = associativity;
    cache_set->lines = lines;
    cache_set->first_index = first_index;
    cache_set->mru_list = malloc(associativity * sizeof(int));

    for (int i = 0; i < associativity; i++) {
        cache_set->lines[first_index + i].is_valid = 0;
        cache_set->mru_list[i] = i;
    }
}

/*
 * Given a value n which is a power of 2 (for example, a block size or a
 * number of sets in a cache), calculate log_2 of n.
 */
static unsigned int logbase2(int value) {
    unsigned int ans = 0;
    while (value > 1) {
        ans++;
        value >>= 1;
    }
    return ans;
}

/*
 * Given a number of bits, return a mask that many bits wide.
 */
static unsigned long maskbits(int nbits) {
    return (1L << nbits) - 1;
}

/*
 * Create a new cache that contains a total of num_bytes bytes, divided into
 * lines each of which is block_size bytes long, with the given associativity,
 * and the given set of cache policies for replacement and write operations.
 */
cache_t *cache_new(size_t num_bytes, size_t block_size,
                   unsigned int associativity, int policies) {

    // Create the cache and initialize constant fields.
    cache_t *cache = (cache_t *)malloc(sizeof(cache_t));
    cache->access_count = 0;
    cache->miss_count = 0;
    cache->policies = policies;

    // Initialize size fields.
    cache->line_size = block_size;
    cache->num_lines = num_bytes / block_size;
    cache->associativity = associativity;
    cache->num_sets = cache->num_lines / associativity;

    // Initialize shifts and masks in cache structure
    uint64_t offset_mask, index_mask;
    unsigned int offset_bits, index_bits;

    offset_bits = logbase2(block_size);
    offset_mask = maskbits(offset_bits);
    index_bits = logbase2(cache->num_sets);
    index_mask = maskbits(index_bits);

    // We shift by the number of bits in the offset to get
    // to the index bits.
    cache->block_offset_mask = offset_mask;
    cache->cache_index_shift = offset_bits;
    cache->cache_index_mask = index_mask << offset_bits;

    // We shift by the number of offset bits and index bits
    // to get the tag bits.
    cache->tag_shift = offset_bits + index_bits;
    cache->tag_mask = maskbits(64 - cache->tag_shift) << cache->tag_shift;

    // Allocate the cache memory
    cache->memory = malloc(num_bytes);
    uint8_t *memory = cache->memory;

    // Initialize cache lines.
    cache->lines = (cache_line_t *)calloc(cache->num_lines, sizeof(cache_line_t));
    for (int i = 0; i < cache->num_lines; i++) {
        cache->lines[i].block = memory;
	memory += cache->line_size;
    }
    
    // Initialize cache sets.
    cache->sets = (cache_set_t *)calloc(cache->num_sets, sizeof(cache_set_t));
    int first_index = 0;
    for (int i = 0; i < cache->num_sets; i++) {
        cache_set_init(&cache->sets[i], associativity, cache->lines, first_index);
	first_index += associativity;
    }

    return cache;
}

/**
 * Frees all memory allocated for a cache.
 */
void cache_free(cache_t *cache) {
    /* TO BE COMPLETED BY THE STUDENT */
    for(int i=0; i<cache->num_sets; i++){
        struct cache_set_s *cache_set = &cache->sets[i];
        for (int j = 0; j < cache->num_lines/cache->num_sets; j++)
            free(cache_set->lines+j);
        free(cache_set->lines);
    }
    free(cache->sets);
    free(cache);
}

/*
 * Determine whether or not a cache line is valid for a given tag.
 */
int cache_line_check_validity_and_tag(cache_line_t *cache_line, uintptr_t tag) {
    /* TO BE COMPLETED BY THE STUDENT */
    if(cache_line->tag == tag)
        return cache_line->is_valid;
    return 0;
}

/*
 * Return long integer data from a cache line.
 */
long cache_line_retrieve_data(cache_line_t *cache_line, size_t offset) {
    /* TO BE COMPLETED BY THE STUDENT */
    return *(cache_line->block+offset/sizeof(uint8_t));
}

/*
 * Move the cache lines inside a cache set so the cache line with the
 * given index is tagged as the most recently used one. The most
 * recently used cache line will be the 0'th one in the set, the
 * second most recently used cache line will be next, etc.  Cache
 * lines whose valid bit is 0 will occur after all cache lines whose
 * valid bit is 1.
 */
static void cache_line_make_mru(cache_t *cache, cache_set_t *cache_set, size_t line_index) {
    int index_of_line_index = -1;
    for (int i = 0; i < cache->associativity; i++) {
        if (cache_set->mru_list[i] == line_index) {
            index_of_line_index = i;
            break;
        }
    }

    for (int i = index_of_line_index - 1; i >= 0; i--) {
        cache_set->mru_list[i + 1] = cache_set->mru_list[i];
    }
    cache_set->mru_list[0] = line_index;
}

/*
 * Retrieve a matching cache line from a set, if one exists.
 */
cache_line_t *cache_set_find_matching_line(cache_t *cache, cache_set_t *cache_set,
                                           uintptr_t tag) {
    /* TO BE COMPLETED BY THE STUDENT */
    /* If the cache replacement is LRU or MRU and you find a matching cache line,
     * don't forget to call cache_line_make_mru(cache, cache_set, i)
     */
    if(cache->policies == CACHE_REPLACEMENTPOLICY_LRU){
        for (int i = 0; i < cache->num_lines/cache->num_sets; i++)
            if(cache_line_check_validity_and_tag(&cache_set->lines[i],tag)){
                cache_line_make_mru(cache,cache_set,i);
                return &cache_set->lines[i];
            }
    }else if(cache->policies == CACHE_REPLACEMENTPOLICY_LRU){
        for (int i = 0; i < cache->num_lines/cache->num_sets; i++)
            if(cache_line_check_validity_and_tag(&cache_set->lines[i],tag)){
                cache_line_make_mru(cache,cache_set,i);
                return &cache_set->lines[i];
            }
    }else if(cache->policies == CACHE_REPLACEMENTPOLICY_RANDOM){
        for (int i = 0; i < cache->num_lines/cache->num_sets; i++)
            if(cache_line_check_validity_and_tag(&cache_set->lines[i],tag))
                return &cache_set->lines[i];
    }
    return 0;
}

/*
 * Function to find a cache line to use for new data. Uses either a
 * line not being used, or a suitable line to be replaced, based on
 * the cache's replacement policy.
 */
cache_line_t *find_available_cache_line(cache_t *cache, cache_set_t *cache_set, func_t generate_random_number) {
    /* TO BE COMPLETED BY THE STUDENT */
    /* If the cache replacement policy is LRU or MRU, don't forget to call
     * cache_line_make_mru(cache, cache_set, i) once you have selected the
     * cache line to use.  To generate a random number in the range [0, n),
     * use "generate_random_number() % n".
     */
    if(cache->policies == CACHE_REPLACEMENTPOLICY_LRU){
        for (int i = 0; i < cache->num_lines/cache->num_sets; i++)
            if(!cache_set->lines[i].is_valid){
                cache_line_make_mru(cache,cache_set,i);
                return &cache_set->lines[i];
            }
        cache_line_make_mru(cache, cache_set, cache_set->mru_list[cache->num_lines/cache->num_sets - 1]);
        return &cache_set->lines[cache_set->mru_list[0]];
    }else if(cache->policies == CACHE_REPLACEMENTPOLICY_MRU){
        for (int i = 0; i < cache->num_lines/cache->num_sets; i++)
            if(!cache_set->lines[i].is_valid){
                cache_line_make_mru(cache,cache_set,i);
                return &cache_set->lines[i];
            }
        cache_line_make_mru(cache, cache_set, cache_set->mru_list[0]);
        return &cache_set->lines[cache_set->mru_list[0]];
    }else if(cache->policies == CACHE_REPLACEMENTPOLICY_RANDOM){
        for (int i = 0; i < cache->num_lines/cache->num_sets; i++)
            if(!cache_set->lines[i].is_valid)
                return &cache_set->lines[i];
        return &cache_set->lines[generate_random_number() % (cache->num_lines/cache->num_sets)];
    }
    return 0;
}

/*
 * Add a block to a given cache set.
 */
static cache_line_t *cache_set_add(cache_t *cache, cache_set_t *cache_set,
                                   uintptr_t address, uintptr_t tag, func_t generate_random_number) {

    // First locate the cache line to use.
    cache_line_t *line = find_available_cache_line(cache, cache_set, generate_random_number);

    // Now set it up.
    line->tag = tag;
    line->is_valid = 1;
    memcpy(line->block, (void *)(address & ~cache->block_offset_mask), cache->line_size);

    // And return it.
    return line;
}

/*
 * Read a single long integer from the cache.
 */
long cache_read(cache_t *cache, uintptr_t address, func_t generate_random_number) {
    /* TO BE COMPLETED BY THE STUDENT */
    int offset = address &  cache->block_offset_mask;
    int index = (address & cache -> cache_index_mask) >> cache->cache_index_shift;
    int tag = address >> cache->tag_shift;
    cache_set_t* set = &cache->sets[index];
    cache_line_t * line = cache_set_find_matching_line(cache, set, tag);

    cache->access_count++;
    if(line == NULL){
        cache->miss_count++;
        line = cache_set_add(cache,set,address,tag,generate_random_number);
    }
    return cache_line_retrieve_data(line,offset);
}

/*
 * Write a single integer to the cache.
 */
void cache_write(cache_t *cache, uintptr_t address, long value, func_t generate_random_number) {
    /* OPTIONAL FUNCTION, YOU ARE NOT REQUIRED TO IMPLEMENT IT */
}

/*
 * Return the number of cache misses since the cache was created.
 */
int cache_miss_count(cache_t *cache) {

    return cache->miss_count;
}

/*
 * Return the number of cache accesses since the cache was created.
 */
int cache_access_count(cache_t *cache) {

    return cache->access_count;
}
