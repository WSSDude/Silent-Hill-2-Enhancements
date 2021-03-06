#pragma once

#include "Patches\Patches.h"

class m_IDirect3DDevice8 : public IDirect3DDevice8
{
private:
	LPDIRECT3DDEVICE8 ProxyInterface;
	m_IDirect3D8* m_pD3D;

	BYTE *SH2_ChapterID = nullptr;
	DWORD *SH2_RoomID = nullptr;
	DWORD *SH2_CutsceneID = nullptr;
	float *SH2_CutsceneCameraPos = nullptr;
	float *SH2_JamesPos = nullptr;
	DWORD *SH2_SpecializedLight1 = nullptr;
	DWORD *SH2_SpecializedLight2 = nullptr;

	BYTE *SH2_FlashlightBeam = nullptr;
	BYTE *SH2_FlashlightSwitch = nullptr;
	float *SH2_FlashlightRed = nullptr;
	float *SH2_FlashlightGreen = nullptr;
	float *SH2_FlashlightBlue = nullptr;

	typedef enum _FLDIMMODE {
		SHADOW_FADING_NONE = 0,
		SHADOW_FADING_IN = 1,
		SHADOW_FADING_OUT = 2,
		SHADOW_REFADING = 3,
	} FLDIMMODE;

	// Shadow fading when using soft shadows
	bool DrawingShadowsFlag = false;
	FLDIMMODE ShadowMode = SHADOW_FADING_NONE;
	BYTE LastFlashlightSwitch = 0;
	DWORD ShadowFadingCounter = 0;
	DWORD ShadowFadingIntensity = 0;	// 0 to 100

	// Xbox shadows to help remove self shadows in certian locations
	bool EnableXboxShadows = false;
	bool shadowVolumeFlag = false;

	// Remove Environment Flicker by skipping frames in certian cutscenes
	bool SkipSceneFlag = false;
	DWORD LastCutsceneID = 0;
	float LastCameraPos = 0;
	float LastJamesPos = 0;
	DWORD SkipSceneCounter = 0;

	// Remove Effects Flicker by resetting texture on first frame
	DWORD EndSceneCounter = 0;
	bool OverrideTextureLoop = false;
	bool PresentFlag = false;

	IDirect3DTexture8 *pInTexture = nullptr;
	IDirect3DSurface8 *pInSurface = nullptr;

	IDirect3DTexture8 *pShrunkTexture = nullptr;
	IDirect3DSurface8 *pShrunkSurface = nullptr;

	IDirect3DTexture8 *pOutTexture = nullptr;
	IDirect3DSurface8 *pOutSurface = nullptr;

	IDirect3DTexture8 *BlankTexture = nullptr;

	IDirect3DTexture8 *silhouetteTexture = nullptr;
	IDirect3DSurface8 *silhouetteSurface = nullptr;

	struct CUSTOMVERTEX
	{
		FLOAT x, y, z, rhw;
		DWORD color;
	};

	struct CUSTOMVERTEX_UV
	{
		FLOAT x, y, z, rhw;
		FLOAT u, v;
	};

	struct D3DSTATE
	{
		DWORD magFilter;
		DWORD minFilter;
		DWORD colorArg1;
		DWORD alphaArg1;
		DWORD alphaBlendEnable;
		DWORD alphaTestEnable;
		DWORD srcBlend;
		DWORD destBlend;
		IDirect3DBaseTexture8 *stage0 = nullptr;
		DWORD vertexShader;
		IDirect3DVertexBuffer8 *pStream0 = nullptr;
		UINT stream0Stride;
	};

public:
	m_IDirect3DDevice8(LPDIRECT3DDEVICE8 pDevice, m_IDirect3D8* pD3D) : ProxyInterface(pDevice), m_pD3D(pD3D)
	{
		Logging::LogDebug() << "Creating device " << __FUNCTION__ << "(" << this << ")";

		ProxyAddressLookupTable = new AddressLookupTable<m_IDirect3DDevice8>(this);

		SH2_ChapterID = GetChapterIDPointer();
		SH2_RoomID = GetRoomIDPointer();
		SH2_CutsceneID = GetCutsceneIDPointer();
		SH2_CutsceneCameraPos = GetCutscenePosPointer();
		SH2_JamesPos = GetJamesPosPointer();
		SH2_SpecializedLight1 = GetSpecializedLightPointer1();
		SH2_SpecializedLight2 = GetSpecializedLightPointer2();

		SH2_FlashlightBeam = GetFlashLightRenderPointer();
		SH2_FlashlightSwitch = GetFlashlightSwitchPointer();
		SH2_FlashlightRed = GetFlashlightBrightnessPointer();
		if (SH2_FlashlightRed)
		{
			SH2_FlashlightGreen = (float*)((DWORD)SH2_FlashlightRed + 4);
			SH2_FlashlightBlue = (float*)((DWORD)SH2_FlashlightRed + 8);
		}

		// Create blank texture for white shader fix
		if (FAILED(ProxyInterface->CreateTexture(1, 1, 1, NULL, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &BlankTexture)))
		{
			BlankTexture = nullptr;
		}
	}
	~m_IDirect3DDevice8()
	{
		Logging::LogDebug() << __FUNCTION__ << "(" << this << ")" << " deleting device!";

		delete ProxyAddressLookupTable;
	}

	LPDIRECT3DDEVICE8 GetProxyInterface() { return ProxyInterface; }
	AddressLookupTable<m_IDirect3DDevice8> *ProxyAddressLookupTable;

	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirect3DDevice8 methods ***/
	STDMETHOD(TestCooperativeLevel)(THIS);
	STDMETHOD_(UINT, GetAvailableTextureMem)(THIS);
	STDMETHOD(ResourceManagerDiscardBytes)(THIS_ DWORD Bytes);
	STDMETHOD(GetDirect3D)(THIS_ IDirect3D8** ppD3D8);
	STDMETHOD(GetDeviceCaps)(THIS_ D3DCAPS8* pCaps);
	STDMETHOD(GetDisplayMode)(THIS_ D3DDISPLAYMODE* pMode);
	STDMETHOD(GetCreationParameters)(THIS_ D3DDEVICE_CREATION_PARAMETERS *pParameters);
	STDMETHOD(SetCursorProperties)(THIS_ UINT XHotSpot, UINT YHotSpot, IDirect3DSurface8* pCursorBitmap);
	STDMETHOD_(void, SetCursorPosition)(THIS_ UINT XScreenSpace, UINT YScreenSpace, DWORD Flags);
	STDMETHOD_(BOOL, ShowCursor)(THIS_ BOOL bShow);
	STDMETHOD(CreateAdditionalSwapChain)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain8** pSwapChain);
	STDMETHOD(Reset)(THIS_ D3DPRESENT_PARAMETERS* pPresentationParameters);
	STDMETHOD(Present)(THIS_ CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
	STDMETHOD(GetBackBuffer)(THIS_ UINT BackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface8** ppBackBuffer);
	STDMETHOD(GetRasterStatus)(THIS_ D3DRASTER_STATUS* pRasterStatus);
	STDMETHOD_(void, SetGammaRamp)(THIS_ DWORD Flags, CONST D3DGAMMARAMP* pRamp);
	STDMETHOD_(void, GetGammaRamp)(THIS_ D3DGAMMARAMP* pRamp);
	STDMETHOD(CreateTexture)(THIS_ UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture8** ppTexture);
	STDMETHOD(CreateVolumeTexture)(THIS_ UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture8** ppVolumeTexture);
	STDMETHOD(CreateCubeTexture)(THIS_ UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture8** ppCubeTexture);
	STDMETHOD(CreateVertexBuffer)(THIS_ UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer8** ppVertexBuffer);
	STDMETHOD(CreateIndexBuffer)(THIS_ UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer8** ppIndexBuffer);
	STDMETHOD(CreateRenderTarget)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, IDirect3DSurface8** ppSurface);
	STDMETHOD(CreateDepthStencilSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, IDirect3DSurface8** ppSurface);
	STDMETHOD(CreateImageSurface)(THIS_ UINT Width, UINT Height, D3DFORMAT Format, IDirect3DSurface8** ppSurface);
	STDMETHOD(CopyRects)(THIS_ IDirect3DSurface8* pSourceSurface, CONST RECT* pSourceRectsArray, UINT cRects, IDirect3DSurface8* pDestinationSurface, CONST POINT* pDestPointsArray);
	STDMETHOD(StretchRect)(THIS_ IDirect3DSurface8* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface8* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter);
	STDMETHOD(UpdateTexture)(THIS_ IDirect3DBaseTexture8* pSourceTexture, IDirect3DBaseTexture8* pDestinationTexture);
	STDMETHOD(GetFrontBuffer)(THIS_ IDirect3DSurface8* pDestSurface);
	STDMETHOD(SetRenderTarget)(THIS_ IDirect3DSurface8* pRenderTarget, IDirect3DSurface8* pNewZStencil);
	STDMETHOD(GetRenderTarget)(THIS_ IDirect3DSurface8** ppRenderTarget);
	STDMETHOD(GetDepthStencilSurface)(THIS_ IDirect3DSurface8** ppZStencilSurface);
	STDMETHOD(BeginScene)(THIS);
	STDMETHOD(EndScene)(THIS);
	STDMETHOD(Clear)(THIS_ DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
	STDMETHOD(SetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix);
	STDMETHOD(GetTransform)(THIS_ D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix);
	STDMETHOD(MultiplyTransform)(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX *pMatrix);
	STDMETHOD(SetViewport)(THIS_ CONST D3DVIEWPORT8* pViewport);
	STDMETHOD(GetViewport)(THIS_ D3DVIEWPORT8* pViewport);
	STDMETHOD(SetMaterial)(THIS_ CONST D3DMATERIAL8* pMaterial);
	STDMETHOD(GetMaterial)(THIS_ D3DMATERIAL8* pMaterial);
	STDMETHOD(SetLight)(THIS_ DWORD Index, CONST D3DLIGHT8* pLight);
	STDMETHOD(GetLight)(THIS_ DWORD Index, D3DLIGHT8*);
	STDMETHOD(LightEnable)(THIS_ DWORD Index, BOOL Enable);
	STDMETHOD(GetLightEnable)(THIS_ DWORD Index, BOOL* pEnable);
	STDMETHOD(SetClipPlane)(THIS_ DWORD Index, CONST float* pPlane);
	STDMETHOD(GetClipPlane)(THIS_ DWORD Index, float* pPlane);
	STDMETHOD(SetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD Value);
	STDMETHOD(GetRenderState)(THIS_ D3DRENDERSTATETYPE State, DWORD* pValue);
	STDMETHOD(BeginStateBlock)(THIS);
	STDMETHOD(EndStateBlock)(THIS_ DWORD* pToken);
	STDMETHOD(ApplyStateBlock)(THIS_ DWORD Token);
	STDMETHOD(CaptureStateBlock)(THIS_ DWORD Token);
	STDMETHOD(DeleteStateBlock)(THIS_ DWORD Token);
	STDMETHOD(CreateStateBlock)(THIS_ D3DSTATEBLOCKTYPE Type, DWORD* pToken);
	STDMETHOD(SetClipStatus)(THIS_ CONST D3DCLIPSTATUS8* pClipStatus);
	STDMETHOD(GetClipStatus)(THIS_ D3DCLIPSTATUS8* pClipStatus);
	STDMETHOD(GetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture8** ppTexture);
	STDMETHOD(SetTexture)(THIS_ DWORD Stage, IDirect3DBaseTexture8* pTexture);
	STDMETHOD(GetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue);
	STDMETHOD(SetTextureStageState)(THIS_ DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
	STDMETHOD(ValidateDevice)(THIS_ DWORD* pNumPasses);
	STDMETHOD(GetInfo)(THIS_ DWORD DevInfoID, void* pDevInfoStruct, DWORD DevInfoStructSize);
	STDMETHOD(SetPaletteEntries)(THIS_ UINT PaletteNumber, CONST PALETTEENTRY* pEntries);
	STDMETHOD(GetPaletteEntries)(THIS_ UINT PaletteNumber, PALETTEENTRY* pEntries);
	STDMETHOD(SetCurrentTexturePalette)(THIS_ UINT PaletteNumber);
	STDMETHOD(GetCurrentTexturePalette)(THIS_ UINT *PaletteNumber);
	STDMETHOD(DrawPrimitive)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
	STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount);
	STDMETHOD(DrawPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	STDMETHOD(DrawIndexedPrimitiveUP)(THIS_ D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	STDMETHOD(ProcessVertices)(THIS_ UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer8* pDestBuffer, DWORD Flags);
	STDMETHOD(CreateVertexShader)(THIS_ CONST DWORD* pDeclaration, CONST DWORD* pFunction, DWORD* pHandle, DWORD Usage);
	STDMETHOD(SetVertexShader)(THIS_ DWORD Handle);
	STDMETHOD(GetVertexShader)(THIS_ DWORD* pHandle);
	STDMETHOD(DeleteVertexShader)(THIS_ DWORD Handle);
	STDMETHOD(SetVertexShaderConstant)(THIS_ DWORD Register, CONST void* pConstantData, DWORD ConstantCount);
	STDMETHOD(GetVertexShaderConstant)(THIS_ DWORD Register, void* pConstantData, DWORD ConstantCount);
	STDMETHOD(GetVertexShaderDeclaration)(THIS_ DWORD Handle, void* pData, DWORD* pSizeOfData);
	STDMETHOD(GetVertexShaderFunction)(THIS_ DWORD Handle, void* pData, DWORD* pSizeOfData);
	STDMETHOD(SetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer8* pStreamData, UINT Stride);
	STDMETHOD(GetStreamSource)(THIS_ UINT StreamNumber, IDirect3DVertexBuffer8** ppStreamData, UINT* pStride);
	STDMETHOD(SetIndices)(THIS_ IDirect3DIndexBuffer8* pIndexData, UINT BaseVertexIndex);
	STDMETHOD(GetIndices)(THIS_ IDirect3DIndexBuffer8** ppIndexData, UINT* pBaseVertexIndex);
	STDMETHOD(CreatePixelShader)(THIS_ CONST DWORD* pFunction, DWORD* pHandle);
	STDMETHOD(SetPixelShader)(THIS_ DWORD Handle);
	STDMETHOD(GetPixelShader)(THIS_ DWORD* pHandle);
	STDMETHOD(DeletePixelShader)(THIS_ DWORD Handle);
	STDMETHOD(SetPixelShaderConstant)(THIS_ DWORD Register, CONST void* pConstantData, DWORD ConstantCount);
	STDMETHOD(GetPixelShaderConstant)(THIS_ DWORD Register, void* pConstantData, DWORD ConstantCount);
	STDMETHOD(GetPixelShaderFunction)(THIS_ DWORD Handle, void* pData, DWORD* pSizeOfData);
	STDMETHOD(DrawRectPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo);
	STDMETHOD(DrawTriPatch)(THIS_ UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo);
	STDMETHOD(DeletePatch)(THIS_ UINT Handle);

private:
	// Extra functions
	HRESULT DrawSoftShadows();
	void BackupState(D3DSTATE *state);
	void RestoreState(D3DSTATE *state);
	template <typename T>
	void ReleaseInterface(T **ppInterface, UINT ReleaseRefNum = 1);
	DWORD GetShadowIntensity();
	void SetShadowFading();
};
