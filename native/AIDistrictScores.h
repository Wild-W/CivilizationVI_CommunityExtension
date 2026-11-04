/* Proposed CE integration points. Not present in released CE1.3. */
#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#ifdef __cplusplus
extern "C" {
#endif
int CE_AIDistrictScoresInitialize(HMODULE core);
void CE_AIDistrictScoresRegisterLua(void *L);
void CE_AIDistrictScoresReset(void);
#ifdef __cplusplus
}
#endif
