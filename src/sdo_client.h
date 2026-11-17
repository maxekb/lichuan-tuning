#pragma once
#include <cstdint>
#include <vector>
#include <optional>
#include <memory>
#include <string>
#include <mutex>

#ifdef HAVE_ETHERCAT_H
struct ec_master;
typedef struct ec_master ec_master_t;
#endif

// -----------------------------------------------------------------------
// SdoType — replaces string-based dispatch ("uint8", "int16", etc.)
// -----------------------------------------------------------------------
enum class SdoType : uint8_t {
    UINT8,
    INT8,
    UINT16,
    INT16,
    UINT32,
    INT32,
    FLOAT,
};

// Backward-compatible helper: map SdoType to the legacy CLI string
constexpr const char* sdo_type_to_cli_string(SdoType t) {
    switch (t) {
        case SdoType::UINT8:  return "uint8";
        case SdoType::INT8:   return "int8";
        case SdoType::UINT16: return "uint16";
        case SdoType::INT16:  return "int16";
        case SdoType::UINT32: return "uint32";
        case SdoType::INT32:  return "int32";
        case SdoType::FLOAT:  return "uint32"; // FLOAT stored as uint32
        default:              return "uint16";
    }
}

struct SdoResult {
    bool success = false;
    std::vector<uint8_t> data;
    uint32_t abort_code = 0;
};

class SdoClient {
public:
    SdoClient(uint16_t slave_pos = 0);
    ~SdoClient();

    // Non-copyable, non-movable (owns std::mutex)
    SdoClient(const SdoClient&) = delete;
    SdoClient& operator=(const SdoClient&) = delete;
    SdoClient(SdoClient&&) = delete;
    SdoClient& operator=(SdoClient&&) = delete;

    bool connect();
    void disconnect();
    bool is_connected() const { return connected_; }
    uint16_t slave_position() const { return slave_pos_; }

    // Reconfigure for a different slave without destroying/reconstructing
    void reset(uint16_t slave_pos);

    [[nodiscard]] SdoResult read(uint16_t index, uint8_t subindex,
                                 SdoType type = SdoType::UINT16);
    [[nodiscard]] bool write(uint16_t index, uint8_t subindex,
                             const std::vector<uint8_t>& data,
                             SdoType type = SdoType::UINT16,
                             uint32_t* abort_code = nullptr);

    std::vector<std::string> list_slaves();
    std::string slave_info(uint16_t pos);

#ifdef HAVE_ETHERCAT_H
    void set_master(ec_master_t* master) { master_ = master; }
#endif

private:
    // Internal disconnect without locking (called from reset())
    void disconnect_internal() {
        connected_ = false;
        backend_.clear();
    }

    uint16_t slave_pos_;
    bool connected_ = false;
    std::string backend_; // "cli" or "ecrt"
    mutable std::mutex mtx_;
#ifdef HAVE_ETHERCAT_H
    ec_master_t* master_ = nullptr;
#endif
};
