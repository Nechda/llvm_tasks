// Анализ трассы использования IR инструкций (User <- Operand):
// частоты opcodes User'ов, частоты пар (User <- Operand) и n-грамм
// (окна 1..5) по последовательности инструкций и пар.
//
// Входы:
//   trace.bin     -- "TRC1", uint64 num_pairs, пары uint32 (user, operand)
//   trace_map.txt -- id \t opcode \t function \t instruction
//
// Использование: analyze <trace> <map> <out> [--top N]

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using std::string;
using std::unordered_map;
using std::vector;

static constexpr int MAXW = 5;

// map: id -> имя opcode (0 = "<non-instruction>")
static vector<string> Name;
static vector<uint32_t> Id2Idx;

static uint32_t readMap(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "analyze: cannot open map %s\n", path);
        exit(1);
    }
    unordered_map<string, uint32_t> nameIdx;
    Name.push_back("<non-instruction>");
    Name.push_back("<?>");
    nameIdx["<non-instruction>"] = 0;
    const uint32_t unknownIdx = 1;
    Id2Idx.push_back(0); // id 0 -> "<non-instruction>"

    char line[4096];
    if (!fgets(line, sizeof line, f)) {
        fclose(f);
        return unknownIdx;
    }
    while (fgets(line, sizeof line, f)) {
        char *t1 = strchr(line, '\t');
        if (!t1)
            continue;
        *t1 = '\0';
        int id = atoi(line);
        char *t2 = strchr(t1 + 1, '\t');
        string op(t1 + 1, t2 ? (size_t)(t2 - t1 - 1) : strlen(t1 + 1));
        while (!op.empty() && (op.back() == '\n' || op.back() == '\r'))
            op.pop_back();
        uint32_t idx;
        auto it = nameIdx.find(op);
        if (it != nameIdx.end()) {
            idx = it->second;
        } else {
            idx = (uint32_t)Name.size();
            Name.push_back(op);
            nameIdx.emplace(op, idx);
        }
        if (id > 0) {
            if ((size_t)id >= Id2Idx.size())
                Id2Idx.resize(id + 1, unknownIdx);
            Id2Idx[id] = idx;
        }
    }
    fclose(f);
    return unknownIdx;
}

// Ключ окна: до 10 значений по 12 бит (окна пар 2*L), лежит в lo/hi.
struct Key {
    uint64_t lo = 0, hi = 0;
    bool operator==(const Key &o) const { return lo == o.lo && hi == o.hi; }
    bool operator<(const Key &o) const {
        return lo != o.lo ? lo < o.lo : hi < o.hi;
    }
};
struct KeyHash {
    static uint64_t mix(uint64_t x) {
        x += 0x9e3779b97f4a7c15ULL;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }
    uint64_t operator()(const Key &k) const { return mix(k.lo ^ mix(k.hi)); }
};

using Counter = unordered_map<Key, uint64_t, KeyHash>;

static inline Key pack(const uint32_t *w, int n) {
    Key k;
    for (int i = 0; i < n; ++i) {
        if (i < 5)
            k.lo |= (uint64_t)w[i] << (12 * i);
        else
            k.hi |= (uint64_t)w[i] << (12 * (i - 5));
    }
    return k;
}

// печать: count<TAB>доля<TAB>ключ (ключ расшифровывается decode-ом)
static void dump(FILE *out, const char *title, Counter &c, uint64_t total,
                 int topn, void (*decode)(FILE *, Key, int /*nvals*/),
                 int nvals = 1) {
    vector<std::pair<Key, uint64_t>> v(c.begin(), c.end());
    auto cmp = [](auto &a, auto &b) {
        return a.second != b.second ? a.second > b.second : a.first < b.first;
    };
    if (v.size() > (size_t)topn) {
        std::partial_sort(v.begin(), v.begin() + topn, v.end(), cmp);
        v.resize(topn);
    } else {
        std::sort(v.begin(), v.end(), cmp);
    }
    fprintf(out, "\n# %s (total %llu, distinct %zu)\n", title,
            (unsigned long long)total, c.size());
    for (auto &e : v) {
        fprintf(out, "%llu\t%.4f\t", (unsigned long long)e.second,
                total ? 100.0 * e.second / total : 0.0);
        decode(out, e.first, nvals);
        fputc('\n', out);
    }
}

static const char *name(uint32_t idx) {
    return idx < Name.size() ? Name[idx].c_str() : "<?>?";
}

static void decName(FILE *out, Key k, int) {
    fputs(name((uint32_t)(k.lo & 0xffffffffu)), out);
}
static void decPair(FILE *out, Key k, int) {
    fprintf(out, "%s <- %s", name((uint32_t)(k.lo >> 12)),
            name((uint32_t)(k.lo & 0xfffu)));
}
static void decWin(FILE *out, Key k, int n) { // n значений через '/'
    for (int i = 0; i < n; ++i) {
        uint32_t idx = i < 5 ? (uint32_t)((k.lo >> (12 * i)) & 0xfffu)
                             : (uint32_t)((k.hi >> (12 * (i - 5))) & 0xfffu);
        if (i)
            fputc('/', out);
        fputs(name(idx), out);
    }
}
static void decPairWin(FILE *out, Key k, int n) { // n пар через ';'
    for (int i = 0; i < n; ++i) {
        uint32_t u = i * 2 < 5
                         ? (uint32_t)((k.lo >> (12 * i * 2)) & 0xfffu)
                         : (uint32_t)((k.hi >> (12 * (i * 2 - 5))) & 0xfffu);
        uint32_t o =
            i * 2 + 1 < 5
                ? (uint32_t)((k.lo >> (12 * (i * 2 + 1))) & 0xfffu)
                : (uint32_t)((k.hi >> (12 * (i * 2 + 1 - 5))) & 0xfffu);
        if (i)
            fputc(';', out);
        fprintf(out, "%s<-%s", name(u), name(o));
    }
}

int main(int argc, char **argv) {
    const char *tracePath = nullptr, *mapPath = nullptr, *outPath = nullptr;
    int top = 30;
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--top") && i + 1 < argc)
            top = atoi(argv[++i]);
        else if (!tracePath)
            tracePath = argv[i];
        else if (!mapPath)
            mapPath = argv[i];
        else
            outPath = argv[i];
    }
    if (!tracePath || !mapPath || !outPath) {
        fprintf(stderr, "usage: analyze <trace> <map> <out> [--top N]\n");
        return 1;
    }
    uint32_t unknownIdx = readMap(mapPath);

    FILE *f = fopen(tracePath, "rb");
    if (!f) {
        fprintf(stderr, "analyze: cannot open trace %s\n", tracePath);
        return 1;
    }
    char magic[4];
    uint64_t declared = 0;
    if (fread(magic, 1, 4, f) != 4 || memcmp(magic, "TRC1", 4) ||
        fread(&declared, 8, 1, f) != 1) {
        fprintf(stderr, "analyze: bad trace header\n");
        return 1;
    }

    Counter pairFreq, uopFreq, oopFreq;
    Counter ugram[MAXW + 1], pgram[MAXW + 1];

    typedef std::pair<uint32_t, uint32_t> UP;
    const size_t CH = 1u << 20;
    vector<UP> buf(CH);
    uint32_t uwin[MAXW] = {0}, pwin[2 * MAXW] = {0};
    uint64_t n = 0, nw = 0, prevUid = ~0ull;

    auto put = [](Counter &c, Key k) {
        auto it = c.find(k);
        if (it == c.end())
            c.emplace(k, 1);
        else
            ++it->second;
    };

    for (;;) {
        size_t got = fread(buf.data(), sizeof(UP), CH, f);
        if (!got)
            break;
        for (size_t i = 0; i < got; ++i) {
            uint32_t u = buf[i].first, o = buf[i].second;
            uint32_t ui = (size_t)u < Id2Idx.size() ? Id2Idx[u] : unknownIdx;
            uint32_t oi = (size_t)o < Id2Idx.size() ? Id2Idx[o] : unknownIdx;
            ++n;

            put(pairFreq, Key{((uint64_t)ui << 12) | oi, 0});
            put(oopFreq, Key{oi, 0});

            memmove(pwin, pwin + 2, sizeof(uint32_t) * 2 * (MAXW - 1));
            pwin[2 * MAXW - 2] = ui;
            pwin[2 * MAXW - 1] = oi;
            for (int L = 1; L <= MAXW; ++L)
                put(pgram[L], pack(&pwin[2 * (MAXW - L)], 2 * L));

            if (u != prevUid) {
                prevUid = u;
                ++nw;
                put(uopFreq, Key{ui, 0});
                memmove(uwin, uwin + 1, sizeof(uint32_t) * (MAXW - 1));
                uwin[MAXW - 1] = ui;
                for (int L = 1; L <= MAXW; ++L)
                    put(ugram[L], pack(&uwin[MAXW - L], L));
            }
        }
    }
    fclose(f);
    if (declared && declared != n)
        fprintf(stderr, "analyze: warning: header %llu, file %llu pairs\n",
                (unsigned long long)declared, (unsigned long long)n);

    FILE *out = fopen(outPath, "w");
    if (!out) {
        fprintf(stderr, "analyze: cannot open out %s\n", outPath);
        return 1;
    }
    fprintf(out, "trace pairs\t%llu\n", (unsigned long long)n);
    fprintf(out, "executed user instructions (collapsed)\t%llu\n",
            (unsigned long long)nw);
    fprintf(out, "distinct (user,operand) pairs\t%zu\n", pairFreq.size());

    dump(out, "user opcode frequency", uopFreq, nw, top, decName);
    dump(out, "operand class frequency", oopFreq, n, top, decName);
    dump(out, "(User <- Operand) pair frequency", pairFreq, n, top, decPair);
    for (int L = 1; L <= MAXW; ++L) {
        char t[64];
        snprintf(t, sizeof t, "user opcode n-grams window=%d", L);
        dump(out, t, ugram[L], nw - L + 1, top, decWin, L);
    }
    for (int L = 1; L <= MAXW; ++L) {
        char t[64];
        snprintf(t, sizeof t, "pair n-grams window=%d", L);
        dump(out, t, pgram[L], n - L + 1, top, decPairWin, L);
    }
    fclose(out);
    printf("stats written to %s\n", outPath);
    return 0;
}
