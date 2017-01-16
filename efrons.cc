/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <set>
#include <unordered_set>
#include <vector>

namespace {

// platonic solid: {4, 6, 8, 12, 20}
const int kNumberOfFaces = 6;

const int kNumberOfLabels = 7;

const int kPrime[] = {
    2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71
};

static_assert(kNumberOfLabels <= static_cast<int>(sizeof(kPrime)/sizeof(kPrime[0])),
              "kNumberOfLabels must be small enough");

void AddFace(std::set<std::int64_t> *s)
{
    std::set<std::int64_t> tmp;
    for (auto n : *s)
        for (int i=0;i<kNumberOfLabels;i++)
            tmp.insert(n * kPrime[i]);
    s->swap(tmp);
}

void GenerateDice(std::set<std::int64_t> *s)
{
    s->insert(1);
    for (int i=0;i<kNumberOfFaces;i++)
        AddFace(s);
}

struct Die {
    int v[kNumberOfFaces];
};

Die DecodeDie(std::int64_t n)
{
    static_assert(sizeof(std::int64_t) <= sizeof(long long),
                  "long long must be large enough");

    Die d;
    std::memset(d.v, 0, sizeof(d.v));

    int k = 0;
    std::lldiv_t x;
    for (int i=0;i<kNumberOfLabels;i++) {
    re:
        x = std::div(static_cast<long long>(n), static_cast<long long>(kPrime[i]));
        if (x.rem == 0) {
            d.v[k++] = i;
            n = x.quot;
            if (n == 1)
                break;
            goto re;
        }
    }
    assert(k == kNumberOfFaces);
    assert(n == 1);
    return d;
}

void Odds(const Die &d1, const Die &d2, int *w1, int *w2)
{
    int n1 = 0, n2 = 0;
    for (int i=0;i<kNumberOfFaces;i++) {
        for (int j=0;j<kNumberOfFaces;j++) {
            if (d1.v[i] < d2.v[j]) {
                ++n2;
            } else if (d1.v[i] > d2.v[j]) {
                ++n1;
            }
        }
    }
    *w1 = n1;
    *w2 = n2;
}

std::int64_t Quadruple(std::int64_t a,
                       std::int64_t b,
                       std::int64_t c,
                       std::int64_t d)
{
    assert(0 <= a && a < 0x10000);
    assert(0 <= b && b < 0x10000);
    assert(0 <= c && c < 0x10000);
    assert(0 <= d && d < 0x10000);
    return (a<<48)|(d<<32)|(c<<16)|(d);
}

void PrintDice(char c, const Die &d)
{
    std::printf("%c:", c);
    for (int j=0;j<kNumberOfFaces;j++)
        std::printf(" %d", d.v[j]);
    std::printf("\n");
}

void PrintFound(const Die &a,
                const Die &b,
                const Die &c,
                const Die &d,
                const int *w1,
                const int *w2)
{
    PrintDice('A', a);
    PrintDice('B', b);
    PrintDice('C', c);
    PrintDice('D', d);
    std::printf("odds: %d:%d, %d:%d, %d:%d, %d:%d",
                w1[0], w2[0], w1[1], w2[1], w1[2], w2[2], w1[3], w2[3]);
    if ( w1[0] + w2[0] == kNumberOfFaces * kNumberOfFaces &&
         w1[1] + w2[1] == kNumberOfFaces * kNumberOfFaces &&
         w1[2] + w2[2] == kNumberOfFaces * kNumberOfFaces &&
         w1[3] + w2[3] == kNumberOfFaces * kNumberOfFaces) {
        std::printf(" (no ties)");
    }
    std::printf("\n");
}

}

int main()
{
    std::vector<Die> dice;
    {
        std::set<std::int64_t> s;
        GenerateDice(&s);
        for (auto n : s)
            dice.emplace_back(DecodeDie(n));
    }
    int n = static_cast<int>(dice.size());
    std::fprintf(stderr, "# of dice: %d\n", n);
    int i = 0;
    int w1[4], w2[4];
    std::unordered_set<std::int64_t> found;
    for (int a=0;a<n;a++) {
        for (int b=0;b<n;b++) {
            if (a == b)
                continue;
            Odds(dice[a], dice[b], &w1[0], &w2[0]);
            if (w1[0] == 0 || w2[0] == 0)
                continue;
            if (w1[0] <= w2[0])
                continue;
            for (int c=0;c<n;c++) {
                if (a == c || b == c)
                    continue;
                Odds(dice[b], dice[c], &w1[1], &w2[1]);
                if (w1[1] == 0 || w2[1] == 0)
                    continue;
                if (w1[1] <= w2[1])
                    continue;
                if (w1[0] * w2[1] != w1[1] * w2[0])
                    continue;
                for (int d=0;d<n;d++) {
                    if (a == d || b == d || c == d)
                        continue;
                    Odds(dice[c], dice[d], &w1[2], &w2[2]);
                    if (w1[2] == 0 || w2[2] == 0)
                        continue;
                    if (w1[2] <= w2[2])
                        continue;
                    if (w1[1] * w2[2] != w1[2] * w2[1])
                        continue;
                    Odds(dice[d], dice[a], &w1[3], &w2[3]);
                    if (w1[3] == 0 || w2[3] == 0)
                        continue;
                    if (w1[3] <= w2[3])
                        continue;
                    if (w1[2] * w2[3] != w1[3] * w2[2])
                        continue;
                    if (w1[3] * w2[0] != w1[0] * w2[3])
                        continue;
                    auto q = Quadruple(a, b, c, d);
                    if (!found.insert(q).second)
                        continue;
                    found.insert(Quadruple(b, c, d, a));
                    found.insert(Quadruple(c, d, a, b));
                    found.insert(Quadruple(d, a, b, c));
                    std::printf("#%d\n", i++);
                    PrintFound(dice[a], dice[b], dice[c], dice[d], w1, w2);
                }
            }
        }
    }
    return EXIT_SUCCESS;
}
