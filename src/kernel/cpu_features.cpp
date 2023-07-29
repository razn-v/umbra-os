#include <kernel/cpu_features.hpp>
#include <cpuid.h>

namespace Cpu {

bool has_feature(EcxFeature feature) {
    uint32_t unused, ecx;
    __get_cpuid(1, &unused, &unused, &ecx, &unused);
    return ecx & (uint32_t)feature;
}

bool has_feature(EdxFeature feature) {
    uint32_t unused, edx;
    __get_cpuid(1, &unused, &unused, &unused, &edx);
    return edx & (uint32_t)feature;
}

}
