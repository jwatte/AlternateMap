// AlternateMap.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "imap.h"
#include <string>
#include <assert.h>

#pragma warning(disable: 4996)

void test_a()
{
    imap<std::string, std::string> test1;

    auto p(test1.find("key1"));
    assert(p == test1.end());

    for (int i = 0; i != 1000; ++i) {
        char key[100];
        char value[100];
        sprintf(key, "key%d", i);
        sprintf(value, "value%d", i);
        test1[key] = value;
        auto iter(test1.find(key));
        assert(iter != test1.end());
        assert(iter->first == key);
        assert((*iter).second == value);
        assert(test1.size() == i + 1);
    }

    auto q(test1.find("key22"));
    assert((*q).second == "value22");

    bool fnd[1000] = { false };
    auto const &ct1(test1);
    for (auto ptr(ct1.begin()), end(ct1.end()); ptr != end; ++ptr) {
        int i = -1;
        sscanf((*ptr).first.c_str(), "key%d", &i);
        assert(i != -1);
        assert(i >= 0 && i < 1000);
        assert(!fnd[i]);
        fnd[i] = true;
    }

    for (int i = 0; i != 1000; ++i) {
        char key[100];
        sprintf(key, "key%d", i);
        auto iter(test1.find(key));
        assert(iter != test1.end());
        test1.erase(iter);
        assert(test1.size() == 999 - i);
        assert(test1.find(key) == test1.end());
    }

    auto zz(test1.find("key1"));
    assert(zz == test1.end());

    test1.erase(test1.end());
    bool thrown = false;
    try {
        test1.erase(q);
    }
    catch (std::runtime_error const &) {
        thrown = true;
    }
    assert(thrown);
}

void test_b()
{
    imap<size_t, std::string> maps[4];
    int adds = 0;
    int addfound = 0;
    int removes = 0;
    int finds = 0;
    int found = 0;
    for (int i = 0, n = 3000000; i != n; ++i)
    {
        if (i % 100000 == 0) {
            fprintf(stderr, "%d items of %d\n", i, n);
        }
        auto &m(maps[(rand() & 0x30)>>4]);
        size_t k = rand() & 0x7fff;
        char val[100];
        sprintf(val, "val%d", k);
        switch ((rand() & 0x70) >> 4)
        {
            case 0:
            case 1:
            case 2:
            case 3:
                removes += 1;
                m.erase(m.find(k));
                break;
            case 4:
            case 5:
            case 6:
                adds += 1;
                if (m.insert(imap<size_t, std::string>::key_value_t(k, val)).second) {
                    addfound += 1;
                }
                break;
            case 7:
                finds += 1;
                if (m.find(k) != m.end()) {
                    found += 1;
                }
                break;
            default:
                assert(!"this case should not happen");
                break;
        }
    }
    fprintf(stderr, "adds %d addfound %d removes %d finds %d found %d\n", adds, addfound, removes, finds, found);
}

int _tmain(int argc, _TCHAR* argv[])
{
    test_a();
    test_b();
    return 0;
}

