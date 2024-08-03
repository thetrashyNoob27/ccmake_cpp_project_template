#ifndef _PROJECT_ARCHIEVE_H_
#define _PROJECT_ARCHIEVE_H_

#include <string>
#include <cstdint>
void projectSourceTarData(uint8_t **tarData, uintptr_t *size);
void saveSourceTarData(const std::string &path, bool *success, std::string *errString);
#endif
