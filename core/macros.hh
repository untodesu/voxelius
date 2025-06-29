#ifndef CORE_MACROS_HH
#define CORE_MACROS_HH 1
#pragma once

#define DISABLE_COPY_OPERATORS(class_name)                 \
public:                                                    \
    explicit class_name(const class_name& other) = delete; \
    class_name& operator=(const class_name& other) = delete

#define DISABLE_MOVE_OPERATORS(class_name)            \
public:                                               \
    explicit class_name(class_name&& other) = delete; \
    class_name& operator=(class_name&& other) = delete

#define DECLARE_DEFAULT_CONSTRUCTOR(class_name) \
public:                                         \
    class_name(void) = default

#endif /* CORE_MACROS_HH */
