#pragma once

#include <stdint.h>
#include <stdlib.h>

#if defined(WIN32)||defined(_WIN32)||defined(__WIN32)&&!defined(__CYGWIN__)
	#define VC_EXTRALEAN
	#define WIN32_LEAN_AND_MEAN
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef VS_BUILD
	#define DLL_EXPORT __declspec(dllexport)
#else
	#define DLL_EXPORT __declspec(dllimport)
#endif

#define BITMASK(x) ((1<<(x))-1)

typedef union {
	uint16_t Register;
	uint8_t Half[2];
} RegisterHalf16;
typedef union {
	uint64_t Register;
	uint32_t Half[2];
} RegisterHalf64;
typedef union {
	float Single;
	uint32_t Memory;
} FloatBinding;

typedef struct {
	uint8_t Memory;
	uint8_t Exponent;
} Float8;


//Exports
DLL_EXPORT const void Encode(Float8* Instance, const float Value) {
	uint32_t ValueMemory = *(uint32_t*)&Value;
	if(!(ValueMemory<<1)) {
		Instance->Memory = 0u;
		return;
	}
	//Truncate the exponent space
	uint8_t ExpMask = BITMASK(Instance->Exponent-1);
	RegisterHalf16 ExpReg = {ValueMemory>>23};
	uint16_t ValueExp = ExpReg.Half[0];
	uint8_t Exponent = (ValueExp&ExpMask);
	Exponent |= ((ValueExp>>(8- Instance->Exponent))&(ExpMask+1));

	uint8_t MntBits = 7-Instance->Exponent;
	uint8_t MntMask = BITMASK(MntBits);
	uint8_t Memory = (ExpReg.Half[1]<<7); //Sign
	Memory |= (Exponent<<MntBits); //Exponent
	Memory |= ((ValueMemory>>(23-MntBits))&MntMask); //Mantissa
	Instance->Memory = Memory;
}
DLL_EXPORT Float8* CreateFloat8(const float Value, const uint8_t ExpBits) {
	//If the exponent is of useless value
	Float8* Buffer = (Float8*)malloc(sizeof(Float8));
	Buffer->Memory = 0u;
	Buffer->Exponent = UINT8_MAX;
	if(!ExpBits || ExpBits>5)
		return Buffer;

	Buffer->Memory = 0u;
	Buffer->Exponent = ExpBits;

	Encode(Buffer, Value);
	return Buffer;
}
DLL_EXPORT const void DestroyFloat8(Float8* Instance) {
	if(Instance)
		free(Instance);
}

DLL_EXPORT const float ToSingle(const Float8* Instance) {
	Float8 inst = *Instance;
	if(!(~inst.Exponent)) //If the value errored
		return (float){0x7FFFFFFFul};

	RegisterHalf16 ValueSign = {inst.Memory<<1u};
	if(!ValueSign.Half[0]) //If the value is zero
		return 0.0f;

	uint8_t MntBits = 7-inst.Exponent;
	uint8_t ExpMask = BITMASK(inst.Exponent);

	//Extrapolate exponent
	uint8_t Exponent = (inst.Memory>>MntBits)&ExpMask;
	Exponent += (127-(ExpMask>>1));

	//Mask out mantissa
	uint8_t MntMask = BITMASK(MntBits);
	uint32_t Mantissa = (inst.Memory&MntMask);

	//Construct the float
	FloatBinding Buffer = {0.0f};
	Buffer.Memory = ValueSign.Half[1]<<31; //Sign
	Buffer.Memory |= Exponent<<23; //Exponent
	Buffer.Memory |= Mantissa<<(23-MntBits); //Mantissa
	return Buffer.Single;
}
DLL_EXPORT const void ChangeExponent(const Float8* Instance, const uint8_t NewExp) {
	Float8 inst = *Instance;
	uint8_t Exp = inst.Exponent;
	uint8_t MntBits = 7-Exp;
	uint8_t N_MntBits = 7-NewExp;

	uint8_t N_Mnt, N_Exp;
	if(NewExp > Exp) {
		//Truncate the mantissa
		uint8_t Diff = (NewExp-Exp);
		uint8_t MntMask = BITMASK(N_MntBits);
		N_Mnt = (inst.Memory>>Diff)&MntMask;

		//Extrapolate the exponent
		uint8_t ExpMask = BITMASK(Exp-1);
		N_Exp = (inst.Memory>>MntBits)&ExpMask;
		N_Exp += (1<<Exp);
	} else {
		//Extrapolate the mantissa
		uint8_t Diff = (Exp-NewExp);
		uint8_t MntMask = BITMASK(MntBits);
		N_Mnt = (inst.Memory&MntMask);
		N_Mnt <<= Diff;

		//Truncate the exponent
		uint8_t ExpMask = BITMASK(Exp);
		uint8_t DiffMask = BITMASK(Diff);
		N_Exp = (inst.Memory>>MntBits)&ExpMask;
		N_Exp -= (DiffMask<<(Exp-2));
	}
	//Construct the new values
	uint8_t NewValue = (N_Exp<<N_MntBits)|N_Mnt;
	inst.Memory = NewValue|(inst.Memory&(1<<7));
	inst.Exponent = NewExp;
	return;
}
DLL_EXPORT const float GetSerialError(const Float8* Instance, const float v) {
	float value = ToSingle(Instance);
	FloatBinding Diff = {value-v};
	Diff.Memory &= 0x7FFFFFFFul;
	return Diff.Single;
}

DLL_EXPORT const uint8_t GetMemory(const Float8* Instance) {
	return Instance->Memory;
}
DLL_EXPORT const uint8_t GetExponent(const Float8* Instance) {
	return Instance->Exponent;
}

DLL_EXPORT const void SetMemory(Float8* Instance, const uint8_t value) {
	Instance->Memory = value;
}
DLL_EXPORT const void SetExponent(Float8* Instance, const uint8_t value) {
	Instance->Exponent = value;
}

#ifdef __cplusplus
}
#endif