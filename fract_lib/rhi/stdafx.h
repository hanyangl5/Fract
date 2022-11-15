//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently.

#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "d3d12.h"
#include "d3dx12.h"

#include <dxgi1_3.h>
#include <dxgi1_6.h>

#include <shellapi.h>
#include <string>
#include <wrl.h>

#include <directx-dxc/dxcapi.h>
#include <directx-dxc/d3d12shader.h>

#include <d3dcompiler.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define DX12_API_VERSION D3D_FEATURE_LEVEL_12_1

#define USE_ASYNC_COMPUTE_TRANSFER 1
