#pragma once
// Copyright(c) 2018 Emura Daisuke
// Apache License 2.0



#include <malloc.h>

#if _MSC_VER//[
// msvc
#define alloca      _alloca
#define Continue    // nop
#elif __clang__//][
// clang
#define Continue    continue
#else//][
// gcc
#define Continue    // nop
#endif//]

#define Auto                decltype(auto)
#define LocalAlloc(T, s)    static_cast<T*>(alloca(s))



// 

template <class T>
class HayateShiki final {
    public:
        static void Sort(T* const aSrc, std::size_t nSrc);
    
    private:
        struct Part;
        struct Unit;
        struct Dive;
        
        static int Msb(std::size_t v);
        static int MsbAlignment(std::size_t v);
        static int LowerLimit(int v, int limit);
        static constexpr std::size_t Bit(int v);
        
        static std::size_t Num(T* a, T* e);
        static T* Copy(T* pDst, T* pSrc, std::size_t nSrc);
        
        static T* Join(T* pJoin, Unit* pUnit, Part* pPart);
        static T* Join(T* aJoin, Unit* pUnit, Part* pPart0, Part* pPart1);
        static T* Join(T* aJoin, Unit* pUnit, Unit* pUnit0, Unit* pUnit1);
        
        static T* InitPart(Part* pPart, T* pSrc, T* eSrc, T** paDsc);
    
    private:
        static const std::size_t bIns = 5;
        static const std::size_t nIns = Bit(bIns);
};



// 

template <class T>
void HayateShiki<T>::Sort(T* const aSrc, std::size_t nSrc)
{
    if (aSrc && nSrc){
        auto aTmp = new T[nSrc];
        
        Auto nDive = LowerLimit((MsbAlignment(nSrc) - bIns), 1);
        Auto aDive = LocalAlloc(Dive, (sizeof(Dive) * (nDive+1)));
        for (int oDive = 0; oDive < nDive; ++oDive) aDive[oDive].mpJoin = (oDive & Bit(0))? aTmp: aSrc;
        
        {   // 
            std::size_t nJoin = 0;
            
            {   // 
                auto pJoin = aTmp;
                
                auto pSrc = &aSrc[0];
                auto eSrc = &aSrc[nSrc];
                while (pSrc){
                    Unit vUnit;
                    
                    {   // 
                        Part vPart0, vPart1;
                        auto aDsc = &aTmp[nSrc];
                        if ((pSrc = InitPart(&vPart0, pSrc, eSrc, &aDsc))){
                            pSrc = InitPart(&vPart1, pSrc, eSrc, &aDsc);
                            pJoin = Join(pJoin, &vUnit, &vPart0, &vPart1);
                        } else {
                            pJoin = Join(pJoin, &vUnit, &vPart0);
                        }
                    }
                    
                    {   // 
                        auto pDive = &aDive[0];
                        auto Carry = nJoin++;
                        Carry = (nJoin ^ Carry) & Carry;
                        for (; Carry; Carry >>= 1, ++pDive){
                            pDive->mpJoin = Join(pDive->mpJoin, &vUnit, &pDive->mUnit, &vUnit);
                        }
                        pDive->mUnit = vUnit;
                    }
                }
            }
            
            {   // 
                auto bJoin = nJoin & -nJoin;
                auto oDive = Msb(bJoin);
                auto pResult = &aDive[oDive++];
                
                {   // 
                    auto pDive = &aDive[oDive];
                    auto Carry = nJoin ^ bJoin;
                    Carry >>= oDive;
                    for (; Carry; Carry >>= 1, ++pDive){
                        if (Carry & Bit(0)){
                            Join(pDive->mpJoin, &pResult->mUnit, &pDive->mUnit, &pResult->mUnit);
                        }
                    }
                }
                
                if (pResult->mUnit.a == aTmp){
                    Copy(aSrc, pResult->mUnit.a, pResult->mUnit.n);
                }
            }
        }
        
        delete[] aTmp;
    }
}



// 

template <class T>
struct HayateShiki<T>::Part
{
    enum oUnit {
        oUnit_Asc,
        oUnit_Dsc,
        oUnit_Num,
    };
    
    T*              a[oUnit_Num];
    std::size_t     n[oUnit_Num];
    oUnit           o;
};



template <class T>
struct HayateShiki<T>::Unit
{
    T*              a;
    std::size_t     n;
};



template <class T>
struct HayateShiki<T>::Dive
{
    Unit            mUnit;
    T*              mpJoin;
};



// 

template <class T>
int HayateShiki<T>::Msb(std::size_t v)
{
    if (v){
        union IEEE_754 {
            float v;
            struct {
                unsigned int fraction:23;
                unsigned int exponent:8;
                unsigned int sign:1;
            } map;
        };
        IEEE_754 lzc;
        lzc.v = v;
        return (lzc.map.exponent - 127);
    }
    return -1;
}



template <class T>
int HayateShiki<T>::MsbAlignment(std::size_t v)
{
    return Msb(v + v - 1);
}



template <class T>
int HayateShiki<T>::LowerLimit(int v, int limit)
{
    return (v > limit)? v: limit;
}



template <class T>
constexpr std::size_t HayateShiki<T>::Bit(int v)
{
    return (1ULL << v);
}



template <class T>
std::size_t HayateShiki<T>::Num(T* a, T* e)
{
    return (e - a);
}



template <class T>
T* HayateShiki<T>::Copy(T* pDst, T* pSrc, std::size_t nSrc)
{
    while (nSrc--) *pDst++ = std::move(*pSrc++);
    return pDst;
}



template <class T>
T* HayateShiki<T>::Join(T* pJoin, Unit* pUnit, Part* pPart)
{
    auto nDsc = pPart->n[Part::oUnit_Dsc];
    auto nAsc = pPart->n[Part::oUnit_Asc];
    
    pUnit->a = pJoin;
    pUnit->n = nDsc + nAsc;
    
    pJoin = Copy(pJoin, pPart->a[Part::oUnit_Dsc], nDsc);
    pJoin = Copy(pJoin, pPart->a[Part::oUnit_Asc], nAsc);
    return pJoin;
}



template <class T>
T* HayateShiki<T>::Join(T* aJoin, Unit* pUnit, Part* pPart0, Part* pPart1)
{
    auto pJoin = aJoin;
    auto o0 = pPart0->o;
    auto o1 = pPart1->o;
    auto p0 = pPart0->a[o0];
    auto p1 = pPart1->a[o1];
    auto n0 = pPart0->n[o0];
    auto n1 = pPart1->n[o1];
    auto v0 = std::move(*p0);
    auto v1 = std::move(*p1);
    
    while (true){
        if (v1 < v0){
            *pJoin++ = std::move(v1);
            if (--n1){
                v1 = std::move(*++p1);
                Continue;
            } else {
                if (o1){
                    o1 = Part::oUnit_Asc;
                    p1 = pPart1->a[o1];
                    n1 = pPart1->n[o1];
                    v1 = std::move(*p1);
                    Continue;
                } else {
                    pJoin = Copy(pJoin, p0, n0);
                    if (o0) pJoin = Copy(pJoin, pPart0->a[Part::oUnit_Asc], pPart0->n[Part::oUnit_Asc]);
                    break;
                }
            }
        } else {
            *pJoin++ = std::move(v0);
            if (--n0){
                v0 = std::move(*++p0);
                Continue;
            } else {
                if (o0){
                    o0 = Part::oUnit_Asc;
                    p0 = pPart0->a[o0];
                    n0 = pPart0->n[o0];
                    v0 = std::move(*p0);
                    Continue;
                } else {
                    pJoin = Copy(pJoin, p1, n1);
                    if (o1) pJoin = Copy(pJoin, pPart1->a[Part::oUnit_Asc], pPart1->n[Part::oUnit_Asc]);
                    break;
                }
            }
        }
    }
    
    pUnit->a = aJoin;
    pUnit->n = Num(aJoin, pJoin);
    return pJoin;
}



template <class T>
T* HayateShiki<T>::Join(T* aJoin, Unit* pUnit, Unit* pUnit0, Unit* pUnit1)
{
    auto pJoin = aJoin;
    auto p0 = pUnit0->a;
    auto p1 = pUnit1->a;
    auto n0 = pUnit0->n;
    auto n1 = pUnit1->n;
    auto v0 = std::move(*p0);
    auto v1 = std::move(*p1);
    
    while (true){
        if (v1 < v0){
            *pJoin++ = std::move(v1);
            if (--n1){
                v1 = std::move(*++p1);
                Continue;
            } else {
                pJoin = Copy(pJoin, p0, n0);
                break;
            }
        } else {
            *pJoin++ = std::move(v0);
            if (--n0){
                v0 = std::move(*++p0);
                Continue;
            } else {
                pJoin = Copy(pJoin, p1, n1);
                break;
            }
        }
    }
    
    pUnit->a = aJoin;
    pUnit->n = Num(aJoin, pJoin);
    return pJoin;
}



template <class T>
T* HayateShiki<T>::InitPart(Part* pPart, T* pSrc, T* eSrc, T** paDsc)
{
    Auto nSrc = Num(pSrc, eSrc);
    nSrc = (nSrc < nIns)? nSrc: nIns;
    
    {   // 
        pPart->a[Part::oUnit_Asc] = pSrc;
        pPart->n[Part::oUnit_Asc] = nSrc;
    }
    
    {   // 
        auto aIns = pSrc;
        auto eIns = pSrc + nSrc;
        while (++aIns < eIns){
            auto v = std::move(aIns[0]);
            if (v < aIns[-1]){
                auto pIns = aIns;
                do {
                    pIns[0] = std::move(pIns[-1]);
                } while (--pIns > pSrc && v < pIns[-1]);
                pIns[0] = std::move(v);
            }
        }
    }
    
    {   // 
        auto pOdd = &pSrc[nSrc];
        auto nOdd = Num(pOdd, eSrc);
        if (nOdd){
            auto aDsc = *paDsc;
            auto eDsc = aDsc;
            
            {   // 
                auto pMin = &pSrc[0];
                auto pMax = &pSrc[nSrc-1];
                
                auto aAsc = pOdd;
                auto eAsc = aAsc;
                while (nOdd--){
                    auto v = std::move(*pOdd++);
                    if (v < *pMax){
                        if (v < *pMin){
                            *--aDsc = std::move(v);
                            pMin = aDsc;
                            Continue;
                        } else {
                            break;
                        }
                    } else {
                        pMax = eAsc;
                        *eAsc++ = std::move(v);
                        Continue;
                    }
                }
                pPart->n[Part::oUnit_Asc] += Num(aAsc, eAsc);
            }
            
            {   // 
                Auto nDsc = Num(aDsc, eDsc);
                pPart->a[Part::oUnit_Dsc] = aDsc;
                pPart->n[Part::oUnit_Dsc] = nDsc;
                pPart->o = (nDsc)? Part::oUnit_Dsc: Part::oUnit_Asc;
                *paDsc = aDsc;
            }
            return (++nOdd)? --pOdd: nullptr;
        } else {
            {   // 
                pPart->a[Part::oUnit_Dsc] = nullptr;
                pPart->n[Part::oUnit_Dsc] = 0;
                pPart->o = Part::oUnit_Asc;
            }
            return nullptr;
        }
    }
}