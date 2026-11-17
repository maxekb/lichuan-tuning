#include "sdo_client.h"
#include <cstdio>
#include <array>
#include <sstream>
#include <iostream>
#include <cstring>

#ifdef HAVE_ETHERCAT_H
#include <ecrt.h>
#endif

SdoClient::SdoClient(uint16_t slave_pos)
    : slave_pos_(slave_pos) {}

SdoClient::~SdoClient()
{
    disconnect();
}

void SdoClient::reset(uint16_t slave_pos)
{
    std::lock_guard<std::mutex> lock(mtx_);
    disconnect_internal();
    slave_pos_ = slave_pos;
    connected_ = false;
}

bool SdoClient::connect()
{
    std::lock_guard<std::mutex> lock(mtx_);
#ifdef HAVE_ETHERCAT_H
    if (master_) {
        // Test if ecrt backend works
        std::vector<uint8_t> buf(2);
        size_t result_size = 0;
        uint32_t abort_code = 0;
        int rc = ecrt_master_sdo_upload(master_, slave_pos_,
            0x2001, 0x03, buf.data(), buf.size(), &result_size, &abort_code);
        if (rc == 0) {
            backend_ = "ecrt";
            connected_ = true;
            return true;
        }
        // ecrt failed — fall through to CLI
        fprintf(stderr, "[SdoClient] ecrt SDO test failed (rc=%d, abort=0x%x), using CLI\n",
            rc, abort_code);
        master_ = nullptr;  // Reset master so future calls use CLI
    }
#endif
    // Fallback: cli mode — validate slave_pos_ is a small integer
    if (slave_pos_ > 254) {
        fprintf(stderr, "[SdoClient] connect FAIL: slave_pos_ %u out of range\n", slave_pos_);
        connected_ = false;
        return false;
    }
    backend_ = "cli";
    connected_ = true;
    return true;
}

void SdoClient::disconnect()
{
    std::lock_guard<std::mutex> lock(mtx_);
    connected_ = false;
}

[[nodiscard]] SdoResult SdoClient::read(uint16_t index, uint8_t subindex, SdoType type)
{
    SdoResult result;
    std::unique_lock<std::mutex> lock(mtx_);
    if (!connected_) return result;
    const uint16_t pos = slave_pos_;

    size_t size = 4;
    switch (type) {
        case SdoType::UINT8: case SdoType::INT8:  size = 1; break;
        case SdoType::UINT16: case SdoType::INT16: size = 2; break;
        case SdoType::UINT32: case SdoType::INT32:
        case SdoType::FLOAT:                       size = 4; break;
    }

#ifdef HAVE_ETHERCAT_H
    if (master_) {
        std::vector<uint8_t> buf(size);
        size_t result_size = 0;
        uint32_t abort_code = 0;

        int rc = ecrt_master_sdo_upload(master_, slave_pos_,
            index, subindex, buf.data(), size, &result_size, &abort_code);

        if (rc == 0) {
            result.data.assign(buf.data(), buf.data() + result_size);
            result.success = true;
        } else {
            result.abort_code = abort_code;
        }
        return result;
    }
#endif

    // CLI backend: release mtx_ before spawning the subprocess so disconnect()/
    // reset() cannot block behind a hanging `timeout -k 1 5 ethercat ...` call.
    lock.unlock();

    std::array<char, 256> buffer;
    char cmd[256];
    // timeout: coreutils, bounds a blocking CLI call so the caller's thread cannot stall forever
    snprintf(cmd, sizeof(cmd),
        "timeout -k 1 5 ethercat upload -p %u --type %s 0x%04X 0x%02X 2>/dev/null",
        pos, sdo_type_to_cli_string(type), index, subindex);

    FILE* pipe = popen(cmd, "r");
    if (!pipe) return result;

    if (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        std::string line(buffer.data());
        while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
            line.pop_back();

        if (line.substr(0, 2) == "0x" || line.substr(0, 2) == "0X") {
            try {
                uint64_t val = std::stoull(line, nullptr, 16);
                result.data.clear();
                switch (type) {
                    case SdoType::UINT8: case SdoType::INT8:
                        result.data.push_back(val & 0xFF);
                        break;
                    case SdoType::UINT16: case SdoType::INT16:
                        result.data.push_back(val & 0xFF);
                        result.data.push_back((val >> 8) & 0xFF);
                        break;
                    case SdoType::UINT32: case SdoType::INT32:
                    case SdoType::FLOAT:
                        result.data.push_back(val & 0xFF);
                        result.data.push_back((val >> 8) & 0xFF);
                        result.data.push_back((val >> 16) & 0xFF);
                        result.data.push_back((val >> 24) & 0xFF);
                        break;
                }
                result.success = true;
            } catch (...) {}
        }
    }

    pclose(pipe);
    return result;
}

[[nodiscard]] bool SdoClient::write(uint16_t index, uint8_t subindex,
                                    const std::vector<uint8_t>& data,
                                    SdoType type, uint32_t* abort_code)
{
    std::unique_lock<std::mutex> lock(mtx_);
    if (!connected_) return false;
    const uint16_t pos = slave_pos_;

#ifdef HAVE_ETHERCAT_H
    if (master_) {
        // ecrt backend is serialized under mtx_ (master is not thread-safe)
        int rc = ecrt_master_sdo_download(master_, slave_pos_,
            index, subindex, data.data(), data.size(), abort_code);
        return rc == 0;
    }
#endif

    if (data.empty() || data.size() > 4096) return false;

    char cmd[256];
    if (type == SdoType::FLOAT) {
        // Pass raw IEEE-754 bits as hex for uint32
        uint32_t bits = 0;
        for (size_t i = 0; i < data.size() && i < 4; ++i)
            bits |= static_cast<uint32_t>(data[i]) << (i * 8);
        snprintf(cmd, sizeof(cmd),
            "timeout -k 1 5 ethercat download -p %u --type uint32 0x%04X 0x%02X 0x%08X 2>&1",
            pos, index, subindex, bits);
    } else {
        uint64_t val = 0;
        for (size_t i = 0; i < data.size() && i < 8; ++i)
            val |= static_cast<uint64_t>(data[i]) << (i * 8);
        snprintf(cmd, sizeof(cmd),
            "timeout -k 1 5 ethercat download -p %u --type %s 0x%04X 0x%02X %lu 2>&1",
            pos, sdo_type_to_cli_string(type), index, subindex, (unsigned long)val);
    }

    // CLI backend: release mtx_ before spawning the subprocess so disconnect()/
    // reset() cannot block behind a hanging `timeout -k 1 5 ethercat ...` call.
    lock.unlock();

    FILE* pipe = popen(cmd, "r");
    if (!pipe) return false;
    char buf[256];
    std::string output;   // bounded CLI diagnostic (stdout+stderr via 2>&1)
    while (fgets(buf, sizeof(buf), pipe))
        if (output.size() < 1024)
            output += buf;
    int ret = pclose(pipe);
    if (ret < 0) {
        fprintf(stderr, "[SdoClient] pclose error: %s\n", strerror(errno));
        return false;
    }
    if (ret != 0) {
        // The CLI never reports a structured abort code — surface its real
        // diagnostic text so field logs can tell a dead bus from a rejected object
        while (!output.empty() && (output.back() == '\n' || output.back() == '\r'))
            output.pop_back();
        fprintf(stderr, "[SdoClient] CLI write failed (exit %d): %s\n", ret, output.c_str());
    }
    return ret == 0;
}

std::vector<std::string> SdoClient::list_slaves()
{
    std::vector<std::string> slaves;
    std::array<char, 256> buffer;
    FILE* pipe = popen("ethercat slaves 2>/dev/null", "r");
    if (!pipe) return slaves;

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
        slaves.push_back(std::string(buffer.data()));

    pclose(pipe);
    return slaves;
}

std::string SdoClient::slave_info(uint16_t pos)
{
    std::array<char, 256> buffer;
    std::string cmd = "ethercat slaves -p " + std::to_string(pos) + " 2>/dev/null";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "No slave";

    std::string info;
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
        info += buffer.data();

    pclose(pipe);
    return info;
}
