// ____________________________
// ██▀▀█▀▀██▀▀▀▀▀▀▀█▀▀█        │
// ██  ▀  █▄  ▀██▄ ▀ ▄█ ▄▀▀ █  │
// █  █ █  ▀▀  ▄█  █  █ ▀▄█ █▄ │
// ▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀▀────────┘
//  by Guillaume 'Aoineko' Blanchard under CC BY-SA license
//─────────────────────────────────────────────────────────────────────────────
// PCCM Ninja-Tap Driver v2.0 for MSX (Primary & C-lib routine)
// Copyright 1994 1995 Knight's Chamber, all rights reserved.
// 	Written by Firm Knight.
//─────────────────────────────────────────────────────────────────────────────
#include "core.h"

//=============================================================================
// DEFINES
//=============================================================================

extern u8 g_NTap_Info[3];
extern u8 g_NTap_Data[8];

//=============================================================================
// FUNCTIONS
//=============================================================================

// Function: NTap_Check
// 
u8 NTap_Check();

// Function: NTap_Update
// 
void NTap_Update();

// Function: NTap_Get
// 
inline u8 NTap_Get(u8 ply) { return g_NTap_Data[ply]; }
