#ifndef _TITAN_KERNEL_OBJECT_H_
#define _TITAN_KERNEL_OBJECT_H_

#include <string>
#include <vector>


struct KernelObjectInfo {
    std::string name;
    std::string type;
};

/**
 * ls kernel object
 * @param path the path to ls
 * @return the list of kernel object in that path
 */
std::vector<KernelObjectInfo>
    get_kernel_object_list(const std::string& path);

#endif __TITAN_KERNEL_OBJECT_H_