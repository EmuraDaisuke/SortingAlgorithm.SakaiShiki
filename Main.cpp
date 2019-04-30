


#define NDEBUG

#include <cassert>
#include <random>
#include <vector>
#include <array>
#include <algorithm>
#include <iostream>
#include <memory.h>
#include "./Lapse.h"

#include "./SakaiShiki.h"



using sort_t = float;

int precision(int v){ return 1; }
float precision(float v){ return (v/0x00ffffff); }
double precision(double v){ return (v/0x000fffffffffffff); }



enum eSrc {
    Rand,
    Equal,
    Asc,
    Dsc,
    Saw,
    AscSaw,
    DscSaw,
    
    Num,
};

static const char* apSrc[eSrc::Num]={
    "Rand",
    "Equal",
    "Asc",
    "Dsc",
    "Saw",
    "AscSaw",
    "DscSaw",
};



#define ORDER   0
#define FAT     0

struct Test
{
    sort_t m;
    #if ORDER//[
    std::size_t o;
    #endif//]
    #if FAT//[
    std::size_t f[FAT];
    #endif
};

bool operator <(const Test& s, const Test& t)
{
    return s.m < t.m;
}

bool operator ==(const Test& s, const Test& t)
{
    #if ORDER//[
    return (s.m == t.m && s.o == t.o);
    #else//][
    return s.m == t.m;
    #endif//]
}



void dump(std::vector<Test>& a)
{
    for (auto& v : a) std::cout << std::fixed << std::setprecision(8) << v.m << std::endl;
    std::cout << std::endl;
}

void increment(std::vector<Test>& a)
{
    sort_t m = 1;
    for (auto& v : a){ v.m = m; m += precision(m); }
}

void decrement(std::vector<Test>& a)
{
    sort_t m = 1;
    for (auto& v : a){ v.m = -m; m += precision(m); }
}

void interleave(std::vector<Test>& a)
{
    auto n = a.size() & ~1;
    for (auto o = n-n; o < n; o += 2) std::swap(a[o], a[o+1]);
}

void init(eSrc Src, std::vector<Test>& a, std::mt19937& rRand, std::uniform_int_distribution<>& rRange)
{
    {   // 
        std::size_t o = 0;
        for (auto& v : a){
            #if ORDER//[
            v.o = o++;
            #endif//]
            #if FAT//[
            for (auto& f : v.f) f = 0;
            #endif//]
        }
    }
    
    {   // 
        switch (Src){
            default:
            case eSrc::Rand:{
                for (auto& v : a) v.m = rRange(rRand);
                break;
            }
            case eSrc::Equal:{
                sort_t m = 1;
                for (auto& v : a) v.m = m;
                break;
            }
            case eSrc::Asc:{
                increment(a);
                break;
            }
            case eSrc::Dsc:{
                decrement(a);
                break;
            }
            case eSrc::Saw:{
                bool b = false;
                for (auto& v : a){ v.m = (b)? 0:1; b = !b; }
                break;
            }
            case eSrc::AscSaw:{
                increment(a);
                interleave(a);
                break;
            }
            case eSrc::DscSaw:{
                decrement(a);
                interleave(a);
                break;
            }
        }
    }
}

void time(const char* p, std::vector<double>& a, int nLoop)
{
    std::cout << p;
    for (auto& v : a) std::cout << std::fixed << std::setprecision(8) << " " << (v / nLoop);
    std::cout << std::endl;
}



void test(eSrc Src, int nTest, int nLoop)
{
    std::random_device Seed;
    std::mt19937 Rand(Seed());
    std::uniform_int_distribution<> Range(0, std::numeric_limits<int>::max());
    auto a = std::vector<Test>(nTest);
    
    #if defined(NDEBUG)//[
    int nLap = 10;
    int nTop = 100;
    #else//][
    int nTop = nTest / 10;
    #endif//]
    nTop = (nTop)? nTop:1;
    
    std::cout << "\n--- " << apSrc[Src] << " " << nTest << std::endl;
    
    #if defined(NDEBUG)//[
    {   // 
        std::vector<double> t0(nLap);
        std::vector<double> t1(nLap);
        std::vector<double> t2(nLap);
        std::vector<double> t3(nLap);
        
        for (auto n = nLoop; n; --n){
            init(Src, a, Rand, Range);
            
            #if 1//[
            {   // 
                auto s = a;
                auto l = Lapse::Now();
                std::sort(s.begin(), s.end());
                t0[0] += Lapse::Now() - l;
            }
            #endif//]
            
            #if 1//[
            {   // 
                auto s = a;
                auto l = Lapse::Now();
                std::stable_sort(s.begin(), s.end());
                t1[0] += Lapse::Now() - l;
            }
            #endif//]
            
            #if 1//[
            {   // 
                auto s = a;
                auto i = s.begin();
                auto e = s.end();
                for (auto t = 0; t < nLap; ++t, i += nTop){
                    auto l = Lapse::Now();
                    std::partial_sort(i, i + nTop, e);
                    t2[t] += Lapse::Now() - l;
                }
            }
            #endif//]
            
            #if 1//[
            {   // 
                auto s = a;
                auto i = s.begin();
                auto e = s.end();
                for (auto t = 0; t < nLap; ++t, i += nTop){
                    auto l = Lapse::Now();
                    SakaiShiki::sort(i, i + nTop, e);
                    t3[t] += Lapse::Now() - l;
                }
            }
            #endif//]
        }
        
        time("std::sort", t0, nLoop);
        time("std::stable_sort", t1, nLoop);
        time("std::partial_sort", t2, nLoop);
        time("SakaiShiki::sort", t3, nLoop);
    }
    #else//][
    for (auto n = nLoop; n; --n){
        init(Src, a, Rand, Range);
        
        auto s0 = a;
        auto s1 = a;
        
        std::stable_sort(s0.begin(), s0.end());
        
        auto i = s1.begin();
        auto e = s1.end();
        while (i != e){
            auto o = std::distance(i, e);
            o = (o < nTop)? o: nTop;
            SakaiShiki::sort(i, i + o, e);
            i += o;
        }
        
        auto bEqual = (s0 == s1);
        
        #if 0//[
        printf("\n");
        printf("%d\n", bEqual);
        #endif//]
        assert(bEqual);
    }
    #endif//]
}



int main(int argc, char* argv[])
{
    #if defined(NDEBUG)//[
    test(eSrc::Rand,       10000, 1000);
    test(eSrc::Rand,     1000000, 100);
    test(eSrc::Rand,   100000000, 10);
    
    test(eSrc::Equal,  100000000, 5);
    test(eSrc::Asc,    100000000, 5);
    test(eSrc::Dsc,    100000000, 5);
    test(eSrc::Saw,    100000000, 5);
    test(eSrc::AscSaw, 100000000, 5);
    test(eSrc::DscSaw, 100000000, 5);
    #else//][
    for (int nTest = 1; nTest < 200; ++nTest){
        test(eSrc::Rand,   nTest, 1000);
        test(eSrc::Equal,  nTest, 1);
        test(eSrc::Asc,    nTest, 1);
        test(eSrc::Dsc,    nTest, 1);
        test(eSrc::Saw,    nTest, 1);
        test(eSrc::AscSaw, nTest, 1);
        test(eSrc::DscSaw, nTest, 1);
    }
    #endif//]
    return 0;
}
