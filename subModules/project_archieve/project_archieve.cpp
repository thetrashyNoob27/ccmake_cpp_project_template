#include "project_archieve.h"
#include "project_archieve_data.h"
void projectSourceTarData(uint8_t *tarData, uintptr_t *size)
{
    tarData = test_h;
    *size = reinterpret_cast<uintptr_t>(&test_h_end) - reinterpret_cast<uintptr_t>(test_h);
    return;
}