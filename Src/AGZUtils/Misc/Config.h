#pragma once

// Comment this out to disable SSE-accelerated classes like F32x4
#define AGZ_USE_SSE2

// Comment this out to disable AVX-accelerated classed like D64x4
//#define AGZ_USE_AVX

// Comment this out to disable thread safety of AGZ::String
#define AGZ_THREAD_SAFE_STRING

// Comment this to enable SSO optimization for string storage
//#define AGZ_ENABLE_STRING_SSO
