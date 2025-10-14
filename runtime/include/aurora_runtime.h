#ifndef AURORA_RUNTIME_H
#define AURORA_RUNTIME_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// Aurora Runtime Layer
// Provides memory management and basic operations for Aurora language features
// Uses Automatic Reference Counting (ARC) for memory management
// ============================================================================

// Object type tags for reference counting
typedef enum {
    AURORA_TYPE_OBJECT = 1,
    AURORA_TYPE_ARRAY = 2,
    AURORA_TYPE_STRING = 3
} AuroraObjectType;

// Reference counting header for all heap-allocated objects
typedef struct {
    int64_t ref_count;
    AuroraObjectType type;
} AuroraRefCountHeader;

// Array Runtime Support
typedef struct {
    AuroraRefCountHeader header;
    int64_t length;
    void* data;
} AuroraArray;

// Create a new array with given element size and count
AuroraArray* aurora_array_create(int64_t element_size, int64_t element_count);

// Free an array
void aurora_array_free(AuroraArray* array);

// Get array length
int64_t aurora_array_length(AuroraArray* array);

// Get pointer to element at index (no bounds checking in release)
void* aurora_array_get_ptr(AuroraArray* array, int64_t index, int64_t element_size);

// Set element at index (copies data)
void aurora_array_set(AuroraArray* array, int64_t index, void* element, int64_t element_size);

// Bounds checking (debug mode)
void aurora_array_bounds_check(AuroraArray* array, int64_t index);

// Object/Class Runtime Support
typedef struct {
    AuroraRefCountHeader header;
    void* vtable;      // Virtual method table (for future)
    void* data;        // Actual object data
    size_t size;       // Size of object
} AuroraObject;

// Allocate object memory
AuroraObject* aurora_object_create(size_t size);

// Free object memory
void aurora_object_free(AuroraObject* obj);

// String Runtime Support
typedef struct {
    AuroraRefCountHeader header;
    int64_t length;
    char* data;
} AuroraString;

AuroraString* aurora_string_create(const char* str);
void aurora_string_free(AuroraString* str);
int64_t aurora_string_length(AuroraString* str);

// Memory Management
void* aurora_malloc(size_t size);
void aurora_free(void* ptr);

// Reference Counting (ARC)
// These functions work on any heap-allocated object with AuroraRefCountHeader
void* aurora_retain(void* ptr);
void aurora_release(void* ptr);
int64_t aurora_get_ref_count(void* ptr);

// Debugging and Assertions
void aurora_assert(int condition, const char* message);
void aurora_panic(const char* message);

#ifdef __cplusplus
}
#endif

#endif // AURORA_RUNTIME_H

