#pragma once
#include <string>
#include <vector>
#include "resource/parse/TokenData.h"

namespace data::module::rawitems
{
// -------------------------------------------------
// Generische Item-Zeile (1 Zeile = 1 Item)
// -------------------------------------------------
struct RawItem
{
    int line = 0;
    std::vector<std::string> fields;
};

// -------------------------------------------------
// Stream = eine Datei
// -------------------------------------------------
struct RawItemStream
{
    std::string moduleId;
    std::string sourceFile;
    std::string domain;

    std::vector<data::Token> rawTokens; // Snapshot
    std::vector<RawItem>     items;
};

// -------------------------------------------------
// Piercing
// -------------------------------------------------
struct RawModifier
{
    std::string type;   // "Piercing"
    std::string target;
    std::string key;
    std::string value;
};

// -------------------------------------------------
// SetItem
// -------------------------------------------------
struct RawSetElement
{
    std::string itemId;
    std::string slot;
};

struct RawSetEffect
{
    std::string stat;
    std::string value;
    std::string required;
};

struct RawSetItem
{
    std::string setId;
    std::string nameToken;
    std::vector<RawSetElement> elements;
    std::vector<RawSetEffect>  effects;
};

// -------------------------------------------------
// RandomOptItem
// -------------------------------------------------
struct RawRandomOptEffect
{
    std::string stat;
    std::string value;
};

struct RawRandomOptItem
{
    std::string id;
    std::string nameToken;
    std::string paramA;
    std::string paramB;
    std::vector<RawRandomOptEffect> effects;
};

// -------------------------------------------------
// Item Type / Slot Hierarchie (TypeNodes)
// 1 { parent(1); title("IDS..."); type1(TYPE1_...); type2(TYPE2_...); }
// -------------------------------------------------
struct RawItemTypeNode
{
    int id = 0;
    int parentId = -1;

    std::string titleId; // IDS_TEXTCLIENT_...
    std::string type1;   // TYPE1_...
    std::string type2;   // TYPE2_...
};

// -------------------------------------------------
// ROOT
// -------------------------------------------------
struct rawItemData
{
    bool valid = false;

    std::vector<RawItemStream> streams;

    std::vector<RawModifier>      modifiers;
    std::vector<RawSetItem>       setItems;
    std::vector<RawRandomOptItem> randomOptItems;
    std::vector<RawItemTypeNode>  itemTypes;
};

} // namespace data::module::rawitems
