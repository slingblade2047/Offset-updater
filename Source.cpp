#include <thread>
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

DWORD GetSigOffset(HANDLE pHandle, DWORD64 mBase, DWORD mSize, BYTE* Sig, const char* Mask, int Len);
bool RPM(HANDLE pHandle, DWORD64 src, void* dst, size_t Size);
bool WPM(HANDLE pHandle, DWORD64 dst, void* src, size_t Size);
DWORD64 AllocEx(HANDLE pHandle, size_t Size);
bool FreeEx(HANDLE pHandle, DWORD64 src);
bool Valid(DWORD64 ptr);

class ProcessData
{
public:
	CHAR* pName;
	CHAR* mName;
	BYTE* mBase;
	DWORD mSize;
	DWORD pPid;
	HANDLE pHandle;
};

const char Version[] = "1.3.0.23";

DWORD LocalPlayerOffset = 0;
BYTE LocalPlayerSig[] = { 0x48, 0x89, 0x05, 0x00, 0x00, 0x00, 0x00, 0xF3, 0x0F, 0x7F, 0x05 };
const char LocalPlayerMask[sizeof(LocalPlayerSig) + 1] = "xxx????xxxx";

DWORD EntityListOffset = 0;
BYTE EntityListSig[] = { 0x44, 0x88, 0x44, 0x24, 0x18, 0x48, 0x83, 0xEC, 0x48, 0x4C, 0x8B, 0xCA, 0x48, 0x85, 0xD2 };
const char EntityListMask[sizeof(EntityListSig) + 1] = "xxxxxxxxxxxxxxx";

DWORD FalloutMainOffset = 0;
BYTE FalloutMainSig[] = { 0x74, 0x4D, 0x48, 0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x48, 0x85, 0xC9, 0x74, 0x41 };
const char FalloutMainMask[sizeof(FalloutMainSig) + 1] = "xxxxx????xxxxx";

DWORD CameraOffset = 0;
BYTE CameraSig[] = { 0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0xF3, 0x0F, 0x10, 0x80, 0x80, 0x00, 0x00, 0x00 };
const char CameraMask[sizeof(CameraSig) + 1] = "xxx????xxxxxxxx";

DWORD ControllerOffset = 0;
BYTE ControllerSig[] = { 0x48, 0x83, 0xEC, 0x28, 0x4C, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0xBA, 0x10, 0x00, 0x00, 0x00 };
const char ControllerMask[sizeof(ControllerSig) + 1] = "xxxxxxx????xxxxx";

DWORD GetPtrA1Offset = 0;
BYTE GetPtrA1Sig[] = { 0x90, 0x89, 0x5C, 0x24, 0x40, 0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x85, 0xC0 };
const char GetPtrA1Mask[sizeof(GetPtrA1Sig) + 1] = "xxxxxxxx????xxx";

DWORD GetPtrA2Offset = 0;
BYTE GetPtrA2Sig[] = { 0x41, 0x8B, 0xC0, 0x4C, 0x8D, 0x1D, 0x00, 0x00, 0x00, 0x00, 0x41, 0xC1, 0xE8, 0x08 };
const char GetPtrA2Mask[sizeof(GetPtrA2Sig) + 1] = "xxxxxx????xxxx";

DWORD MessageOffset = 0;
BYTE MessageSig[] = { 0x48, 0x8D, 0x54, 0x24, 0x48, 0x48, 0x8D, 0x4D, 0x08, 0xE8, 0x00, 0x00, 0x00, 0x00, 0x90, 0x48, 0x8D, 0x4D, 0x08 };
const char MessageMask[sizeof(MessageSig) + 1] = "xxxxxxxxxx????xxxxx";

DWORD HandlerOffset = 0;
BYTE HandlerSig[] = { 0x75, 0xC8, 0x48, 0x8B, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x8D, 0x44, 0x24, 0x50 };
const char HandlerMask[sizeof(HandlerSig) + 1] = "xxxxx????xxxxx";

DWORD InfiniteAmmoOffset = 0;
BYTE InfiniteAmmoSig[] = { 0x8B, 0x45, 0x77, 0x48, 0x81, 0xC4, 0x90, 0x00, 0x00, 0x00, 0x5D, 0xC3 };
const char InfiniteAmmoMask[sizeof(InfiniteAmmoSig) + 1] = "xxxxxxxxxxxx";

DWORD RedirectionOffset = 0;
BYTE RedirectionSig[] = { 0x48, 0x8B, 0x7C, 0x24, 0x50, 0x48, 0x85, 0xFF, 0x0F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x49, 0x8B, 0xCF };
const char RedirectionMask[sizeof(RedirectionSig) + 1] = "xxxxxxxxxx????xxx";

DWORD RedirectionJmpOffset = 0;

DWORD RedirectionRetOffset = 0;

DWORD NoclipOffsetA = 0;
BYTE NoclipSigA[] = { 0x48, 0x83, 0xC1, 0x10, 0x49, 0x89, 0x48, 0x08, 0x48, 0x8B, 0xD5, 0x48, 0x8D, 0x4E, 0x10 };
const char NoclipMaskA[sizeof(NoclipSigA) + 1] = "xxxxxxxxxxxxxxx";

DWORD NoclipOffsetB = 0;
BYTE NoclipSigB[] = { 0x74, 0x69, 0x49, 0x8B, 0xD6, 0x48, 0x8B, 0xC8 };
const char NoclipMaskB[sizeof(NoclipSigB) + 1] = "xxxxxxxx";

DWORD NoclipOffsetC = 0;
DWORD NoclipOffsetD = 0;

DWORD ActorValueOffset = 0;
BYTE ActorValueSig[] = { 0xF3, 0x0F, 0x11, 0x54, 0x24, 0x40, 0x48, 0x89, 0x54, 0x24, 0x38 };
const char ActorValueMask[sizeof(ActorValueSig) + 1] = "xxxxxxxxxxx";

DWORD EnemyPositionBeginOffset = 0;
BYTE EnemyPositionBeginSig[] = { 0x48, 0x83, 0xC2, 0x40, 0x0F, 0x29, 0x44, 0x24, 0x40 };
const char EnemyPositionBeginMask[sizeof(EnemyPositionBeginSig) + 1] = "xxxxxxxxx";

DWORD EnemyPositionEndOffset = 0;

DWORD ActorValueRegenOffset = 0;
BYTE ActorValueRegenSig[] = { 0x0F, 0x57, 0xC0, 0x48, 0x8B, 0x5C, 0x24, 0x30, 0x48, 0x8B, 0x74, 0x24, 0x38, 0x48, 0x83, 0xC4, 0x20, 0x5F, 0xC3 };
const char ActorValueRegenMask[sizeof(ActorValueRegenSig) + 1] = "xxxxxxxxxxxxxxxxxxx";

DWORD ServerPositionOffset = 0;
BYTE ServerPositionSig[] = { 0x3B, 0xC2, 0x7C, 0x0F, 0x41, 0xB8, 0xFF, 0xFF, 0x07, 0x00 };
const char ServerPositionMask[sizeof(ServerPositionSig) + 1] = "xxxxxxxxxx";

DWORD EntityIdOffset = 0;
BYTE EntityIdSig[] = { 0x48, 0x8D, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x41, 0xD1, 0xE8 };
const char EntityIdMask[sizeof(EntityIdSig) + 1] = "xxx????xxx";

DWORD NukeCodeOffset = 0;
BYTE NukeCodeSig[] = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x83, 0xC4, 0x38, 0xC3 };
const char NukeCodeMask[sizeof(NukeCodeSig) + 1] = "x????xxx????xxxxx";

DWORD VTABLE_TESNPC = 0;
DWORD VTABLE_TESOBJECTCONT = 0;
DWORD VTABLE_TESOBJECTMISC = 0;
DWORD VTABLE_TESOBJECTBOOK = 0;
DWORD VTABLE_ALCHEMYITEM = 0;
DWORD VTABLE_TESAMMO = 0;
DWORD VTABLE_TESOBJECTWEAP = 0;
DWORD VTABLE_TESOBJECTARMO = 0;
DWORD VTABLE_TESUTILITYITEM = 0;
DWORD VTABLE_BGSNOTE = 0;
DWORD VTABLE_TESKEY = 0;
DWORD VTABLE_TESFLORA = 0;
DWORD VTABLE_TESLEVITEM = 0;
DWORD VTABLE_CURRENCYOBJECT = 0;

DWORD RequestActivateOffset = 0;
BYTE RequestActivateSig[] = { 0x40, 0x84, 0xFF, 0x74, 0x29, 0x41, 0x8B, 0x4F, 0x20 };
const char RequestActivateMask[sizeof(RequestActivateSig) + 1] = "xxxxxxxxx";

DWORD RequestTransferOffset = 0;
BYTE RequestTransferSig[] = { 0x44, 0x8B, 0x00, 0x48, 0x8B, 0x47, 0x18, 0x8B, 0x08 };
const char RequestTransferMask[sizeof(RequestTransferSig) + 1] = "xxxxxxxxx";

DWORD RequestTeleportOffset = 0;
BYTE RequestTeleportSig[] = { 0x74, 0x06, 0xF6, 0x43, 0x38, 0x01, 0x75, 0x02, 0x33, 0xDB };
const char RequestTeleportMask[sizeof(RequestTeleportSig) + 1] = "xxxxxxxxxx";

DWORD ClientStateOffset = 0;
BYTE ClientStateSig[] = { 0x48, 0x8D, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x89, 0x44, 0x24, 0x38, 0x48, 0x8D, 0x4C, 0x24, 0x48 };
const char ClientStateMask[sizeof(ClientStateSig) + 1] = "xxx????xxxxxxxxxx";

DWORD RequestHitsOffset = 0;
BYTE RequestHitsSig[] = { 0x4C, 0x8D, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x89, 0x7D, 0x90, 0x4C, 0x89, 0x5D, 0x98 };
const char RequestHitsMask[sizeof(RequestHitsSig) + 1] = "xxx????xxxxxxxx";

DWORD UnknownTransferId = 0;

BYTE NoclipCallA[] = { 0x00, 0x00, 0x00, 0x00, 0x00 };

BYTE NoclipCallB[] = { 0x00, 0x00, 0x00, 0x00, 0x00 };

DWORD64 rttiGetPtr(HANDLE pHandle, DWORD64 mBase, DWORD mSize, DWORD64 src, bool ExtraRead)
{
	DWORD64 Address = src;

	if (ExtraRead)
	{
		DWORD64 BufferA;
		if (!RPM(pHandle, src, &BufferA, sizeof(BufferA))) return 0;
		if (!Valid(BufferA)) return 0;

		DWORD64 BufferB;
		if (!RPM(pHandle, BufferA, &BufferB, sizeof(BufferB))) return 0;
		if (!Valid(BufferB)) return 0;

		Address = BufferB;
	}

	DWORD64 Buffer;
	if (!RPM(pHandle, Address - 0x8, &Buffer, sizeof(Buffer))) return 0;
	if (!Valid(Buffer)) return 0;

	DWORD Offset;
	if (!RPM(pHandle, Buffer + 0xC, &Offset, sizeof(Offset))) return 0;
	if (Offset == 0 || Offset > mSize) return 0;

	return mBase + Offset + 0x10;
}

bool rttiValid(HANDLE pHandle, DWORD64 mBase, DWORD mSize, DWORD SigOffset, DWORD Offset, bool Negative, bool ExtraRead, const char* rtti, DWORD* dst)
{
	DWORD Buffer = 0;
	if (Negative)
	{
		if (!RPM(pHandle, mBase + SigOffset - Offset, &Buffer, sizeof(Buffer))) return false;
		Buffer += (SigOffset + 0x4 - Offset);
	}
	else
	{
		if (!RPM(pHandle, mBase + SigOffset + Offset, &Buffer, sizeof(Buffer))) return false;
		Buffer += (SigOffset + 0x4 + Offset);
	}

	DWORD64 rttiPtr = rttiGetPtr(pHandle, mBase, mSize, mBase + Buffer, ExtraRead);
	if (!rttiPtr) return false;

	char rttiCheck[256];
	if (!RPM(pHandle, rttiPtr, &rttiCheck, sizeof(rttiCheck))) return false;
	if (strcmp(rttiCheck, rtti)) return false;

	*dst = Buffer;
	return true;
}

DWORD rttiGetSigOffset(HANDLE pHandle, DWORD64 mBase, DWORD mSize, BYTE* Sig, const char* Mask, int Len, DWORD Offset, bool Negative, bool ExtraRead, bool ReturnSigOffset, const char* rtti)
{
	DWORD SigOffset = 0;
	BYTE* ModuleArray = new BYTE[mSize];

	if (ReadProcessMemory(pHandle, (void*)(mBase), &*ModuleArray, mSize, NULL))
	{
		for (DWORD i = 0; i < mSize; i++)
		{
			if (i + Len >= mSize) break;
			for (int c = 0; c < Len; c++)
			{
				if (ModuleArray[i + c] != Sig[c] && Mask[c] != '?') break;
				if (c == Len - 1)
				{
					DWORD Buffer = 0;
					if (rttiValid(pHandle, mBase, mSize, i, Offset, Negative, ExtraRead, rtti, &Buffer))
					{
						delete[]ModuleArray;
						if (ReturnSigOffset)
						{
							return i;
						}
						else
						{
							return Buffer;
						}
					}
				}
			}
		}
	}

	delete[]ModuleArray;
	return SigOffset;
}

DWORD GetMultiSigOffset(HANDLE pHandle, DWORD64 mBase, DWORD mSize, BYTE* Sig, const char* Mask, int Len, int ResultNumber)
{
	DWORD SigOffset = 0;
	BYTE* ModuleArray = new BYTE[mSize];

	int ResultCounter = 0;
	if (ReadProcessMemory(pHandle, (void*)(mBase), &*ModuleArray, mSize, NULL))
	{
		for (DWORD i = 0; i < mSize; i++)
		{
			if (i + Len >= mSize) break;
			for (int c = 0; c < Len; c++)
			{
				if (ModuleArray[i + c] != Sig[c] && Mask[c] != '?') break;
				if (c == Len - 1)
				{
					ResultCounter++;
					if (ResultCounter == ResultNumber)
					{
						delete[]ModuleArray;
						return i;
					}
				}
			}
		}
	}

	delete[]ModuleArray;
	return SigOffset;
}

DWORD64 GetAddress(HANDLE pHandle, DWORD64 mBase, DWORD Formid)
{
	DWORD64 v1;
	if (!RPM(pHandle, mBase + GetPtrA1Offset, &v1, sizeof(v1))) return 0;
	if (!Valid(v1)) return 0;

	DWORD v2;
	if (!RPM(pHandle, v1 + 0x8 + 0x18, &v2, sizeof(v2))) return 0;
	if (v2 == 0) return 0;

	DWORD v3 = 0;

	for (int i = 0; i < sizeof(Formid); i++)
	{
		DWORD v4 = ((v3 ^ (Formid >> (i * 0x8))) & 0xFF);

		DWORD v5;
		if (!RPM(pHandle, mBase + GetPtrA2Offset + v4 * 0x4, &v5, sizeof(v5))) return 0;

		v3 = ((v3 >> 0x8) ^ v5);
	}

	DWORD v6 = (v3 & (v2 - 1));

	DWORD64 v7;
	if (!RPM(pHandle, v1 + 0x8 + 0x10, &v7, sizeof(v7))) return 0;
	if (!Valid(v7)) return 0;

	DWORD v8;
	if (!RPM(pHandle, v7 + (v6 + v6 * 2) * 0x8 + 0x10, &v8, sizeof(v8))) return 0;
	if (v8 == 0xFFFFFFFF) return 0;

	DWORD v9 = v2;

	for (int i = 0; i < 100; i++)
	{
		DWORD v10;
		if (!RPM(pHandle, v7 + (v6 + v6 * 2) * 0x8, &v10, sizeof(v10))) return 0;
		if (v10 == Formid)
		{
			v9 = v6;
			if (v9 != v2) break;
		}
		else
		{
			if (!RPM(pHandle, v7 + (v6 + v6 * 2) * 0x8 + 0x10, &v6, sizeof(v6))) return 0;
			if (v6 == v2) break;
		}
	}

	if (v9 == v2) return 0;

	return v7 + (v9 + v9 * 2) * 0x8 + 0x8;
}

DWORD64 GetPtr(HANDLE pHandle, DWORD64 mBase, DWORD Formid)
{
	DWORD64 Address = GetAddress(pHandle, mBase, Formid);
	if (Address == 0) return 0;

	DWORD64 Ptr;
	if (!RPM(pHandle, Address, &Ptr, sizeof(Ptr))) return 0;

	return Ptr;
}

DWORD vtableReference(HANDLE pHandle, DWORD64 mBase, DWORD Formid)
{
	DWORD64 Buffer = GetPtr(pHandle, mBase, Formid);
	if (!Buffer) return 0;

	DWORD64 vtable;
	if (!RPM(pHandle, Buffer, &vtable, sizeof(vtable))) return 0;
	if (vtable < mBase) return 0;

	return DWORD(vtable - mBase);
}

void GetOffsets(HANDLE pHandle, DWORD64 mBase, DWORD mSize)
{
	DWORD LocalPlayerBuffer = GetSigOffset(pHandle, mBase, mSize, LocalPlayerSig, LocalPlayerMask, sizeof(LocalPlayerSig));
	if (LocalPlayerBuffer > 0 && RPM(pHandle, mBase + LocalPlayerBuffer + 0xB, &LocalPlayerOffset, sizeof(LocalPlayerOffset)))
	{
		LocalPlayerOffset += LocalPlayerBuffer + 0xF;
	}
	else
	{
		LocalPlayerOffset = 0;
	}

	DWORD EntityListBuffer = GetSigOffset(pHandle, mBase, mSize, EntityListSig, EntityListMask, sizeof(EntityListSig));
	if (EntityListBuffer > 0 && RPM(pHandle, mBase + EntityListBuffer + 0x67, &EntityListOffset, sizeof(EntityListOffset)))
	{
		EntityListOffset += EntityListBuffer + 0x6B;
	}
	else
	{
		EntityListOffset = 0;
	}

	DWORD FalloutMainBuffer = GetSigOffset(pHandle, mBase, mSize, FalloutMainSig, FalloutMainMask, sizeof(FalloutMainSig));
	if (FalloutMainBuffer > 0 && RPM(pHandle, mBase + FalloutMainBuffer + 0x5, &FalloutMainOffset, sizeof(FalloutMainOffset)))
	{
		FalloutMainOffset += FalloutMainBuffer + 0x9;
	}
	else
	{
		FalloutMainOffset = 0;
	}

	DWORD CameraBuffer = GetSigOffset(pHandle, mBase, mSize, CameraSig, CameraMask, sizeof(CameraSig));
	if (CameraBuffer > 0 && RPM(pHandle, mBase + CameraBuffer + 0x3, &CameraOffset, sizeof(CameraOffset)))
	{
		CameraOffset += CameraBuffer + 0x7;
	}
	else
	{
		CameraOffset = 0;
	}

	DWORD ControllerBuffer = rttiGetSigOffset(pHandle, mBase, mSize, ControllerSig, ControllerMask, sizeof(ControllerSig), 0x7, false, true, false, ".?AVbhkCharProxyController@@");
	if (ControllerBuffer > 0)
	{
		ControllerOffset = ControllerBuffer;
	}
	else
	{
		DWORD ControllerBuffer = GetMultiSigOffset(pHandle, mBase, mSize, ControllerSig, ControllerMask, sizeof(ControllerSig), 9);
		if (ControllerBuffer > 0 && RPM(pHandle, mBase + ControllerBuffer + 0x7, &ControllerOffset, sizeof(ControllerOffset)))
		{
			ControllerOffset += ControllerBuffer + 0xB;
		}
	}

	DWORD GetPtrA1Buffer = GetSigOffset(pHandle, mBase, mSize, GetPtrA1Sig, GetPtrA1Mask, sizeof(GetPtrA1Sig));
	if (GetPtrA1Buffer > 0 && RPM(pHandle, mBase + GetPtrA1Buffer + 0x8, &GetPtrA1Offset, sizeof(GetPtrA1Offset)))
	{
		GetPtrA1Offset += GetPtrA1Buffer + 0xC;
	}
	else
	{
		GetPtrA1Offset = 0;
	}

	DWORD GetPtrA2Buffer = GetSigOffset(pHandle, mBase, mSize, GetPtrA2Sig, GetPtrA2Mask, sizeof(GetPtrA2Sig));
	if (GetPtrA2Buffer > 0 && RPM(pHandle, mBase + GetPtrA2Buffer + 0x6, &GetPtrA2Offset, sizeof(GetPtrA2Offset)))
	{
		GetPtrA2Offset += GetPtrA2Buffer + 0xA;
	}
	else
	{
		GetPtrA2Offset = 0;
	}

	DWORD MessageBuffer = GetSigOffset(pHandle, mBase, mSize, MessageSig, MessageMask, sizeof(MessageSig));
	if (MessageBuffer > 0 && RPM(pHandle, mBase + MessageBuffer + 0x14, &MessageOffset, sizeof(MessageOffset)))
	{
		MessageOffset += MessageBuffer + 0x18;
	}
	else
	{
		MessageOffset = 0;
	}

	DWORD HandlerBuffer = GetSigOffset(pHandle, mBase, mSize, HandlerSig, HandlerMask, sizeof(HandlerSig));
	if (HandlerBuffer > 0 && RPM(pHandle, mBase + HandlerBuffer + 0x5, &HandlerOffset, sizeof(HandlerOffset)))
	{
		HandlerOffset += HandlerBuffer + 0x9;
	}
	else
	{
		HandlerOffset = 0;
	}

	InfiniteAmmoOffset = GetSigOffset(pHandle, mBase, mSize, InfiniteAmmoSig, InfiniteAmmoMask, sizeof(InfiniteAmmoSig));

	RedirectionOffset = GetSigOffset(pHandle, mBase, mSize, RedirectionSig, RedirectionMask, sizeof(RedirectionSig));
	if (RedirectionOffset > 0)
	{
		RedirectionRetOffset = RedirectionOffset + 0x5;
		RedirectionJmpOffset = RedirectionRetOffset + 0xB3D;
	}

	NoclipOffsetA = GetSigOffset(pHandle, mBase, mSize, NoclipSigA, NoclipMaskA, sizeof(NoclipSigA));
	if (NoclipOffsetA > 0)
	{
		NoclipOffsetA += 0xF;
		NoclipOffsetC = NoclipOffsetA - 0x48;
		if (!RPM(pHandle, mBase + NoclipOffsetA, &NoclipCallA, sizeof(NoclipCallA)))
		{
			memset(NoclipCallA, 0x00, sizeof(NoclipCallA));
		}
	}

	NoclipOffsetB = GetSigOffset(pHandle, mBase, mSize, NoclipSigB, NoclipMaskB, sizeof(NoclipSigB));
	if (NoclipOffsetB > 0)
	{
		NoclipOffsetB += 0x8;
		NoclipOffsetD = NoclipOffsetB + 0x3E;
		if (!RPM(pHandle, mBase + NoclipOffsetB, &NoclipCallB, sizeof(NoclipCallB)))
		{
			memset(NoclipCallB, 0x00, sizeof(NoclipCallB));
		}
	}

	ActorValueOffset = GetSigOffset(pHandle, mBase, mSize, ActorValueSig, ActorValueMask, sizeof(ActorValueSig));
	if (ActorValueOffset > 0)
	{
		ActorValueOffset -= 0x1A;
	}

	EnemyPositionBeginOffset = GetSigOffset(pHandle, mBase, mSize, EnemyPositionBeginSig, EnemyPositionBeginMask, sizeof(EnemyPositionBeginSig));
	if (EnemyPositionBeginOffset > 0)
	{
		EnemyPositionBeginOffset += 0xE;
		EnemyPositionEndOffset = EnemyPositionBeginOffset + 0x1B;
	}

	ActorValueRegenOffset = GetSigOffset(pHandle, mBase, mSize, ActorValueRegenSig, ActorValueRegenMask, sizeof(ActorValueRegenSig));
	if (ActorValueRegenOffset > 0)
	{
		ActorValueRegenOffset += 0x13;
	}

	ServerPositionOffset = GetSigOffset(pHandle, mBase, mSize, ServerPositionSig, ServerPositionMask, sizeof(ServerPositionSig));

	DWORD EntityIdBuffer = GetSigOffset(pHandle, mBase, mSize, EntityIdSig, EntityIdMask, sizeof(EntityIdSig));
	if (EntityIdBuffer > 0 && RPM(pHandle, mBase + EntityIdBuffer + 0x3, &EntityIdOffset, sizeof(EntityIdOffset)))
	{
		EntityIdOffset += EntityIdBuffer + 0x7;
	}
	else
	{
		EntityIdOffset = 0;
	}

	VTABLE_TESNPC = vtableReference(pHandle, mBase, 0x003F878E);
	VTABLE_TESOBJECTCONT = vtableReference(pHandle, mBase, 0x003D1014);
	VTABLE_TESOBJECTMISC = vtableReference(pHandle, mBase, 0x0007E941);
	VTABLE_TESOBJECTBOOK = vtableReference(pHandle, mBase, 0x003CFB3B);
	VTABLE_ALCHEMYITEM = vtableReference(pHandle, mBase, 0x000459C5);
	VTABLE_TESAMMO = vtableReference(pHandle, mBase, 0x001025AA);
	VTABLE_TESOBJECTWEAP = vtableReference(pHandle, mBase, 0x000C2C27);
	VTABLE_TESOBJECTARMO = vtableReference(pHandle, mBase, 0x003F32BB);
	VTABLE_TESUTILITYITEM = vtableReference(pHandle, mBase, 0x0041ADEC);
	VTABLE_BGSNOTE = vtableReference(pHandle, mBase, 0x003C7B0E);
	VTABLE_TESKEY = vtableReference(pHandle, mBase, 0x00295C58);
	VTABLE_TESFLORA = vtableReference(pHandle, mBase, 0x0023D5C2);
	VTABLE_TESLEVITEM = vtableReference(pHandle, mBase, 0x004FD294);
	VTABLE_CURRENCYOBJECT = vtableReference(pHandle, mBase, 0x0000000F);

	DWORD RequestActivateBuffer = GetSigOffset(pHandle, mBase, mSize, RequestActivateSig, RequestActivateMask, sizeof(RequestActivateSig));
	if (RequestActivateBuffer > 0 && RPM(pHandle, mBase + RequestActivateBuffer + 0xC, &RequestActivateOffset, sizeof(RequestActivateOffset)))
	{
		RequestActivateOffset += RequestActivateBuffer + 0x10;
	}
	else
	{
		RequestActivateOffset = 0;
	}

	DWORD RequestTransferBuffer = GetSigOffset(pHandle, mBase, mSize, RequestTransferSig, RequestTransferMask, sizeof(RequestTransferSig));
	if (RequestTransferBuffer > 0 && RPM(pHandle, mBase + RequestTransferBuffer + 0xC, &RequestTransferOffset, sizeof(RequestTransferOffset)))
	{
		RequestTransferOffset += RequestTransferBuffer + 0x10;
		if (!RPM(pHandle, mBase + RequestTransferBuffer + 0x1E, &UnknownTransferId, sizeof(UnknownTransferId)))
		{
			UnknownTransferId = 0;
		}
	}
	else
	{
		RequestTransferOffset = 0;
	}

	DWORD RequestTeleportBuffer = GetSigOffset(pHandle, mBase, mSize, RequestTeleportSig, RequestTeleportMask, sizeof(RequestTeleportSig));
	if (RequestTeleportBuffer > 0 && RPM(pHandle, mBase + RequestTeleportBuffer + 0xD, &RequestTeleportOffset, sizeof(RequestTeleportOffset)))
	{
		RequestTeleportOffset += RequestTeleportBuffer + 0x11;
	}
	else
	{
		RequestTeleportOffset = 0;
	}

	DWORD ClientStateBuffer = GetSigOffset(pHandle, mBase, mSize, ClientStateSig, ClientStateMask, sizeof(ClientStateSig));
	if (ClientStateBuffer > 0 && RPM(pHandle, mBase + ClientStateBuffer + 0x3, &ClientStateOffset, sizeof(ClientStateOffset)))
	{
		ClientStateOffset += ClientStateBuffer + 0x7;
	}
	else
	{
		ClientStateOffset = 0;
	}

	DWORD RequestHitsBuffer = GetSigOffset(pHandle, mBase, mSize, RequestHitsSig, RequestHitsMask, sizeof(RequestHitsSig));
	if (RequestHitsBuffer > 0 && RPM(pHandle, mBase + RequestHitsBuffer + 0x3, &RequestHitsOffset, sizeof(RequestHitsOffset)))
	{
		RequestHitsOffset += RequestHitsBuffer + 0x7;
	}
	else
	{
		RequestHitsOffset = 0;
	}

	DWORD NukeCodeBuffer = rttiGetSigOffset(pHandle, mBase, mSize, NukeCodeSig, NukeCodeMask, sizeof(NukeCodeSig), 0x3A, true, false, true, ".?AV?$BSTEventSource@UTESFormDeleteEvent@@@@");
	if (NukeCodeBuffer > 0 && RPM(pHandle, mBase + NukeCodeBuffer - 0x87, &NukeCodeOffset, sizeof(NukeCodeOffset)))
	{
		NukeCodeOffset += NukeCodeBuffer - 0x83;
	}

	printf("//Offsets\n");
	printf("#define OFFSET_LOCAL_PLAYER                             0x%08lXUL//%s\n", LocalPlayerOffset, Version);
	printf("#define OFFSET_ENTITY_LIST                              0x%08lXUL//%s\n", EntityListOffset, Version);
	printf("#define OFFSET_MAIN                                     0x%08lXUL//%s\n", FalloutMainOffset, Version);
	printf("#define OFFSET_CAMERA                                   0x%08lXUL//%s\n", CameraOffset, Version);
	printf("#define OFFSET_CHAR_CONTROLLER                          0x%08lXUL//%s\n", ControllerOffset, Version);
	printf("#define OFFSET_GET_PTR_A1                               0x%08lXUL//%s\n", GetPtrA1Offset, Version);
	printf("#define OFFSET_GET_PTR_A2                               0x%08lXUL//%s\n", GetPtrA2Offset, Version);
	printf("#define OFFSET_MESSAGE_SENDER                           0x%08lXUL//%s\n", MessageOffset, Version);
	printf("#define OFFSET_DATA_HANDLER                             0x%08lXUL//%s\n", HandlerOffset, Version);
	printf("#define OFFSET_INFINITE_AMMO                            0x%08lXUL//%s\n", InfiniteAmmoOffset, Version);
	printf("#define OFFSET_REDIRECTION                              0x%08lXUL//%s\n", RedirectionOffset, Version);
	printf("#define OFFSET_REDIRECTION_JMP                          0x%08lXUL//%s\n", RedirectionJmpOffset, Version);
	printf("#define OFFSET_REDIRECTION_RET                          0x%08lXUL//%s\n", RedirectionRetOffset, Version);
	printf("#define OFFSET_NOCLIP_A                                 0x%08lXUL//%s\n", NoclipOffsetA, Version);
	printf("#define OFFSET_NOCLIP_B                                 0x%08lXUL//%s\n", NoclipOffsetB, Version);
	printf("#define OFFSET_NOCLIP_C                                 0x%08lXUL//%s\n", NoclipOffsetC, Version);
	printf("#define OFFSET_NOCLIP_D                                 0x%08lXUL//%s\n", NoclipOffsetD, Version);
	printf("#define OFFSET_ACTOR_VALUE                              0x%08lXUL//%s\n", ActorValueOffset, Version);
	printf("#define OFFSET_OPK_BEGIN                                0x%08lXUL//%s\n", EnemyPositionBeginOffset, Version);
	printf("#define OFFSET_OPK_END                                  0x%08lXUL//%s\n", EnemyPositionEndOffset, Version);
	printf("#define OFFSET_AV_REGEN                                 0x%08lXUL//%s\n", ActorValueRegenOffset, Version);
	printf("#define OFFSET_SERVER_POSITION                          0x%08lXUL//%s\n", ServerPositionOffset, Version);
	printf("#define OFFSET_ENTITY_ID                                0x%08lXUL//%s\n", EntityIdOffset, Version);
	printf("#define OFFSET_NUKE_CODE                                0x%08lXUL//%s\n", NukeCodeOffset, Version);
	printf("\n//vtables\n");
	printf("#define VTABLE_TESNPC                                   0x%08lXUL//%s\n", VTABLE_TESNPC, Version);
	printf("#define VTABLE_TESOBJECTCONT                            0x%08lXUL//%s\n", VTABLE_TESOBJECTCONT, Version);
	printf("#define VTABLE_TESOBJECTMISC                            0x%08lXUL//%s\n", VTABLE_TESOBJECTMISC, Version);
	printf("#define VTABLE_TESOBJECTBOOK                            0x%08lXUL//%s\n", VTABLE_TESOBJECTBOOK, Version);
	printf("#define VTABLE_ALCHEMYITEM                              0x%08lXUL//%s\n", VTABLE_ALCHEMYITEM, Version);
	printf("#define VTABLE_TESAMMO                                  0x%08lXUL//%s\n", VTABLE_TESAMMO, Version);
	printf("#define VTABLE_TESOBJECTWEAP                            0x%08lXUL//%s\n", VTABLE_TESOBJECTWEAP, Version);
	printf("#define VTABLE_TESOBJECTARMO                            0x%08lXUL//%s\n", VTABLE_TESOBJECTARMO, Version);
	printf("#define VTABLE_TESUTILITYITEM                           0x%08lXUL//%s\n", VTABLE_TESUTILITYITEM, Version);
	printf("#define VTABLE_BGSNOTE                                  0x%08lXUL//%s\n", VTABLE_BGSNOTE, Version);
	printf("#define VTABLE_TESKEY                                   0x%08lXUL//%s\n", VTABLE_TESKEY, Version);
	printf("#define VTABLE_TESFLORA                                 0x%08lXUL//%s\n", VTABLE_TESFLORA, Version);
	printf("#define VTABLE_TESLEVITEM                               0x%08lXUL//%s\n", VTABLE_TESLEVITEM, Version);
	printf("#define VTABLE_CURRENCYOBJECT                           0x%08lXUL//%s\n", VTABLE_CURRENCYOBJECT, Version);
	printf("#define VTABLE_REQUESTACTIVATEREFMSG                    0x%08lXUL//%s\n", RequestActivateOffset, Version);
	printf("#define VTABLE_REQUESTTRANSFERITEMMSG                   0x%08lXUL//%s\n", RequestTransferOffset, Version);
	printf("#define VTABLE_REQUESTTELEPORTTOLOCATIONMSG             0x%08lXUL//%s\n", RequestTeleportOffset, Version);
	printf("#define VTABLE_CLIENTSTATEMSG                           0x%08lXUL//%s\n", ClientStateOffset, Version);
	printf("#define VTABLE_REQUESTHITSONACTORS                      0x%08lXUL//%s\n", RequestHitsOffset, Version);
	printf("\n//Item Transferring Id - Old Id (1.2.7.26 - 1.3.0.23):  0xE0001F7AUL\n");
	printf("#define UNKNOWN_TRANSFER_ID                             0x%08lXUL//%s\n", UnknownTransferId, Version);
	printf("\n//Noclip\n");
	printf("#define NOCLIP_DYNAMIC_CALL_A       { 0x%02hhX, 0x%02hhX, 0x%02hhX, 0x%02hhX, 0x%02hhX }//%s\n", NoclipCallA[0], NoclipCallA[1], NoclipCallA[2], NoclipCallA[3], NoclipCallA[4], Version);
	printf("#define NOCLIP_DYNAMIC_CALL_B       { 0x%02hhX, 0x%02hhX, 0x%02hhX, 0x%02hhX, 0x%02hhX }//%s\n", NoclipCallB[0], NoclipCallB[1], NoclipCallB[2], NoclipCallB[3], NoclipCallB[4], Version);
}

void ProcessMain(ProcessData pData)
{
	GetOffsets(pData.pHandle, DWORD64(pData.mBase), pData.mSize);
	std::cin.get();
}

DWORD GetSigOffset(HANDLE pHandle, DWORD64 mBase, DWORD mSize, BYTE* Sig, const char* Mask, int Len)
{
	DWORD SigOffset = 0;
	BYTE* ModuleArray = new BYTE[mSize];

	if (ReadProcessMemory(pHandle, (void*)(mBase), &*ModuleArray, mSize, NULL))
	{
		for (DWORD i = 0; i < mSize; i++)
		{
			if (i + Len >= mSize) break;
			for (int c = 0; c < Len; c++)
			{
				if (ModuleArray[i + c] != Sig[c] && Mask[c] != '?') break;
				if (c == Len - 1)
				{
					delete[]ModuleArray;
					return i;
				}
			}
		}
	}

	delete[]ModuleArray;
	return SigOffset;
}

bool RPM(HANDLE pHandle, DWORD64 src, void* dst, size_t Size)
{
	return ReadProcessMemory(pHandle, (void*)(src), dst, Size, NULL);
}

bool WPM(HANDLE pHandle, DWORD64 dst, void* src, size_t Size)
{
	return WriteProcessMemory(pHandle, (void*)(dst), src, Size, NULL);
}

DWORD64 AllocEx(HANDLE pHandle, size_t Size)
{
	return DWORD64(VirtualAllocEx(pHandle, NULL, Size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
}

bool FreeEx(HANDLE pHandle, DWORD64 src)
{
	return VirtualFreeEx(pHandle, LPVOID(src), 0, MEM_RELEASE);
}

bool Valid(DWORD64 ptr)
{
	if (ptr < 0x7FFF || ptr > 0x7FFFFFFFFFFF) return false;
	else return true;
}

bool GetModuleData(ProcessData* pData)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pData->pPid);
	if (hSnapshot == INVALID_HANDLE_VALUE) return false;

	MODULEENTRY32 lpme;
	lpme.dwSize = sizeof(lpme);

	while (Module32Next(hSnapshot, &lpme))
	{
		if (!strcmp(lpme.szModule, pData->mName))
		{
			pData->mBase = lpme.modBaseAddr;
			pData->mSize = lpme.modBaseSize;
			CloseHandle(hSnapshot);
			return true;
		}
	}

	CloseHandle(hSnapshot);
	return false;
}

int GetProcessData(ProcessData* BaseData, ProcessData* pData = nullptr, int pCount = 0)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

	PROCESSENTRY32 lppe;
	lppe.dwSize = sizeof(lppe);

	int ProcessCount = 0;
	while (Process32Next(hSnapshot, &lppe))
	{
		if (!strcmp(lppe.szExeFile, BaseData->pName))
		{
			if (pData != nullptr && pCount > 0 && ProcessCount < pCount)
			{
				pData[ProcessCount].pPid = lppe.th32ProcessID;
			}

			ProcessCount++;
		}
	}

	CloseHandle(hSnapshot);
	return ProcessCount;
}

int main()
{
	ProcessData BaseData;
	BaseData.pName = (CHAR*)("Fallout76.exe");//Process
	BaseData.mName = (CHAR*)("Fallout76.exe");//Module

	int pCount = GetProcessData(&BaseData);
	if (pCount == 0) return 1;

	ProcessData* pData = new ProcessData[pCount];
	for (int i = 0; i < pCount; i++) memcpy(&pData[i], &BaseData, sizeof(ProcessData));

	if (GetProcessData(&BaseData, pData, pCount) != pCount)
	{
		delete[]pData;
		return 2;
	}

	int Index = 0;
	if (pCount > 1)
	{
		for (int i = 0; i < pCount; i++)
		{
			printf("%08lX - %s - Index: %d\n", pData[i].pPid, pData[i].pName, i + 1);
		}

		printf("Enter target process index: ");
		std::cin >> Index;
		printf("--------------------------------\n");

		if (Index == 0 || Index > pCount)
		{
			delete[]pData;
			return 3;
		}
		else
		{
			Index--;
		}
	}

	if (!GetModuleData(&pData[Index]))
	{
		delete[]pData;
		return 4;
	}

	pData[Index].pHandle = OpenProcess(PROCESS_ALL_ACCESS, false, pData[Index].pPid);
	if (pData[Index].pHandle == NULL)
	{
		delete[]pData;
		return 5;
	}

	ProcessMain(pData[Index]);

	CloseHandle(pData[Index].pHandle);
	delete[]pData;
	return 0;
}