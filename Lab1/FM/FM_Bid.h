#ifndef FM_BID_H
#define FM_BID_H

#include <functional>


struct FM_Bid {
    FM_Bid(){}
    FM_Bid(int f, int n): fd(f), num(n) {}
    int fd;
    int num;

    bool operator==(const FM_Bid& id) const {
        return fd == id.fd && num == id.num;
    }
};


template <typename T>
inline void hash_combine(std::size_t &seed, const T &val) {
    seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
// auxiliary generic functions to create a hash value using a seed
template <typename T> inline void hash_val(std::size_t &seed, const T &val) {
    hash_combine(seed, val);
}
template <typename T, typename... Types>
inline void hash_val(std::size_t &seed, const T &val, const Types &... args) {
    hash_combine(seed, val);
    hash_val(seed, args...);
}

template <typename... Types>
inline std::size_t hash_val(const Types &... args) {
    std::size_t seed = 0;
    hash_val(seed, args...);
    return seed;
}

struct Bid_Hash {
    std::size_t operator()(const FM_Bid &p) const {
        return hash_val(p.fd, p.num);
    }
};
#endif