#include "project_archieve.h"
#include "project_archieve_data.h"

#include <fstream>
#include <cstdint>
#include <filesystem>
#include <cstring>
#include <cerrno>
void projectSourceTarData(uint8_t **tarData, uintptr_t *size)
{
    *tarData = test_h;
    *size = reinterpret_cast<uintptr_t>(&test_h_end) - reinterpret_cast<uintptr_t>(test_h);
    return;
}

void saveSourceTarData(const std::string &path, bool *success, std::string *errString)
{
    uint8_t *tarData = nullptr;
    uintptr_t dataSize = 0;
    projectSourceTarData(&tarData, &dataSize);
    if (success)
    {
        *success = false;
    }
    if (errString)
    {
        *errString = "___not_finish___";
    }
    const std::string fileName("project.tar.gz");

    if (!std::filesystem::exists(path) || !std::filesystem::is_directory(path))
    {
        if (errString)
        {
            *errString = "path is not a dir";
        }
        return;
    }

    std::filesystem::path fullName = path + fileName;

    std::ofstream tarFile(fullName, std::ios::binary);
    if (!tarFile)
    {
        // file open fail
        goto fileFail;
    }

    tarFile.write(reinterpret_cast<const char *>(tarData), dataSize);
    if (tarFile.fail())
    {
        goto fileFail;
    }

    tarFile.close();
    if (tarFile.fail())
    {
        goto fileFail;
    }
    if (success)
    {
        *success = true;
    }
    if (errString)
    {
        std::ostringstream oss;
        oss << "dump project tar package to " << "[" << fullName << "]";
        *errString = oss.str();
    }
    return;
fileFail:
    if (errString)
    {
        *errString = std::strerror(errno);
    }
    return;
}