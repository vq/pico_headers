/**
    @file pico_ecs.h
    @brief A pure and simple ECS written in C99.

    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

    Features:
    ---------
    - Written in C99
    - Single header library for easy build system integration
    - Excellent performance
    - Pure ECS design (strict separation between data and logic)
    - Simple and concise API
    - Permissive license (zlib or public domain)

    Summary:
    --------

    This library implements an ECS (Entity-Component-System). Entities
    (sometimes called game objects) are defined by their components. For
    example, an entity might have position, sprite, and physics components.
    Systems operate on the components of entities that match the system's
    requirements. Entities are matched to systems based upon which components
    they have and also the system's matching crieria.

    In the above example, a sprite renderer system would match entities having
    poition and sprite components. The system would send the appropriate
    geometry and texture ID to the game's graphics API.

    Traditional game engines tightly couple state with logic. For example, in
    C++ a game object typically has its own update method that operates
    on that state. They also usually rely heavily on inheritance.

    If the state specified by the class changes this could ripple through the
    class, and perhaps subclasses as well. It could well be that the class no
    longer belongs in the existing class hierarchy forcing even more revisions.
    It could even be true that a class doesn't neatly fit into the inheritance
    tree at all.

    An ECS solves these problems while also granting more flexibility in
    general.

    Please see the examples and unit tests for more details.

    Usage:
    ------

    To use this library in your project, add the following

    > #define PICO_ECS_IMPLEMENTATION
    > #include "pico_ecs.h"

    to a source file (once), then simply include the header normally.

    Constants:
    --------

    - PICO_ECS_MAX_COMPONENTS (default: 32)
    - PICO_ECS_MAX_SYSTEMS (default: 16)

    Must be defined before PICO_ECS_IMPLEMENTATION

    Todo:
    -----
    - Better default assertion macro
    - Port Rogue demo to Windows
*/

#ifndef PICO_ECS_H
#define PICO_ECS_H

#include <stdbool.h> // bool, true, false
#include <stddef.h>  // size_t
#include <stdint.h>  // uint32_t

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief ECS context
 */
typedef struct ecs_s ecs_t;

/**
 * @brief ID used for entity and components
 */
typedef uint32_t ecs_id_t;

/**
 * @brief Return code for update callback and calling functions
 */
typedef int8_t ecs_ret_t;

/**
 * @brief Determine floating point type
 */
#ifdef ECS_USE_FLOAT
    typedef float  ecs_dt_t;
#else
    typedef double ecs_dt_t;
#endif

/**
 * @brief System update callback
 *
 * Systems implement the core logic of an ECS by manipulating entities
 * and components.
 *
 * @param ecs          The ECS instance
 * @param entities     An array of entity IDs managed by the system
 * @param entity_count The number of entities in the array
 * @param dt           The time delta
 * @param udata        The user data associated with the system
 */
typedef ecs_ret_t (*ecs_system_fn)(ecs_t* ecs,
                                   ecs_id_t* entities,
                                   int entity_count,
                                   ecs_dt_t dt,
                                   void* udata);


/**
 * @brief Called when an entity is added to a system
 *
 * @param ecs       The ECS instance
 * @param entity_id The enitty being added
 * @param udata     The user data passed to the callback
 */
typedef void (*ecs_added_fn)(ecs_t* ecs, ecs_id_t entity_id, void * udata);

/**
 * @brief Called when an entity is removed from a system
 *
 * @param ecs       The ECS instance
 * @param entity_id The enitty being removed
 * @param udata     The user data passed to the callback
 */
typedef void (*ecs_removed_fn)(ecs_t* ecs, ecs_id_t entity_id, void *udata);

/**
 * @brief Creates an ECS instance.
 *
 * @param entity_count The inital number of pooled entities
 * @param mem_ctx The  Context for a custom allocator
 *
 * @returns An ECS instance or NULL if out of memory
 */
ecs_t* ecs_new(size_t entity_count, void* mem_ctx);

/**
 * @brief Destroys an ECS instance
 *
 * @param ecs The ECS instance
 */
void ecs_free(ecs_t* ecs);

/**
 * @brief Removes all entities from the ECS, preserving systems and components.
 */
void ecs_reset(ecs_t* ecs);

/**
 * @brief Registers a component
 *
 * Registers a component with the specfied component ID. Components define the
 * game state (usually contained within structs) and are manipulated by systems.
 *
 * @param ecs      The ECS instance
 * @param comp_id  The component ID to use (must be less than
 *                 ECS_MAX_COMPONENTS)
 * @param size     The number of bytes to allocate for each component instance
 */
void ecs_register_component(ecs_t* ecs, ecs_id_t comp_id, size_t size);

/**
 * @brief Registers a system
 *
 * Registers a system with the user specified system ID. Systems contain the
 * core logic of a game by manipulating game state as defined by components.
 *
 * @param ecs       The ECS instance
 * @param sys_id    The system ID to use (must be less than ECS_MAX_SYSTEMS)
 * @param system_cb Callback that is fired every update
 * @param add_cb    Called when an entity is added to the system (can be NULL)
 * @param remove_cb Called when an entity is removed from the system (can be NULL)
 * @param udata     The user data passed to the callbacks
 */
void ecs_register_system(ecs_t* ecs,
                         ecs_id_t sys_id,
                         ecs_system_fn system_cb,
                         ecs_added_fn add_cb,
                         ecs_removed_fn remove_cb,
                         void* udata);
/**
 * @brief Determines which components are available to the specified system.
 *
 * @param ecs     The ECS instance
 * @param sys_id  The target system ID
 * @param comp_id The component ID
 */
void ecs_require_component(ecs_t* ecs, ecs_id_t sys_id, ecs_id_t comp_id);

/**
 * @brief Enables a system
 *
 * @param ecs    The ECS instance
 * @param sys_id The specified system ID
 */
void ecs_enable_system(ecs_t* ecs, ecs_id_t sys_id);

/**
 * @brief Disables a system
 *
 * @param ecs    The ECS instance
 * @param sys_id The specified system ID
 */
void ecs_disable_system(ecs_t* ecs, ecs_id_t sys_id);

/**
 * @brief Creates an entity
 *
 * @param ecs The ECS instance
 *
 * @returns The new entity ID
 */
ecs_id_t ecs_create(ecs_t* ecs);

/**
 * @brief Returns true if the entity is currently active
 *
 * @param ecs The ECS instance
 * @param entity_id The target entity
 */
bool ecs_is_ready(ecs_t* ecs, ecs_id_t entity_id);

/**
 * @brief Destroys an entity
 *
 * Destroys an entity, releasing resources and returning it to the pool.
 *
 * @param ecs       The ECS instance
 * @param entity_id The ID of the entity to destroy
 */
void ecs_destroy(ecs_t* ecs, ecs_id_t entity_id);

/**
 * @brief Test if entity has the specified component
 *
 * @param ecs       The ECS instance
 * @param entity_id The entity ID
 * @param comp_id   The component ID
 *
 * @returns True if the entity has the component
 */
bool ecs_has(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id);

/**
 * @brief Adds a component instance to an entity
 *
 * @param ecs       The ECS instance
 * @param entity_id The entity ID
 * @param comp_id   The component ID
 *
 * @returns The component instance
 */
void* ecs_add(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id);

/**
 * @brief Gets a component instance associated with an entity
 *
 * @param ecs       The ECS instance
 * @param entity_id The entity ID
 * @param comp_id   The component ID
 *
 * @returns The component instance
 */
void* ecs_get(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id);

/**
 * @brief Removes a component instance from an entity
 *
 * @param ecs       The ECS instance
 * @param entity_id The entity ID
 * @param comp_id   The component ID
 */
void ecs_remove(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id);

/**
 * @brief Queues an entity for destruction at the end of system execution
 *
 * Queued entities are destroyed after the curent iteration.
 *
 * @param ecs       The ECS instance
 * @param entity_id The ID of the entity to destroy
 */
void ecs_queue_destroy(ecs_t* ecs, ecs_id_t entity_id);

/**
 * @brief Queues a component for removable
 *
 * Queued entity/component pairs that will be deleted after the current system
 * returns
 *
 * @param ecs       The ECS instance
 * @param entity_id The ID of the entity that has the component
 * @param comp_id   The component to remove
 */
void ecs_queue_remove(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id);

/**
 * @brief Update an individual system
 *
 * This function should be called once per frame.
 *
 * @param ecs The ECS instance
 * @param sys_id The system to update
 * @param dt  The time delta
 */
ecs_ret_t ecs_update_system(ecs_t* ecs, ecs_id_t sys_id, ecs_dt_t dt);

/**
 * @brief Updates all systems
 *
 * This function should be called once per frame.
 *
 * @param ecs The ECS instance
 * @param dt  The time delta
 */
ecs_ret_t ecs_update_systems(ecs_t* ecs, ecs_dt_t dt);

#ifdef __cplusplus
}
#endif

#endif // PICO_ECS_H

#ifdef PICO_ECS_IMPLEMENTATION // Define once

#include <stdint.h> // uint32_t
#include <stdlib.h> // malloc, realloc, free
#include <string.h> // memcpy, memset

static const size_t ECS_NULL = (size_t)-1;

#ifndef PICO_ECS_MAX_COMPONENTS
#define PICO_ECS_MAX_COMPONENTS 32
#endif

#ifndef PICO_ECS_MAX_SYSTEMS
#define PICO_ECS_MAX_SYSTEMS 16
#endif

#ifdef NDEBUG
    #define PICO_ECS_ASSERT(expr) ((void)0)
#else
    #ifndef PICO_ECS_ASSERT
        #include <assert.h>
        #define PICO_ECS_ASSERT(expr) (assert(expr))
    #endif
#endif

#if !defined(PICO_ECS_MALLOC) || !defined(PICO_ECS_REALLOC) || !defined(PICO_ECS_FREE)
#include <stdlib.h>
#define PICO_ECS_MALLOC(size, ctx)       (malloc(size))
#define PICO_ECS_REALLOC(ptr, size, ctx) (realloc(ptr, size))
#define PICO_ECS_FREE(ptr, ctx)          (free(ptr))
#endif

/*=============================================================================
 * Internal aliases
 *============================================================================*/

#define ECS_ASSERT          PICO_ECS_ASSERT
#define ECS_MAX_COMPONENTS  PICO_ECS_MAX_COMPONENTS
#define ECS_MAX_SYSTEMS     PICO_ECS_MAX_SYSTEMS
#define ECS_MALLOC          PICO_ECS_MALLOC
#define ECS_REALLOC         PICO_ECS_REALLOC
#define ECS_FREE            PICO_ECS_FREE

/*=============================================================================
 * Internal data structures
 *============================================================================*/

#if ECS_MAX_COMPONENTS <= 32
typedef uint32_t ecs_bitset_t;
#elif ECS_MAX_COMPONENTS <= 64
typedef uint64_t ecs_bitset_t;
#else
#define ECS_BITSET_WIDTH 64
#define ECS_BITSET_SIZE (((ECS_MAX_COMPONENTS - 1) / ECS_BITSET_WIDTH) + 1)

typedef struct
{
    uint64_t array[ECS_BITSET_SIZE];
} ecs_bitset_t;
#endif // ECS_MAX_COMPONENTS

// Data-structure for a packed array implementation that provides O(1) functions
// for adding, removing, and accessing entity IDs
typedef struct
{
    size_t    capacity;
    size_t    size;
    size_t*   sparse;
    ecs_id_t* dense;
} ecs_sparse_set_t;

// Data-structure for an ID pool that provides O(1) operations for pooling IDs
typedef struct
{
    size_t    capacity;
    size_t    size;
    ecs_id_t* array;
} ecs_stack_t;

typedef struct
{
    size_t capacity;
    size_t count;
    size_t size;
    void*  data;
} ecs_array_t;

typedef struct
{
    ecs_bitset_t comp_bits;
    bool         ready;
} ecs_entity_t;

typedef struct
{
    bool             ready;
    bool             active;
    ecs_sparse_set_t entity_ids;
    ecs_system_fn    system_cb;
    ecs_added_fn     add_cb;
    ecs_removed_fn   remove_cb;
    ecs_bitset_t     comp_bits;
    void*            udata;
} ecs_sys_t;

struct ecs_s
{
    ecs_stack_t   entity_pool;
    ecs_stack_t   destroy_queue;
    ecs_stack_t   remove_queue;
    ecs_entity_t* entities;
    size_t        entity_count;
    ecs_array_t   comps[ECS_MAX_COMPONENTS];
    size_t        comp_count;
    //ecs_array_t   systems;
    ecs_sys_t     systems[ECS_MAX_SYSTEMS];
    void*         mem_ctx;
};

/*=============================================================================
 * Internal realloc wrapper
 *============================================================================*/
void* ecs_realloc_zero(ecs_t* ecs, void* ptr, size_t old_size, size_t new_size);

/*=============================================================================
 * Internal functions to flush destroyed entities and removed component
 *============================================================================*/
static void ecs_flush_destroyed(ecs_t* ecs);
static void ecs_flush_removed(ecs_t* ecs);

/*=============================================================================
 * Internal bit set functions
 *============================================================================*/
static inline void  ecs_bitset_flip(ecs_bitset_t* set, int bit, bool on);
static inline bool  ecs_bitset_test(ecs_bitset_t* set, int bit);
static inline ecs_bitset_t ecs_bitset_and(ecs_bitset_t* set1, ecs_bitset_t* set2);
static inline bool  ecs_bitset_equal(ecs_bitset_t* set1, ecs_bitset_t* set2);
static inline bool  ecs_bitset_true(ecs_bitset_t* set);

/*=============================================================================
 * Internal sparse set functions
 *============================================================================*/
static void   ecs_sparse_set_init(ecs_t* ecs, ecs_sparse_set_t* set, size_t capacity);
static void   ecs_sparse_set_free(ecs_t* ecs, ecs_sparse_set_t* set);
static bool   ecs_sparse_set_add(ecs_t* ecs, ecs_sparse_set_t* set, ecs_id_t id);
static size_t ecs_sparse_set_find(ecs_sparse_set_t* set, ecs_id_t id);
static bool   ecs_sparse_set_remove(ecs_sparse_set_t* set, ecs_id_t id);

/*=============================================================================
 * Internal system entity add/remove functions
 *============================================================================*/
static bool ecs_entity_system_test(ecs_bitset_t* sys_bits,
                                   ecs_bitset_t* entity_bits);

/*=============================================================================
 * Internal ID pool functions
 *============================================================================*/
static void     ecs_stack_init(ecs_t* ecs, ecs_stack_t* pool, int capacity);
static void     ecs_stack_free(ecs_t* ecs, ecs_stack_t* pool);
static void     ecs_stack_push(ecs_t* ecs, ecs_stack_t* pool, ecs_id_t id);
static ecs_id_t ecs_stack_pop(ecs_stack_t* pool);
static int      ecs_stack_size(ecs_stack_t* pool);

/*=============================================================================
 * Internal array functions
 *============================================================================*/
static void   ecs_array_init(ecs_t* ecs, ecs_array_t* array, size_t size, size_t capacity);
static void   ecs_array_free(ecs_t* ecs, ecs_array_t* array);
static void   ecs_array_resize(ecs_t* ecs, ecs_array_t* array, size_t capacity);
static void   ecs_array_push(ecs_t* ecs, ecs_array_t* array, void* item);
static size_t ecs_array_count(const ecs_array_t* array);
static void*  ecs_array_get(const ecs_array_t* array, size_t index);

/*=============================================================================
 * Internal validation functions
 *============================================================================*/
#ifndef NDEBUG
static bool ecs_is_not_null(void* ptr);
static bool ecs_is_valid_component_id(ecs_id_t id);
static bool ecs_is_valid_system_id(ecs_id_t id);
static bool ecs_is_entity_ready(ecs_t* ecs, ecs_id_t entity_id);
static bool ecs_is_component_ready(ecs_t* ecs, ecs_id_t comp_id);
static bool ecs_is_system_ready(ecs_t* ecs, ecs_id_t sys_id);
#endif // NDEBUG
/*=============================================================================
 * Public API implementation
 *============================================================================*/

ecs_t* ecs_new(size_t entity_count, void* mem_ctx)
{
    ECS_ASSERT(entity_count > 0);

    ecs_t* ecs = (ecs_t*)ECS_MALLOC(sizeof(ecs_t), mem_ctx);

    // Out of memory
    if (NULL == ecs)
        return NULL;

    memset(ecs, 0, sizeof(ecs_t));

    ecs->entity_count = entity_count;
    ecs->mem_ctx      = mem_ctx;

    // Initialize entity pool and queues
    ecs_stack_init(ecs, &ecs->entity_pool,   entity_count);
    ecs_stack_init(ecs, &ecs->destroy_queue, entity_count);
    ecs_stack_init(ecs, &ecs->remove_queue,  entity_count * 2);

    // Allocate entity array
    ecs->entities = (ecs_entity_t*)ECS_MALLOC(ecs->entity_count * sizeof(ecs_entity_t),
                                              ecs->mem_ctx);

    // Zero entity array
    memset(ecs->entities, 0, ecs->entity_count * sizeof(ecs_entity_t));

    // Pre-populate the the ID pool
    for (ecs_id_t id = 0; id < entity_count; id++)
    {
        ecs_stack_push(ecs, &ecs->entity_pool, id);
    }

    return ecs;
}

void ecs_free(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));

    ecs_stack_free(ecs, &ecs->entity_pool);
    ecs_stack_free(ecs, &ecs->destroy_queue);
    ecs_stack_free(ecs, &ecs->remove_queue);

    for (ecs_id_t comp_id = 0; comp_id < ecs->comp_count; comp_id++)
    {
        ecs_array_t* comp = &ecs->comps[comp_id];
        ecs_array_free(ecs, comp);
    }

    for (ecs_id_t sys_id = 0; sys_id < ECS_MAX_SYSTEMS; sys_id++)
    {
        ecs_sys_t* sys = &ecs->systems[sys_id];

        if (!sys->ready)
            continue;

        ecs_sparse_set_free(ecs, &sys->entity_ids);
    }

    ECS_FREE(ecs->entities, ecs->mem_ctx);
    ECS_FREE(ecs, ecs->mem_ctx);
}

void ecs_reset(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));

    ecs->entity_pool.size   = 0;
    ecs->destroy_queue.size = 0;
    ecs->remove_queue.size  = 0;

    memset(ecs->entities, 0, ecs->entity_count * sizeof(ecs_entity_t));

    for (ecs_id_t entity_id = 0; entity_id < ecs->entity_count; entity_id++)
    {
        ecs_stack_push(ecs, &ecs->entity_pool, entity_id);
    }

    for (ecs_id_t sys_id = 0; sys_id < ECS_MAX_SYSTEMS; sys_id++)
    {
        ecs->systems[sys_id].entity_ids.size = 0;
    }
}

void ecs_register_component(ecs_t* ecs, ecs_id_t comp_id, size_t size)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_component_id(comp_id));
    ECS_ASSERT(!ecs_is_component_ready(ecs, comp_id));
    ECS_ASSERT(size > 0);

    ecs_array_t* comp = &ecs->comps[comp_id];
    ecs_array_init(ecs, comp, size, ecs->entity_count);
}

void ecs_register_system(ecs_t* ecs,
                         ecs_id_t sys_id,
                         ecs_system_fn system_cb,
                         ecs_added_fn add_cb,
                         ecs_removed_fn remove_cb,
                         void* udata)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys_id));
    ECS_ASSERT(!ecs_is_system_ready(ecs, sys_id));
    ECS_ASSERT(NULL != system_cb);

    ecs_sys_t* sys = &ecs->systems[sys_id];

    ecs_sparse_set_init(ecs, &sys->entity_ids, ecs->entity_count);

    sys->ready = true;
    sys->active = true;
    sys->system_cb = system_cb;
    sys->add_cb = add_cb;
    sys->remove_cb = remove_cb;
    sys->udata = udata;
}

void ecs_require_component(ecs_t* ecs, ecs_id_t sys_id, ecs_id_t comp_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys_id));
    ECS_ASSERT(ecs_is_valid_component_id(comp_id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys_id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp_id));

    // Set system component bit for the specified component
    ecs_sys_t* sys = &ecs->systems[sys_id];
    ecs_bitset_flip(&sys->comp_bits, comp_id, true);
}

void ecs_enable_system(ecs_t* ecs, ecs_id_t sys_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys_id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys_id));

    ecs_sys_t* sys = &ecs->systems[sys_id];
    sys->active = true;
}

void ecs_disable_system(ecs_t* ecs, ecs_id_t sys_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys_id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys_id));

    ecs_sys_t* sys = &ecs->systems[sys_id];
    sys->active = false;
}

ecs_id_t ecs_create(ecs_t* ecs)
{
    ECS_ASSERT(ecs_is_not_null(ecs));

    ecs_stack_t* pool = &ecs->entity_pool;

    // If pool is empty, increase the number of entity IDs
    if (0 == ecs_stack_size(pool))
    {
        size_t old_count = ecs->entity_count;
        size_t new_count = old_count + ((old_count + 1) >> 1);

        // Reallocates entities and zeros new ones
        ecs->entities = (ecs_entity_t*)ecs_realloc_zero(ecs, ecs->entities,
                                                        old_count * sizeof(ecs_entity_t),
                                                        new_count * sizeof(ecs_entity_t));

        // Push new entity IDs into the pool
        for (ecs_id_t id = old_count; id < new_count; id++)
        {
            ecs_stack_push(ecs, pool, id);
        }

        // Update entity count
        ecs->entity_count = new_count;
    }

    ecs_id_t entity_id = ecs_stack_pop(pool);
    ecs->entities[entity_id].ready = true;

    return entity_id;
}

bool ecs_is_ready(ecs_t* ecs, ecs_id_t entity_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));

    return ecs->entities[entity_id].ready;
}

void ecs_destroy(ecs_t* ecs, ecs_id_t entity_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    // Load entity
    ecs_entity_t* entity = &ecs->entities[entity_id];

    // Remove entity from systems
    for (ecs_id_t sys_id = 0; sys_id < ECS_MAX_SYSTEMS; sys_id++)
    {
        ecs_sys_t* sys = &ecs->systems[sys_id];

        if (!sys->ready)
            continue;

        // Just attempting to remove the entity from the sparse set is faster
        // than calling ecs_entity_system_test
        if (ecs_sparse_set_remove(&sys->entity_ids, entity_id))
        {
            if (sys->remove_cb)
                sys->remove_cb(ecs, entity_id, sys->udata);
        }
    }

    // Push entity ID back into pool
    ecs_stack_t* pool = &ecs->entity_pool;
    ecs_stack_push(ecs, pool, entity_id);

    // Reset entity (sets bitset to 0 and ready to false)
    memset(entity, 0, sizeof(ecs_entity_t));
}

bool ecs_has(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_component_id(comp_id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    // Load  entity
    ecs_entity_t* entity = &ecs->entities[entity_id];

    // Return true if the component belongs to the entity
    return ecs_bitset_test(&entity->comp_bits, comp_id);
}

void* ecs_get(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_component_id(comp_id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp_id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    // Return pointer to component
    ecs_array_t* comp = &ecs->comps[comp_id]; //  eid0,  eid1   eid2, ...
                                              // [comp0, comp1, comp2, ...]
    return (char*)comp->data + (comp->size * entity_id);
}

void* ecs_add(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_component_id(comp_id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity_id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp_id));

    // Load entity
    ecs_entity_t* entity = &ecs->entities[entity_id];

    // Load component
    ecs_array_t* comp = &ecs->comps[comp_id];

    // Set entity component bit that determines which systems this entity
    // belongs to
    ecs_bitset_flip(&entity->comp_bits, comp_id, true);

    // Add entity to systems
    for (ecs_id_t sys_id = 0; sys_id < ECS_MAX_SYSTEMS; sys_id++)
    {
        ecs_sys_t* sys = &ecs->systems[sys_id];

        if (!sys->ready)
            continue;

        if (ecs_entity_system_test(&sys->comp_bits, &entity->comp_bits))
        {
            if (ecs_sparse_set_add(ecs, &sys->entity_ids, entity_id))
            {
                if (sys->add_cb)
                    sys->add_cb(ecs, entity_id, sys->udata);
            }
        }
    }

    // Grow the component array
    ecs_array_resize(ecs, comp, entity_id);

    // Get pointer to component
    void* ptr = ecs_get(ecs, entity_id, comp_id);

    // Reset component
    memset(ptr, 0, comp->size);

    // Return component
    return ptr;
}

void ecs_remove(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_component_id(comp_id));
    ECS_ASSERT(ecs_is_component_ready(ecs, comp_id));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    // Load entity
    ecs_entity_t* entity = &ecs->entities[entity_id];

    // Remove entity from systems
    for (ecs_id_t sys_id = 0; sys_id < ECS_MAX_SYSTEMS; sys_id++)
    {
        ecs_sys_t* sys = &ecs->systems[sys_id];

        if (!sys->ready)
            continue;

        if (ecs_entity_system_test(&sys->comp_bits, &entity->comp_bits))
        {
            if (ecs_sparse_set_remove(&sys->entity_ids, entity_id))
            {
                if (sys->add_cb)
                    sys->add_cb(ecs, entity_id, sys->udata);
            }
        }
    }

    // Reset the relevant component mask bit
    ecs_bitset_flip(&entity->comp_bits, comp_id, false);
}

void ecs_queue_destroy(ecs_t* ecs, ecs_id_t entity_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity_id));

    ecs_stack_push(ecs, &ecs->destroy_queue, entity_id);
}

void ecs_queue_remove(ecs_t* ecs, ecs_id_t entity_id, ecs_id_t comp_id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_entity_ready(ecs, entity_id));
    ECS_ASSERT(ecs_has(ecs, entity_id, comp_id));

    ecs_stack_push(ecs, &ecs->remove_queue, entity_id);
    ecs_stack_push(ecs, &ecs->remove_queue, comp_id);
}

ecs_ret_t ecs_update_system(ecs_t* ecs, ecs_id_t sys_id, ecs_dt_t dt)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_valid_system_id(sys_id));
    ECS_ASSERT(ecs_is_system_ready(ecs, sys_id));
    ECS_ASSERT(dt >= 0.0f);

    ecs_sys_t* sys = &ecs->systems[sys_id];

    if (!sys->active)
        return 0;

    ecs_ret_t code = sys->system_cb(ecs,
                     sys->entity_ids.dense,
                     sys->entity_ids.size,
                     dt,
                     sys->udata);

    ecs_flush_destroyed(ecs);
    ecs_flush_removed(ecs);

    return code;
}

ecs_ret_t ecs_update_systems(ecs_t* ecs, ecs_dt_t dt)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(dt >= 0.0f);

    for (ecs_id_t sys_id = 0; sys_id < ECS_MAX_SYSTEMS; sys_id++)
    {
        ecs_sys_t* sys = &ecs->systems[sys_id];

        if (!sys->ready)
            continue;

        ecs_ret_t code = ecs_update_system(ecs, sys_id, dt);

        if (0 != code)
            return code;
    }

    return 0;
}

/*=============================================================================
 * Internal realloc wrapper
 *============================================================================*/
void* ecs_realloc_zero(ecs_t* ecs, void* ptr, size_t old_size, size_t new_size)
{
    (void)ecs;

    ptr = ECS_REALLOC(ptr, new_size, ecs->mem_ctx);

    if (new_size > old_size && ptr) {
        size_t diff = new_size - old_size;
        void* start = ((char*)ptr)+ old_size;
        memset(start, 0, diff);
    }

    return ptr;
}

/*=============================================================================
 * Internal functions to flush destroyed entity and removed component
 *============================================================================*/

static void ecs_flush_destroyed(ecs_t* ecs)
{
    ecs_stack_t* destroy_queue = &ecs->destroy_queue;

    for (size_t i = 0; i < destroy_queue->size; i++)
    {
        ecs_id_t entity_id = destroy_queue->array[i];

        if (ecs_is_ready(ecs, entity_id))
            ecs_destroy(ecs, entity_id);
    }

    destroy_queue->size = 0;
}

static void ecs_flush_removed(ecs_t* ecs)
{
    ecs_stack_t* remove_queue = &ecs->remove_queue;

    for (size_t i = 0; i < remove_queue->size; i += 2)
    {
        ecs_id_t entity_id = remove_queue->array[i];

        if (ecs_is_ready(ecs, entity_id))
        {
            ecs_id_t comp_id = remove_queue->array[i + 1];
            ecs_remove(ecs, entity_id, comp_id);
        }
    }

    remove_queue->size = 0;
}


/*=============================================================================
 * Internal bitset functions
 *============================================================================*/

#if ECS_MAX_COMPONENTS <= 64

static inline void ecs_bitset_flip(ecs_bitset_t* set, int bit, bool on)
{
    if (on)
        *set |=  (1 << bit);
    else
        *set &= ~(1 << bit);
}

static inline bool ecs_bitset_test(ecs_bitset_t* set, int bit)
{
    return *set & (1 << bit);
}

static inline ecs_bitset_t ecs_bitset_and(ecs_bitset_t* set1, ecs_bitset_t* set2)
{
    return *set1 & *set2;
}

static inline bool ecs_bitset_equal(ecs_bitset_t* set1, ecs_bitset_t* set2)
{
    return *set1 == *set2;
}

static inline bool ecs_bitset_true(ecs_bitset_t* set)
{
    return *set;
}

#else // ECS_MAX_COMPONENTS

static inline void ecs_bitset_flip(ecs_bitset_t* set, int bit, bool on)
{
    int index = bit / ECS_BITSET_WIDTH;

    if (on)
        set->array[index] |=  (1 << bit % ECS_BITSET_WIDTH);
    else
        set->array[index] &= ~(1 << bit % ECS_BITSET_WIDTH);
}

static inline bool ecs_bitset_test(ecs_bitset_t* set, int bit)
{
    int index = bit / ECS_BITSET_WIDTH;
    return set->array[index] & (1 << bit % ECS_BITSET_WIDTH);
}

static inline ecs_bitset_t ecs_bitset_and(ecs_bitset_t* set1,
                                          ecs_bitset_t* set2)
{
    ecs_bitset_t set;

    for (int i = 0; i < ECS_BITSET_SIZE; i++)
    {
        set.array[i] = set1->array[i] & set2->array[i];
    }

    return set;
}

static inline bool ecs_bitset_equal(ecs_bitset_t* set1, ecs_bitset_t* set2)
{
    for (int i = 0; i < ECS_BITSET_SIZE; i++)
    {
        if (set1->array[i] != set2->array[i])
        {
            return false;
        }
    }

    return true;
}

static inline bool ecs_bitset_true(ecs_bitset_t* set)
{
    for (int i = 0; i < ECS_BITSET_SIZE; i++)
    {
        if (set->array[i])
            return true;
    }

    return false;
}

#endif // ECS_MAX_COMPONENTS

/*=============================================================================
 * Internal sparse set functions
 *============================================================================*/

static void ecs_sparse_set_init(ecs_t* ecs, ecs_sparse_set_t* set, size_t capacity)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(set));
    ECS_ASSERT(capacity > 0);

    (void)ecs;

    set->capacity = capacity;
    set->size = 0;

    set->dense  = (ecs_id_t*)ECS_MALLOC(capacity * sizeof(ecs_id_t), ecs->mem_ctx);
    set->sparse = (size_t*)  ECS_MALLOC(capacity * sizeof(size_t),   ecs->mem_ctx);

    memset(set->sparse, 0, capacity * sizeof(size_t));
}

static void ecs_sparse_set_free(ecs_t* ecs, ecs_sparse_set_t* set)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(set));

    (void)ecs;

    ECS_FREE(set->dense,  ecs->mem_ctx);
    ECS_FREE(set->sparse, ecs->mem_ctx);
}

static bool ecs_sparse_set_add(ecs_t* ecs, ecs_sparse_set_t* set, ecs_id_t id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(set));

    (void)ecs;

    // Grow sparse set if necessary
    if (id >= set->capacity)
    {
        size_t old_capacity = set->capacity;
        size_t new_capacity = old_capacity;

        // Calculate new capacity
        while (new_capacity <= id)
        {
            new_capacity += (new_capacity + 1) >> 1;
        }

        // Grow dense array
        set->dense = (ecs_id_t*)ECS_REALLOC(set->dense,
                                            new_capacity * sizeof(ecs_id_t),
                                            ecs->mem_ctx);

        // Grow sparse array and zero it
        set->sparse = (size_t*)ecs_realloc_zero(ecs,
                                                set->sparse,
                                                old_capacity * sizeof(size_t),
                                                new_capacity * sizeof(size_t));

        // Set the new capacity
        set->capacity = new_capacity;
    }

    // Check if ID exists within the set
    if (ECS_NULL != ecs_sparse_set_find(set, id))
        return false;

    // Add ID to set
    set->dense[set->size] = id;
    set->sparse[id] = set->size;

    set->size++;

    return true;
}

static size_t ecs_sparse_set_find(ecs_sparse_set_t* set, ecs_id_t id)
{
    ECS_ASSERT(ecs_is_not_null(set));

    if (set->sparse[id] < set->size && set->dense[set->sparse[id]] == id)
        return set->sparse[id];
    else
        return ECS_NULL;
}

static bool ecs_sparse_set_remove(ecs_sparse_set_t* set, ecs_id_t id)
{
    ECS_ASSERT(ecs_is_not_null(set));

    if (ECS_NULL == ecs_sparse_set_find(set, id))
        return false;

    // Swap and remove (changes order of array)
    ecs_id_t tmp = set->dense[set->size - 1];
    set->dense[set->sparse[id]] = tmp;
    set->sparse[tmp] = set->sparse[id];

    set->size--;

    return true;
}

/*=============================================================================
 * Internal system entity add/remove functions
 *============================================================================*/

inline static bool ecs_entity_system_test(ecs_bitset_t* sys_bits,
                                          ecs_bitset_t* entity_bits)
{
    ecs_bitset_t tmp = ecs_bitset_and(sys_bits, entity_bits);
    return ecs_bitset_equal(sys_bits, &tmp);
}

/*=============================================================================
 * Internal ID pool functions
 *============================================================================*/

inline static void ecs_stack_init(ecs_t* ecs, ecs_stack_t* stack, int capacity)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(stack));
    ECS_ASSERT(capacity > 0);

    (void)ecs;

    stack->size = 0;
    stack->capacity = capacity;
    stack->array = (ecs_id_t*)ECS_MALLOC(capacity * sizeof(ecs_id_t), ecs->mem_ctx);
}

inline static void ecs_stack_free(ecs_t* ecs, ecs_stack_t* stack)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(stack));

    (void)ecs;

    ECS_FREE(stack->array, ecs->mem_ctx);
}

inline static void ecs_stack_push(ecs_t* ecs, ecs_stack_t* stack, ecs_id_t id)
{
    ECS_ASSERT(ecs_is_not_null(ecs));
    ECS_ASSERT(ecs_is_not_null(stack));
    ECS_ASSERT(stack->capacity > 0);

    (void)ecs;

    if (stack->size == stack->capacity)
    {
        stack->capacity += (stack->capacity + 1) >> 1;

        stack->array = (ecs_id_t*)ECS_REALLOC(stack->array,
                                              stack->capacity * sizeof(ecs_id_t),
                                              ecs->mem_ctx);
    }

    stack->array[stack->size++] = id;
}

inline static ecs_id_t ecs_stack_pop(ecs_stack_t* stack)
{
    ECS_ASSERT(ecs_is_not_null(stack));
    return stack->array[--stack->size];
}

inline static int ecs_stack_size(ecs_stack_t* stack)
{
    return stack->size;
}

static void ecs_array_init(ecs_t* ecs, ecs_array_t* array, size_t size, size_t capacity)
{
    memset(array, 0, sizeof(ecs_array_t));

    (void)ecs;

    array->capacity = capacity;
    array->count = 0;
    array->size = size;
    array->data = ECS_MALLOC(size * capacity, ecs->mem_ctx);
}

static void ecs_array_free(ecs_t* ecs, ecs_array_t* array)
{
    (void)ecs;
    ECS_FREE(array->data, ecs->mem_ctx);
}

static void ecs_array_resize(ecs_t* ecs, ecs_array_t* array, size_t capacity)
{
    (void)ecs;

    if (capacity >= array->capacity)
    {
        while (array->capacity <= capacity)
        {
            array->capacity += (array->capacity / 2);
        }

        array->data = ECS_REALLOC(array->data,
                                  array->capacity * array->size,
                                  ecs->mem_ctx);

        array->capacity = capacity;
    }
}

static void ecs_array_push(ecs_t* ecs, ecs_array_t* array, void* item)
{
    ecs_array_resize(ecs, array, array->count);

    memcpy(ecs_array_get(array, array->count), item, array->size);

    array->count++;
}

static size_t ecs_array_count(const ecs_array_t* array)
{
    return array->count;
}

static void* ecs_array_get(const ecs_array_t* array, size_t index)
{
    return (char*)array->data + (array->size * index);
}

/*=============================================================================
 * Internal validation functions
 *============================================================================*/
#ifndef NDEBUG
static bool ecs_is_not_null(void* ptr)
{
    return NULL != ptr;
}

static bool ecs_is_valid_component_id(ecs_id_t id)
{
    return id < ECS_MAX_COMPONENTS;
}

static bool ecs_is_valid_system_id(ecs_id_t id)
{
    return id < ECS_MAX_SYSTEMS;
}

static bool ecs_is_entity_ready(ecs_t* ecs, ecs_id_t entity_id)
{
    return ecs->entities[entity_id].ready;
}

static bool ecs_is_component_ready(ecs_t* ecs, ecs_id_t comp_id)
{
    return comp_id < ecs->comp_count;
}

static bool ecs_is_system_ready(ecs_t* ecs, ecs_id_t sys_id)
{
    return ecs->systems[sys_id].ready;
}

#endif // NDEBUG

#endif // PICO_ECS_IMPLEMENTATION

/*
    ----------------------------------------------------------------------------
    This software is available under two licenses (A) or (B). You may choose
    either one as you wish:
    ----------------------------------------------------------------------------

    (A) The zlib License

    Copyright (c) 2021 James McLean

    This software is provided 'as-is', without any express or implied warranty.
    In no event will the authors be held liable for any damages arising from the
    use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software in a
    product, an acknowledgment in the product documentation would be appreciated
    but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.

    ----------------------------------------------------------------------------

    (B) Public Domain (www.unlicense.org)

    This is free and unencumbered software released into the public domain.

    Anyone is free to copy, modify, publish, use, compile, sell, or distribute
    this software, either in source code form or as a compiled binary, for any
    purpose, commercial or non-commercial, and by any means.

    In jurisdictions that recognize copyright laws, the author or authors of
    this software dedicate any and all copyright interest in the software to the
    public domain. We make this dedication for the benefit of the public at
    large and to the detriment of our heirs and successors. We intend this
    dedication to be an overt act of relinquishment in perpetuity of all present
    and future rights to this software under copyright law.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
    ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// EoF
