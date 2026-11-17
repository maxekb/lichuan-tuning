#pragma once
/*****************************************************************************
 * Lichuan A5-E Parameter Database
 * 
 * All 181 parameters (P01–P0D) with:
 * - Names, descriptions, ranges, units, defaults
 * - SDO index/subindex mapping
 * - Validation rules
 * - Display formatting
 ****************************************************************************/

#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <cstdint>
#include <functional>
#include <map>

// -----------------------------------------------------------------------
// Parameter type enum
// -----------------------------------------------------------------------
enum class ParamType : uint8_t {
    UINT8,
    INT8,
    UINT16,
    INT16,
    UINT32,
    INT32,
    FLOAT,      // single precision (rare)
    STRING,     // text parameter
};

// -----------------------------------------------------------------------
// Apply mode — how/when the parameter takes effect
// -----------------------------------------------------------------------
enum class ApplyMode : uint8_t {
    IMMEDIATE,      // 立即生效: takes effect right after writing
    SHUTDOWN,       // 停机设定: takes effect after next servo OFF → ON
    POWER_CYCLE,    // 再次通电: requires full power cycle
    READ_ONLY,      // read-only (monitoring)
};

// -----------------------------------------------------------------------
// Parameter group
// -----------------------------------------------------------------------
enum class ParamGroup : uint8_t {
    BASIC     = 0,  // P01
    CONTROL   = 1,  // P02
    DI        = 2,  // P03
    DO        = 3,  // P04
    POSITION  = 4,  // P05
    JOG       = 5,  // P06
    TORQUE    = 6,  // P07
    GAINS     = 7,  // P08
    AUTOTUNE  = 8,  // P09
    FAULT     = 9,  // P0A
    MONITOR   = 10, // P0B
    COMM      = 11, // P0C
    AUX       = 12, // P0D
};

// -----------------------------------------------------------------------
// String helpers for enums
// -----------------------------------------------------------------------
std::string_view param_group_name(ParamGroup g);
std::string_view param_type_name(ParamType t);
std::string_view apply_mode_name(ApplyMode m);

// -----------------------------------------------------------------------
// Parameter definition
// -----------------------------------------------------------------------
struct ParamDef {
    uint16_t    index;          // EtherCAT SDO index (e.g., 0x2008)
    uint8_t     subindex;       // SDO subindex (e.g., 0x10 for P08.15)
    ParamType   type;           // data type for SDO access
    ApplyMode   apply;          // when does the change take effect
    int8_t      decimal_places; // display scaling: -1 = auto, 0 = integer, 1 = one decimal, etc.
    double      min_val;        // minimum value (in display units)
    double      max_val;        // maximum value (in display units)
    double      default_val;    // factory default (in display units)
    std::string_view name;      // "P08.15"
    std::string_view desc;      // short description
    std::string_view unit;      // display unit (e.g., "Hz", "%", "ms")
    std::string_view group_name;// parameter group name string
    ParamGroup group;           // enum for fast filtering
    std::map<int64_t, std::string> value_descs;  // value -> description for enum params

    // Validate a raw integer value against min/max
    // raw_val: the raw SDO value (uint16, int32, etc.)
    // returns: true if valid
    [[nodiscard]] bool validate_raw(int64_t raw_val) const;

    // Validate a display value (string input)
    std::optional<std::string> validate_display(const std::string& str_val) const;

    // Convert raw SDO value to display string
    std::string raw_to_display(int64_t raw_val) const;

    // Convert display string to raw SDO value
    // returns: std::nullopt on parse error
    std::optional<int64_t> display_to_raw(const std::string& str_val) const;

    // Get scaling factor: raw = display * scale
    double scale_factor() const;
};

// -----------------------------------------------------------------------
// Parameter database
// -----------------------------------------------------------------------
class ParamDatabase {
public:
    ParamDatabase();

    // Lookup by name ("P08.15", "P02.00")
    const ParamDef* find(std::string_view name) const;

    // Lookup by index/subindex
    const ParamDef* find(uint16_t index, uint8_t subindex) const;

    // Get all parameters in a group
    std::vector<const ParamDef*> group(ParamGroup g) const;

    // Get all parameters
    const std::vector<ParamDef>& all() const { return params_; }

    // List all parameter names
    std::vector<std::string_view> names() const;

    // Parse a parameter group name like "P02" or "P08"
    static std::optional<ParamGroup> parse_group(std::string_view s);

    // Get the EtherCAT SDO index for a group (e.g., P02 → 0x2002)
    static uint16_t group_index(ParamGroup g);

    // Get the panel number string for a group (e.g., GAINS → "P08")
    static std::string_view group_panel(ParamGroup g);

private:
    std::vector<ParamDef> params_;
    void add(ParamDef pd);
    void add_p01();
    void add_p03();
    void add_p04();
    void add_p05();
    void add_p06();
    void add_p07();
    void add_p08();
    void add_p09();
    void add_p0a();
    void add_p0b();
    void add_p0c();
    void add_p0d();
};

// -----------------------------------------------------------------------
// Helper: parse hex/dec number from string
// -----------------------------------------------------------------------
std::optional<int64_t> parse_number(std::string_view s);

// -----------------------------------------------------------------------
// Global singleton accessor
// -----------------------------------------------------------------------
const ParamDatabase& param_db();
