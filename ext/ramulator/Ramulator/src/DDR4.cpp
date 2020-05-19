
#include "DDR4.h"
#include "DRAM.h"

#include <vector>
#include <functional>
#include <cassert>

using namespace std;
using namespace ramulator;

string DDR4::standard_name = "DDR4";
string DDR4::level_str [int(Level::MAX)] = {"Ch", "Ra", "Bg", "Ba", "Ro", "Co"};

map<string, enum DDR4::Org> DDR4::org_map = {
    {"DDR4_2Gb_x4", DDR4::Org::DDR4_2Gb_x4}, {"DDR4_2Gb_x8", DDR4::Org::DDR4_2Gb_x8}, {"DDR4_2Gb_x16", DDR4::Org::DDR4_2Gb_x16},
    {"DDR4_4Gb_x4", DDR4::Org::DDR4_4Gb_x4}, {"DDR4_4Gb_x8", DDR4::Org::DDR4_4Gb_x8}, {"DDR4_4Gb_x16", DDR4::Org::DDR4_4Gb_x16},
    {"DDR4_8Gb_x4", DDR4::Org::DDR4_8Gb_x4}, {"DDR4_8Gb_x8", DDR4::Org::DDR4_8Gb_x8}, {"DDR4_8Gb_x16", DDR4::Org::DDR4_8Gb_x16},
    {"DDR4_4Gb_x8_w16", DDR4::Org::DDR4_4Gb_x8_w16}, {"DDR4_4Gb_x8_w32", DDR4::Org::DDR4_4Gb_x8_w32}, {"DDR4_4Gb_x8_w64", DDR4::Org::DDR4_4Gb_x8_w64},
    {"DDR4_4Gb_x8_w8", DDR4::Org::DDR4_4Gb_x8_w8},
    {"DDR4_4Gb_x8_4xBank", DDR4::Org::DDR4_4Gb_x8_4xBank},
    {"DDR4_4Gb_x8_2xBank", DDR4::Org::DDR4_4Gb_x8_2xBank}
};

map<string, enum DDR4::Speed> DDR4::speed_map = {
    {"DDR4_1600K", DDR4::Speed::DDR4_1600K},
    {"DDR4_1600L", DDR4::Speed::DDR4_1600L},
    {"DDR4_1866M", DDR4::Speed::DDR4_1866M},
    {"DDR4_1866N", DDR4::Speed::DDR4_1866N},
    {"DDR4_2133P", DDR4::Speed::DDR4_2133P},
    {"DDR4_2133R", DDR4::Speed::DDR4_2133R},
    {"DDR4_2400R", DDR4::Speed::DDR4_2400R},
    // gagan
    {"DDR4_2400R_base", DDR4::Speed::DDR4_2400R_base},
    {"DDR4_2400R_ideal_nbr_lbb", DDR4::Speed::DDR4_2400R_ideal_nbr_lbb},
    {"DDR4_2933R_ideal_nbr_lbb_ts", DDR4::Speed::DDR4_2933R_ideal_nbr_lbb_ts},
    {"DDR4_2933R_ideal_nbr_lbb_sts", DDR4::Speed::DDR4_2933R_ideal_nbr_lbb_sts},
    // Ramulator
    {"DDR4_2400U", DDR4::Speed::DDR4_2400U},
    {"DDR4_3200", DDR4::Speed::DDR4_3200},
    // daz3
    {"DDR4_3200_base", DDR4::Speed::DDR4_3200_base},
    {"DDR4_3200_ideal_v1", DDR4::Speed::DDR4_3200_ideal_v1},
    {"DDR4_3200_ideal_v2", DDR4::Speed::DDR4_3200_ideal_v2},
    {"DDR4_3200_ideal_v2_modified", DDR4::Speed::DDR4_3200_ideal_v2_modified},
    {"DDR4_3200_ideal_v3", DDR4::Speed::DDR4_3200_ideal_v3},
    {"DDR4_3200_ideal_v4", DDR4::Speed::DDR4_3200_ideal_v4},
    {"DDR4_3200_ideal_v7", DDR4::Speed::DDR4_3200_ideal_v7},
    {"DDR4_3200_ideal_v4s", DDR4::Speed::DDR4_3200_ideal_v4s},
    {"DDR4_3200_ideal_v5a", DDR4::Speed::DDR4_3200_ideal_v5a},
    {"DDR4_3600_ideal_v4", DDR4::Speed::DDR4_3600_ideal_v4},
    {"DDR4_3866_ideal_v4", DDR4::Speed::DDR4_3866_ideal_v4},
    {"DDR4_3200_ideal_nbr_lbb", DDR4::Speed::DDR4_3200_ideal_nbr_lbb},
    {"DDR4_3866_ideal_nbr_lbb_ts", DDR4::Speed::DDR4_3866_ideal_nbr_lbb_ts},
    {"DDR4_3866_ideal_nbr_lbb_sts", DDR4::Speed::DDR4_3866_ideal_nbr_lbb_sts},
    {"DDR4_3200_base_half_w8", DDR4::Speed::DDR4_3200_base_half_w8},
    {"DDR4_3200_base_half_w16", DDR4::Speed::DDR4_3200_base_half_w16},
    {"DDR4_3200_base_half_w32", DDR4::Speed::DDR4_3200_base_half_w32},
    {"DDR4_3200_base_half_w64", DDR4::Speed::DDR4_3200_base_half_w64},
    {"DDR4_3200_base_full_w8", DDR4::Speed::DDR4_3200_base_full_w8},
    {"DDR4_3200_base_full_w16", DDR4::Speed::DDR4_3200_base_full_w16},
    {"DDR4_3200_base_full_w32", DDR4::Speed::DDR4_3200_base_full_w32},
    {"DDR4_3200_base_full_w64", DDR4::Speed::DDR4_3200_base_full_w64},
    {"DDR4_3200_base_quarter_w8", DDR4::Speed::DDR4_3200_base_quarter_w8},
    {"DDR4_3200_base_quarter_w16", DDR4::Speed::DDR4_3200_base_quarter_w16},
    {"DDR4_3200_base_quarter_w32", DDR4::Speed::DDR4_3200_base_quarter_w32},
    {"DDR4_3200_base_quarter_w64", DDR4::Speed::DDR4_3200_base_quarter_w64},
};


DDR4::DDR4(Org org, Speed speed)
    : org_entry(org_table[int(org)]),
    speed_entry(speed_table[int(speed)]), 
    read_latency(speed_entry.nCL + speed_entry.nBL),
    mySpeed(speed)//daz3
{
    if(int(org) == int(DDR4::Org::DDR4_4Gb_x8_w8))
    {
        channel_width = 8;
    }
    else if(int(org) == int(DDR4::Org::DDR4_4Gb_x8_w16))
    {
        channel_width = 16;
    }
    else if(int(org) == int(DDR4::Org::DDR4_4Gb_x8_w32))
    {
        channel_width = 32;
    }
    else if(int(org) == int(DDR4::Org::DDR4_4Gb_x8_w64))
    {
        channel_width = 64;
    }

    if(int(speed) == int(DDR4::Speed::DDR4_3200_ideal_v4) || int(speed) == int(DDR4::Speed::DDR4_3600_ideal_v4) ||
       int(speed) == int(DDR4::Speed::DDR4_3866_ideal_v4) || int(speed) == int(DDR4::Speed::DDR4_3866_ideal_nbr_lbb_ts) ||
       int(speed) == int(DDR4::Speed::DDR4_2933R_ideal_nbr_lbb_ts))
    {
        SpeedEntry& s = speed_entry;
        int _RCD = s.nRCD;
        _RCD *= 0.75;
        int diff_RCD = _RCD - s.nRCD;
        s.nRCD = _RCD;
        s.nRC = s.nRC + diff_RCD;

        int _RP = s.nRP;
        _RP *= 0.75;
        int diff_RP = _RP - s.nRP;
        s.nRP = _RP;

        s.nRAS = s.nRAS + diff_RCD + diff_RP;

      /*
	// gagan : 
        SpeedEntry& s = speed_entry;
        int _RCD = s.nRCD;
        _RCD *= 0.75;
        int diff_RCD = _RCD - s.nRCD;
        s.nRCD = _RCD;

        int _RP = s.nRP;
        _RP *= 0.75;
        int diff_RP = _RP - s.nRP;
        s.nRP = _RP;

        s.nRC = s.nRC + diff_RCD + diff_RP;
        s.nRAS = s.nRAS + diff_RCD + diff_RP;
      */
    }

    if(int(speed) == int(DDR4::Speed::DDR4_3200_ideal_v4s) || int(speed) == int(DDR4::Speed::DDR4_3866_ideal_nbr_lbb_sts) ||
       int(speed) == int(DDR4::Speed::DDR4_2933R_ideal_nbr_lbb_sts))
    {
        SpeedEntry& s = speed_entry;
        int _RCD = s.nRCD;
        _RCD -= (_RCD - 1);
        int diff_RCD = _RCD - s.nRCD;
        s.nRCD = _RCD;
        s.nRC = s.nRC + diff_RCD;

        int _RP = s.nRP;
        _RP -= (_RP - 1);
        int diff_RP = _RP - s.nRP;
        s.nRP = _RP;

	s.nRAS = s.nRAS + diff_RCD + diff_RP;
    }

    if(int(speed) == int(DDR4::Speed::DDR4_3200_ideal_v5a))
    {
    }

    


    std::cout << "ramulator channel width = " << channel_width << std::endl;
    init_speed();
    init_prereq();
    init_rowhit(); // SAUGATA: added row hit function
    init_rowopen();
    init_lambda();
    init_timing();
}

DDR4::DDR4(const string& org_str, const string& speed_str) :
    DDR4(org_map[org_str], speed_map[speed_str]) 
{
}

void DDR4::set_channel_number(int channel) {
  org_entry.count[int(Level::Channel)] = channel;
}

void DDR4::set_rank_number(int rank) {
  org_entry.count[int(Level::Rank)] = rank;
}

void DDR4::init_speed()
{
    const static int RRDS_TABLE[2][7] = {
      {4, 4, 4, 4, 4, 4, 4},
      {5, 5, 6, 7, 9, 10, 11}
    };
    const static int RRDL_TABLE[2][7] = {
        {5, 5, 6, 6, 8},
        {6, 6, 7, 8, 11}
    };
    const static int FAW_TABLE[3][7] = {
      {16, 16, 16, 16, 16, 16, 16},
      {20, 22, 23, 26, 34, 38, 41},
      {28, 28, 32, 36, 48, 54, 58}
    };
    const static int RFC_TABLE[int(RefreshMode::MAX)][3][7] = {{   
	{128, 150, 171, 192, 256, 288, 310},
	{208, 243, 278, 312, 416, 468, 503},
	{280, 327, 374, 420, 560, 630, 677}
        },{
	{88, 103, 118, 132,  176, 198, 213},
	{128, 150, 171, 192, 256, 288, 310},
	{208, 243, 278, 312, 416, 468, 503} 
        },{
	{72, 84, 96, 108, 144, 162, 174},
	{88, 103, 118, 132, 176, 198, 213},
	{128, 150, 171, 192, 256, 288, 310}  
        }
    };
    const static int REFI_TABLE[7] = {
      6240, 7280, 8320, 9360, 12480, 14040, 15077
    };
    const static int XS_TABLE[3][7] = {
      {136, 159, 182, 204, 272, 306, 329},
      {216, 252, 288, 324, 432, 486, 522},
      {288, 336, 384, 432, 576, 648, 697}
    };

    int speed = 0, density = 0;
    switch (speed_entry.rate) {
        case 1600: speed = 0; break;
        case 1866: speed = 1; break;
        case 2133: speed = 2; break;
        case 2400: speed = 3; break;
        case 2933: break; // gagan
        case 3200: speed = 4; break;
        case 3600: speed = 5; break;
        case 3866: speed = 6; break;
        case 6400: break;//daz3
        case 800: break;//daz3
        case 12800: break;
        case 25600: break;
        default: assert(false);
    };
    switch (org_entry.size >> 10){
        case 2: density = 0; break;
        case 4: density = 1; break;
        case 8: density = 2; break;
        default: assert(false);
    }
    // daz3
    if(mySpeed != DDR4::Speed::DDR4_3200_base 
            && mySpeed != DDR4::Speed::DDR4_3200_ideal_v1 
            && mySpeed != DDR4::Speed::DDR4_3200_ideal_v2 
            && mySpeed != DDR4::Speed::DDR4_3200_ideal_v3
            && mySpeed != DDR4::Speed::DDR4_3200_ideal_v4
            && mySpeed != DDR4::Speed::DDR4_3200_ideal_v7
            && mySpeed != DDR4::Speed::DDR4_3200_ideal_v4s
            && mySpeed != DDR4::Speed::DDR4_3200_ideal_v5a
            && mySpeed != DDR4::Speed::DDR4_3600_ideal_v4
            && mySpeed != DDR4::Speed::DDR4_3866_ideal_v4
            && mySpeed != DDR4::Speed::DDR4_3200_ideal_nbr_lbb
            && mySpeed != DDR4::Speed::DDR4_3866_ideal_nbr_lbb_ts
            && mySpeed != DDR4::Speed::DDR4_3866_ideal_nbr_lbb_sts
            && mySpeed != DDR4::Speed::DDR4_3200_base_half_w8
            && mySpeed != DDR4::Speed::DDR4_3200_base_half_w16
            && mySpeed != DDR4::Speed::DDR4_3200_base_half_w32
            && mySpeed != DDR4::Speed::DDR4_3200_base_half_w64
            && mySpeed != DDR4::Speed::DDR4_3200_base_full_w8
            && mySpeed != DDR4::Speed::DDR4_3200_base_full_w16
            && mySpeed != DDR4::Speed::DDR4_3200_base_full_w32
            && mySpeed != DDR4::Speed::DDR4_3200_base_full_w64
            && mySpeed != DDR4::Speed::DDR4_3200_base_quarter_w8
            && mySpeed != DDR4::Speed::DDR4_3200_base_quarter_w16
            && mySpeed != DDR4::Speed::DDR4_3200_base_quarter_w32
            && mySpeed != DDR4::Speed::DDR4_3200_base_quarter_w64
       // gagan
            && mySpeed != DDR4::Speed::DDR4_2400R_base
            && mySpeed != DDR4::Speed::DDR4_2400R_ideal_nbr_lbb
            && mySpeed != DDR4::Speed::DDR4_2933R_ideal_nbr_lbb_ts
            && mySpeed != DDR4::Speed::DDR4_2933R_ideal_nbr_lbb_sts)
    {
        speed_entry.nRRDS = RRDS_TABLE[org_entry.dq == 16? 1: 0][speed];
        speed_entry.nRRDL = RRDL_TABLE[org_entry.dq == 16? 1: 0][speed];
        speed_entry.nFAW = FAW_TABLE[org_entry.dq == 4? 0: org_entry.dq == 8? 1: 2][speed];
        speed_entry.nRFC = RFC_TABLE[(int)refresh_mode][density][speed];
        speed_entry.nREFI = (REFI_TABLE[speed] >> int(refresh_mode));
        speed_entry.nXS = XS_TABLE[density][speed];
    }
    // speed_entry.nRRDS = RRDS_TABLE[org_entry.dq == 16? 1: 0][speed];
    // speed_entry.nRRDL = RRDL_TABLE[org_entry.dq == 16? 1: 0][speed];
    // speed_entry.nFAW = FAW_TABLE[org_entry.dq == 4? 0: org_entry.dq == 8? 1: 2][speed];
    // speed_entry.nRFC = RFC_TABLE[(int)refresh_mode][density][speed];
    // speed_entry.nREFI = (REFI_TABLE[speed] >> int(refresh_mode));
    // speed_entry.nXS = XS_TABLE[density][speed];
}


void DDR4::init_prereq()
{
    // RD
    prereq[int(Level::Rank)][int(Command::RD)] = [] (DRAM<DDR4>* node, Command cmd, int id) {
        switch (int(node->state)) {
            case int(State::PowerUp): return Command::MAX;
            case int(State::ActPowerDown): return Command::PDX;
            case int(State::PrePowerDown): return Command::PDX;
            case int(State::SelfRefresh): return Command::SRX;
            default: assert(false);
        }};
    prereq[int(Level::Bank)][int(Command::RD)] = [] (DRAM<DDR4>* node, Command cmd, int id) {
        switch (int(node->state)) {
            case int(State::Closed): return Command::ACT;
            case int(State::Opened):
                if (node->row_state.find(id) != node->row_state.end())
                    return cmd;
                else return Command::PRE;
            default: assert(false);
        }};

    // WR
    prereq[int(Level::Rank)][int(Command::WR)] = prereq[int(Level::Rank)][int(Command::RD)];
    prereq[int(Level::Bank)][int(Command::WR)] = prereq[int(Level::Bank)][int(Command::RD)];

    // REF
    prereq[int(Level::Rank)][int(Command::REF)] = [] (DRAM<DDR4>* node, Command cmd, int id) {
        for (auto bg : node->children)
            for (auto bank: bg->children) {
                if (bank->state == State::Closed)
                    continue;
                return Command::PREA;
            }
        return Command::REF;};

    // PD
    prereq[int(Level::Rank)][int(Command::PDE)] = [] (DRAM<DDR4>* node, Command cmd, int id) {
        switch (int(node->state)) {
            case int(State::PowerUp): return Command::PDE;
            case int(State::ActPowerDown): return Command::PDE;
            case int(State::PrePowerDown): return Command::PDE;
            case int(State::SelfRefresh): return Command::SRX;
            default: assert(false);
        }};

    // SR
    prereq[int(Level::Rank)][int(Command::SRE)] = [] (DRAM<DDR4>* node, Command cmd, int id) {
        switch (int(node->state)) {
            case int(State::PowerUp): return Command::SRE;
            case int(State::ActPowerDown): return Command::PDX;
            case int(State::PrePowerDown): return Command::PDX;
            case int(State::SelfRefresh): return Command::SRE;
            default: assert(false);
        }};
}

// SAUGATA: added row hit check functions to see if the desired location is currently open
void DDR4::init_rowhit()
{
    // RD
    rowhit[int(Level::Bank)][int(Command::RD)] = [] (DRAM<DDR4>* node, Command cmd, int id) {
        switch (int(node->state)) {
            case int(State::Closed): return false;
            case int(State::Opened):
                if (node->row_state.find(id) != node->row_state.end())
                    return true;
                return false;
            default: assert(false);
        }};

    // WR
    rowhit[int(Level::Bank)][int(Command::WR)] = rowhit[int(Level::Bank)][int(Command::RD)];
}

void DDR4::init_rowopen()
{
    // RD
    rowopen[int(Level::Bank)][int(Command::RD)] = [] (DRAM<DDR4>* node, Command cmd, int id) {
        switch (int(node->state)) {
            case int(State::Closed): return false;
            case int(State::Opened): return true;
            default: assert(false);
        }};

    // WR
    rowopen[int(Level::Bank)][int(Command::WR)] = rowopen[int(Level::Bank)][int(Command::RD)];
}

void DDR4::init_lambda()
{
    lambda[int(Level::Bank)][int(Command::ACT)] = [] (DRAM<DDR4>* node, int id) {
        node->state = State::Opened;
        node->row_state[id] = State::Opened;};
    lambda[int(Level::Bank)][int(Command::PRE)] = [] (DRAM<DDR4>* node, int id) {
        node->state = State::Closed;
        node->row_state.clear();};
    lambda[int(Level::Rank)][int(Command::PREA)] = [] (DRAM<DDR4>* node, int id) {
        for (auto bg : node->children)
            for (auto bank : bg->children) {
                bank->state = State::Closed;
                bank->row_state.clear();
            }};
    lambda[int(Level::Rank)][int(Command::REF)] = [] (DRAM<DDR4>* node, int id) {};
    lambda[int(Level::Bank)][int(Command::RD)] = [] (DRAM<DDR4>* node, int id) {};
    lambda[int(Level::Bank)][int(Command::WR)] = [] (DRAM<DDR4>* node, int id) {};
    lambda[int(Level::Bank)][int(Command::RDA)] = [] (DRAM<DDR4>* node, int id) {
        node->state = State::Closed;
        node->row_state.clear();};
    lambda[int(Level::Bank)][int(Command::WRA)] = [] (DRAM<DDR4>* node, int id) {
        node->state = State::Closed;
        node->row_state.clear();};
    lambda[int(Level::Rank)][int(Command::PDE)] = [] (DRAM<DDR4>* node, int id) {
        for (auto bg : node->children)
            for (auto bank : bg->children) {
                if (bank->state == State::Closed)
                    continue;
                node->state = State::ActPowerDown;
                return;
            }
        node->state = State::PrePowerDown;};
    lambda[int(Level::Rank)][int(Command::PDX)] = [] (DRAM<DDR4>* node, int id) {
        node->state = State::PowerUp;};
    lambda[int(Level::Rank)][int(Command::SRE)] = [] (DRAM<DDR4>* node, int id) {
        node->state = State::SelfRefresh;};
    lambda[int(Level::Rank)][int(Command::SRX)] = [] (DRAM<DDR4>* node, int id) {
        node->state = State::PowerUp;};
}


void DDR4::init_timing()
{
    SpeedEntry& s = speed_entry;
    vector<TimingEntry> *t;

    /*** Channel ***/ 
    t = timing[int(Level::Channel)];

    // CAS <-> CAS
    t[int(Command::RD)].push_back({Command::RD, 1, s.nBL});
    t[int(Command::RD)].push_back({Command::RDA, 1, s.nBL});
    t[int(Command::RDA)].push_back({Command::RD, 1, s.nBL});
    t[int(Command::RDA)].push_back({Command::RDA, 1, s.nBL});
    t[int(Command::WR)].push_back({Command::WR, 1, s.nBL});
    t[int(Command::WR)].push_back({Command::WRA, 1, s.nBL});
    t[int(Command::WRA)].push_back({Command::WR, 1, s.nBL});
    t[int(Command::WRA)].push_back({Command::WRA, 1, s.nBL});


    /*** Rank ***/ 
    t = timing[int(Level::Rank)];

    // CAS <-> CAS
    t[int(Command::RD)].push_back({Command::RD, 1, s.nCCDS});
    t[int(Command::RD)].push_back({Command::RDA, 1, s.nCCDS});
    t[int(Command::RDA)].push_back({Command::RD, 1, s.nCCDS});
    t[int(Command::RDA)].push_back({Command::RDA, 1, s.nCCDS});
    t[int(Command::WR)].push_back({Command::WR, 1, s.nCCDS});
    t[int(Command::WR)].push_back({Command::WRA, 1, s.nCCDS});
    t[int(Command::WRA)].push_back({Command::WR, 1, s.nCCDS});
    t[int(Command::WRA)].push_back({Command::WRA, 1, s.nCCDS});
    t[int(Command::RD)].push_back({Command::WR, 1, s.nCL + s.nBL + 2 - s.nCWL});
    t[int(Command::RD)].push_back({Command::WRA, 1, s.nCL + s.nBL + 2 - s.nCWL});
    t[int(Command::RDA)].push_back({Command::WR, 1, s.nCL + s.nBL + 2 - s.nCWL});
    t[int(Command::RDA)].push_back({Command::WRA, 1, s.nCL + s.nBL + 2 - s.nCWL});
    t[int(Command::WR)].push_back({Command::RD, 1, s.nCWL + s.nBL + s.nWTRS});
    t[int(Command::WR)].push_back({Command::RDA, 1, s.nCWL + s.nBL + s.nWTRS});
    t[int(Command::WRA)].push_back({Command::RD, 1, s.nCWL + s.nBL + s.nWTRS});
    t[int(Command::WRA)].push_back({Command::RDA, 1, s.nCWL + s.nBL + s.nWTRS});

    // CAS <-> CAS (between sibling ranks)
    t[int(Command::RD)].push_back({Command::RD, 1, s.nBL + s.nRTRS, true});
    t[int(Command::RD)].push_back({Command::RDA, 1, s.nBL + s.nRTRS, true});
    t[int(Command::RDA)].push_back({Command::RD, 1, s.nBL + s.nRTRS, true});
    t[int(Command::RDA)].push_back({Command::RDA, 1, s.nBL + s.nRTRS, true});
    t[int(Command::RD)].push_back({Command::WR, 1, s.nBL + s.nRTRS, true});
    t[int(Command::RD)].push_back({Command::WRA, 1, s.nBL + s.nRTRS, true});
    t[int(Command::RDA)].push_back({Command::WR, 1, s.nBL + s.nRTRS, true});
    t[int(Command::RDA)].push_back({Command::WRA, 1, s.nBL + s.nRTRS, true});
    t[int(Command::RD)].push_back({Command::WR, 1, s.nCL + s.nBL + s.nRTRS - s.nCWL, true});
    t[int(Command::RD)].push_back({Command::WRA, 1, s.nCL + s.nBL + s.nRTRS - s.nCWL, true});
    t[int(Command::RDA)].push_back({Command::WR, 1, s.nCL + s.nBL + s.nRTRS - s.nCWL, true});
    t[int(Command::RDA)].push_back({Command::WRA, 1, s.nCL + s.nBL + s.nRTRS - s.nCWL, true});
    t[int(Command::WR)].push_back({Command::RD, 1, s.nCWL + s.nBL + s.nRTRS - s.nCL, true});
    t[int(Command::WR)].push_back({Command::RDA, 1, s.nCWL + s.nBL + s.nRTRS - s.nCL, true});
    t[int(Command::WRA)].push_back({Command::RD, 1, s.nCWL + s.nBL + s.nRTRS - s.nCL, true});
    t[int(Command::WRA)].push_back({Command::RDA, 1, s.nCWL + s.nBL + s.nRTRS - s.nCL, true});

    t[int(Command::RD)].push_back({Command::PREA, 1, s.nRTP});
    t[int(Command::WR)].push_back({Command::PREA, 1, s.nCWL + s.nBL + s.nWR});

    // CAS <-> PD
    t[int(Command::RD)].push_back({Command::PDE, 1, s.nCL + s.nBL + 1});
    t[int(Command::RDA)].push_back({Command::PDE, 1, s.nCL + s.nBL + 1});
    t[int(Command::WR)].push_back({Command::PDE, 1, s.nCWL + s.nBL + s.nWR});
    t[int(Command::WRA)].push_back({Command::PDE, 1, s.nCWL + s.nBL + s.nWR + 1}); // +1 for pre
    t[int(Command::PDX)].push_back({Command::RD, 1, s.nXP});
    t[int(Command::PDX)].push_back({Command::RDA, 1, s.nXP});
    t[int(Command::PDX)].push_back({Command::WR, 1, s.nXP});
    t[int(Command::PDX)].push_back({Command::WRA, 1, s.nXP});
    
    // CAS <-> SR: none (all banks have to be precharged)

    // RAS <-> RAS
    t[int(Command::ACT)].push_back({Command::ACT, 1, s.nRRDS});
    t[int(Command::ACT)].push_back({Command::ACT, 4, s.nFAW});
    t[int(Command::ACT)].push_back({Command::PREA, 1, s.nRAS});
    t[int(Command::PREA)].push_back({Command::ACT, 1, s.nRP});

    // RAS <-> REF
    t[int(Command::PRE)].push_back({Command::REF, 1, s.nRP});
    t[int(Command::PREA)].push_back({Command::REF, 1, s.nRP});
    t[int(Command::RDA)].push_back({Command::REF, 1, s.nRTP + s.nRP});
    t[int(Command::WRA)].push_back({Command::REF, 1, s.nCWL + s.nBL + s.nWR + s.nRP});
    t[int(Command::REF)].push_back({Command::ACT, 1, s.nRFC});

    // RAS <-> PD
    t[int(Command::ACT)].push_back({Command::PDE, 1, 1});
    t[int(Command::PDX)].push_back({Command::ACT, 1, s.nXP});
    t[int(Command::PDX)].push_back({Command::PRE, 1, s.nXP});
    t[int(Command::PDX)].push_back({Command::PREA, 1, s.nXP});

    // RAS <-> SR
    t[int(Command::PRE)].push_back({Command::SRE, 1, s.nRP});
    t[int(Command::PREA)].push_back({Command::SRE, 1, s.nRP});
    t[int(Command::SRX)].push_back({Command::ACT, 1, s.nXS});

    // REF <-> REF
    t[int(Command::REF)].push_back({Command::REF, 1, s.nRFC});

    // REF <-> PD
    t[int(Command::REF)].push_back({Command::PDE, 1, 1});
    t[int(Command::PDX)].push_back({Command::REF, 1, s.nXP});

    // REF <-> SR
    t[int(Command::SRX)].push_back({Command::REF, 1, s.nXS});
    
    // PD <-> PD
    t[int(Command::PDE)].push_back({Command::PDX, 1, s.nPD});
    t[int(Command::PDX)].push_back({Command::PDE, 1, s.nXP});

    // PD <-> SR
    t[int(Command::PDX)].push_back({Command::SRE, 1, s.nXP});
    t[int(Command::SRX)].push_back({Command::PDE, 1, s.nXS});
    
    // SR <-> SR
    t[int(Command::SRE)].push_back({Command::SRX, 1, s.nCKESR});
    t[int(Command::SRX)].push_back({Command::SRE, 1, s.nXS});

    /*** Bank Group ***/ 
    t = timing[int(Level::BankGroup)];
    // CAS <-> CAS
    t[int(Command::RD)].push_back({Command::RD, 1, s.nCCDL});
    t[int(Command::RD)].push_back({Command::RDA, 1, s.nCCDL});
    t[int(Command::RDA)].push_back({Command::RD, 1, s.nCCDL});
    t[int(Command::RDA)].push_back({Command::RDA, 1, s.nCCDL});
    t[int(Command::WR)].push_back({Command::WR, 1, s.nCCDL});
    t[int(Command::WR)].push_back({Command::WRA, 1, s.nCCDL});
    t[int(Command::WRA)].push_back({Command::WR, 1, s.nCCDL});
    t[int(Command::WRA)].push_back({Command::WRA, 1, s.nCCDL});
    t[int(Command::WR)].push_back({Command::RD, 1, s.nCWL + s.nBL + s.nWTRL});
    t[int(Command::WR)].push_back({Command::RDA, 1, s.nCWL + s.nBL + s.nWTRL});
    t[int(Command::WRA)].push_back({Command::RD, 1, s.nCWL + s.nBL + s.nWTRL});
    t[int(Command::WRA)].push_back({Command::RDA, 1, s.nCWL + s.nBL + s.nWTRL});

    // RAS <-> RAS
    t[int(Command::ACT)].push_back({Command::ACT, 1, s.nRRDL});

    /*** Bank ***/ 
    t = timing[int(Level::Bank)];

    // CAS <-> RAS
    t[int(Command::ACT)].push_back({Command::RD, 1, s.nRCD});
    t[int(Command::ACT)].push_back({Command::RDA, 1, s.nRCD});
    t[int(Command::ACT)].push_back({Command::WR, 1, s.nRCD});
    t[int(Command::ACT)].push_back({Command::WRA, 1, s.nRCD});

    t[int(Command::RD)].push_back({Command::PRE, 1, s.nRTP});
    t[int(Command::WR)].push_back({Command::PRE, 1, s.nCWL + s.nBL + s.nWR});

    t[int(Command::RDA)].push_back({Command::ACT, 1, s.nRTP + s.nRP});
    t[int(Command::WRA)].push_back({Command::ACT, 1, s.nCWL + s.nBL + s.nWR + s.nRP});

    // RAS <-> RAS
    t[int(Command::ACT)].push_back({Command::ACT, 1, s.nRC});
    t[int(Command::ACT)].push_back({Command::PRE, 1, s.nRAS});
    t[int(Command::PRE)].push_back({Command::ACT, 1, s.nRP});

    // daz3
    std::cout << "rate " << s.rate << std::endl; 
    std::cout << "freq " << s.freq << std::endl; 
    std::cout << "tCK " << s.tCK << std::endl; 
    std::cout << "nBL " << s.nBL << std::endl; 
    std::cout << "nCCDS " << s.nCCDS << std::endl; 
    std::cout << "nCCDL " << s.nCCDL << std::endl; 
    std::cout << "nRTRS " << s.nRTRS << std::endl; 
    std::cout << "nCL " << s.nCL << std::endl; 
    std::cout << "nRCD " << s.nRCD << std::endl; 
    std::cout << "nRP " << s.nRP << std::endl; 
    std::cout << "nCWL " << s.nCWL << std::endl; 
    std::cout << "nRAS " << s.nRAS << std::endl; 
    std::cout << "nRC " << s.nRC << std::endl; 
    std::cout << "nRTP " << s.nRTP << std::endl; 
    std::cout << "nWTRS " << s.nWTRS << std::endl; 
    std::cout << "nWTRL " << s.nWTRL << std::endl; 
    std::cout << "nWR " << s.nWR << std::endl; 
    std::cout << "nRRDS " << s.nRRDS << std::endl; 
    std::cout << "nRRDL " << s.nRRDL << std::endl; 
    std::cout << "nFAW " << s.nFAW << std::endl; 
    std::cout << "nRFC " << s.nRFC << std::endl; 
    std::cout << "nREFI " << s.nREFI << std::endl; 
    std::cout << "nPD " << s.nPD << std::endl; 
    std::cout << "nXP " << s.nXP << std::endl; 
    std::cout << "nXPDLL " << s.nXPDLL << std::endl; 
    std::cout << "nCKESR " << s.nCKESR << std::endl; 
    std::cout << "nXS " << s.nXS << std::endl; 
    std::cout << "nXSDLL " << s.nXSDLL << std::endl; 
    std::cout << "nRCD " << s.nRCD << std::endl; 
    std::cout << "nRC " << s.nRC << std::endl; 
    std::cout << "nRP " << s.nRP << std::endl; 
    std::cout << "channel_id " << org_entry.count[int(Level::Channel)] << std::endl; 
    std::cout << "ranks " << org_entry.count[int(Level::Rank)] << std::endl; 
}
