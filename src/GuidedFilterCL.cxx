#include "baseline/GuidedFilterCL.h"

GuidedFilterCL::GuidedFilterCL(){
  // Get list of OpenCL platforms.
  std::vector<cl::Platform> platform;
  cl::Platform::get(&platform);
  if (platform.empty()) {
     std::cerr << "OpenCL platforms not found." << std::endl;
    return 1;
  }
  std::cout << "platform has size" << platform.size() << '\n';
}
