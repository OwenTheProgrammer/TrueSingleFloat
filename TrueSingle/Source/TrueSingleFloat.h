#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <intrin.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _MSC_VER
	#define STRUCT_ALIGNED __pragma(pack(push, 1))
	#define ALIGN_END __pragma(pack(pop))
#elif defined(__GNUC__)
	#define STRUCT_ALIGNED __attribute__((packed))
	#define ALIGN_END 
#endif

#if defined(WIN32)||defined(_WIN32)||defined(__WIN32)&&!defined(__CYGWIN__)
	#define VC_EXTRALEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#ifdef VS_BUILD
	#define TSF_API __declspec(dllexport)
#else
	#define TSF_API __declspec(dllimport)
#endif

#define BITMASK(x) ((1<<(x))-1)

typedef enum EncoderType {
	F8_E0, F8_E1,
	F8_E2, F8_E3,
	F8_E4, F8_E5,
	F8_E6, F8_E7,
	ZERO,
	F16_E2, F16_E3,
	F16_E4, F16_E5,
	F16_E6, F16_E7,
	F32
} EncoderType_t;
typedef enum ValueType {
	vt_F8,
	vt_F16
} ValueType_t;
typedef union {
	float Single;
	uint32_t Memory;
} FloatBinding;
STRUCT_ALIGNED
typedef struct {
	EncoderType_t Type;
	uint8_t Memory;
} Float8;
ALIGN_END
STRUCT_ALIGNED
typedef struct {
	EncoderType_t Type;
	uint16_t Memory;
} Float16;
ALIGN_END

static const inline float EncoderError(const float x, const float y) {
	FloatBinding Diff = {x-y};
	Diff.Memory &= ~(1<<31);
	return Diff.Single;
}
static const inline uint8_t BitStride(const uint32_t x) {
	unsigned long pos;
	return (uint8_t)(_BitScanReverse(&pos, x) + pos);
}
static const inline uint8_t Absolute(const uint8_t x, uint8_t* s) {
	*s = x>>7;
	return (x^((*s)*0xFF))+(*s);
}

TSF_API const Float8* CreateFloat8();
TSF_API const Float16* CreateFloat16();
TSF_API void DestroyInstance(void* Instance);
TSF_API uint8_t GetMemory_F8(const Float8* Instance);
TSF_API uint16_t GetMemory_F16(const Float16* Instance);
TSF_API EncoderType_t GetExponent_F8(const Float8* Instance);
TSF_API EncoderType_t GetExponent_F16(const Float16* Instance);
TSF_API void SetMemory_F8(Float8* Instance, const uint8_t value);
TSF_API void SetMemory_F16(Float16* Instance, const uint16_t value);
TSF_API void SetExponent_F8(Float8* Instance, const uint8_t value);
TSF_API void SetExponent_F16(Float16* Instance, const uint8_t value);
TSF_API const void EncodeFloat8(Float8* Instance, const float Value);
TSF_API const void EncodeFloat16(Float16* Instance, const float Value);
TSF_API const float ToSingleF8(const Float8* Instance);
TSF_API const float ToSingleF16(const Float16* Instance);

#ifdef __cplusplus
}
#endif