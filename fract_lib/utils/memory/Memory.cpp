/*****************************************************************//**
 * \file   Memory.cpp
 * \brief  
 * 
 * \author hylu
 * \date   November 2022
 *********************************************************************/

#include "Memory.h"

namespace Fract::Memory {

std::pmr::memory_resource *global_memory_resource;

std::pmr::memory_resource *local_memory_resource;

void initialize() { 
    global_memory_resource = new GlobalMemoryAllocator(); 
}

void destroy() { 
    delete global_memory_resource; 
}

} // namespace Fract::Memory