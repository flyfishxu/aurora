#include "aurora_runtime.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ============================================================================
// Reference Counting Implementation
// ============================================================================

void* aurora_retain(void* ptr) {
    if (!ptr) return NULL;
    
    AuroraRefCountHeader* header = (AuroraRefCountHeader*)ptr;
    header->ref_count++;
    return ptr;
}

void aurora_release(void* ptr) {
    if (!ptr) return;
    
    AuroraRefCountHeader* header = (AuroraRefCountHeader*)ptr;
    header->ref_count--;
    
    if (header->ref_count <= 0) {
        // Call appropriate free function based on type tag
        switch (header->type) {
            case AURORA_TYPE_OBJECT:
                aurora_object_free((AuroraObject*)ptr);
                break;
            case AURORA_TYPE_ARRAY:
                aurora_array_free((AuroraArray*)ptr);
                break;
            case AURORA_TYPE_STRING:
                aurora_string_free((AuroraString*)ptr);
                break;
            default:
                aurora_panic("Unknown object type in aurora_release");
        }
    }
}

int64_t aurora_get_ref_count(void* ptr) {
    if (!ptr) return 0;
    AuroraRefCountHeader* header = (AuroraRefCountHeader*)ptr;
    return header->ref_count;
}

// ============================================================================
// Array Implementation
// ============================================================================

AuroraArray* aurora_array_create(int64_t element_size, int64_t element_count) {
    AuroraArray* array = (AuroraArray*)malloc(sizeof(AuroraArray));
    if (!array) {
        aurora_panic("Failed to allocate array structure");
    }
    
    array->header.ref_count = 1;
    array->header.type = AURORA_TYPE_ARRAY;
    array->length = element_count;
    array->data = malloc(element_size * element_count);
    
    if (!array->data && element_count > 0) {
        free(array);
        aurora_panic("Failed to allocate array data");
    }
    
    // Initialize to zero
    if (array->data) {
        memset(array->data, 0, element_size * element_count);
    }
    
    return array;
}

void aurora_array_free(AuroraArray* array) {
    if (array) {
        if (array->data) {
            free(array->data);
        }
        free(array);
    }
}

int64_t aurora_array_length(AuroraArray* array) {
    return array ? array->length : 0;
}

void* aurora_array_get_ptr(AuroraArray* array, int64_t index, int64_t element_size) {
    if (!array || !array->data) {
        aurora_panic("Null array access");
    }
    
    #ifdef AURORA_DEBUG
    aurora_array_bounds_check(array, index);
    #endif
    
    return (char*)array->data + (index * element_size);
}

void aurora_array_set(AuroraArray* array, int64_t index, void* element, int64_t element_size) {
    void* ptr = aurora_array_get_ptr(array, index, element_size);
    memcpy(ptr, element, element_size);
}

void aurora_array_bounds_check(AuroraArray* array, int64_t index) {
    if (!array) {
        aurora_panic("Null array in bounds check");
    }
    if (index < 0 || index >= array->length) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), 
                "Array index out of bounds: index=%lld, length=%lld", 
                (long long)index, (long long)array->length);
        aurora_panic(buffer);
    }
}

// ============================================================================
// Object Implementation
// ============================================================================

AuroraObject* aurora_object_create(size_t size) {
    AuroraObject* obj = (AuroraObject*)malloc(sizeof(AuroraObject));
    if (!obj) {
        aurora_panic("Failed to allocate object structure");
    }
    
    obj->header.ref_count = 1;
    obj->header.type = AURORA_TYPE_OBJECT;
    obj->vtable = NULL;
    obj->size = size;
    obj->data = malloc(size);
    
    if (!obj->data && size > 0) {
        free(obj);
        aurora_panic("Failed to allocate object data");
    }
    
    if (obj->data) {
        memset(obj->data, 0, size);
    }
    
    return obj;
}

void aurora_object_free(AuroraObject* obj) {
    if (obj) {
        if (obj->data) {
            free(obj->data);
        }
        free(obj);
    }
}

// ============================================================================
// String Implementation
// ============================================================================

AuroraString* aurora_string_create(const char* str) {
    AuroraString* astr = (AuroraString*)malloc(sizeof(AuroraString));
    if (!astr) {
        aurora_panic("Failed to allocate string structure");
    }
    
    astr->header.ref_count = 1;
    astr->header.type = AURORA_TYPE_STRING;
    astr->length = str ? strlen(str) : 0;
    astr->data = (char*)malloc(astr->length + 1);
    
    if (!astr->data) {
        free(astr);
        aurora_panic("Failed to allocate string data");
    }
    
    if (str) {
        strcpy(astr->data, str);
    } else {
        astr->data[0] = '\0';
    }
    
    return astr;
}

void aurora_string_free(AuroraString* str) {
    if (str) {
        if (str->data) {
            free(str->data);
        }
        free(str);
    }
}

int64_t aurora_string_length(AuroraString* str) {
    return str ? str->length : 0;
}

// ============================================================================
// Memory Management
// ============================================================================

void* aurora_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr && size > 0) {
        aurora_panic("Memory allocation failed");
    }
    return ptr;
}

void aurora_free(void* ptr) {
    if (ptr) {
        free(ptr);
    }
}

// ============================================================================
// Debugging
// ============================================================================

void aurora_assert(int condition, const char* message) {
    if (!condition) {
        aurora_panic(message);
    }
}

void aurora_panic(const char* message) {
    fprintf(stderr, "\n[Aurora Runtime Panic] %s\n", message);
    abort();
}

