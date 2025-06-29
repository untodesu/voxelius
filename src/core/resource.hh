#ifndef CORE_RESOURCE_HH
#define CORE_RESOURCE_HH 1
#pragma once

template<typename T>
using resource_ptr = std::shared_ptr<const T>;

namespace resource
{
template<typename T>
resource_ptr<T> load(const char* name, unsigned int flags = 0U);
template<typename T>
void hard_cleanup(void);
template<typename T>
void soft_cleanup(void);
} // namespace resource

#endif /* CORE_RESOURCE_HH */
