#include "param_db.h"

#define ADD_PARAM(grp, idx, sub, tp, ap, dp, mn, mx, def, nm, ds, un, gn) \
    do { \
        ParamDef _p; \
        _p.index = (idx); _p.subindex = (sub); _p.type = ParamType::tp; _p.apply = ApplyMode::ap; \
        _p.decimal_places = (dp); _p.min_val = (mn); _p.max_val = (mx); _p.default_val = (def); \
        _p.name = (nm); _p.desc = (ds); _p.unit = (un); _p.group_name = (gn); \
        _p.group = static_cast<ParamGroup>(grp); \
        params_.push_back(_p); \
    } while(0)

static void fill_p02(std::vector<ParamDef>& params);
static void fill_p03(std::vector<ParamDef>& params);
static void fill_p04(std::vector<ParamDef>& params);
static void fill_p05(std::vector<ParamDef>& params);
// P08 gains implementation below

void ParamDatabase::add_p01()
{
    // Basic — 2 params, Index: 0x2001
    // subindex = panel hex + 1 (P01.02 → sub 0x03, P01.50 → sub 0x33)
    ADD_PARAM(ParamGroup::BASIC, 0x2001, 0x03, UINT16, READ_ONLY, 0, 0, 65535, 0,
               "P01.02", "Servo drive number", "", "P01");
    ADD_PARAM(ParamGroup::BASIC, 0x2001, 0x33, STRING, READ_ONLY, 0, 0, 0, 0,
              "P01.50", "Software version", "", "P01");
}

ParamDatabase::ParamDatabase()
{
    add_p01();

    // P02 Control / Basic Settings — 22 params
    fill_p02(params_);

    // P03 DI Functions — 13 params
    fill_p03(params_);

    // P04 DO Functions — 7 params
    fill_p04(params_);

    // P05 Position — 14 params
    fill_p05(params_);

    add_p08();
    add_p09();

    add_p0a();
    add_p0c();
    add_p0b();
    add_p0d();

    add_p06();
    add_p07();
}

static void fill_p03(std::vector<ParamDef>& params)
{
    ParamDef pd;

    pd.index = 0x2003; pd.subindex = 0x01;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::POWER_CYCLE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 0xFFFF; pd.default_val = 0;
    pd.name = "P03.00"; pd.desc = "Effective DI function allocation 1 (FunIN.1-16)"; pd.unit = "";
    pd.group_name = "P03";
    pd.group = ParamGroup::DI;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2003; pd.subindex = 0x02;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::POWER_CYCLE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 0xFFFF; pd.default_val = 0;
    pd.name = "P03.01"; pd.desc = "Effective DI function allocation 2 (FunIN.17-32)"; pd.unit = "";
    pd.group_name = "P03";
    pd.group = ParamGroup::DI;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2003; pd.subindex = 0x03;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::POWER_CYCLE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 39; pd.default_val = 14;
    pd.name = "P03.02"; pd.desc = "DI0 terminal function selection"; pd.unit = "";
    pd.group_name = "P03";
    pd.value_descs = {
        {0, "P03.02_val_0"}, {1, "P03.02_val_1"}, {2, "P03.02_val_2"}, {3, "P03.02_val_3"},
        {4, "P03.02_val_4"}, {5, "P03.02_val_5"}, {6, "P03.02_val_6"}, {7, "P03.02_val_7"},
        {8, "P03.02_val_8"}, {9, "P03.02_val_9"}, {10, "P03.02_val_10"}, {11, "P03.02_val_11"},
        {12, "P03.02_val_12"}, {13, "P03.02_val_13"}, {14, "P03.02_val_14"}, {15, "P03.02_val_15"},
        {16, "P03.02_val_16"}, {17, "P03.02_val_17"}, {18, "P03.02_val_18"}, {19, "P03.02_val_19"},
        {20, "P03.02_val_20"}, {21, "P03.02_val_21"}, {22, "P03.02_val_22"}, {23, "P03.02_val_23"},
        {24, "P03.02_val_24"}, {25, "P03.02_val_25"}, {26, "P03.02_val_26"}, {27, "P03.02_val_27"},
        {28, "P03.02_val_28"}, {29, "P03.02_val_29"}, {30, "P03.02_val_30"}, {31, "P03.02_val_31"},
        {32, "P03.02_val_32"}, {33, "P03.02_val_33"}, {34, "P03.02_val_34"}, {35, "P03.02_val_35"},
        {36, "P03.02_val_36"}, {37, "P03.02_val_37"}, {38, "P03.02_val_38"}, {39, "P03.02_val_39"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2003; pd.subindex = 0x04;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 4; pd.default_val = 0;
    pd.name = "P03.03"; pd.desc = "DI0 terminal logic selection"; pd.unit = "";
    pd.group_name = "P03";
    pd.group = ParamGroup::DI;
    pd.value_descs = {
        {0, "P03.03_val_0"}, {1, "P03.03_val_1"}, {2, "P03.03_val_2"}, {3, "P03.03_val_3"}, {4, "P03.03_val_4"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2003; pd.subindex = 0x05;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::POWER_CYCLE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 39; pd.default_val = 15;
    pd.name = "P03.04"; pd.desc = "DI1 terminal function selection"; pd.unit = "";
    pd.group_name = "P03";
    pd.value_descs = {
        {0, "P03.02_val_0"}, {1, "P03.02_val_1"}, {2, "P03.02_val_2"}, {3, "P03.02_val_3"},
        {4, "P03.02_val_4"}, {5, "P03.02_val_5"}, {6, "P03.02_val_6"}, {7, "P03.02_val_7"},
        {8, "P03.02_val_8"}, {9, "P03.02_val_9"}, {10, "P03.02_val_10"}, {11, "P03.02_val_11"},
        {12, "P03.02_val_12"}, {13, "P03.02_val_13"}, {14, "P03.02_val_14"}, {15, "P03.02_val_15"},
        {16, "P03.02_val_16"}, {17, "P03.02_val_17"}, {18, "P03.02_val_18"}, {19, "P03.02_val_19"},
        {20, "P03.02_val_20"}, {21, "P03.02_val_21"}, {22, "P03.02_val_22"}, {23, "P03.02_val_23"},
        {24, "P03.02_val_24"}, {25, "P03.02_val_25"}, {26, "P03.02_val_26"}, {27, "P03.02_val_27"},
        {28, "P03.02_val_28"}, {29, "P03.02_val_29"}, {30, "P03.02_val_30"}, {31, "P03.02_val_31"},
        {32, "P03.02_val_32"}, {33, "P03.02_val_33"}, {34, "P03.02_val_34"}, {35, "P03.02_val_35"},
        {36, "P03.02_val_36"}, {37, "P03.02_val_37"}, {38, "P03.02_val_38"}, {39, "P03.02_val_39"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2003; pd.subindex = 0x06;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 4; pd.default_val = 0;
    pd.name = "P03.05"; pd.desc = "DI1 terminal logic selection"; pd.unit = "";
    pd.group_name = "P03";
    pd.group = ParamGroup::DI;
    pd.value_descs = {
        {0, "P03.05_val_0"}, {1, "P03.05_val_1"}, {2, "P03.05_val_2"}, {3, "P03.05_val_3"}, {4, "P03.05_val_4"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2003; pd.subindex = 0x07;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::POWER_CYCLE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 39; pd.default_val = 31;
    pd.name = "P03.06"; pd.desc = "DI2 terminal function selection"; pd.unit = "";
    pd.group_name = "P03";
    pd.value_descs = {
        {0, "P03.02_val_0"}, {1, "P03.02_val_1"}, {2, "P03.02_val_2"}, {3, "P03.02_val_3"},
        {4, "P03.02_val_4"}, {5, "P03.02_val_5"}, {6, "P03.02_val_6"}, {7, "P03.02_val_7"},
        {8, "P03.02_val_8"}, {9, "P03.02_val_9"}, {10, "P03.02_val_10"}, {11, "P03.02_val_11"},
        {12, "P03.02_val_12"}, {13, "P03.02_val_13"}, {14, "P03.02_val_14"}, {15, "P03.02_val_15"},
        {16, "P03.02_val_16"}, {17, "P03.02_val_17"}, {18, "P03.02_val_18"}, {19, "P03.02_val_19"},
        {20, "P03.02_val_20"}, {21, "P03.02_val_21"}, {22, "P03.02_val_22"}, {23, "P03.02_val_23"},
        {24, "P03.02_val_24"}, {25, "P03.02_val_25"}, {26, "P03.02_val_26"}, {27, "P03.02_val_27"},
        {28, "P03.02_val_28"}, {29, "P03.02_val_29"}, {30, "P03.02_val_30"}, {31, "P03.02_val_31"},
        {32, "P03.02_val_32"}, {33, "P03.02_val_33"}, {34, "P03.02_val_34"}, {35, "P03.02_val_35"},
        {36, "P03.02_val_36"}, {37, "P03.02_val_37"}, {38, "P03.02_val_38"}, {39, "P03.02_val_39"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2003; pd.subindex = 0x08;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 4; pd.default_val = 0;
    pd.name = "P03.07"; pd.desc = "DI2 terminal logic selection"; pd.unit = "";
    pd.group_name = "P03";
    pd.group = ParamGroup::DI;
    pd.value_descs = {
        {0, "P03.07_val_0"}, {1, "P03.07_val_1"}, {2, "P03.07_val_2"}, {3, "P03.07_val_3"}, {4, "P03.07_val_4"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2003; pd.subindex = 0x09;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::POWER_CYCLE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 39; pd.default_val = 0;
    pd.name = "P03.08"; pd.desc = "DI3 terminal function selection"; pd.unit = "";
    pd.group_name = "P03";
    pd.value_descs = {
        {0, "P03.02_val_0"}, {1, "P03.02_val_1"}, {2, "P03.02_val_2"}, {3, "P03.02_val_3"},
        {4, "P03.02_val_4"}, {5, "P03.02_val_5"}, {6, "P03.02_val_6"}, {7, "P03.02_val_7"},
        {8, "P03.02_val_8"}, {9, "P03.02_val_9"}, {10, "P03.02_val_10"}, {11, "P03.02_val_11"},
        {12, "P03.02_val_12"}, {13, "P03.02_val_13"}, {14, "P03.02_val_14"}, {15, "P03.02_val_15"},
        {16, "P03.02_val_16"}, {17, "P03.02_val_17"}, {18, "P03.02_val_18"}, {19, "P03.02_val_19"},
        {20, "P03.02_val_20"}, {21, "P03.02_val_21"}, {22, "P03.02_val_22"}, {23, "P03.02_val_23"},
        {24, "P03.02_val_24"}, {25, "P03.02_val_25"}, {26, "P03.02_val_26"}, {27, "P03.02_val_27"},
        {28, "P03.02_val_28"}, {29, "P03.02_val_29"}, {30, "P03.02_val_30"}, {31, "P03.02_val_31"},
        {32, "P03.02_val_32"}, {33, "P03.02_val_33"}, {34, "P03.02_val_34"}, {35, "P03.02_val_35"},
        {36, "P03.02_val_36"}, {37, "P03.02_val_37"}, {38, "P03.02_val_38"}, {39, "P03.02_val_39"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2003; pd.subindex = 0x0A;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 4; pd.default_val = 0;
    pd.name = "P03.09"; pd.desc = "DI3 terminal logic selection"; pd.unit = "";
    pd.group_name = "P03";
    pd.group = ParamGroup::DI;
    pd.value_descs = {
        {0, "P03.09_val_0"}, {1, "P03.09_val_1"}, {2, "P03.09_val_2"}, {3, "P03.09_val_3"}, {4, "P03.09_val_4"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2003; pd.subindex = 0x0B;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::POWER_CYCLE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 39; pd.default_val = 0;
    pd.name = "P03.10"; pd.desc = "DI4 terminal function selection"; pd.unit = "";
    pd.group_name = "P03";
    pd.value_descs = {
        {0, "P03.02_val_0"}, {1, "P03.02_val_1"}, {2, "P03.02_val_2"}, {3, "P03.02_val_3"},
        {4, "P03.02_val_4"}, {5, "P03.02_val_5"}, {6, "P03.02_val_6"}, {7, "P03.02_val_7"},
        {8, "P03.02_val_8"}, {9, "P03.02_val_9"}, {10, "P03.02_val_10"}, {11, "P03.02_val_11"},
        {12, "P03.02_val_12"}, {13, "P03.02_val_13"}, {14, "P03.02_val_14"}, {15, "P03.02_val_15"},
        {16, "P03.02_val_16"}, {17, "P03.02_val_17"}, {18, "P03.02_val_18"}, {19, "P03.02_val_19"},
        {20, "P03.02_val_20"}, {21, "P03.02_val_21"}, {22, "P03.02_val_22"}, {23, "P03.02_val_23"},
        {24, "P03.02_val_24"}, {25, "P03.02_val_25"}, {26, "P03.02_val_26"}, {27, "P03.02_val_27"},
        {28, "P03.02_val_28"}, {29, "P03.02_val_29"}, {30, "P03.02_val_30"}, {31, "P03.02_val_31"},
        {32, "P03.02_val_32"}, {33, "P03.02_val_33"}, {34, "P03.02_val_34"}, {35, "P03.02_val_35"},
        {36, "P03.02_val_36"}, {37, "P03.02_val_37"}, {38, "P03.02_val_38"}, {39, "P03.02_val_39"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2003; pd.subindex = 0x0C;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 4; pd.default_val = 0;
    pd.name = "P03.11"; pd.desc = "DI4 terminal logic selection"; pd.unit = "";
    pd.group_name = "P03";
    pd.group = ParamGroup::DI;
    pd.value_descs = {
        {0, "P03.11_val_0"}, {1, "P03.11_val_1"}, {2, "P03.11_val_2"}, {3, "P03.11_val_3"}, {4, "P03.11_val_4"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2003; pd.subindex = 0x23;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::POWER_CYCLE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 0xFFFF; pd.default_val = 0;
    pd.name = "P03.34"; pd.desc = "Effective DI function allocation 3 (FunIN.33-48)"; pd.unit = "";
    pd.group_name = "P03";
    params.push_back(pd);
    pd.value_descs.clear();
}

static void fill_p04(std::vector<ParamDef>& params)
{
    ParamDef pd;

    pd.index = 0x2004; pd.subindex = 0x01;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::POWER_CYCLE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 20; pd.default_val = 1;
    pd.name = "P04.00"; pd.desc = "DO0 terminal function selection"; pd.unit = "";
    pd.group_name = "P04";
    pd.group = ParamGroup::DO;
    pd.value_descs = {
        {0, "P04.00_val_0"}, {1, "P04.00_val_1"}, {2, "P04.00_val_2"}, {3, "P04.00_val_3"},
        {4, "P04.00_val_4"}, {5, "P04.00_val_5"}, {6, "P04.00_val_6"}, {7, "P04.00_val_7"},
        {8, "P04.00_val_8"}, {9, "P04.00_val_9"}, {10, "P04.00_val_10"}, {11, "P04.00_val_11"},
        {12, "P04.00_val_12"}, {13, "P04.00_val_13"}, {14, "P04.00_val_14"}, {15, "P04.00_val_15"},
        {16, "P04.00_val_16"}, {17, "P04.00_val_17"}, {18, "P04.00_val_18"}, {19, "P04.00_val_19"},
        {20, "P04.00_val_20"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2004; pd.subindex = 0x02;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1; pd.default_val = 0;
    pd.name = "P04.01"; pd.desc = "DO0 terminal logic selection"; pd.unit = "";
    pd.group_name = "P04";
    pd.group = ParamGroup::DO;
    pd.value_descs = {{0, "P04.01_val_0"}, {1, "P04.01_val_1"}};
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2004; pd.subindex = 0x03;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::POWER_CYCLE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 20; pd.default_val = 5;
    pd.name = "P04.02"; pd.desc = "DO1 terminal function selection"; pd.unit = "";
    pd.group_name = "P04";
    pd.value_descs = {
        {0, "P04.02_val_0"}, {1, "P04.02_val_1"}, {2, "P04.02_val_2"}, {3, "P04.02_val_3"},
        {4, "P04.02_val_4"}, {5, "P04.02_val_5"}, {6, "P04.02_val_6"}, {7, "P04.02_val_7"},
        {8, "P04.02_val_8"}, {9, "P04.02_val_9"}, {10, "P04.02_val_10"}, {11, "P04.02_val_11"},
        {12, "P04.02_val_12"}, {13, "P04.02_val_13"}, {14, "P04.02_val_14"}, {15, "P04.02_val_15"},
        {16, "P04.02_val_16"}, {17, "P04.02_val_17"}, {18, "P04.02_val_18"}, {19, "P04.02_val_19"},
        {20, "P04.02_val_20"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2004; pd.subindex = 0x04;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1; pd.default_val = 0;
    pd.name = "P04.03"; pd.desc = "DO1 terminal logic selection"; pd.unit = "";
    pd.group_name = "P04";
    pd.group = ParamGroup::DO;
    pd.value_descs = {{0, "P04.03_val_0"}, {1, "P04.03_val_1"}};
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2004; pd.subindex = 0x05;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::POWER_CYCLE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 20; pd.default_val = 3;
    pd.name = "P04.04"; pd.desc = "DO2 terminal function selection"; pd.unit = "";
    pd.group_name = "P04";
    pd.value_descs = {
        {0, "P04.04_val_0"}, {1, "P04.04_val_1"}, {2, "P04.04_val_2"}, {3, "P04.04_val_3"},
        {4, "P04.04_val_4"}, {5, "P04.04_val_5"}, {6, "P04.04_val_6"}, {7, "P04.04_val_7"},
        {8, "P04.04_val_8"}, {9, "P04.04_val_9"}, {10, "P04.04_val_10"}, {11, "P04.04_val_11"},
        {12, "P04.04_val_12"}, {13, "P04.04_val_13"}, {14, "P04.04_val_14"}, {15, "P04.04_val_15"},
        {16, "P04.04_val_16"}, {17, "P04.04_val_17"}, {18, "P04.04_val_18"}, {19, "P04.04_val_19"},
        {20, "P04.04_val_20"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2004; pd.subindex = 0x06;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1; pd.default_val = 0;
    pd.name = "P04.05"; pd.desc = "DO2 terminal logic selection"; pd.unit = "";
    pd.group_name = "P04";
    pd.group = ParamGroup::DO;
    pd.value_descs = {{0, "P04.05_val_0"}, {1, "P04.05_val_1"}};
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2004; pd.subindex = 0x17;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1; pd.default_val = 0;
    pd.name = "P04.22"; pd.desc = "DO source selection (0=internal, 1=comm)"; pd.unit = "";
    pd.group_name = "P04";
    pd.value_descs = {{0, "P04.22_val_0"}, {1, "P04.22_val_1"}};
    params.push_back(pd);
    pd.value_descs.clear();
}

static void fill_p05(std::vector<ParamDef>& params)
{
    ParamDef pd;

    pd.index = 0x2005; pd.subindex = 0x05;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0; pd.max_val = 6553.5; pd.default_val = 0.0;
    pd.name = "P05.04"; pd.desc = "First-order LP filter time constant"; pd.unit = "ms";
    pd.group_name = "P05";
    pd.group = ParamGroup::POSITION;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2005; pd.subindex = 0x07;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0; pd.max_val = 128.0; pd.default_val = 0.0;
    pd.name = "P05.06"; pd.desc = "Average filter time constant"; pd.unit = "ms";
    pd.group_name = "P05";
    pd.group = ParamGroup::POSITION;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2005; pd.subindex = 0x11;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 2; pd.default_val = 0;
    pd.name = "P05.16"; pd.desc = "Clear position deviation action selection"; pd.unit = "";
    pd.group_name = "P05";
    pd.value_descs = {
        {0, "P05.16_val_0"}, {1, "P05.16_val_1"}, {2, "P05.16_val_2"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2005; pd.subindex = 0x12;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 65535; pd.default_val = 2500;
    pd.name = "P05.17"; pd.desc = "Encoder frequency division pulse number"; pd.unit = "pulse";
    pd.group_name = "P05";
    pd.group = ParamGroup::POSITION;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2005; pd.subindex = 0x14;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 2; pd.default_val = 1;
    pd.name = "P05.19"; pd.desc = "Speed feedforward control selection"; pd.unit = "";
    pd.group_name = "P05";
    pd.value_descs = {
        {0, "P05.19_val_0"}, {1, "P05.19_val_1"}, {2, "P05.19_val_2"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2005; pd.subindex = 0x20;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 9; pd.default_val = 0;
    pd.name = "P05.31"; pd.desc = "Return to origin mode"; pd.unit = "";
    pd.group_name = "P05";
    pd.group = ParamGroup::POSITION;
    pd.value_descs = {
        {0, "P05.31_val_0"}, {1, "P05.31_val_1"}, {2, "P05.31_val_2"},
        {3, "P05.31_val_3"}, {4, "P05.31_val_4"}, {5, "P05.31_val_5"},
        {6, "P05.31_val_6"}, {7, "P05.31_val_7"}, {8, "P05.31_val_8"}, {9, "P05.31_val_9"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2005; pd.subindex = 0x24;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 65535; pd.default_val = 5000;
    pd.name = "P05.35"; pd.desc = "Limit time to find origin"; pd.unit = "10ms";
    pd.group_name = "P05";
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2005; pd.subindex = 0x2D;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 65535; pd.default_val = 0;
    pd.name = "P05.44"; pd.desc = "Encoder multi-turn data offset"; pd.unit = "enc";
    pd.group_name = "P05";
    pd.group = ParamGroup::POSITION;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2005; pd.subindex = 0x2F;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 65535; pd.default_val = 0;
    pd.name = "P05.46"; pd.desc = "Absolute position linear mode bias low"; pd.unit = "enc";
    pd.group_name = "P05";
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2005; pd.subindex = 0x31;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 65535; pd.default_val = 0;
    pd.name = "P05.48"; pd.desc = "Absolute position rotation numerator"; pd.unit = "";
    pd.group_name = "P05";
    pd.group = ParamGroup::POSITION;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2005; pd.subindex = 0x32;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 65535; pd.default_val = 0;
    pd.name = "P05.49"; pd.desc = "Absolute position rotation denominator"; pd.unit = "";
    pd.group_name = "P05";
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2005; pd.subindex = 0x35;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 65535; pd.default_val = 0;
    pd.name = "P05.52"; pd.desc = "Absolute position rotation low"; pd.unit = "enc";
    pd.group_name = "P05";
    pd.group = ParamGroup::POSITION;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2005; pd.subindex = 0x37;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 127; pd.default_val = 0;
    pd.name = "P05.54"; pd.desc = "Absolute position rotation high"; pd.unit = "enc";
    pd.group_name = "P05";
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2005; pd.subindex = 0x3E;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1; pd.default_val = 1;
    pd.name = "P05.61"; pd.desc = "Position arrival threshold unit (0=enc, 1=cmd)"; pd.unit = "";
    pd.group_name = "P05";
    pd.group = ParamGroup::POSITION;
    pd.value_descs = {{0, "P05.61_val_0"}, {1, "P05.61_val_1"}};
    params.push_back(pd);
    pd.value_descs.clear();
}

void ParamDatabase::add_p08()
{
    ParamDef pd;

    // Gains — 25 params, Index: 0x2008
    pd.index = 0x2008; pd.subindex = 0x01;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.1; pd.max_val = 2000.0; pd.default_val = 25.0;
    pd.name = "P08.00"; pd.desc = "Speed loop gain"; pd.unit = "Hz";
    pd.group_name = "P08";
    pd.group = ParamGroup::GAINS;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x02;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 2;
    pd.min_val = 0.15; pd.max_val = 512.00; pd.default_val = 31.83;
    pd.name = "P08.01"; pd.desc = "Speed loop integration time constant"; pd.unit = "ms";
    pd.group_name = "P08";
    pd.group = ParamGroup::GAINS;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x03;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.0; pd.max_val = 2000.0; pd.default_val = 40.0;
    pd.name = "P08.02"; pd.desc = "Position loop gain"; pd.unit = "Hz";
    pd.group_name = "P08";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x04;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.1; pd.max_val = 2000.0; pd.default_val = 40.0;
    pd.name = "P08.03"; pd.desc = "2nd speed loop gain"; pd.unit = "Hz";
    pd.group_name = "P08";
    pd.group = ParamGroup::GAINS;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x05;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 2;
    pd.min_val = 0.15; pd.max_val = 512.00; pd.default_val = 40.00;
    pd.name = "P08.04"; pd.desc = "2nd speed loop integration time constant"; pd.unit = "ms";
    pd.group_name = "P08";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x06;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.0; pd.max_val = 2000.0; pd.default_val = 64.0;
    pd.name = "P08.05"; pd.desc = "2nd position loop gain"; pd.unit = "Hz";
    pd.group_name = "P08";
    pd.group = ParamGroup::GAINS;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x09;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1; pd.default_val = 1;
    pd.name = "P08.08"; pd.desc = "2nd gain mode setting"; pd.unit = "";
    pd.group_name = "P08";
    pd.value_descs = {
        {0, "P08.08_val_0"}, {1, "P08.08_val_1"}
    };
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x0A;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 10; pd.default_val = 0;
    pd.name = "P08.09"; pd.desc = "Gain switching condition selection"; pd.unit = "";
    pd.group_name = "P08";
    pd.group = ParamGroup::GAINS;
    pd.value_descs = {
        {0, "P08.09_val_0"}, {1, "P08.09_val_1"}, {2, "P08.09_val_2"},
        {3, "P08.09_val_3"}, {4, "P08.09_val_4"}, {5, "P08.09_val_5"},
        {6, "P08.09_val_6"}, {7, "P08.09_val_7"}, {8, "P08.09_val_8"},
        {9, "P08.09_val_9"}, {10, "P08.09_val_10"}
    };
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x0B;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.0; pd.max_val = 1000.0; pd.default_val = 5.0;
    pd.name = "P08.10"; pd.desc = "Gain switching delay time"; pd.unit = "ms";
    pd.group_name = "P08";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x0C;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 20000; pd.default_val = 50;
    pd.name = "P08.11"; pd.desc = "Gain switching level"; pd.unit = "";
    pd.group_name = "P08";
    pd.group = ParamGroup::GAINS;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x0D;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 20000; pd.default_val = 30;
    pd.name = "P08.12"; pd.desc = "Gain switching time lag"; pd.unit = "";
    pd.group_name = "P08";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x0E;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.0; pd.max_val = 1000.0; pd.default_val = 3.0;
    pd.name = "P08.13"; pd.desc = "Position gain switching time"; pd.unit = "ms";
    pd.group_name = "P08";
    pd.group = ParamGroup::GAINS;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x10;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 2;
    pd.min_val = 0.00; pd.max_val = 120.00; pd.default_val = 1.00;
    pd.name = "P08.15"; pd.desc = "Load inertia ratio"; pd.unit = "x";
    pd.group_name = "P08";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x13;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 2;
    pd.min_val = 0.00; pd.max_val = 64.00; pd.default_val = 0.50;
    pd.name = "P08.18"; pd.desc = "Speed feedforward filter time constant"; pd.unit = "ms";
    pd.group_name = "P08";
    pd.group = ParamGroup::GAINS;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x14;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.0; pd.max_val = 100.0; pd.default_val = 0.0;
    pd.name = "P08.19"; pd.desc = "Speed feedforward gain"; pd.unit = "%";
    pd.group_name = "P08";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x15;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 2;
    pd.min_val = 0.00; pd.max_val = 64.00; pd.default_val = 0.50;
    pd.name = "P08.20"; pd.desc = "Torque feedforward filter time constant"; pd.unit = "ms";
    pd.group_name = "P08";
    pd.group = ParamGroup::GAINS;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x16;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.0; pd.max_val = 200.0; pd.default_val = 0.0;
    pd.name = "P08.21"; pd.desc = "Torque feedforward gain"; pd.unit = "%";
    pd.group_name = "P08";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x17;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 4; pd.default_val = 0;
    pd.name = "P08.22"; pd.desc = "Speed feedback filtering options"; pd.unit = "";
    pd.group_name = "P08";
    pd.group = ParamGroup::GAINS;
    pd.value_descs = {
        {0, "P08.22_val_0"}, {1, "P08.22_val_1"}, {2, "P08.22_val_2"}, {3, "P08.22_val_3"}, {4, "P08.22_val_4"}
    };
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x18;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 100; pd.max_val = 4000; pd.default_val = 4000;
    pd.name = "P08.23"; pd.desc = "Speed feedback low-pass filter cutoff frequency"; pd.unit = "Hz";
    pd.group_name = "P08";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2008; pd.subindex = 0x19;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.0; pd.max_val = 100.0; pd.default_val = 100.0;
    pd.name = "P08.24"; pd.desc = "Pseudo-differential feedforward control coefficient"; pd.unit = "";
    pd.group_name = "P08";
    pd.group = ParamGroup::GAINS;
    params_.push_back(pd);
    pd.value_descs.clear();
}

void ParamDatabase::add_p09()
{
    ParamDef pd;

    // Self-tuning — 26 params, Index: 0x2009
    // subindex = panel hex + 1

    pd.index = 0x2009; pd.subindex = 0x01;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 4; pd.default_val = 0;
    pd.name = "P09.00"; pd.desc = "Self-adjusting mode selection (0=manual,1=param,2=pos,3=param+fric,4=pos+fric)"; pd.unit = "";
    pd.group_name = "P09";
    pd.group = ParamGroup::AUTOTUNE;
    pd.value_descs = {
        {0, "P09.00_val_0"}, {1, "P09.00_val_1"}, {2, "P09.00_val_2"},
        {3, "P09.00_val_3"}, {4, "P09.00_val_4"}
    };
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x02;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 31; pd.default_val = 12;
    pd.name = "P09.01"; pd.desc = "Group1 rigidity level"; pd.unit = "";
    pd.group_name = "P09";
    pd.group = ParamGroup::AUTOTUNE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x03;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 4; pd.default_val = 0;
    pd.name = "P09.02"; pd.desc = "Adaptive notch mode selection"; pd.unit = "";
    pd.group_name = "P09";
    pd.value_descs = {
        {0, "P09.02_val_0"}, {1, "P09.02_val_1"}, {2, "P09.02_val_2"}, {3, "P09.02_val_3"}, {4, "P09.02_val_4"}
    };
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x04;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 3; pd.default_val = 0;
    pd.name = "P09.03"; pd.desc = "Online inertia identification mode"; pd.unit = "";
    pd.group_name = "P09";
    pd.group = ParamGroup::AUTOTUNE;
    pd.value_descs = {
        {0, "P09.03_val_0"}, {1, "P09.03_val_1"}, {2, "P09.03_val_2"}, {3, "P09.03_val_3"}, {4, "P09.03_val_4"}
    };
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x05;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1; pd.default_val = 0;
    pd.name = "P09.04"; pd.desc = "Low frequency resonance suppression mode"; pd.unit = "";
    pd.group_name = "P09";
    pd.value_descs = {{0, "P09.04_val_0"}, {1, "P09.04_val_1"}};
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x06;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1; pd.default_val = 0;
    pd.name = "P09.05"; pd.desc = "Offline inertia identification mode (0=triangle,1=JOG)"; pd.unit = "";
    pd.group_name = "P09";
    pd.group = ParamGroup::AUTOTUNE;
    pd.value_descs = {{0, "P09.05_val_0"}, {1, "P09.05_val_1"}};
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x07;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 100; pd.max_val = 1000; pd.default_val = 500;
    pd.name = "P09.06"; pd.desc = "Inertia identification maximum speed"; pd.unit = "rpm";
    pd.group_name = "P09";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x08;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 20; pd.max_val = 800; pd.default_val = 125;
    pd.name = "P09.07"; pd.desc = "Accel time to max speed during inertia ID"; pd.unit = "ms";
    pd.group_name = "P09";
    pd.group = ParamGroup::AUTOTUNE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x09;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 50; pd.max_val = 10000; pd.default_val = 800;
    pd.name = "P09.08"; pd.desc = "Waiting time after single inertia ID"; pd.unit = "ms";
    pd.group_name = "P09";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x0A;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::READ_ONLY;
    pd.decimal_places = 2;
    pd.min_val = 0.00; pd.max_val = 2.00; pd.default_val = 0.00;
    pd.name = "P09.09"; pd.desc = "Completed single inertia ID motor turns"; pd.unit = "r";
    pd.group_name = "P09";
    pd.group = ParamGroup::AUTOTUNE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x0D;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 50; pd.max_val = 4000; pd.default_val = 4000;
    pd.name = "P09.12"; pd.desc = "Group1 notch frequency"; pd.unit = "Hz";
    pd.group_name = "P09";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x0E;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 20; pd.default_val = 2;
    pd.name = "P09.13"; pd.desc = "Group1 notch width"; pd.unit = "";
    pd.group_name = "P09";
    pd.group = ParamGroup::AUTOTUNE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x0F;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 99; pd.default_val = 0;
    pd.name = "P09.14"; pd.desc = "Group1 notch depth"; pd.unit = "";
    pd.group_name = "P09";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x10;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 50; pd.max_val = 4000; pd.default_val = 4000;
    pd.name = "P09.15"; pd.desc = "Group2 notch frequency"; pd.unit = "Hz";
    pd.group_name = "P09";
    pd.group = ParamGroup::AUTOTUNE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x11;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 20; pd.default_val = 2;
    pd.name = "P09.16"; pd.desc = "Group2 notch width"; pd.unit = "";
    pd.group_name = "P09";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x12;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 99; pd.default_val = 0;
    pd.name = "P09.17"; pd.desc = "Group2 notch depth"; pd.unit = "";
    pd.group_name = "P09";
    pd.group = ParamGroup::AUTOTUNE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x13;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 50; pd.max_val = 4000; pd.default_val = 4000;
    pd.name = "P09.18"; pd.desc = "Group3 notch frequency"; pd.unit = "Hz";
    pd.group_name = "P09";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x14;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 20; pd.default_val = 2;
    pd.name = "P09.19"; pd.desc = "Group3 notch width"; pd.unit = "";
    pd.group_name = "P09";
    pd.group = ParamGroup::AUTOTUNE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x15;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 99; pd.default_val = 0;
    pd.name = "P09.20"; pd.desc = "Group3 notch depth"; pd.unit = "";
    pd.group_name = "P09";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x16;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 50; pd.max_val = 4000; pd.default_val = 4000;
    pd.name = "P09.21"; pd.desc = "Group4 notch frequency"; pd.unit = "Hz";
    pd.group_name = "P09";
    pd.group = ParamGroup::AUTOTUNE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x17;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 20; pd.default_val = 2;
    pd.name = "P09.22"; pd.desc = "Group4 notch width"; pd.unit = "";
    pd.group_name = "P09";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x18;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 99; pd.default_val = 0;
    pd.name = "P09.23"; pd.desc = "Group4 notch depth"; pd.unit = "";
    pd.group_name = "P09";
    pd.group = ParamGroup::AUTOTUNE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x19;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::READ_ONLY;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 2; pd.default_val = 0;
    pd.name = "P09.24"; pd.desc = "Resonance frequency identification results"; pd.unit = "Hz";
    pd.group_name = "P09";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x1F;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.0; pd.max_val = 100.0; pd.default_val = 0.0;
    pd.name = "P09.30"; pd.desc = "Torque disturbance compensation gain"; pd.unit = "%";
    pd.group_name = "P09";
    pd.group = ParamGroup::AUTOTUNE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x20;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 2;
    pd.min_val = 0.00; pd.max_val = 25.00; pd.default_val = 0.50;
    pd.name = "P09.31"; pd.desc = "Torque disturbance observer filter time constant"; pd.unit = "ms";
    pd.group_name = "P09";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x27;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 1.0; pd.max_val = 100.0; pd.default_val = 100.0;
    pd.name = "P09.38"; pd.desc = "Low frequency resonance frequency"; pd.unit = "Hz";
    pd.group_name = "P09";
    pd.group = ParamGroup::AUTOTUNE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2009; pd.subindex = 0x28;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 10; pd.default_val = 2;
    pd.name = "P09.39"; pd.desc = "Low frequency resonance frequency filter setting"; pd.unit = "";
    pd.group_name = "P09";
    params_.push_back(pd);
    pd.value_descs.clear();
}

void ParamDatabase::add_p0a()
{
    // Fault/Protection — 14 params, Index: 0x200A
    // subindex = panel hex + 1
    ADD_PARAM(ParamGroup::FAULT, 0x200A, 0x01, UINT16, IMMEDIATE, 0, 0, 2, 0,
              "P0A.00", "Power input phase loss protection selection", "", "P0A");
    ADD_PARAM(ParamGroup::FAULT, 0x200A, 0x02, UINT16, IMMEDIATE, 0, 0, 2, 0,
              "P0A.01", "Absolute position limit settings", "", "P0A");
    ADD_PARAM(ParamGroup::FAULT, 0x200A, 0x04, UINT16, IMMEDIATE, 0, 0, 1, 0,
              "P0A.03", "Power-off save function enable", "", "P0A");
    ADD_PARAM(ParamGroup::FAULT, 0x200A, 0x05, UINT16, IMMEDIATE, 0, 50, 300, 100,
              "P0A.04", "Motor overload protection gain", "%", "P0A");
    ADD_PARAM(ParamGroup::FAULT, 0x200A, 0x09, UINT16, IMMEDIATE, 0, 0, 10000, 0,
              "P0A.08", "Overspeed fault threshold (0=1.2x max)", "rpm", "P0A");
    ADD_PARAM(ParamGroup::FAULT, 0x200A, 0x0D, UINT16, IMMEDIATE, 0, 0, 1, 1,
              "P0A.12", "Overrun protection function", "", "P0A");
    ADD_PARAM(ParamGroup::FAULT, 0x200A, 0x11, UINT16, IMMEDIATE, 0, 1, 1000, 5,
              "P0A.16", "Low frequency resonance position deviation threshold", "enc", "P0A");
    ADD_PARAM(ParamGroup::FAULT, 0x200A, 0x1A, UINT16, IMMEDIATE, 0, 0, 5000, 50,
              "P0A.25", "Speed feedback display filter time constant", "ms", "P0A");
    ADD_PARAM(ParamGroup::FAULT, 0x200A, 0x1B, UINT16, IMMEDIATE, 0, 0, 1, 0,
              "P0A.26", "Motor overload shield enable", "", "P0A");
    ADD_PARAM(ParamGroup::FAULT, 0x200A, 0x1C, UINT16, IMMEDIATE, 0, 0, 5000, 10,
              "P0A.27", "Speed DO filter time constant", "ms", "P0A");
    ADD_PARAM(ParamGroup::FAULT, 0x200A, 0x1D, UINT16, IMMEDIATE, 0, 0, 255, 30,
              "P0A.28", "Quadrature encoder filter time constant", "x25ns", "P0A");
    ADD_PARAM(ParamGroup::FAULT, 0x200A, 0x21, UINT16, IMMEDIATE, 0, 10, 65535, 200,
              "P0A.32", "Locked rotor over-temperature protection time window", "ms", "P0A");
    ADD_PARAM(ParamGroup::FAULT, 0x200A, 0x22, UINT16, IMMEDIATE, 0, 0, 1, 1,
              "P0A.33", "Stalled rotor over-temperature protection", "", "P0A");
    ADD_PARAM(ParamGroup::FAULT, 0x200A, 0x25, UINT16, IMMEDIATE, 0, 0, 1, 0,
              "P0A.36", "Encoder multi-turn overflow fault selection", "", "P0A");
}

void ParamDatabase::add_p0c()
{
    // Communication — 18 params, Index: 0x200C
    // subindex = panel hex + 1
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x01, UINT16, POWER_CYCLE, 0, 1, 247, 1,
              "P0C.00", "Drive address", "", "P0C");
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x03, UINT16, POWER_CYCLE, 0, 0, 5, 5,
              "P0C.02", "Serial port baud rate", "", "P0C");
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x04, UINT16, POWER_CYCLE, 0, 0, 3, 0,
              "P0C.03", "MODBUS data format", "", "P0C");
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x05, UINT16, READ_ONLY, 0, 0, 0, 0,
              "P0C.04", "Site name (auto-assigned station number)", "", "P0C");
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x06, UINT16, IMMEDIATE, 0, 0, 0, 0,
              "P0C.05", "Site alias (manual station number)", "", "P0C");
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x0E, UINT16, IMMEDIATE, 0, 0, 3, 0,
              "P0C.13", "EEPROM save for communication writes", "", "P0C");
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x24, UINT16, IMMEDIATE, 0, 4, 20, 9,
              "P0C.35", "EtherCAT sync interrupt lost disallowed times", "ms", "P0C");
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x25, UINT16, READ_ONLY, 0, 0, 65535, 0,
              "P0C.36", "Port0 CRC check error counter", "", "P0C");
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x26, UINT16, READ_ONLY, 0, 0, 65535, 0,
              "P0C.37", "Port1 CRC check error counter", "", "P0C");
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x27, UINT16, READ_ONLY, 0, 0, 65535, 0,
              "P0C.38", "Port0/1 data forwarding error counter", "", "P0C");
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x28, UINT16, READ_ONLY, 0, 0, 65535, 0,
              "P0C.39", "Processing unit and PDI errors counter", "", "P0C");
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x29, UINT16, READ_ONLY, 0, 0, 65535, 0,
              "P0C.40", "Port0/1 link lost counter", "", "P0C");
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x2B, UINT16, IMMEDIATE, 0, 0, 1, 0,
              "P0C.42", "Sync error monitoring mode", "", "P0C");
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x2C, UINT16, IMMEDIATE, 0, 0, 2, 2,
              "P0C.43", "Sync mode (0=async,1=jitter≤1us,2=jitter>1us)", "", "P0C");
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x2D, UINT16, IMMEDIATE, 0, 0, 2000, 500,
              "P0C.44", "Sync error threshold", "ns", "P0C");
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x2E, UINT16, IMMEDIATE, 0, 0, 1, 1,
              "P0C.45", "Location cache settings", "", "P0C");
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x2F, UINT16, IMMEDIATE, 0, 1, 7, 3,
              "P0C.46", "CSP position command increment excessive threshold", "", "P0C");
    ADD_PARAM(ParamGroup::COMM, 0x200C, 0x30, UINT16, READ_ONLY, 0, 0, 65535, 0,
              "P0C.47", "CSP position command increment too large times", "", "P0C");
}

void ParamDatabase::add_p0d()
{
    // Auxiliary — 10 params, Index: 0x200D
    ParamDef pd;
    pd.index = 0x200D; pd.subindex = 0x01;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1; pd.default_val = 0;
    pd.name = "P0D.00"; pd.desc = "Software reset"; pd.unit = "";
    pd.group_name = "P0D";
    pd.group = ParamGroup::AUX;
    pd.value_descs = {{0, "P0D.00_val_0"}, {1, "P0D.00_val_1"}};
    params_.push_back(pd);
    pd.value_descs.clear();
    pd.index = 0x200D; pd.subindex = 0x02;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1; pd.default_val = 0;
    pd.name = "P0D.01"; pd.desc = "Fault reset"; pd.unit = "";
    pd.group_name = "P0D";
    pd.group = ParamGroup::AUX;
    pd.value_descs = {{0, "P0D.01_val_0"}, {1, "P0D.01_val_1"}};
    params_.push_back(pd);
    pd.value_descs.clear();
    ADD_PARAM(ParamGroup::AUX, 0x200D, 0x03, UINT16, IMMEDIATE, 0, 0, 0, 0,
              "P0D.02", "Offline inertia identification function", "", "P0D");
    ADD_PARAM(ParamGroup::AUX, 0x200D, 0x04, UINT16, IMMEDIATE, 0, 0, 1, 0,
              "P0D.03", "Initial angle recognition", "", "P0D");
    pd.index = 0x200D; pd.subindex = 0x06;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1; pd.default_val = 0;
    pd.name = "P0D.05"; pd.desc = "Emergency shutdown"; pd.unit = "";
    pd.group_name = "P0D";
    pd.value_descs = {{0, "P0D.05_val_0"}, {1, "P0D.05_val_1"}};
    params_.push_back(pd);
    pd.value_descs.clear();
    ADD_PARAM(ParamGroup::AUX, 0x200D, 0x0C, UINT16, IMMEDIATE, 0, 0, 0, 0,
              "P0D.11", "JOG trial operation function (with filter)", "", "P0D");
    pd.index = 0x200D; pd.subindex = 0x12;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1; pd.default_val = 0;
    pd.name = "P0D.17"; pd.desc = "DIDO forced input and output enable"; pd.unit = "";
    pd.group_name = "P0D";
    pd.group = ParamGroup::AUX;
    pd.value_descs = {{0, "P0D.17_val_0"}, {1, "P0D.17_val_1"}, {2, "P0D.17_val_2"}, {3, "P0D.17_val_3"}, {4, "P0D.17_val_4"}};
    params_.push_back(pd);
    pd.value_descs.clear();
    ADD_PARAM(ParamGroup::AUX, 0x200D, 0x13, UINT16, IMMEDIATE, 0, 0, 0x01FF, 0x01FF,
              "P0D.18", "DI forced input given", "", "P0D");
    ADD_PARAM(ParamGroup::AUX, 0x200D, 0x14, UINT16, IMMEDIATE, 0, 0, 0x001F, 0,
              "P0D.19", "DO forced output given", "", "P0D");
    pd.index = 0x200D; pd.subindex = 0x15;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::POWER_CYCLE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 2; pd.default_val = 0;
    pd.name = "P0D.20"; pd.desc = "Absolute encoder reset enable"; pd.unit = "";
    pd.group_name = "P0D";
    pd.value_descs = {
        {0, "P0D.20_val_0"}, {1, "P0D.20_val_1"}, {2, "P0D.20_val_2"}
    };
    params_.push_back(pd);
    pd.value_descs.clear();
}

void ParamDatabase::add_p0b()
{
    // Monitoring — 33 params, all READ_ONLY, Index: 0x200B
    // subindex = panel hex + 1 (P0B.07 → panel 07 → sub 0x08)
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x01, INT16, READ_ONLY, 0, 0, 0, 0,
              "P0B.00", "Actual motor speed", "rpm", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x02, INT16, READ_ONLY, 0, 0, 0, 0,
              "P0B.01", "Speed command", "rpm", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x03, INT16, READ_ONLY, 1, 0.0, 0.0, 0.0,
              "P0B.02", "Internal torque command", "%", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x04, INT16, READ_ONLY, 0, 0, 0, 0,
              "P0B.03", "DI signal monitoring (5 terminals)", "", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x06, INT16, READ_ONLY, 0, 0, 0, 0,
              "P0B.05", "DO signal monitoring (3 terminals)", "", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x08, INT32, READ_ONLY, 0, 0, 0, 0,
              "P0B.07", "Absolute position counter (32-bit, cmd unit)", "cnt", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x0E, INT32, READ_ONLY, 0, 0, 0, 0,
              "P0B.13", "Input position command count", "cnt", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x10, INT32, READ_ONLY, 0, 0, 0, 0,
              "P0B.15", "Encoder position deviation value", "enc", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x12, INT32, READ_ONLY, 0, 0, 0, 0,
              "P0B.17", "Feedback pulse counter", "enc", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x14, INT16, READ_ONLY, 0, 0, 0, 0,
              "P0B.19", "DI signal monitoring (5 terminals, detailed)", "", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x19, UINT16, READ_ONLY, 2, 0.00, 0.00, 0.00,
              "P0B.24", "Phase current RMS value", "A", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x1B, UINT16, READ_ONLY, 1, 0.0, 0.0, 0.0,
              "P0B.26", "Bus voltage value (DC)", "V", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x1C, UINT16, READ_ONLY, 0, 0, 0, 0,
              "P0B.27", "Module temperature value", "C", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x22, UINT16, READ_ONLY, 0, 0, 9, 0,
              "P0B.33", "Fault recording select (0=current...9=9th ago)", "", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x23, UINT16, READ_ONLY, 0, 0, 0, 0,
              "P0B.34", "Fault code of selected time", "", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x24, INT32, READ_ONLY, 0, 0, 0, 0,
              "P0B.35", "Selected fault timestamp (total servo run time)", "s", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x26, INT16, READ_ONLY, 0, 0, 0, 0,
              "P0B.37", "Motor speed at selected fault", "rpm", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x27, INT16, READ_ONLY, 0, 0, 0, 0,
              "P0B.38", "Motor U-phase current at selected fault", "A", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x28, INT16, READ_ONLY, 0, 0, 0, 0,
              "P0B.39", "Motor V-phase current at selected fault", "A", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x29, UINT16, READ_ONLY, 0, 0, 0, 0,
              "P0B.40", "Bus voltage at selected fault", "V", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x2A, UINT16, READ_ONLY, 0, 0, 0, 0,
              "P0B.41", "Input terminal status at selected fault", "", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x2B, UINT16, READ_ONLY, 0, 0, 0, 0,
              "P0B.42", "Output terminal status at selected fault", "", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x36, INT32, READ_ONLY, 0, 0, 0, 0,
              "P0B.53", "Position deviation counter (cmd unit - encoder)", "cnt", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x38, INT32, READ_ONLY, 1, 0.0, 0.0, 0.0,
              "P0B.55", "Actual motor speed", "0.1rpm", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x3A, UINT16, READ_ONLY, 0, 0, 0, 0,
              "P0B.57", "Control power bus voltage", "", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x3B, INT32, READ_ONLY, 0, 0, 0, 0,
              "P0B.58", "Mechanical absolute position low 32-bit", "enc", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x3D, INT32, READ_ONLY, 0, 0, 0, 0,
              "P0B.60", "Mechanical absolute position high 32-bit", "enc", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x41, INT32, READ_ONLY, 0, 0, 0, 0,
              "P0B.64", "Input position instruction count (before gear)", "cnt", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x47, UINT16, READ_ONLY, 0, 0, 0, 0,
              "P0B.70", "Absolute encoder rotation number", "r", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x48, INT32, READ_ONLY, 0, 0, 0, 0,
              "P0B.71", "Absolute encoder position within 1 turn", "enc", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x4E, INT32, READ_ONLY, 0, 0, 0, 0,
              "P0B.77", "Absolute encoder position low 32-bit", "enc", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x50, INT32, READ_ONLY, 0, 0, 0, 0,
              "P0B.79", "Absolute encoder position high 32-bit", "enc", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x52, UINT32, READ_ONLY, 0, 0, 0, 0,
              "P0B.81", "Rotating load single circle position low 32-bit", "enc", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x54, UINT32, READ_ONLY, 0, 0, 0, 0,
              "P0B.83", "Rotating load single circle position high 32-bit", "enc", "P0B");
    ADD_PARAM(ParamGroup::MONITOR, 0x200B, 0x56, UINT32, READ_ONLY, 0, 0, 0, 0,
              "P0B.85", "Rotating load single circle position", "cnt", "P0B");
}

void ParamDatabase::add_p06()
{
    ParamDef pd;

    // JOG — 3 params, Index: 0x2006
    pd.index = 0x2006; pd.subindex = 0x05;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 6000; pd.default_val = 100;
    pd.name = "P06.04"; pd.desc = "Jog speed setting"; pd.unit = "rpm";
    pd.group_name = "P06";
    pd.group = ParamGroup::JOG;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2006; pd.subindex = 0x0C;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 2; pd.default_val = 1;
    pd.name = "P06.11"; pd.desc = "Torque feedforward control selection"; pd.unit = "";
    pd.group_name = "P06";
    pd.group = ParamGroup::JOG;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2006; pd.subindex = 0x10;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 6000; pd.default_val = 10;
    pd.name = "P06.15"; pd.desc = "Zero position fixed speed threshold"; pd.unit = "rpm";
    pd.group_name = "P06";
    pd.group = ParamGroup::JOG;
    params_.push_back(pd);
    pd.value_descs.clear();
}

void ParamDatabase::add_p07()
{
    ParamDef pd;

    // Torque Control — 16 params, Index: 0x2007
    pd.index = 0x2007; pd.subindex = 0x06;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 2;
    pd.min_val = 0.00; pd.max_val = 30.00; pd.default_val = 0.79;
    pd.name = "P07.05"; pd.desc = "Torque command filter time constant"; pd.unit = "ms";
    pd.group_name = "P07";
    pd.group = ParamGroup::TORQUE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2007; pd.subindex = 0x07;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 2;
    pd.min_val = 0.00; pd.max_val = 30.00; pd.default_val = 0.79;
    pd.name = "P07.06"; pd.desc = "2nd torque command filter time constant"; pd.unit = "ms";
    pd.group_name = "P07";
    pd.group = ParamGroup::TORQUE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2007; pd.subindex = 0x08;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 4; pd.default_val = 2;
    pd.name = "P07.07"; pd.desc = "Torque limit source"; pd.unit = "";
    pd.group_name = "P07";
    pd.value_descs = {
        {0, "P07.07_val_0"}, {1, "P07.07_val_1"},
        {2, "P07.07_val_2"}, {3, "P07.07_val_3"}, {4, "P07.07_val_4"}
    };
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2007; pd.subindex = 0x0A;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.0; pd.max_val = 300.0; pd.default_val = 300.0;
    pd.name = "P07.09"; pd.desc = "Positive internal torque limit"; pd.unit = "%";
    pd.group_name = "P07";
    pd.group = ParamGroup::TORQUE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2007; pd.subindex = 0x0B;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.0; pd.max_val = 300.0; pd.default_val = 300.0;
    pd.name = "P07.10"; pd.desc = "Negative internal torque limit"; pd.unit = "%";
    pd.group_name = "P07";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2007; pd.subindex = 0x0C;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.0; pd.max_val = 300.0; pd.default_val = 300.0;
    pd.name = "P07.11"; pd.desc = "Positive external torque limit"; pd.unit = "%";
    pd.group_name = "P07";
    pd.group = ParamGroup::TORQUE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2007; pd.subindex = 0x0D;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.0; pd.max_val = 300.0; pd.default_val = 300.0;
    pd.name = "P07.12"; pd.desc = "Negative external torque limit"; pd.unit = "%";
    pd.group_name = "P07";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2007; pd.subindex = 0x10;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.0; pd.max_val = 300.0; pd.default_val = 300.0;
    pd.name = "P07.15"; pd.desc = "Emergency stop torque"; pd.unit = "%";
    pd.group_name = "P07";
    pd.group = ParamGroup::TORQUE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2007; pd.subindex = 0x12;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 2; pd.default_val = 0;
    pd.name = "P07.17"; pd.desc = "Speed limit source selection"; pd.unit = "";
    pd.group_name = "P07";
    pd.value_descs = {
        {0, "P07.17_val_0"}, {1, "P07.17_val_1"},
        {2, "P07.17_val_2"}
    };
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2007; pd.subindex = 0x14;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 6000; pd.default_val = 3000;
    pd.name = "P07.19"; pd.desc = "Torque control forward speed limit / speed limit 1"; pd.unit = "rpm";
    pd.group_name = "P07";
    pd.group = ParamGroup::TORQUE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2007; pd.subindex = 0x15;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 6000; pd.default_val = 3000;
    pd.name = "P07.20"; pd.desc = "Torque control negative speed limit / speed limit 2"; pd.unit = "rpm";
    pd.group_name = "P07";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2007; pd.subindex = 0x16;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.0; pd.max_val = 300.0; pd.default_val = 0.0;
    pd.name = "P07.21"; pd.desc = "Torque reaches reference value"; pd.unit = "%";
    pd.group_name = "P07";
    pd.group = ParamGroup::TORQUE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2007; pd.subindex = 0x17;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.0; pd.max_val = 300.0; pd.default_val = 20.0;
    pd.name = "P07.22"; pd.desc = "Torque reaches effective value"; pd.unit = "%";
    pd.group_name = "P07";
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2007; pd.subindex = 0x18;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.0; pd.max_val = 300.0; pd.default_val = 10.0;
    pd.name = "P07.23"; pd.desc = "Torque reaches invalid value"; pd.unit = "%";
    pd.group_name = "P07";
    pd.group = ParamGroup::TORQUE;
    params_.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2007; pd.subindex = 0x29;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 1;
    pd.min_val = 0.5; pd.max_val = 30.0; pd.default_val = 1.0;
    pd.name = "P07.40"; pd.desc = "Speed limited window in torque mode"; pd.unit = "ms";
    pd.group_name = "P07";
    params_.push_back(pd);
    pd.value_descs.clear();
}

static void fill_p02(std::vector<ParamDef>& params)
{
    ParamDef pd;

    pd.index = 0x2002; pd.subindex = 0x01;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::SHUTDOWN;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 9; pd.default_val = 9;
    pd.name = "P02.00"; pd.desc = "Control mode selection"; pd.unit = "";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    pd.value_descs = {
        {9, "P02.00_val_9"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x02;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::POWER_CYCLE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1; pd.default_val = 0;
    pd.name = "P02.01"; pd.desc = "Encoder type selection"; pd.unit = "";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    pd.value_descs = {
        {0, "P02.01_val_0"}, {1, "P02.01_val_1"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x03;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::POWER_CYCLE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1; pd.default_val = 0;
    pd.name = "P02.02"; pd.desc = "Rotation direction selection"; pd.unit = "";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    pd.value_descs = {
        {0, "P02.02_val_0"}, {1, "P02.02_val_1"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x04;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::POWER_CYCLE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1; pd.default_val = 0;
    pd.name = "P02.03"; pd.desc = "Output pulse phase"; pd.unit = "";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    pd.value_descs = {
        {0, "P02.03_val_0"}, {1, "P02.03_val_1"}, {2, "P02.03_val_2"}, {3, "P02.03_val_3"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x06;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::SHUTDOWN;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1; pd.default_val = 0;
    pd.name = "P02.05"; pd.desc = "Servo OFF shutdown mode"; pd.unit = "";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    pd.value_descs = {
        {0, "P02.05_val_0"}, {1, "P02.05_val_1"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x08;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 2; pd.default_val = 1;
    pd.name = "P02.07"; pd.desc = "Overtravel stop mode"; pd.unit = "";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    pd.value_descs = {
        {0, "P02.07_val_0"}, {1, "P02.07_val_1"}, {2, "P02.07_val_2"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x09;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 2; pd.default_val = 0;
    pd.name = "P02.08"; pd.desc = "Fault No.1 shutdown mode"; pd.unit = "";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    pd.value_descs = {
        {0, "P02.08_val_0"}, {1, "P02.08_val_1"}, {2, "P02.08_val_2"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x0A;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 500; pd.default_val = 250;
    pd.name = "P02.09"; pd.desc = "Brake ON delay"; pd.unit = "ms";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x0B;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 1; pd.max_val = 1000; pd.default_val = 150;
    pd.name = "P02.10"; pd.desc = "Brake OFF delay (static)"; pd.unit = "ms";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x0C;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 3000; pd.default_val = 30;
    pd.name = "P02.11"; pd.desc = "Speed threshold for brake OFF"; pd.unit = "rpm";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x0D;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 1; pd.max_val = 1000; pd.default_val = 500;
    pd.name = "P02.12"; pd.desc = "Servo OFF to brake OFF delay (rotating)"; pd.unit = "ms";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x10;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1; pd.default_val = 0;
    pd.name = "P02.15"; pd.desc = "LED warning display selection"; pd.unit = "";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    pd.value_descs = {
        {0, "P02.15_val_0"}, {1, "P02.15_val_1"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x16;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::READ_ONLY;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 65535; pd.default_val = 40;
    pd.name = "P02.21"; pd.desc = "Minimum braking resistor"; pd.unit = "ohm";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x17;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::READ_ONLY;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 65535; pd.default_val = 50;
    pd.name = "P02.22"; pd.desc = "Built-in braking resistor power"; pd.unit = "W";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x18;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::READ_ONLY;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 65535; pd.default_val = 50;
    pd.name = "P02.23"; pd.desc = "Built-in braking resistor resistance"; pd.unit = "ohm";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x19;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 100; pd.default_val = 30;
    pd.name = "P02.24"; pd.desc = "Resistor heat dissipation coefficient"; pd.unit = "%";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x1A;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::POWER_CYCLE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 3; pd.default_val = 0;
    pd.name = "P02.25"; pd.desc = "Braking resistor setting"; pd.unit = "";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    pd.value_descs = {
        {0, "P02.25_val_0"}, {1, "P02.25_val_1"}, {2, "P02.25_val_2"}, {3, "P02.25_val_3"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x1B;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 65535; pd.default_val = 0;
    pd.name = "P02.26"; pd.desc = "External braking resistor power"; pd.unit = "W";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x1C;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 1000; pd.default_val = 0;
    pd.name = "P02.27"; pd.desc = "External braking resistor resistance"; pd.unit = "ohm";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x20;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::POWER_CYCLE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 2; pd.default_val = 0;
    pd.name = "P02.31"; pd.desc = "System parameter initialization"; pd.unit = "";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    pd.value_descs = {
        {0, "P02.31_val_0"}, {1, "P02.31_val_1"}, {2, "P02.31_val_2"}
    };
    params.push_back(pd);
    pd.value_descs.clear();

    pd.index = 0x2002; pd.subindex = 0x21;
    pd.type = ParamType::UINT16; pd.apply = ApplyMode::IMMEDIATE;
    pd.decimal_places = 0;
    pd.min_val = 0; pd.max_val = 99; pd.default_val = 50;
    pd.name = "P02.32"; pd.desc = "Panel default display function"; pd.unit = "";
    pd.group_name = "P02";
    pd.group = ParamGroup::CONTROL;
    params.push_back(pd);
    pd.value_descs.clear();
}

const ParamDef* ParamDatabase::find(std::string_view name) const {
    for (auto& p : params_)
        if (p.name == name) return &p;
    return nullptr;
}

const ParamDef* ParamDatabase::find(uint16_t index, uint8_t subindex) const {
    for (auto& p : params_)
        if (p.index == index && p.subindex == subindex) return &p;
    return nullptr;
}

std::vector<const ParamDef*> ParamDatabase::group(ParamGroup g) const {
    std::vector<const ParamDef*> result;
    for (auto& p : params_)
        if (p.group == g)
            result.push_back(&p);
    return result;
}

std::vector<std::string_view> ParamDatabase::names() const {
    std::vector<std::string_view> result;
    for (auto& p : params_)
        result.push_back(p.name);
    return result;
}

std::optional<ParamGroup> ParamDatabase::parse_group(std::string_view s) {
    if (s == "P01" || s == "p01") return ParamGroup::BASIC;
    if (s == "P02" || s == "p02") return ParamGroup::CONTROL;
    if (s == "P03" || s == "p03") return ParamGroup::DI;
    if (s == "P04" || s == "p04") return ParamGroup::DO;
    if (s == "P05" || s == "p05") return ParamGroup::POSITION;
    if (s == "P06" || s == "p06") return ParamGroup::JOG;
    if (s == "P07" || s == "p07") return ParamGroup::TORQUE;
    if (s == "P08" || s == "p08") return ParamGroup::GAINS;
    if (s == "P09" || s == "p09") return ParamGroup::AUTOTUNE;
    if (s == "P0B" || s == "p0b") return ParamGroup::MONITOR;
    if (s == "P0C" || s == "p0c") return ParamGroup::COMM;
    if (s == "P0A" || s == "p0a") return ParamGroup::FAULT;
    if (s == "P0D" || s == "p0d") return ParamGroup::AUX;
    return std::nullopt;
}

uint16_t ParamDatabase::group_index(ParamGroup g) {
    switch (g) {
        case ParamGroup::BASIC: return 0x2001;
        case ParamGroup::CONTROL: return 0x2002;
        case ParamGroup::DI: return 0x2003;
        case ParamGroup::DO: return 0x2004;
        case ParamGroup::POSITION: return 0x2005;
        case ParamGroup::JOG: return 0x2006;
        case ParamGroup::TORQUE: return 0x2007;
        case ParamGroup::GAINS: return 0x2008;
        case ParamGroup::AUTOTUNE: return 0x2009;
        case ParamGroup::FAULT: return 0x200A;
        case ParamGroup::MONITOR: return 0x200B;
        case ParamGroup::COMM: return 0x200C;
        case ParamGroup::AUX: return 0x200D;
    }
    return 0; // Unknown group
}

std::string_view ParamDatabase::group_panel(ParamGroup g) {
    switch (g) {
        case ParamGroup::BASIC: return "P01";
        case ParamGroup::CONTROL: return "P02";
        case ParamGroup::DI: return "P03";
        case ParamGroup::DO: return "P04";
        case ParamGroup::POSITION: return "P05";
        case ParamGroup::JOG: return "P06";
        case ParamGroup::TORQUE: return "P07";
        case ParamGroup::GAINS: return "P08";
        case ParamGroup::AUTOTUNE: return "P09";
        case ParamGroup::FAULT: return "P0A";
        case ParamGroup::MONITOR: return "P0B";
        case ParamGroup::COMM: return "P0C";
        case ParamGroup::AUX: return "P0D";
    }
    return "P08";
}

bool ParamDef::validate_raw(int64_t raw_val) const {
    // Compare in display units to avoid double-precision issues
    double raw_display = static_cast<double>(raw_val) / scale_factor();
    return raw_display >= min_val && raw_display <= max_val;
}

std::optional<std::string> ParamDef::validate_display(const std::string& str_val) const {
    try {
        double val = std::stod(str_val);
        if (val < min_val || val > max_val)
            return "Value out of range [" + std::to_string(min_val)
                   + ", " + std::to_string(max_val) + "] " + std::string(unit);
        return std::nullopt;
    } catch (...) {
        return "Invalid number format";
    }
}

std::string ParamDef::raw_to_display(int64_t raw_val) const {
    double val = raw_val / scale_factor();
    auto s = std::to_string(val);
    auto dot = s.find('.');
    if (dot != std::string::npos && decimal_places >= 0)
        s = s.substr(0, dot + 1 + decimal_places);
    return s + " " + std::string(unit);
}

std::optional<int64_t> ParamDef::display_to_raw(const std::string& str_val) const {
    try {
        double val = std::stod(str_val);
        return static_cast<int64_t>(val * scale_factor());
    } catch (...) {
        return std::nullopt;
    }
}

double ParamDef::scale_factor() const {
    if (decimal_places <= 0) return 1.0;
    double f = 1.0;
    for (int i = 0; i < decimal_places; ++i) f *= 10.0;
    return f;
}

const ParamDatabase& param_db() {
    static ParamDatabase db;
    return db;
}

std::string_view param_group_name(ParamGroup g) { return ParamDatabase::group_panel(g); }
std::string_view param_type_name(ParamType t) {
    switch (t) {
        case ParamType::UINT8:  return "uint8";
        case ParamType::INT8:   return "int8";
        case ParamType::UINT16: return "uint16";
        case ParamType::INT16:  return "int16";
        case ParamType::UINT32: return "uint32";
        case ParamType::INT32:  return "int32";
        case ParamType::FLOAT:  return "float";
        case ParamType::STRING: return "string";
    }
    return "unknown";
}
std::string_view apply_mode_name(ApplyMode m) {
    switch (m) {
        case ApplyMode::IMMEDIATE: return "Immediate";
        case ApplyMode::SHUTDOWN: return "Shutdown";
        case ApplyMode::POWER_CYCLE: return "Power cycle";
        case ApplyMode::READ_ONLY: return "Read only";
    }
    return "Unknown";
}

std::optional<int64_t> parse_number(std::string_view s) {
    try {
        std::string str(s);
        // Trim whitespace
        size_t b = str.find_first_not_of(" \t");
        size_t e = str.find_last_not_of(" \t");
        if (b == std::string::npos) return std::nullopt;
        str = str.substr(b, e - b + 1);
        // Hex (0x prefix)
        if (str.size() > 2 && (str[0] == '0') && (str[1] == 'x' || str[1] == 'X'))
            return std::stoll(str, nullptr, 16);
        return std::stoll(str, nullptr, 10);
    }
    catch (...) { return std::nullopt; }
}
