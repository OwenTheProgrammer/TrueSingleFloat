#include "TrueSingleFloat.h"

const Float8* CreateFloat8() {
	Float8* Instance = (Float8*)malloc(sizeof(Float8));
	Instance->Type = ZERO;
	Instance->Memory = 0u;
	return Instance;
}
const Float16* CreateFloat16() {
	Float16* Instance = (Float16*)malloc(sizeof(Float16));
	Instance->Type = ZERO;
	Instance->Memory = 0u;
	return Instance;
}
void DestroyInstance(void* Instance) {
	free(Instance);
}

static const void EncodeGeneric(void* Instance, const float Value, const ValueType_t vType) {
	uint32_t Memory = *(uint32_t*)&Value;
	uint32_t Mantissa = Memory & 0x7FFFFF;
	uint8_t m_Exponent = (uint8_t)(Memory>>23);
	uint8_t m_Sign = (uint8_t)(Memory>>31);

	//Calculate the min exponent
	uint8_t Negative = 0;
	uint8_t u_exp = Absolute(m_Exponent-127, &Negative);
	//uint8_t ExpBits = BitStride(u_exp)+1;
	uint8_t ExpBits = 0;

	EncoderType_t InstType = *(EncoderType_t*)Instance;
	if(InstType == ZERO) {
		ExpBits = BitStride(u_exp)+1;
		//Min exponent of 2 for Float16
		if(vType==vt_F16 && ExpBits<2)
			ExpBits = 2;
	} else {
		ExpBits = ((InstType&7u)+vType);
	}

	//Add signature to exp if present before
	uint8_t ExpMask = BITMASK(ExpBits)*Negative;
	uint8_t ExpValue = (u_exp^ExpMask)+Negative;

	//Truncate the mantissa
	uint8_t vStride = (8*(vType+1))-1;
	uint8_t MntBits = vStride-ExpBits;
	uint16_t Mnt = Mantissa>>(23-MntBits);

	//Construct the buffer
	uint16_t b_Memory = ((m_Sign<<vStride)|(ExpValue<<MntBits)|Mnt);
	if(vType == vt_F8) {
		Float8* ptr = (Float8*)Instance;
		ptr->Type = (EncoderType_t)ExpBits;
		ptr->Memory = (uint8_t)b_Memory;
	} else {
		Float16* ptr = (Float16*)Instance;
		ptr->Type = (EncoderType_t)(8|(ExpBits-1));
		ptr->Memory = b_Memory;
	}
}
static const float ToSingleGeneral(const Float16 Instance, const ValueType_t vType) {
	uint8_t InstExp = (Instance.Type&7u)+vType;
	uint8_t vStride = (8*(vType+1))-1;
	uint8_t MntBits = vStride-InstExp;

	//Extrapolate the exponent
	uint8_t ExpMask = BITMASK(InstExp);
	uint8_t Exponent = (Instance.Memory>>MntBits)&ExpMask;
	uint8_t ExpNeg = Exponent>>(InstExp-1);
	Exponent |= (~ExpMask)*ExpNeg;
	Exponent += 127u;

	//Mask out mantissa
	uint16_t MntMask = BITMASK(MntBits);
	uint32_t Mantissa = (Instance.Memory&MntMask);

	FloatBinding Buffer;
	Buffer.Memory = (Instance.Memory>>vStride)<<31; //Sign
	Buffer.Memory |= (Exponent<<23); //Exponent
	Buffer.Memory |= Mantissa<<(23-MntBits); //Mantissa
	return Buffer.Single;
}

uint8_t GetMemory_F8(const Float8* Instance) {
	return Instance->Memory;
}
uint16_t GetMemory_F16(const Float16* Instance) {
	return Instance->Memory;
}
EncoderType_t GetExponent_F8(const Float8* Instance) {
	return Instance->Type;
}
EncoderType_t GetExponent_F16(const Float16* Instance) {
	return Instance->Type;
}

void SetMemory_F8(Float8* Instance, const uint8_t value) {
	Instance->Memory = value;
}
void SetMemory_F16(Float16* Instance, const uint16_t value) {
	Instance->Memory = value;
}
void SetExponent_F8(Float8* Instance, const uint8_t value) {
	Instance->Type = (EncoderType_t)value;
}
void SetExponent_F16(Float16* Instance, const uint8_t value) {
	Instance->Type = (EncoderType_t)(value+7);
}

const void EncodeFloat8(Float8* Instance, const float Value) {
	EncodeGeneric(Instance, Value, vt_F8);
}
const void EncodeFloat16(Float16* Instance, const float Value) {
	EncodeGeneric(Instance, Value, vt_F16);
}
const float ToSingleF8(const Float8* Instance) {
	Float16 Handover;
	Handover.Type = Instance->Type;
	Handover.Memory = Instance->Memory;
	return ToSingleGeneral(Handover, vt_F8);
}
const float ToSingleF16(const Float16* Instance) {
	return ToSingleGeneral(*Instance, vt_F16);
}