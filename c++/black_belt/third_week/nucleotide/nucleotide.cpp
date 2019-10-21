#include <cstddef>
#include <cstdint>

struct Nucleotide {
    char Symbol;
    size_t Position;
    int ChromosomeNum;
    int GeneNum;
    bool IsMarked;
    char ServiceInfo;
};


struct CompactNucleotide {
    uint64_t Symbol:2;
    uint64_t Position:32;
    uint64_t ChromosomeNum:6;
    uint64_t GeneNum:15;
    uint64_t IsMarked:1;
    uint64_t ServiceInfo:8;

    static uint64_t fromSymbol(char symbol) {
        if (symbol == 'A') {
            return 0;
        } else if (symbol == 'C') {
            return 1;
        } else if (symbol == 'G') {
            return 2;
        } else {
            return 3;
        }
    }

    static char toSymbol(uint64_t symbol) {
        if (symbol == 0) {
            return 'A';
        } else if (symbol == 1) {
            return 'C';
        } else if (symbol == 2) {
            return 'G';
        } else {
            return 'T';
        }
    }
};


bool operator == (const Nucleotide& lhs, const Nucleotide& rhs) {
    return (lhs.Symbol == rhs.Symbol)
           && (lhs.Position == rhs.Position)
           && (lhs.ChromosomeNum == rhs.ChromosomeNum)
           && (lhs.GeneNum == rhs.GeneNum)
           && (lhs.IsMarked == rhs.IsMarked)
           && (lhs.ServiceInfo == rhs.ServiceInfo);
}


CompactNucleotide Compress(const Nucleotide& n) {
    return CompactNucleotide{
        .Symbol = CompactNucleotide::fromSymbol(n.Symbol),
        .Position = static_cast<uint64_t>(n.Position),
        .ChromosomeNum = static_cast<uint64_t>(n.ChromosomeNum),
        .GeneNum = static_cast<uint64_t>(n.GeneNum),
        .IsMarked = static_cast<uint64_t>(n.IsMarked),
        .ServiceInfo = static_cast<uint64_t>(n.ServiceInfo),
    };
};


Nucleotide Decompress(const CompactNucleotide& cn) {
    return Nucleotide{
            .Symbol = CompactNucleotide::toSymbol(cn.Symbol),
            .Position = cn.Position,
            .ChromosomeNum = static_cast<int>(cn.ChromosomeNum),
            .GeneNum = static_cast<int>(cn.GeneNum),
            .IsMarked = static_cast<bool>(cn.IsMarked),
            .ServiceInfo = static_cast<char>(cn.ServiceInfo),
    };
}
