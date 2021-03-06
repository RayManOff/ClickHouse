#pragma once

#include <map>
#include <list>
#include <string>
#include <set>

#include <DataTypes/IDataType.h>
#include <Core/Names.h>


namespace DB
{

struct NameAndTypePair
{
    String name;
    DataTypePtr type;

    NameAndTypePair() {}
    NameAndTypePair(const String & name_, const DataTypePtr & type_) : name(name_), type(type_) {}

    bool operator<(const NameAndTypePair & rhs) const
    {
        return std::forward_as_tuple(name, type->getName()) < std::forward_as_tuple(rhs.name, rhs.type->getName());
    }

    bool operator==(const NameAndTypePair & rhs) const
    {
        return name == rhs.name && type->getName() == rhs.type->getName();
    }
};

using NamesAndTypes = std::vector<NameAndTypePair>;

class NamesAndTypesList : public std::list<NameAndTypePair>
{
public:
    using std::list<NameAndTypePair>::list;

    void readText(ReadBuffer & buf);
    void writeText(WriteBuffer & buf) const;

    String toString() const;
    static NamesAndTypesList parse(const String & s);

    /// All `rhs` elements must be different.
    bool isSubsetOf(const NamesAndTypesList & rhs) const;

    /// Hamming distance between sets
    ///  (in other words, the added and deleted columns are counted once, the columns that changed the type - twice).
    size_t sizeOfDifference(const NamesAndTypesList & rhs) const;

    Names getNames() const;

    /// Leave only the columns whose names are in the `names`. In `names` there can be superfluous columns.
    NamesAndTypesList filter(const NameSet & names) const;

    /// Leave only the columns whose names are in the `names`. In `names` there can be superfluous columns.
    NamesAndTypesList filter(const Names & names) const;

    /// Unlike `filter`, returns columns in the order in which they go in `names`.
    NamesAndTypesList addTypes(const Names & names) const;
};

using NamesAndTypesListPtr = std::shared_ptr<NamesAndTypesList>;

}
