#pragma once

#include <iostream>

//opencl
#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.h>
#include <CL/cl_intel.h>
#include <baseline/cl.hpp>

class GuidedFilterCL{
public:
  GuidedFilterCL();
};
