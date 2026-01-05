#pragma once
#include <string>
#include <vector>
#include "resource/parse/TokenData.h"

namespace data::module::rawmonsters
{

// -------------------------------------------------
// Flat Line (wie bisher): 1 Zeile -> fields
// -------------------------------------------------
struct RawMonsterEntry
{
    int line = 0;
    std::vector<std::string> fields;
};

// -------------------------------------------------
// Aggro Config (propAggro.txt) - Blockstruktur
// -------------------------------------------------
struct RawAggroEntry
{
    int line = 0;
    std::string key;    // z.B. BUFF_AGGRO_RATE oder SI_LOD_SUP_ANGER
    std::string value;  // z.B. "3"
};

struct RawAggroBlock
{
    std::string name;           // z.B. NORMAL_AGGRO, SINGLE_AGGRO
    int headerLine = 0;

    std::vector<RawAggroEntry> entries;
};

// -------------------------------------------------
// Stream (Datei)
// -------------------------------------------------
struct RawMonsterStream
{
    std::string moduleId;
    std::string sourceFile;
    std::string domain;

    std::vector<data::Token> rawTokens;        // Snapshot (verlustfrei)
    std::vector<RawMonsterEntry> monsters;     // propMover*, propMoverEx: 1 Zeile = 1 Monster/Entry

    // propAggro.txt: Blockdaten (zusätzlich zum Snapshot)
    std::vector<RawAggroBlock> aggroBlocks;
};

// -------------------------------------------------
// Root
// -------------------------------------------------
struct rawMonsterData
{
    bool valid = false;
    std::vector<RawMonsterStream> streams;
};

} // namespace data::module::rawmonsters
