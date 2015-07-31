/*  This file is part of the Vc library. {{{
Copyright © 2011-2014 Matthias Kretz <kretz@kde.org>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the names of contributing organizations nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

}}}*/

#include "../common/x86_prefetches.h"
#include "../common/gatherimplementation.h"
#include "../common/scatterimplementation.h"
#include "limits.h"
#include "const.h"
#include "../common/set.h"
#include "macros.h"

namespace Vc_VERSIONED_NAMESPACE
{
///////////////////////////////////////////////////////////////////////////////////////////
// constants {{{1
template <typename T> Vc_INTRINSIC Vector<T, VectorAbi::Avx>::Vector(VectorSpecialInitializerZero::ZEnum) : d{} {}

template <> Vc_INTRINSIC AVX2::double_v::Vector(VectorSpecialInitializerOne::OEnum) : d(AVX::setone_pd()) {}
template <> Vc_INTRINSIC  AVX2::float_v::Vector(VectorSpecialInitializerOne::OEnum) : d(AVX::setone_ps()) {}
template <> Vc_INTRINSIC    AVX2::int_v::Vector(VectorSpecialInitializerOne::OEnum) : d(AVX::_mm_setone_epi32()) {}
template <> Vc_INTRINSIC   AVX2::uint_v::Vector(VectorSpecialInitializerOne::OEnum) : d(AVX::_mm_setone_epu32()) {}
template <> Vc_INTRINSIC  AVX2::short_v::Vector(VectorSpecialInitializerOne::OEnum) : d(AVX::_mm_setone_epi16()) {}
template <> Vc_INTRINSIC AVX2::ushort_v::Vector(VectorSpecialInitializerOne::OEnum) : d(AVX::_mm_setone_epu16()) {}
template <> Vc_INTRINSIC AVX2::Vector<  signed char>::Vector(VectorSpecialInitializerOne::OEnum) : d(AVX::_mm_setone_epi8()) {}
template <> Vc_INTRINSIC AVX2::Vector<unsigned char>::Vector(VectorSpecialInitializerOne::OEnum) : d(AVX::_mm_setone_epu8()) {}

template<typename T> Vc_ALWAYS_INLINE Vector<T, VectorAbi::Avx>::Vector(VectorSpecialInitializerIndexesFromZero::IEnum)
    : d(HV::template load<AlignedTag>(AVX::IndexesFromZeroData<T>::address())) {}

template <>
Vc_ALWAYS_INLINE AVX2::float_v::Vector(VectorSpecialInitializerIndexesFromZero::IEnum)
    : Vector(AVX::IndexesFromZeroData<int>::address(), Vc::Aligned)
{
}
template <>
Vc_ALWAYS_INLINE AVX2::double_v::Vector(VectorSpecialInitializerIndexesFromZero::IEnum)
    : Vector(AVX::IndexesFromZeroData<int>::address(), Vc::Aligned)
{
}

///////////////////////////////////////////////////////////////////////////////////////////
// load member functions {{{1
// general load, implemented via LoadHelper {{{2
template <typename DstT>
template <typename SrcT, typename Flags, typename>
Vc_INTRINSIC void Vector<DstT, VectorAbi::Avx>::load(const SrcT *mem, Flags flags)
{
    Common::handleLoadPrefetches(mem, flags);
    d.v() = Detail::load<VectorType, DstT>(mem, flags);
}

///////////////////////////////////////////////////////////////////////////////////////////
// zeroing {{{1
template<typename T> Vc_INTRINSIC void Vector<T, VectorAbi::Avx>::setZero()
{
    data() = HV::zero();
}
template<typename T> Vc_INTRINSIC void Vector<T, VectorAbi::Avx>::setZero(const Mask &k)
{
    data() = HV::andnot_(AVX::avx_cast<VectorType>(k.data()), data());
}
template<typename T> Vc_INTRINSIC void Vector<T, VectorAbi::Avx>::setZeroInverted(const Mask &k)
{
    data() = HV::and_(AVX::avx_cast<VectorType>(k.data()), data());
}

template<> Vc_INTRINSIC void AVX2::double_v::setQnan()
{
    data() = Detail::allone<VectorType>();
}
template<> Vc_INTRINSIC void AVX2::double_v::setQnan(MaskArgument k)
{
    data() = _mm256_or_pd(data(), k.dataD());
}
template<> Vc_INTRINSIC void AVX2::float_v::setQnan()
{
    data() = Detail::allone<VectorType>();
}
template<> Vc_INTRINSIC void AVX2::float_v::setQnan(MaskArgument k)
{
    data() = _mm256_or_ps(data(), k.data());
}

///////////////////////////////////////////////////////////////////////////////////////////
// stores {{{1
template <typename T>
template <typename U,
          typename Flags,
          typename>
Vc_INTRINSIC void Vector<T, VectorAbi::Avx>::store(U *mem, Flags flags) const
{
    Common::handleStorePrefetches(mem, flags);
    HV::template store<Flags>(mem, data());
}

template <typename T>
template <typename U,
          typename Flags,
          typename>
Vc_INTRINSIC void Vector<T, VectorAbi::Avx>::store(U *mem, Mask mask, Flags flags) const
{
    Common::handleStorePrefetches(mem, flags);
    HV::template store<Flags>(mem, data(), AVX::avx_cast<VectorType>(mask.data()));
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
// swizzles {{{1
template<typename T> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE &Vector<T, VectorAbi::Avx>::abcd() const { return *this; }
template<typename T> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE  Vector<T, VectorAbi::Avx>::cdab() const { return Mem::permute<X2, X3, X0, X1>(data()); }
template<typename T> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE  Vector<T, VectorAbi::Avx>::badc() const { return Mem::permute<X1, X0, X3, X2>(data()); }
template<typename T> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE  Vector<T, VectorAbi::Avx>::aaaa() const { return Mem::permute<X0, X0, X0, X0>(data()); }
template<typename T> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE  Vector<T, VectorAbi::Avx>::bbbb() const { return Mem::permute<X1, X1, X1, X1>(data()); }
template<typename T> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE  Vector<T, VectorAbi::Avx>::cccc() const { return Mem::permute<X2, X2, X2, X2>(data()); }
template<typename T> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE  Vector<T, VectorAbi::Avx>::dddd() const { return Mem::permute<X3, X3, X3, X3>(data()); }
template<typename T> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE  Vector<T, VectorAbi::Avx>::bcad() const { return Mem::permute<X1, X2, X0, X3>(data()); }
template<typename T> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE  Vector<T, VectorAbi::Avx>::bcda() const { return Mem::permute<X1, X2, X3, X0>(data()); }
template<typename T> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE  Vector<T, VectorAbi::Avx>::dabc() const { return Mem::permute<X3, X0, X1, X2>(data()); }
template<typename T> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE  Vector<T, VectorAbi::Avx>::acbd() const { return Mem::permute<X0, X2, X1, X3>(data()); }
template<typename T> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE  Vector<T, VectorAbi::Avx>::dbca() const { return Mem::permute<X3, X1, X2, X0>(data()); }
template<typename T> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE  Vector<T, VectorAbi::Avx>::dcba() const { return Mem::permute<X3, X2, X1, X0>(data()); }

template<> Vc_INTRINSIC const AVX2::double_v Vc_PURE AVX2::double_v::cdab() const { return Mem::shuffle128<X1, X0>(data(), data()); }
template<> Vc_INTRINSIC const AVX2::double_v Vc_PURE AVX2::double_v::badc() const { return Mem::permute<X1, X0, X3, X2>(data()); }
template<> Vc_INTRINSIC const AVX2::double_v Vc_PURE AVX2::double_v::aaaa() const { const double &tmp = d.m(0); return _mm256_broadcast_sd(&tmp); }
template<> Vc_INTRINSIC const AVX2::double_v Vc_PURE AVX2::double_v::bbbb() const { const double &tmp = d.m(1); return _mm256_broadcast_sd(&tmp); }
template<> Vc_INTRINSIC const AVX2::double_v Vc_PURE AVX2::double_v::cccc() const { const double &tmp = d.m(2); return _mm256_broadcast_sd(&tmp); }
template<> Vc_INTRINSIC const AVX2::double_v Vc_PURE AVX2::double_v::dddd() const { const double &tmp = d.m(3); return _mm256_broadcast_sd(&tmp); }
template<> Vc_INTRINSIC const AVX2::double_v Vc_PURE AVX2::double_v::bcad() const { return Mem::shuffle<X1, Y0, X2, Y3>(Mem::shuffle128<X0, X0>(data(), data()), Mem::shuffle128<X1, X1>(data(), data())); }
template<> Vc_INTRINSIC const AVX2::double_v Vc_PURE AVX2::double_v::bcda() const { return Mem::shuffle<X1, Y0, X3, Y2>(data(), Mem::shuffle128<X1, X0>(data(), data())); }
template<> Vc_INTRINSIC const AVX2::double_v Vc_PURE AVX2::double_v::dabc() const { return Mem::shuffle<X1, Y0, X3, Y2>(Mem::shuffle128<X1, X0>(data(), data()), data()); }
template<> Vc_INTRINSIC const AVX2::double_v Vc_PURE AVX2::double_v::acbd() const { return Mem::shuffle<X0, Y0, X3, Y3>(Mem::shuffle128<X0, X0>(data(), data()), Mem::shuffle128<X1, X1>(data(), data())); }
template<> Vc_INTRINSIC const AVX2::double_v Vc_PURE AVX2::double_v::dbca() const { return Mem::shuffle<X1, Y1, X2, Y2>(Mem::shuffle128<X1, X1>(data(), data()), Mem::shuffle128<X0, X0>(data(), data())); }
template<> Vc_INTRINSIC const AVX2::double_v Vc_PURE AVX2::double_v::dcba() const { return cdab().badc(); }

#define VC_SWIZZLES_16BIT_IMPL(T) \
template<> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE Vector<T, VectorAbi::Avx>::cdab() const { return Mem::permute<X2, X3, X0, X1, X6, X7, X4, X5>(data()); } \
template<> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE Vector<T, VectorAbi::Avx>::badc() const { return Mem::permute<X1, X0, X3, X2, X5, X4, X7, X6>(data()); } \
template<> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE Vector<T, VectorAbi::Avx>::aaaa() const { return Mem::permute<X0, X0, X0, X0, X4, X4, X4, X4>(data()); } \
template<> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE Vector<T, VectorAbi::Avx>::bbbb() const { return Mem::permute<X1, X1, X1, X1, X5, X5, X5, X5>(data()); } \
template<> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE Vector<T, VectorAbi::Avx>::cccc() const { return Mem::permute<X2, X2, X2, X2, X6, X6, X6, X6>(data()); } \
template<> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE Vector<T, VectorAbi::Avx>::dddd() const { return Mem::permute<X3, X3, X3, X3, X7, X7, X7, X7>(data()); } \
template<> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE Vector<T, VectorAbi::Avx>::bcad() const { return Mem::permute<X1, X2, X0, X3, X5, X6, X4, X7>(data()); } \
template<> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE Vector<T, VectorAbi::Avx>::bcda() const { return Mem::permute<X1, X2, X3, X0, X5, X6, X7, X4>(data()); } \
template<> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE Vector<T, VectorAbi::Avx>::dabc() const { return Mem::permute<X3, X0, X1, X2, X7, X4, X5, X6>(data()); } \
template<> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE Vector<T, VectorAbi::Avx>::acbd() const { return Mem::permute<X0, X2, X1, X3, X4, X6, X5, X7>(data()); } \
template<> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE Vector<T, VectorAbi::Avx>::dbca() const { return Mem::permute<X3, X1, X2, X0, X7, X5, X6, X4>(data()); } \
template<> Vc_INTRINSIC const AVX2::Vector<T> Vc_PURE Vector<T, VectorAbi::Avx>::dcba() const { return Mem::permute<X3, X2, X1, X0, X7, X6, X5, X4>(data()); }
VC_SWIZZLES_16BIT_IMPL(short)
VC_SWIZZLES_16BIT_IMPL(unsigned short)
#undef VC_SWIZZLES_16BIT_IMPL

///////////////////////////////////////////////////////////////////////////////////////////
// division {{{1
template<typename T> inline AVX2::Vector<T> &Vector<T, VectorAbi::Avx>::operator/=(EntryType x)
{
    if (HasVectorDivision) {
        return operator/=(AVX2::Vector<T>(x));
    }
    for_all_vector_entries(i, { d.set(i, d.m(i) / x); });
    return *this;
}
// per default fall back to scalar division
template<typename T> inline AVX2::Vector<T> &Vector<T, VectorAbi::Avx>::operator/=(VC_ALIGNED_PARAMETER(AVX2::Vector<T>) x)
{
    for_all_vector_entries(i, { d.set(i, d.m(i) / x.d.m(i)); });
    return *this;
}

template<typename T> inline Vc_PURE AVX2::Vector<T> Vector<T, VectorAbi::Avx>::operator/(VC_ALIGNED_PARAMETER(AVX2::Vector<T>) x) const
{
    AVX2::Vector<T> r;
    for_all_vector_entries(i,
            r.d.set(i, d.m(i) / x.d.m(i));
            );
    return r;
}
// specialize division on type
static Vc_INTRINSIC __m128i Vc_CONST divInt(__m128i a, __m128i b)
{
    return _mm256_cvttpd_epi32(
        _mm256_div_pd(_mm256_cvtepi32_pd(a), _mm256_cvtepi32_pd(b)));
}
template<> inline AVX2::int_v &AVX2::int_v::operator/=(VC_ALIGNED_PARAMETER(AVX2::int_v) x)
{
    d.v() = divInt(d.v(), x.d.v());
    return *this;
}
template<> inline AVX2::int_v Vc_PURE AVX2::int_v::operator/(VC_ALIGNED_PARAMETER(AVX2::int_v) x) const
{
    return divInt(d.v(), x.d.v());
}
static inline __m256i Vc_CONST divUInt(__m256i a, __m256i b) {
    // SSE/AVX only has signed int conversion to doubles. Therefore we first adjust the input before
    // conversion and take the adjustment back after the conversion.
    // It could be argued that for b this is not really important because division by a b >= 2^31 is
    // useless. But for full correctness it cannot be ignored.
#ifdef VC_IMPL_AVX2
    const __m256i aa = add_epi32(a, AVX::set1_epi32(-2147483648));
    const __m256i bb = add_epi32(b, AVX::set1_epi32(-2147483648));
    const __m256d loa = _mm256_add_pd(_mm256_cvtepi32_pd(AVX::lo128(aa)), AVX::set1_pd(2147483648.));
    const __m256d hia = _mm256_add_pd(_mm256_cvtepi32_pd(AVX::hi128(aa)), AVX::set1_pd(2147483648.));
    const __m256d lob = _mm256_add_pd(_mm256_cvtepi32_pd(AVX::lo128(bb)), AVX::set1_pd(2147483648.));
    const __m256d hib = _mm256_add_pd(_mm256_cvtepi32_pd(AVX::hi128(bb)), AVX::set1_pd(2147483648.));
#else
    const auto a0 = _mm_add_epi32(AVX::lo128(a), _mm_set1_epi32(-2147483648));
    const auto a1 = _mm_add_epi32(AVX::hi128(a), _mm_set1_epi32(-2147483648));
    const auto b0 = _mm_add_epi32(AVX::lo128(b), _mm_set1_epi32(-2147483648));
    const auto b1 = _mm_add_epi32(AVX::hi128(b), _mm_set1_epi32(-2147483648));
    const __m256d loa = _mm256_add_pd(_mm256_cvtepi32_pd(a0), AVX::set1_pd(2147483648.));
    const __m256d hia = _mm256_add_pd(_mm256_cvtepi32_pd(a1), AVX::set1_pd(2147483648.));
    const __m256d lob = _mm256_add_pd(_mm256_cvtepi32_pd(b0), AVX::set1_pd(2147483648.));
    const __m256d hib = _mm256_add_pd(_mm256_cvtepi32_pd(b1), AVX::set1_pd(2147483648.));
#endif
    // there is one remaining problem: a >= 2^31 and b == 1
    // in that case the return value would be 2^31
    return AVX::avx_cast<__m256i>(_mm256_blendv_ps(
        AVX::avx_cast<__m256>(AVX::concat(_mm256_cvttpd_epi32(_mm256_div_pd(loa, lob)),
                                          _mm256_cvttpd_epi32(_mm256_div_pd(hia, hib)))),
        AVX::avx_cast<__m256>(a),
        AVX::avx_cast<__m256>(AVX::cmpeq_epi32(b, AVX::setone_epi32()))));
}
static inline __m128i Vc_CONST divUInt(__m128i a, __m128i b)
{
    const auto a0 = _mm_add_epi32(a, _mm_set1_epi32(-2147483648));
    const auto b0 = _mm_add_epi32(b, _mm_set1_epi32(-2147483648));
    const __m256d loa = _mm256_add_pd(_mm256_cvtepi32_pd(a0), AVX::set1_pd(2147483648.));
    const __m256d lob = _mm256_add_pd(_mm256_cvtepi32_pd(b0), AVX::set1_pd(2147483648.));
    // there is one remaining problem: a >= 2^31 and b == 1
    // in that case the return value would be 2^31
    return _mm_blendv_epi8(_mm256_cvttpd_epi32(_mm256_div_pd(loa, lob)), a,
                           _mm_cmpeq_epi32(b, AVX::_mm_setone_epi32()));
}
template<> Vc_ALWAYS_INLINE AVX2::uint_v &AVX2::uint_v::operator/=(VC_ALIGNED_PARAMETER(AVX2::uint_v) x)
{
    d.v() = divUInt(d.v(), x.d.v());
    return *this;
}
template<> Vc_ALWAYS_INLINE AVX2::uint_v Vc_PURE AVX2::uint_v::operator/(VC_ALIGNED_PARAMETER(AVX2::uint_v) x) const
{
    return divUInt(d.v(), x.d.v());
}
template<typename T> static inline __m128i Vc_CONST divShort(__m128i a, __m128i b)
{
    const __m256 r = _mm256_div_ps(AVX::StaticCastHelper<T, float>::cast(a),
                                   AVX::StaticCastHelper<T, float>::cast(b));
    return AVX::StaticCastHelper<float, T>::cast(r);
}
template<> Vc_ALWAYS_INLINE AVX2::short_v &AVX2::short_v::operator/=(VC_ALIGNED_PARAMETER(AVX2::short_v) x)
{
    d.v() = divShort<short>(d.v(), x.d.v());
    return *this;
}
template<> Vc_ALWAYS_INLINE AVX2::short_v Vc_PURE AVX2::short_v::operator/(VC_ALIGNED_PARAMETER(AVX2::short_v) x) const
{
    return divShort<short>(d.v(), x.d.v());
}
template<> Vc_ALWAYS_INLINE AVX2::ushort_v &AVX2::ushort_v::operator/=(VC_ALIGNED_PARAMETER(AVX2::ushort_v) x)
{
    d.v() = divShort<unsigned short>(d.v(), x.d.v());
    return *this;
}
template<> Vc_ALWAYS_INLINE AVX2::ushort_v Vc_PURE AVX2::ushort_v::operator/(VC_ALIGNED_PARAMETER(AVX2::ushort_v) x) const
{
    return divShort<unsigned short>(d.v(), x.d.v());
}
template<> Vc_INTRINSIC AVX2::float_v &AVX2::float_v::operator/=(VC_ALIGNED_PARAMETER(AVX2::float_v) x)
{
    d.v() = _mm256_div_ps(d.v(), x.d.v());
    return *this;
}
template<> Vc_INTRINSIC AVX2::float_v Vc_PURE AVX2::float_v::operator/(VC_ALIGNED_PARAMETER(AVX2::float_v) x) const
{
    return _mm256_div_ps(d.v(), x.d.v());
}
template<> Vc_INTRINSIC AVX2::double_v &AVX2::double_v::operator/=(VC_ALIGNED_PARAMETER(AVX2::double_v) x)
{
    d.v() = _mm256_div_pd(d.v(), x.d.v());
    return *this;
}
template<> Vc_INTRINSIC AVX2::double_v Vc_PURE AVX2::double_v::operator/(VC_ALIGNED_PARAMETER(AVX2::double_v) x) const
{
    return _mm256_div_pd(d.v(), x.d.v());
}

///////////////////////////////////////////////////////////////////////////////////////////
// integer ops {{{1
template <> inline Vc_PURE AVX2::int_v AVX2::int_v::operator%(const AVX2::int_v &n) const
{
    return *this -
           n * AVX2::int_v(_mm256_cvttpd_epi32(_mm256_div_pd(_mm256_cvtepi32_pd(data()),
                                                       _mm256_cvtepi32_pd(n.data()))));
}
template <> inline Vc_PURE AVX2::uint_v AVX2::uint_v::operator%(const AVX2::uint_v &n) const
{
    auto &&cvt = [](__m128i v) {
        return _mm256_add_pd(
            _mm256_cvtepi32_pd(_mm_sub_epi32(v, AVX::_mm_setmin_epi32())),
            AVX::set1_pd(1u << 31));
    };
    auto &&cvt2 = [](__m256d v) {
        return __m128i(_mm256_cvttpd_epi32(
                                 _mm256_sub_pd(_mm256_floor_pd(v), AVX::set1_pd(0x80000000u))));
    };
    return *this -
           n * AVX2::uint_v(_mm_add_epi32(cvt2(_mm256_div_pd(cvt(data()), cvt(n.data()))),
                                         AVX::_mm_set2power31_epu32()));
}
template <> inline Vc_PURE AVX2::short_v AVX2::short_v::operator%(const AVX2::short_v &n) const
{
    return *this - n * static_cast<AVX2::short_v>(static_cast<AVX2::float_v>(*this) / static_cast<AVX2::float_v>(n));
}
template <> inline Vc_PURE AVX2::ushort_v AVX2::ushort_v::operator%(const AVX2::ushort_v &n) const
{
    return *this - n * static_cast<AVX2::ushort_v>(static_cast<AVX2::float_v>(*this) / static_cast<AVX2::float_v>(n));
}

#define OP_IMPL(T, symbol)                                                               \
    template <> Vc_ALWAYS_INLINE AVX2::Vector<T> &Vector<T, VectorAbi::Avx>::operator symbol##=(AsArg x)       \
    {                                                                                    \
        Common::unrolled_loop<std::size_t, 0, Size>(                                     \
            [&](std::size_t i) { d.set(i, d.m(i) symbol x.d.m(i)); });                   \
        return *this;                                                                    \
    }                                                                                    \
    template <>                                                                          \
    Vc_ALWAYS_INLINE Vc_PURE AVX2::Vector<T> Vector<T, VectorAbi::Avx>::operator symbol(AsArg x) const         \
    {                                                                                    \
        AVX2::Vector<T> r;                                                                     \
        Common::unrolled_loop<std::size_t, 0, Size>(                                     \
            [&](std::size_t i) { r.d.set(i, d.m(i) symbol x.d.m(i)); });                 \
        return r;                                                                        \
    }
OP_IMPL(int, <<)
OP_IMPL(int, >>)
OP_IMPL(unsigned int, <<)
OP_IMPL(unsigned int, >>)
OP_IMPL(short, <<)
OP_IMPL(short, >>)
OP_IMPL(unsigned short, <<)
OP_IMPL(unsigned short, >>)
#undef OP_IMPL

template<typename T> Vc_ALWAYS_INLINE AVX2::Vector<T> &Vector<T, VectorAbi::Avx>::operator>>=(int shift) {
    d.v() = HT::shiftRight(d.v(), shift);
    return *static_cast<AVX2::Vector<T> *>(this);
}
template<typename T> Vc_ALWAYS_INLINE Vc_PURE AVX2::Vector<T> Vector<T, VectorAbi::Avx>::operator>>(int shift) const {
    return HT::shiftRight(d.v(), shift);
}
template<typename T> Vc_ALWAYS_INLINE AVX2::Vector<T> &Vector<T, VectorAbi::Avx>::operator<<=(int shift) {
    d.v() = HT::shiftLeft(d.v(), shift);
    return *static_cast<AVX2::Vector<T> *>(this);
}
template<typename T> Vc_ALWAYS_INLINE Vc_PURE AVX2::Vector<T> Vector<T, VectorAbi::Avx>::operator<<(int shift) const {
    return HT::shiftLeft(d.v(), shift);
}

#define OP_IMPL(T, symbol, fun) \
  template<> Vc_ALWAYS_INLINE AVX2::Vector<T> &Vector<T, VectorAbi::Avx>::operator symbol##=(AsArg x) { d.v() = HV::fun(d.v(), x.d.v()); return *this; } \
  template<> Vc_ALWAYS_INLINE Vc_PURE AVX2::Vector<T>  Vector<T, VectorAbi::Avx>::operator symbol(AsArg x) const { return AVX2::Vector<T>(HV::fun(d.v(), x.d.v())); }
  OP_IMPL(int, &, and_)
  OP_IMPL(int, |, or_)
  OP_IMPL(int, ^, xor_)
  OP_IMPL(unsigned int, &, and_)
  OP_IMPL(unsigned int, |, or_)
  OP_IMPL(unsigned int, ^, xor_)
  OP_IMPL(short, &, and_)
  OP_IMPL(short, |, or_)
  OP_IMPL(short, ^, xor_)
  OP_IMPL(unsigned short, &, and_)
  OP_IMPL(unsigned short, |, or_)
  OP_IMPL(unsigned short, ^, xor_)
#ifdef VC_ENABLE_FLOAT_BIT_OPERATORS
  OP_IMPL(float, &, and_)
  OP_IMPL(float, |, or_)
  OP_IMPL(float, ^, xor_)
  OP_IMPL(double, &, and_)
  OP_IMPL(double, |, or_)
  OP_IMPL(double, ^, xor_)
#endif
#undef OP_IMPL

// isNegative {{{1
template<> Vc_INTRINSIC Vc_PURE AVX2::float_m AVX2::float_v::isNegative() const
{
    return AVX::avx_cast<__m256>(AVX::srai_epi32<31>(
        AVX::avx_cast<__m256i>(_mm256_and_ps(AVX::setsignmask_ps(), d.v()))));
}
template<> Vc_INTRINSIC Vc_PURE AVX2::double_m AVX2::double_v::isNegative() const
{
    return Mem::permute<X1, X1, X3, X3>(AVX::avx_cast<__m256>(AVX::srai_epi32<31>(
        AVX::avx_cast<__m256i>(_mm256_and_pd(AVX::setsignmask_pd(), d.v())))));
}
// gathers {{{1
template <>
template <typename MT, typename IT>
inline void AVX2::double_v::gatherImplementation(const MT *mem, IT &&indexes)
{
    d.v() = _mm256_setr_pd(mem[indexes[0]], mem[indexes[1]], mem[indexes[2]], mem[indexes[3]]);
}

template <>
template <typename MT, typename IT>
inline void AVX2::float_v::gatherImplementation(const MT *mem, IT &&indexes)
{
    d.v() = _mm256_setr_ps(mem[indexes[0]],
                           mem[indexes[1]],
                           mem[indexes[2]],
                           mem[indexes[3]],
                           mem[indexes[4]],
                           mem[indexes[5]],
                           mem[indexes[6]],
                           mem[indexes[7]]);
}

template <>
template <typename MT, typename IT>
inline void AVX2::int_v::gatherImplementation(const MT *mem, IT &&indexes)
{
    d.v() = _mm_setr_epi32(mem[indexes[0]], mem[indexes[1]], mem[indexes[2]],
                           mem[indexes[3]]);
}

template <>
template <typename MT, typename IT>
inline void AVX2::uint_v::gatherImplementation(const MT *mem, IT &&indexes)
{
    d.v() = _mm_setr_epi32(mem[indexes[0]], mem[indexes[1]], mem[indexes[2]],
                           mem[indexes[3]]);
}

template <>
template <typename MT, typename IT>
inline void AVX2::short_v::gatherImplementation(const MT *mem, IT &&indexes)
{
    d.v() = set(mem[indexes[0]], mem[indexes[1]], mem[indexes[2]], mem[indexes[3]],
                mem[indexes[4]], mem[indexes[5]], mem[indexes[6]], mem[indexes[7]]);
}

template <>
template <typename MT, typename IT>
inline void AVX2::ushort_v::gatherImplementation(const MT *mem, IT &&indexes)
{
    d.v() = set(mem[indexes[0]], mem[indexes[1]], mem[indexes[2]], mem[indexes[3]],
                mem[indexes[4]], mem[indexes[5]], mem[indexes[6]], mem[indexes[7]]);
}

template <typename T>
template <typename MT, typename IT>
inline void Vector<T, VectorAbi::Avx>::gatherImplementation(const MT *mem, IT &&indexes, MaskArgument mask)
{
    using Selector = std::integral_constant < Common::GatherScatterImplementation,
#ifdef VC_USE_SET_GATHERS
          Traits::is_simd_vector<IT>::value ? Common::GatherScatterImplementation::SetIndexZero :
#endif
#ifdef VC_USE_BSF_GATHERS
                                            Common::GatherScatterImplementation::BitScanLoop
#elif defined VC_USE_POPCNT_BSF_GATHERS
              Common::GatherScatterImplementation::PopcntSwitch
#else
              Common::GatherScatterImplementation::SimpleLoop
#endif
                                                > ;
    Common::executeGather(Selector(), *this, mem, indexes, mask);
}

template <typename T>
template <typename MT, typename IT>
inline void Vector<T, VectorAbi::Avx>::scatterImplementation(MT *mem, IT &&indexes) const
{
    Common::unrolled_loop<std::size_t, 0, Size>([&](std::size_t i) { mem[indexes[i]] = d.m(i); });
}

template <typename T>
template <typename MT, typename IT>
inline void Vector<T, VectorAbi::Avx>::scatterImplementation(MT *mem, IT &&indexes, MaskArgument mask) const
{
    using Selector = std::integral_constant < Common::GatherScatterImplementation,
#ifdef VC_USE_SET_GATHERS
          Traits::is_simd_vector<IT>::value ? Common::GatherScatterImplementation::SetIndexZero :
#endif
#ifdef VC_USE_BSF_GATHERS
                                            Common::GatherScatterImplementation::BitScanLoop
#elif defined VC_USE_POPCNT_BSF_GATHERS
              Common::GatherScatterImplementation::PopcntSwitch
#else
              Common::GatherScatterImplementation::SimpleLoop
#endif
                                                > ;
    Common::executeScatter(Selector(), *this, mem, indexes, mask);
}

#if defined(VC_MSVC) && VC_MSVC >= 170000000
// MSVC miscompiles the store mem[indexes[1]] = d.m(1) for T = (u)short
template <>
template <typename MT, typename IT>
Vc_ALWAYS_INLINE void AVX2::short_v::scatterImplementation(MT *mem, IT &&indexes) const
{
    const unsigned int tmp = d.v()._d.__m128i_u32[0];
    mem[indexes[0]] = tmp & 0xffff;
    mem[indexes[1]] = tmp >> 16;
    mem[indexes[2]] = _mm_extract_epi16(d.v(), 2);
    mem[indexes[3]] = _mm_extract_epi16(d.v(), 3);
    mem[indexes[4]] = _mm_extract_epi16(d.v(), 4);
    mem[indexes[5]] = _mm_extract_epi16(d.v(), 5);
    mem[indexes[6]] = _mm_extract_epi16(d.v(), 6);
    mem[indexes[7]] = _mm_extract_epi16(d.v(), 7);
}
template <>
template <typename MT, typename IT>
Vc_ALWAYS_INLINE void AVX2::ushort_v::scatterImplementation(MT *mem, IT &&indexes) const
{
    const unsigned int tmp = d.v()._d.__m128i_u32[0];
    mem[indexes[0]] = tmp & 0xffff;
    mem[indexes[1]] = tmp >> 16;
    mem[indexes[2]] = _mm_extract_epi16(d.v(), 2);
    mem[indexes[3]] = _mm_extract_epi16(d.v(), 3);
    mem[indexes[4]] = _mm_extract_epi16(d.v(), 4);
    mem[indexes[5]] = _mm_extract_epi16(d.v(), 5);
    mem[indexes[6]] = _mm_extract_epi16(d.v(), 6);
    mem[indexes[7]] = _mm_extract_epi16(d.v(), 7);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////
// operator- {{{1
#ifdef VC_USE_BUILTIN_VECTOR_TYPES
template<typename T> Vc_ALWAYS_INLINE Vc_PURE AVX2::Vector<T> Vector<T, VectorAbi::Avx>::operator-() const
{
    return VectorType(-d.builtin());
}
#else
namespace Internal
{
Vc_ALWAYS_INLINE Vc_CONST __m256 negate(__m256 v, std::integral_constant<std::size_t, 4>)
{
    return _mm256_xor_ps(v, AVX::setsignmask_ps());
}
Vc_ALWAYS_INLINE Vc_CONST __m256d negate(__m256d v, std::integral_constant<std::size_t, 8>)
{
    return _mm256_xor_pd(v, AVX::setsignmask_pd());
}
Vc_ALWAYS_INLINE Vc_CONST __m256i negate(__m256i v, std::integral_constant<std::size_t, 4>)
{
    return AVX::sign_epi32(v, Detail::allone<__m256i>());
}
Vc_ALWAYS_INLINE Vc_CONST __m128i negate(__m128i v, std::integral_constant<std::size_t, 4>)
{
    return _mm_sign_epi32(v, Detail::allone<__m128i>());
}
Vc_ALWAYS_INLINE Vc_CONST __m256i negate(__m256i v, std::integral_constant<std::size_t, 2>)
{
    return sign_epi16(v, Detail::allone<__m256i>());
}
Vc_ALWAYS_INLINE Vc_CONST __m128i negate(__m128i v, std::integral_constant<std::size_t, 2>)
{
    return _mm_sign_epi16(v, Detail::allone<__m128i>());
}
}  // namespace

template<typename T> Vc_ALWAYS_INLINE Vc_PURE AVX2::Vector<T> Vector<T, VectorAbi::Avx>::operator-() const
{
    return Internal::negate(d.v(), std::integral_constant<std::size_t, sizeof(T)>());
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////
// horizontal ops {{{1
template <typename T> Vc_INTRINSIC std::pair<AVX2::Vector<T>, int> Vector<T, VectorAbi::Avx>::minIndex() const
{
    AVX2::Vector<T> x = min();
    return std::make_pair(x, (*this == x).firstOne());
}
template <typename T> Vc_INTRINSIC std::pair<AVX2::Vector<T>, int> Vector<T, VectorAbi::Avx>::maxIndex() const
{
    AVX2::Vector<T> x = max();
    return std::make_pair(x, (*this == x).firstOne());
}
template <> Vc_INTRINSIC std::pair<AVX2::float_v, int> AVX2::float_v::minIndex() const
{
    /*
    // 28 cycles latency:
    __m256 x = _mm256_min_ps(Mem::permute128<X1, X0>(d.v()), d.v());
    x = _mm256_min_ps(x, Reg::permute<X2, X3, X0, X1>(x));
    AVX2::float_v xx = _mm256_min_ps(x, Reg::permute<X1, X0, X3, X2>(x));
    AVX2::uint_v idx = AVX2::uint_v::IndexesFromZero();
    idx = _mm256_castps_si256(
        _mm256_or_ps((*this != xx).data(), _mm256_castsi256_ps(idx.data())));
    return std::make_pair(xx, (*this == xx).firstOne());

    __m128 loData = AVX::lo128(d.v());
    __m128 hiData = AVX::hi128(d.v());
    const __m128 less2 = _mm_cmplt_ps(hiData, loData);
    loData = _mm_min_ps(loData, hiData);
    hiData = Mem::permute<X2, X3, X0, X1>(loData);
    const __m128 less1 = _mm_cmplt_ps(hiData, loData);
    loData = _mm_min_ps(loData, hiData);
    hiData = Mem::permute<X1, X0, X3, X2>(loData);
    const __m128 less0 = _mm_cmplt_ps(hiData, loData);
    unsigned bits = _mm_movemask_ps(less0) & 0x1;
    bits |= ((_mm_movemask_ps(less1) << 1) - bits) & 0x2;
    bits |= ((_mm_movemask_ps(less2) << 3) - bits) & 0x4;
    loData = _mm_min_ps(loData, hiData);
    return std::make_pair(AVX::concat(loData, loData), bits);
    */

    // 28 cycles Latency:
    __m256 x = d.v();
    __m256 idx = _mm256_castsi256_ps(AVX::VectorHelper<__m256i>::load<AlignedTag>(
        AVX::IndexesFromZeroData<int>::address()));
    __m256 y = Mem::permute128<X1, X0>(x);
    __m256 idy = Mem::permute128<X1, X0>(idx);
    __m256 less = AVX::cmplt_ps(x, y);

    x = _mm256_blendv_ps(y, x, less);
    idx = _mm256_blendv_ps(idy, idx, less);
    y = Reg::permute<X2, X3, X0, X1>(x);
    idy = Reg::permute<X2, X3, X0, X1>(idx);
    less = AVX::cmplt_ps(x, y);

    x = _mm256_blendv_ps(y, x, less);
    idx = _mm256_blendv_ps(idy, idx, less);
    y = Reg::permute<X1, X0, X3, X2>(x);
    idy = Reg::permute<X1, X0, X3, X2>(idx);
    less = AVX::cmplt_ps(x, y);

    idx = _mm256_blendv_ps(idy, idx, less);

    const auto index = _mm_cvtsi128_si32(AVX::avx_cast<__m128i>(idx));
    __asm__ __volatile__(""); // help GCC to order the instructions better
    x = _mm256_blendv_ps(y, x, less);
    return std::make_pair(x, index);
}
template<typename T> Vc_ALWAYS_INLINE AVX2::Vector<T> Vector<T, VectorAbi::Avx>::partialSum() const
{
    //   a    b    c    d    e    f    g    h
    // +      a    b    c    d    e    f    g    -> a ab bc  cd   de    ef     fg      gh
    // +           a    ab   bc   cd   de   ef   -> a ab abc abcd bcde  cdef   defg    efgh
    // +                     a    ab   abc  abcd -> a ab abc abcd abcde abcdef abcdefg abcdefgh
    AVX2::Vector<T> tmp = *this;
    if (Size >  1) tmp += tmp.shifted(-1);
    if (Size >  2) tmp += tmp.shifted(-2);
    if (Size >  4) tmp += tmp.shifted(-4);
    if (Size >  8) tmp += tmp.shifted(-8);
    if (Size > 16) tmp += tmp.shifted(-16);
    return tmp;
}

/* This function requires correct masking because the neutral element of \p op is not necessarily 0
 *
template<typename T> template<typename BinaryOperation> Vc_ALWAYS_INLINE AVX2::Vector<T> Vector<T, VectorAbi::Avx>::partialSum(BinaryOperation op) const
{
    //   a    b    c    d    e    f    g    h
    // +      a    b    c    d    e    f    g    -> a ab bc  cd   de    ef     fg      gh
    // +           a    ab   bc   cd   de   ef   -> a ab abc abcd bcde  cdef   defg    efgh
    // +                     a    ab   abc  abcd -> a ab abc abcd abcde abcdef abcdefg abcdefgh
    AVX2::Vector<T> tmp = *this;
    Mask mask(true);
    if (Size >  1) tmp(mask) = op(tmp, tmp.shifted(-1));
    if (Size >  2) tmp(mask) = op(tmp, tmp.shifted(-2));
    if (Size >  4) tmp(mask) = op(tmp, tmp.shifted(-4));
    if (Size >  8) tmp(mask) = op(tmp, tmp.shifted(-8));
    if (Size > 16) tmp(mask) = op(tmp, tmp.shifted(-16));
    return tmp;
}
*/

template<typename T> Vc_ALWAYS_INLINE typename Vector<T, VectorAbi::Avx>::EntryType Vector<T, VectorAbi::Avx>::min(MaskArg m) const
{
    AVX2::Vector<T> tmp = std::numeric_limits<AVX2::Vector<T> >::max();
    tmp(m) = *this;
    return tmp.min();
}
template<typename T> Vc_ALWAYS_INLINE typename Vector<T, VectorAbi::Avx>::EntryType Vector<T, VectorAbi::Avx>::max(MaskArg m) const
{
    AVX2::Vector<T> tmp = std::numeric_limits<AVX2::Vector<T> >::min();
    tmp(m) = *this;
    return tmp.max();
}
template<typename T> Vc_ALWAYS_INLINE typename Vector<T, VectorAbi::Avx>::EntryType Vector<T, VectorAbi::Avx>::product(MaskArg m) const
{
    AVX2::Vector<T> tmp(VectorSpecialInitializerOne::One);
    tmp(m) = *this;
    return tmp.product();
}
template<typename T> Vc_ALWAYS_INLINE typename Vector<T, VectorAbi::Avx>::EntryType Vector<T, VectorAbi::Avx>::sum(MaskArg m) const
{
    AVX2::Vector<T> tmp(VectorSpecialInitializerZero::Zero);
    tmp(m) = *this;
    return tmp.sum();
}//}}}
// copySign {{{1
template<> Vc_INTRINSIC AVX2::float_v AVX2::float_v::copySign(AVX2::float_v::AsArg reference) const
{
    return _mm256_or_ps(
            _mm256_and_ps(reference.d.v(), AVX::setsignmask_ps()),
            _mm256_and_ps(d.v(), AVX::setabsmask_ps())
            );
}
template<> Vc_INTRINSIC AVX2::double_v AVX2::double_v::copySign(AVX2::double_v::AsArg reference) const
{
    return _mm256_or_pd(
            _mm256_and_pd(reference.d.v(), AVX::setsignmask_pd()),
            _mm256_and_pd(d.v(), AVX::setabsmask_pd())
            );
}//}}}1
// exponent {{{1
namespace Detail
{
Vc_INTRINSIC Vc_CONST __m256 exponent(__m256 v)
{
    using namespace AVX;
    __m128i tmp0 = _mm_srli_epi32(avx_cast<__m128i>(v), 23);
    __m128i tmp1 = _mm_srli_epi32(avx_cast<__m128i>(hi128(v)), 23);
    tmp0 = _mm_sub_epi32(tmp0, _mm_set1_epi32(0x7f));
    tmp1 = _mm_sub_epi32(tmp1, _mm_set1_epi32(0x7f));
    return _mm256_cvtepi32_ps(concat(tmp0, tmp1));
}
Vc_INTRINSIC Vc_CONST __m256d exponent(__m256d v)
{
    using namespace AVX;
    __m128i tmp0 = _mm_srli_epi64(avx_cast<__m128i>(v), 52);
    __m128i tmp1 = _mm_srli_epi64(avx_cast<__m128i>(hi128(v)), 52);
    tmp0 = _mm_sub_epi32(tmp0, _mm_set1_epi32(0x3ff));
    tmp1 = _mm_sub_epi32(tmp1, _mm_set1_epi32(0x3ff));
    return _mm256_cvtepi32_pd(avx_cast<__m128i>(Mem::shuffle<X0, X2, Y0, Y2>(avx_cast<__m128>(tmp0), avx_cast<__m128>(tmp1))));
}
} // namespace Detail

template<> Vc_INTRINSIC AVX2::float_v AVX2::float_v::exponent() const
{
    VC_ASSERT((*this >= 0.f).isFull());
    return Detail::exponent(d.v());
}
template<> Vc_INTRINSIC AVX2::double_v AVX2::double_v::exponent() const
{
    VC_ASSERT((*this >= 0.).isFull());
    return Detail::exponent(d.v());
}
// }}}1
// Random {{{1
static Vc_ALWAYS_INLINE AVX2::uint_v _doRandomStep()
{
    AVX2::uint_v state0(&Common::RandomState[0]);
    AVX2::uint_v state1(&Common::RandomState[AVX2::uint_v::Size]);
    (state1 * 0xdeece66du + 11).store(&Common::RandomState[AVX2::uint_v::Size]);
    AVX2::uint_v(_mm_xor_si128((state0 * 0xdeece66du + 11).data(), _mm_srli_epi32(state1.data(), 16))).store(&Common::RandomState[0]);
    return state0;
}

template<typename T> Vc_ALWAYS_INLINE AVX2::Vector<T> Vector<T, VectorAbi::Avx>::Random()
{
    const AVX2::uint_v state0 = _doRandomStep();
    return {state0.data()};
}

template<> Vc_ALWAYS_INLINE AVX2::float_v AVX2::float_v::Random()
{
    const AVX2::uint_v rand0 = _doRandomStep();
    const AVX2::uint_v rand1 = _doRandomStep();
    return HT::sub(
        HV::or_(_cast(AVX::srli_epi32<2>(AVX::concat(rand0.data(), rand1.data()))), HT::one()),
        HT::one());
}

template<> Vc_ALWAYS_INLINE AVX2::double_v AVX2::double_v::Random()
{
    const __m256i state =
        AVX::VectorHelper<__m256i>::load<AlignedTag>(&Common::RandomState[0]);
    for (size_t k = 0; k < 8; k += 2) {
        typedef unsigned long long uint64 Vc_MAY_ALIAS;
        const uint64 stateX = *reinterpret_cast<const uint64 *>(&Common::RandomState[k]);
        *reinterpret_cast<uint64 *>(&Common::RandomState[k]) = (stateX * 0x5deece66dull + 11);
    }
    return HT::sub(HV::or_(_cast(AVX::srli_epi64<12>(state)), HT::one()), HT::one());
}
// }}}1
// shifted / rotated {{{1
namespace Detail
{
template <typename V> constexpr size_t sanitize(size_t n)
{
    return n >= sizeof(V) ? 0 : n;
}
template <typename T, typename V>
static Vc_INTRINSIC Vc_CONST enable_if<(sizeof(V) == 32), V> shifted(
    VC_ALIGNED_PARAMETER(V) v, int amount)
{
    using namespace AVX;
    switch (amount) {
    case  0: return v;
    case  1: return avx_cast<V>(srli_si256<sanitize<V>(1 * sizeof(T))>(avx_cast<__m256i>(v)));
    case  2: return avx_cast<V>(srli_si256<sanitize<V>(2 * sizeof(T))>(avx_cast<__m256i>(v)));
    case  3: return avx_cast<V>(srli_si256<sanitize<V>(3 * sizeof(T))>(avx_cast<__m256i>(v)));
    case -1: return avx_cast<V>(slli_si256<sanitize<V>(1 * sizeof(T))>(avx_cast<__m256i>(v)));
    case -2: return avx_cast<V>(slli_si256<sanitize<V>(2 * sizeof(T))>(avx_cast<__m256i>(v)));
    case -3: return avx_cast<V>(slli_si256<sanitize<V>(3 * sizeof(T))>(avx_cast<__m256i>(v)));
    }
    if (sizeof(T) <= 4) {
        switch (amount) {
        case  4: return avx_cast<V>(srli_si256<sanitize<V>(4 * sizeof(T))>(avx_cast<__m256i>(v)));
        case  5: return avx_cast<V>(srli_si256<sanitize<V>(5 * sizeof(T))>(avx_cast<__m256i>(v)));
        case  6: return avx_cast<V>(srli_si256<sanitize<V>(6 * sizeof(T))>(avx_cast<__m256i>(v)));
        case  7: return avx_cast<V>(srli_si256<sanitize<V>(7 * sizeof(T))>(avx_cast<__m256i>(v)));
        case -4: return avx_cast<V>(slli_si256<sanitize<V>(4 * sizeof(T))>(avx_cast<__m256i>(v)));
        case -5: return avx_cast<V>(slli_si256<sanitize<V>(5 * sizeof(T))>(avx_cast<__m256i>(v)));
        case -6: return avx_cast<V>(slli_si256<sanitize<V>(6 * sizeof(T))>(avx_cast<__m256i>(v)));
        case -7: return avx_cast<V>(slli_si256<sanitize<V>(7 * sizeof(T))>(avx_cast<__m256i>(v)));
        }
    }
    return avx_cast<V>(_mm256_setzero_ps());
}

template <typename T, typename V>
static Vc_INTRINSIC Vc_CONST enable_if<(sizeof(V) == 16), V> shifted(
    VC_ALIGNED_PARAMETER(V) v, int amount)
{
    using namespace AVX;
    switch (amount) {
    case  0: return v;
    case  1: return avx_cast<V>(_mm_srli_si128(avx_cast<__m128i>(v), sanitize<V>(1 * sizeof(T))));
    case  2: return avx_cast<V>(_mm_srli_si128(avx_cast<__m128i>(v), sanitize<V>(2 * sizeof(T))));
    case  3: return avx_cast<V>(_mm_srli_si128(avx_cast<__m128i>(v), sanitize<V>(3 * sizeof(T))));
    case -1: return avx_cast<V>(_mm_slli_si128(avx_cast<__m128i>(v), sanitize<V>(1 * sizeof(T))));
    case -2: return avx_cast<V>(_mm_slli_si128(avx_cast<__m128i>(v), sanitize<V>(2 * sizeof(T))));
    case -3: return avx_cast<V>(_mm_slli_si128(avx_cast<__m128i>(v), sanitize<V>(3 * sizeof(T))));
    }
    if (sizeof(T) <= 2) {
        switch (amount) {
        case  4: return avx_cast<V>(_mm_srli_si128(avx_cast<__m128i>(v), sanitize<V>(4 * sizeof(T))));
        case  5: return avx_cast<V>(_mm_srli_si128(avx_cast<__m128i>(v), sanitize<V>(5 * sizeof(T))));
        case  6: return avx_cast<V>(_mm_srli_si128(avx_cast<__m128i>(v), sanitize<V>(6 * sizeof(T))));
        case  7: return avx_cast<V>(_mm_srli_si128(avx_cast<__m128i>(v), sanitize<V>(7 * sizeof(T))));
        case -4: return avx_cast<V>(_mm_slli_si128(avx_cast<__m128i>(v), sanitize<V>(4 * sizeof(T))));
        case -5: return avx_cast<V>(_mm_slli_si128(avx_cast<__m128i>(v), sanitize<V>(5 * sizeof(T))));
        case -6: return avx_cast<V>(_mm_slli_si128(avx_cast<__m128i>(v), sanitize<V>(6 * sizeof(T))));
        case -7: return avx_cast<V>(_mm_slli_si128(avx_cast<__m128i>(v), sanitize<V>(7 * sizeof(T))));
        }
    }
    return avx_cast<V>(_mm_setzero_ps());
}

template <typename T, size_t N, typename V>
static Vc_INTRINSIC Vc_CONST enable_if<(sizeof(V) == 32 && N == 4), V> rotated(
    VC_ALIGNED_PARAMETER(V) v, int amount)
{
    using namespace AVX;
    const __m128i vLo = avx_cast<__m128i>(lo128(v));
    const __m128i vHi = avx_cast<__m128i>(hi128(v));
    switch (static_cast<unsigned int>(amount) % N) {
    case 0:
        return v;
    case 1:
        return avx_cast<V>(concat(_mm_alignr_epi8(vHi, vLo, sizeof(T)),
                                       _mm_alignr_epi8(vLo, vHi, sizeof(T))));
    case 2:
        return Mem::permute128<X1, X0>(v);
    case 3:
        return avx_cast<V>(concat(_mm_alignr_epi8(vLo, vHi, sizeof(T)),
                                       _mm_alignr_epi8(vHi, vLo, sizeof(T))));
    }
    return avx_cast<V>(_mm256_setzero_ps());
}

template <typename T, size_t N, typename V>
static Vc_INTRINSIC Vc_CONST enable_if<(sizeof(V) == 32 && N == 8), V> rotated(
    VC_ALIGNED_PARAMETER(V) v, int amount)
{
    using namespace AVX;
    const __m128i vLo = avx_cast<__m128i>(lo128(v));
    const __m128i vHi = avx_cast<__m128i>(hi128(v));
    switch (static_cast<unsigned int>(amount) % N) {
    case 0:
        return v;
    case 1:
        return avx_cast<V>(concat(_mm_alignr_epi8(vHi, vLo, 1 * sizeof(T)),
                                       _mm_alignr_epi8(vLo, vHi, 1 * sizeof(T))));
    case 2:
        return avx_cast<V>(concat(_mm_alignr_epi8(vHi, vLo, 2 * sizeof(T)),
                                       _mm_alignr_epi8(vLo, vHi, 2 * sizeof(T))));
    case 3:
        return avx_cast<V>(concat(_mm_alignr_epi8(vHi, vLo, 3 * sizeof(T)),
                                       _mm_alignr_epi8(vLo, vHi, 3 * sizeof(T))));
    case 4:
        return Mem::permute128<X1, X0>(v);
    case 5:
        return avx_cast<V>(concat(_mm_alignr_epi8(vLo, vHi, 1 * sizeof(T)),
                                       _mm_alignr_epi8(vHi, vLo, 1 * sizeof(T))));
    case 6:
        return avx_cast<V>(concat(_mm_alignr_epi8(vLo, vHi, 2 * sizeof(T)),
                                       _mm_alignr_epi8(vHi, vLo, 2 * sizeof(T))));
    case 7:
        return avx_cast<V>(concat(_mm_alignr_epi8(vLo, vHi, 3 * sizeof(T)),
                                       _mm_alignr_epi8(vHi, vLo, 3 * sizeof(T))));
    }
    return avx_cast<V>(_mm256_setzero_ps());
}

template <typename T, size_t N, typename V>
static Vc_INTRINSIC Vc_CONST enable_if<(sizeof(V) == 16), V> rotated(
    VC_ALIGNED_PARAMETER(V) v, int amount)
{
    using namespace AVX;
    switch (static_cast<unsigned int>(amount) % N) {
    case 0:
        return v;
    case 1:
        return avx_cast<V>(_mm_alignr_epi8(v, v, sanitize<V>(1 * sizeof(T))));
    case 2:
        return avx_cast<V>(_mm_alignr_epi8(v, v, sanitize<V>(2 * sizeof(T))));
    case 3:
        return avx_cast<V>(_mm_alignr_epi8(v, v, sanitize<V>(3 * sizeof(T))));
    case 4:
        return avx_cast<V>(_mm_alignr_epi8(v, v, sanitize<V>(4 * sizeof(T))));
    case 5:
        return avx_cast<V>(_mm_alignr_epi8(v, v, sanitize<V>(5 * sizeof(T))));
    case 6:
        return avx_cast<V>(_mm_alignr_epi8(v, v, sanitize<V>(6 * sizeof(T))));
    case 7:
        return avx_cast<V>(_mm_alignr_epi8(v, v, sanitize<V>(7 * sizeof(T))));
    }
    return avx_cast<V>(_mm_setzero_si128());
}

}  // namespace Detail
template<typename T> Vc_INTRINSIC AVX2::Vector<T> Vector<T, VectorAbi::Avx>::shifted(int amount) const
{
    return Detail::shifted<EntryType>(d.v(), amount);
}

template <typename VectorType>
Vc_INTRINSIC Vc_CONST VectorType shifted_shortcut(VectorType left, VectorType right, Common::WidthT<__m128>)
{
    return Mem::shuffle<X2, X3, Y0, Y1>(left, right);
}
template <typename VectorType>
Vc_INTRINSIC Vc_CONST VectorType shifted_shortcut(VectorType left, VectorType right, Common::WidthT<__m256>)
{
    return Mem::shuffle128<X1, Y0>(left, right);
}

template<typename T> Vc_INTRINSIC AVX2::Vector<T> Vector<T, VectorAbi::Avx>::shifted(int amount, Vector shiftIn) const
{
#ifdef __GNUC__
    if (__builtin_constant_p(amount)) {
        switch (amount * 2) {
        case int(Size):
            return shifted_shortcut(d.v(), shiftIn.d.v(), WidthT());
        case -int(Size):
            return shifted_shortcut(shiftIn.d.v(), d.v(), WidthT());
        }
    }
#endif
    return shifted(amount) | (amount > 0 ?
                              shiftIn.shifted(amount - Size) :
                              shiftIn.shifted(Size + amount));
}
template<typename T> Vc_INTRINSIC AVX2::Vector<T> Vector<T, VectorAbi::Avx>::rotated(int amount) const
{
    return Detail::rotated<EntryType, size()>(d.v(), amount);
}
// interleaveLow/-High {{{1
template <> Vc_INTRINSIC AVX2::double_v AVX2::double_v::interleaveLow(AVX2::double_v x) const
{
    return Mem::shuffle128<X0, Y0>(_mm256_unpacklo_pd(data(), x.data()),
                                   _mm256_unpackhi_pd(data(), x.data()));
}
template <> Vc_INTRINSIC AVX2::double_v AVX2::double_v::interleaveHigh(AVX2::double_v x) const
{
    return Mem::shuffle128<X1, Y1>(_mm256_unpacklo_pd(data(), x.data()),
                                   _mm256_unpackhi_pd(data(), x.data()));
}
template <> Vc_INTRINSIC AVX2::float_v AVX2::float_v::interleaveLow(AVX2::float_v x) const
{
    return Mem::shuffle128<X0, Y0>(_mm256_unpacklo_ps(data(), x.data()),
                                   _mm256_unpackhi_ps(data(), x.data()));
}
template <> Vc_INTRINSIC AVX2::float_v AVX2::float_v::interleaveHigh(AVX2::float_v x) const
{
    return Mem::shuffle128<X1, Y1>(_mm256_unpacklo_ps(data(), x.data()),
                                   _mm256_unpackhi_ps(data(), x.data()));
}
#ifdef VC_IMPL_AVX2
template <> Vc_INTRINSIC    AVX2::int_v    AVX2::int_v::interleaveLow (   AVX2::int_v x) const { return unpacklo_epi32(data(), x.data()); }
template <> Vc_INTRINSIC    AVX2::int_v    AVX2::int_v::interleaveHigh(   AVX2::int_v x) const { return unpackhi_epi32(data(), x.data()); }
template <> Vc_INTRINSIC   AVX2::uint_v   AVX2::uint_v::interleaveLow (  AVX2::uint_v x) const { return unpacklo_epi32(data(), x.data()); }
template <> Vc_INTRINSIC   AVX2::uint_v   AVX2::uint_v::interleaveHigh(  AVX2::uint_v x) const { return unpackhi_epi32(data(), x.data()); }
// TODO:
//template <> Vc_INTRINSIC  AVX2::short_v  AVX2::short_v::interleaveLow ( AVX2::short_v x) const { return unpacklo_epi16(data(), x.data()); }
//template <> Vc_INTRINSIC  AVX2::short_v  AVX2::short_v::interleaveHigh( AVX2::short_v x) const { return unpackhi_epi16(data(), x.data()); }
//template <> Vc_INTRINSIC AVX2::ushort_v AVX2::ushort_v::interleaveLow (AVX2::ushort_v x) const { return unpacklo_epi16(data(), x.data()); }
//template <> Vc_INTRINSIC AVX2::ushort_v AVX2::ushort_v::interleaveHigh(AVX2::ushort_v x) const { return unpackhi_epi16(data(), x.data()); }
template <> Vc_INTRINSIC  AVX2::short_v  AVX2::short_v::interleaveLow ( AVX2::short_v x) const { return _mm_unpacklo_epi16(data(), x.data()); }
template <> Vc_INTRINSIC  AVX2::short_v  AVX2::short_v::interleaveHigh( AVX2::short_v x) const { return _mm_unpackhi_epi16(data(), x.data()); }
template <> Vc_INTRINSIC AVX2::ushort_v AVX2::ushort_v::interleaveLow (AVX2::ushort_v x) const { return _mm_unpacklo_epi16(data(), x.data()); }
template <> Vc_INTRINSIC AVX2::ushort_v AVX2::ushort_v::interleaveHigh(AVX2::ushort_v x) const { return _mm_unpackhi_epi16(data(), x.data()); }
#else
template <> Vc_INTRINSIC    AVX2::int_v    AVX2::int_v::interleaveLow (   AVX2::int_v x) const { return _mm_unpacklo_epi32(data(), x.data()); }
template <> Vc_INTRINSIC    AVX2::int_v    AVX2::int_v::interleaveHigh(   AVX2::int_v x) const { return _mm_unpackhi_epi32(data(), x.data()); }
template <> Vc_INTRINSIC   AVX2::uint_v   AVX2::uint_v::interleaveLow (  AVX2::uint_v x) const { return _mm_unpacklo_epi32(data(), x.data()); }
template <> Vc_INTRINSIC   AVX2::uint_v   AVX2::uint_v::interleaveHigh(  AVX2::uint_v x) const { return _mm_unpackhi_epi32(data(), x.data()); }
template <> Vc_INTRINSIC  AVX2::short_v  AVX2::short_v::interleaveLow ( AVX2::short_v x) const { return _mm_unpacklo_epi16(data(), x.data()); }
template <> Vc_INTRINSIC  AVX2::short_v  AVX2::short_v::interleaveHigh( AVX2::short_v x) const { return _mm_unpackhi_epi16(data(), x.data()); }
template <> Vc_INTRINSIC AVX2::ushort_v AVX2::ushort_v::interleaveLow (AVX2::ushort_v x) const { return _mm_unpacklo_epi16(data(), x.data()); }
template <> Vc_INTRINSIC AVX2::ushort_v AVX2::ushort_v::interleaveHigh(AVX2::ushort_v x) const { return _mm_unpackhi_epi16(data(), x.data()); }
#endif
// generate {{{1
template <> template <typename G> Vc_INTRINSIC AVX2::double_v AVX2::double_v::generate(G gen)
{
    const auto tmp0 = gen(0);
    const auto tmp1 = gen(1);
    const auto tmp2 = gen(2);
    const auto tmp3 = gen(3);
    return _mm256_setr_pd(tmp0, tmp1, tmp2, tmp3);
}
template <> template <typename G> Vc_INTRINSIC AVX2::float_v AVX2::float_v::generate(G gen)
{
    const auto tmp0 = gen(0);
    const auto tmp1 = gen(1);
    const auto tmp2 = gen(2);
    const auto tmp3 = gen(3);
    const auto tmp4 = gen(4);
    const auto tmp5 = gen(5);
    const auto tmp6 = gen(6);
    const auto tmp7 = gen(7);
    return _mm256_setr_ps(tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7);
}
template <> template <typename G> Vc_INTRINSIC AVX2::int_v AVX2::int_v::generate(G gen)
{
    const auto tmp0 = gen(0);
    const auto tmp1 = gen(1);
    const auto tmp2 = gen(2);
    const auto tmp3 = gen(3);
    return _mm_setr_epi32(tmp0, tmp1, tmp2, tmp3);
}
template <> template <typename G> Vc_INTRINSIC AVX2::uint_v AVX2::uint_v::generate(G gen)
{
    const auto tmp0 = gen(0);
    const auto tmp1 = gen(1);
    const auto tmp2 = gen(2);
    const auto tmp3 = gen(3);
    return _mm_setr_epi32(tmp0, tmp1, tmp2, tmp3);
}
template <> template <typename G> Vc_INTRINSIC AVX2::short_v AVX2::short_v::generate(G gen)
{
    const auto tmp0 = gen(0);
    const auto tmp1 = gen(1);
    const auto tmp2 = gen(2);
    const auto tmp3 = gen(3);
    const auto tmp4 = gen(4);
    const auto tmp5 = gen(5);
    const auto tmp6 = gen(6);
    const auto tmp7 = gen(7);
    return _mm_setr_epi16(tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7);
}
template <> template <typename G> Vc_INTRINSIC AVX2::ushort_v AVX2::ushort_v::generate(G gen)
{
    const auto tmp0 = gen(0);
    const auto tmp1 = gen(1);
    const auto tmp2 = gen(2);
    const auto tmp3 = gen(3);
    const auto tmp4 = gen(4);
    const auto tmp5 = gen(5);
    const auto tmp6 = gen(6);
    const auto tmp7 = gen(7);
    return _mm_setr_epi16(tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7);
}
// }}}1
// reversed {{{1
template <> Vc_INTRINSIC Vc_PURE AVX2::double_v AVX2::double_v::reversed() const
{
    return Mem::permute128<X1, X0>(Mem::permute<X1, X0, X3, X2>(d.v()));
}
template <> Vc_INTRINSIC Vc_PURE AVX2::float_v AVX2::float_v::reversed() const
{
    return Mem::permute128<X1, X0>(Mem::permute<X3, X2, X1, X0>(d.v()));
}
#ifdef VC_IMPL_AVX2
template <> Vc_INTRINSIC Vc_PURE AVX2::int_v AVX2::int_v::reversed() const
{
    return Mem::permute128<X1, X0>(Mem::permute<X3, X2, X1, X0>(d.v()));
}
template <> Vc_INTRINSIC Vc_PURE AVX2::uint_v AVX2::uint_v::reversed() const
{
    return Mem::permute128<X1, X0>(Mem::permute<X3, X2, X1, X0>(d.v()));
}
#else
template <> Vc_INTRINSIC Vc_PURE AVX2::int_v AVX2::int_v::reversed() const
{
    return Mem::permute<X3, X2, X1, X0>(d.v());
}
template <> Vc_INTRINSIC Vc_PURE AVX2::uint_v AVX2::uint_v::reversed() const
{
    return Mem::permute<X3, X2, X1, X0>(d.v());
}
template <> Vc_INTRINSIC Vc_PURE AVX2::short_v AVX2::short_v::reversed() const
{
    return AVX::avx_cast<__m128i>(Mem::shuffle<X1, Y0>(
        AVX::avx_cast<__m128d>(Mem::permuteHi<X7, X6, X5, X4>(d.v())),
        AVX::avx_cast<__m128d>(Mem::permuteLo<X3, X2, X1, X0>(d.v()))));
}
template <> Vc_INTRINSIC Vc_PURE AVX2::ushort_v AVX2::ushort_v::reversed() const
{
    return AVX::avx_cast<__m128i>(Mem::shuffle<X1, Y0>(
        AVX::avx_cast<__m128d>(Mem::permuteHi<X7, X6, X5, X4>(d.v())),
        AVX::avx_cast<__m128d>(Mem::permuteLo<X3, X2, X1, X0>(d.v()))));
}
#endif
// permutation via operator[] {{{1
template <> Vc_INTRINSIC AVX2::float_v AVX2::float_v::operator[](const IndexType &/*perm*/) const
{
    // TODO
    return *this;
#ifdef VC_IMPL_AVX2
#else
    /*
    const int_m cross128 = AVX::concat(_mm_cmpgt_epi32(AVX::lo128(perm.data()), _mm_set1_epi32(3)),
                                  _mm_cmplt_epi32(AVX::hi128(perm.data()), _mm_set1_epi32(4)));
    if (cross128.isNotEmpty()) {
    AVX2::float_v x = _mm256_permutevar_ps(d.v(), perm.data());
        x(cross128) = _mm256_permutevar_ps(Mem::permute128<X1, X0>(d.v()), perm.data());
        return x;
    } else {
    */
#endif
}
// broadcast from constexpr index {{{1
template <> template <int Index> Vc_INTRINSIC AVX2::float_v AVX2::float_v::broadcast() const
{
    constexpr VecPos Inner = static_cast<VecPos>(Index & 0x3);
    constexpr VecPos Outer = static_cast<VecPos>((Index & 0x4) / 4);
    return Mem::permute<Inner, Inner, Inner, Inner>(Mem::permute128<Outer, Outer>(d.v()));
}
template <> template <int Index> Vc_INTRINSIC AVX2::double_v AVX2::double_v::broadcast() const
{
    constexpr VecPos Inner = static_cast<VecPos>(Index & 0x1);
    constexpr VecPos Outer = static_cast<VecPos>((Index & 0x2) / 2);
    return Mem::permute<Inner, Inner>(Mem::permute128<Outer, Outer>(d.v()));
}
// }}}1
}  // namespace Vc

#include "undomacros.h"

// vim: foldmethod=marker
