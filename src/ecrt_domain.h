#pragma once

#include <cstdint>
#include <cstdio>
#include <string>
#include <thread>
#include <chrono>

#ifdef HAVE_ETHERCAT_H
#include <ecrt.h>
#endif

class EcrtDomain {
public:
    EcrtDomain(uint16_t slave_pos = 0);
    ~EcrtDomain();

    [[nodiscard]] bool open();
    void close();
    bool is_active() const { return active_; }
    

    // Write controlword via PDO (the ONLY reliable way)
    void write_controlword(uint16_t value);

    // Read from PDO domain (fast, every 500µs)
    uint16_t read_statusword();
    int32_t read_actual_position();   // 0x6064 — position actual value


    // Sync helpers
    void sync();  // domain_queue + domain_send + domain_receive + domain_process

    uint16_t slave_position() const { return slave_pos_; }

#ifdef HAVE_ETHERCAT_H
    ec_master_t* get_master() const { return master_; }
#endif

private:
    bool register_pdo_entries();

#ifdef HAVE_ETHERCAT_H
    ec_master_t* master_ = nullptr;
    ec_slave_info_t slave_info_;
    ec_slave_config_t* slave_config_ = nullptr;
    ec_domain_t* domain_ = nullptr;
    uint8_t* domain_data_ = nullptr;
#endif

    // PDO entry offsets within domain
    unsigned int off_controlword_ = 0;
    unsigned int off_statusword_ = 0;
    unsigned int off_actual_position_ = 0;
    unsigned int off_target_velocity_ = 0;
    unsigned int off_target_torque_ = 0;
    unsigned int off_actual_velocity_ = 0;
    unsigned int off_actual_torque_ = 0;
    unsigned int off_target_position_ = 0;


    uint16_t slave_pos_ = 0;
    bool active_ = false;
    bool pdo_registered_ = false;
    bool ref_time_initialized_ = false;
    uint64_t ref_time_ = 0;
    uint32_t prev_ref_32_ = 0;

};

// ---- inline implementations ----

inline EcrtDomain::EcrtDomain(uint16_t slave_pos)
    : slave_pos_(slave_pos) {}

inline EcrtDomain::~EcrtDomain() {
    close();
}

inline bool EcrtDomain::open() {
#ifdef HAVE_ETHERCAT_H
#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] Requesting master...\n");
#endif
    master_ = ecrt_request_master(0);
    if (!master_) {
        fprintf(stderr, "[Ecrt] FAIL: ecrt_request_master returned NULL\n");
        return false;
    }
#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] Master acquired ✓\n");
#endif

#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] Getting slave %d info...\n", slave_pos_);
#endif
    if (ecrt_master_get_slave(master_, slave_pos_, &slave_info_) < 0) {
        fprintf(stderr, "[Ecrt] FAIL: slave %d not found\n", slave_pos_);
        return false;
    }
#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] Slave: vendor=0x%08X product=0x%08X ✓\n",
        slave_info_.vendor_id, slave_info_.product_code);
#endif

#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] Creating domain...\n");
#endif
    domain_ = ecrt_master_create_domain(master_);
    if (!domain_) {
        fprintf(stderr, "[Ecrt] FAIL: domain creation failed\n");
        return false;
    }
#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] Domain created ✓\n");
#endif

#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] Creating slave config...\n");
#endif
    slave_config_ = ecrt_master_slave_config(
        master_, 0, slave_pos_,
        slave_info_.vendor_id, slave_info_.product_code);
    if (!slave_config_) {
        fprintf(stderr, "[Ecrt] FAIL: slave_config creation failed\n");
        return false;
    }
#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] Slave config created ✓\n");
#endif

    // Configure PDO mapping explicitly (like lcec does) — CSP mode
#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] Configuring PDO mapping...\n");
#endif
    ec_pdo_entry_info_t rx_entries[] = {
        {0x6040, 0x00, 16},  // Controlword
        {0x60FF, 0x00, 32},  // Target Velocity
        {0x6071, 0x00, 32},  // Target Torque
        {0x607A, 0x00, 32},  // Target Position
    };
    ec_pdo_entry_info_t tx_entries[] = {
        {0x6041, 0x00, 16},  // Statusword
        {0x6064, 0x00, 32},  // Actual Position
        {0x606C, 0x00, 32},  // Actual Velocity
        {0x6077, 0x00, 32},  // Actual Torque
    };
    ec_pdo_info_t rx_pdos[] = {
        {0x1600, 4, rx_entries},
    };
    ec_pdo_info_t tx_pdos[] = {
        {0x1A00, 4, tx_entries},
    };
    ec_sync_info_t syncs[] = {
        {0, EC_DIR_OUTPUT},
        {1, EC_DIR_INPUT},
        {2, EC_DIR_OUTPUT, 1, rx_pdos},
        {3, EC_DIR_INPUT, 1, tx_pdos},
        {0xff}
    };
    if (ecrt_slave_config_pdos(slave_config_, EC_END, syncs)) {
        fprintf(stderr, "[Ecrt] WARNING: PDO configuration failed, using defaults\n");
    }
#ifdef DEBUG_ECRT
    else {
        fprintf(stderr, "[Ecrt] PDO mapping configured ✓\n");
    }
#endif

    if (!register_pdo_entries()) {
        fprintf(stderr, "[Ecrt] FAIL: PDO registration failed\n");
        return false;
    }
#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] PDO entries registered:\n");
    fprintf(stderr, "  ctrl=0x%x status=0x%x pos=0x%x\n",
        off_controlword_, off_statusword_, off_actual_position_);
#endif

#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] Configuring DC...\n");
#endif
    ecrt_slave_config_dc(slave_config_, 0x0300, 500000, 0, 0, 0);
#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] DC configured ✓\n");
#endif

#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] Activating master...\n");
#endif
    if (ecrt_master_activate(master_) < 0) {
        fprintf(stderr, "[Ecrt] FAIL: master activation failed\n");
        return false;
    }
#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] Master activated ✓\n");
#endif

    domain_data_ = ecrt_domain_data(domain_);
    if (!domain_data_) {
        fprintf(stderr, "[Ecrt] FAIL: domain data is NULL\n");
        return false;
    }
#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] Domain data: %p ✓\n", (void*)domain_data_);
#endif

    active_ = true;

#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] Warming up bus...\n");
#endif
    for (int i = 0; i < 20; i++) {
        sync();
        std::this_thread::sleep_for(std::chrono::microseconds(10000));
    }
#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] Warmup done ✓\n");
#endif

    ec_master_state_t ms = {};
    ecrt_master_state(master_, &ms);
#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] Master state after warmup: responding=%u, al_states=0x%02X\n",
        ms.slaves_responding, ms.al_states);
#endif
    if (ms.slaves_responding == 0) {
        fprintf(stderr, "[Ecrt] WARNING: no slaves responding after warmup (al_states=0x%02X)\n",
            ms.al_states);
    }

#ifdef DEBUG_ECRT
    fprintf(stderr, "[Ecrt] OPEN SUCCESS ✓\n");
#endif
    return true;
#else
    fprintf(stderr, "[Ecrt] FAIL: HAVE_ETHERCAT_H not defined\n");
    return false;
#endif
}

inline void EcrtDomain::close() {
#ifdef HAVE_ETHERCAT_H
    if (master_) {
        ecrt_master_deactivate(master_);
        ecrt_release_master(master_);
        master_ = nullptr;
    }
    slave_config_ = nullptr;
    domain_ = nullptr;
    domain_data_ = nullptr;
#endif
    pdo_registered_ = false;
    active_ = false;
    ref_time_initialized_ = false;
    ref_time_ = 0;
    prev_ref_32_ = 0;
}

inline bool EcrtDomain::register_pdo_entries() {
#ifdef HAVE_ETHERCAT_H
    int rc;

    rc = ecrt_slave_config_reg_pdo_entry(
        slave_config_, 0x6040, 0x00, domain_, nullptr);
    if (rc < 0) { fprintf(stderr, "[Ecrt] FAIL: cw reg rc=%d\n", rc); return false; }
    off_controlword_ = rc;

    rc = ecrt_slave_config_reg_pdo_entry(
        slave_config_, 0x60FF, 0x00, domain_, nullptr);
    if (rc < 0) { fprintf(stderr, "[Ecrt] FAIL: target_vel reg rc=%d\n", rc); return false; }
    off_target_velocity_ = rc;

    rc = ecrt_slave_config_reg_pdo_entry(
        slave_config_, 0x6071, 0x00, domain_, nullptr);
    if (rc < 0) { fprintf(stderr, "[Ecrt] FAIL: target_tq reg rc=%d\n", rc); return false; }
    off_target_torque_ = rc;

    rc = ecrt_slave_config_reg_pdo_entry(
        slave_config_, 0x607A, 0x00, domain_, nullptr);
    if (rc < 0) { fprintf(stderr, "[Ecrt] FAIL: target_pos reg rc=%d\n", rc); return false; }
    off_target_position_ = rc;

    rc = ecrt_slave_config_reg_pdo_entry(
        slave_config_, 0x6041, 0x00, domain_, nullptr);
    if (rc < 0) { fprintf(stderr, "[Ecrt] FAIL: sw reg rc=%d\n", rc); return false; }
    off_statusword_ = rc;

    rc = ecrt_slave_config_reg_pdo_entry(
        slave_config_, 0x6064, 0x00, domain_, nullptr);
    if (rc < 0) { fprintf(stderr, "[Ecrt] FAIL: pos reg rc=%d\n", rc); return false; }
    off_actual_position_ = rc;

    rc = ecrt_slave_config_reg_pdo_entry(
        slave_config_, 0x606C, 0x00, domain_, nullptr);
    if (rc < 0) { fprintf(stderr, "[Ecrt] FAIL: actual_vel reg rc=%d\n", rc); return false; }
    off_actual_velocity_ = rc;

    rc = ecrt_slave_config_reg_pdo_entry(
        slave_config_, 0x6077, 0x00, domain_, nullptr);
    if (rc < 0) { fprintf(stderr, "[Ecrt] FAIL: actual_tq reg rc=%d\n", rc); return false; }
    off_actual_torque_ = rc;

    fprintf(stderr, "[Ecrt] PDO offsets: cw=0x%x sw=0x%x pos=0x%x vel=0x%x tq=0x%x\n",
        off_controlword_, off_statusword_, off_actual_position_,
        off_actual_velocity_, off_actual_torque_);

    pdo_registered_ = true;
    return true;
#else
    return false;
#endif
}

inline void EcrtDomain::write_controlword(uint16_t value) {
#ifdef HAVE_ETHERCAT_H
    if (active_ && domain_data_)
        EC_WRITE_U16(domain_data_ + off_controlword_, value);
#endif
}

inline uint16_t EcrtDomain::read_statusword() {
#ifdef HAVE_ETHERCAT_H
    if (active_ && domain_data_)
        return EC_READ_U16(domain_data_ + off_statusword_);
#endif
    return 0;
}

inline int32_t EcrtDomain::read_actual_position() {
#ifdef HAVE_ETHERCAT_H
    if (active_ && domain_data_)
        return EC_READ_S32(domain_data_ + off_actual_position_);
#endif
    return 0;
}



inline void EcrtDomain::sync() {
#ifdef HAVE_ETHERCAT_H
    if (!active_ || !master_ || !domain_) return;

    // Reference clock sync with 32-bit rollover handling
    uint32_t now_32;
    ecrt_master_reference_clock_time(master_, &now_32);
    if (!ref_time_initialized_) {
        ref_time_ = now_32;
        prev_ref_32_ = now_32;
        ref_time_initialized_ = true;
    } else {
        int64_t delta = static_cast<int64_t>(now_32) - static_cast<int64_t>(prev_ref_32_);
        ref_time_ += static_cast<uint64_t>(delta);
    }
    prev_ref_32_ = now_32;

    ecrt_master_application_time(master_, ref_time_);
    ecrt_master_sync_reference_clock(master_);
    ecrt_master_sync_slave_clocks(master_);

    ecrt_domain_queue(domain_);
    ecrt_master_send(master_);
    ecrt_master_receive(master_);
    ecrt_domain_process(domain_);
#endif
}
