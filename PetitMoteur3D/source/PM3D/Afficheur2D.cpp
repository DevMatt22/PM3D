#include "StdAfx.h"
#include "Afficheur2D.h"
#include "resource.h"
#include "MoteurWindows.h"
#include "util.h"
namespace PM3D
{

	struct ShadersParams
	{
		XMMATRIX matWVP; // la matrice totale
	};

	// Definir l�organisation de notre sommet
	D3D11_INPUT_ELEMENT_DESC CSommetSprite::layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	UINT CSommetSprite::numElements = ARRAYSIZE(layout);

	CSommetSprite CAfficheur2D::sommets[6] = {
		CSommetSprite(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f)),
		CSommetSprite(XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f)),
		CSommetSprite(XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f)),
		CSommetSprite(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f)),
		CSommetSprite(XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f)),
		CSommetSprite(XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f))
	};

	CAfficheur2D::CAfficheur2D(CDispositifD3D11* _pDispositif)
		: pDispositif(_pDispositif)
		, pVertexBuffer(nullptr)
		, pConstantBuffer(nullptr)
		, pEffet(nullptr)
		, pTechnique(nullptr)
		, pPasse(nullptr)
		, pVertexLayout(nullptr)
		, pSampleState(nullptr)
	{
		// Cr�ation du vertex buffer et copie des sommets
		ID3D11Device* pD3DDevice = pDispositif->GetD3DDevice();
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(sommets);
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = sommets;
		DXEssayer(pD3DDevice->CreateBuffer(&bd, &InitData, &pVertexBuffer),
			DXE_CREATIONVERTEXBUFFER);
		// Initialisation de l�effet
		InitEffet();

	}
	CAfficheur2D::~CAfficheur2D()
	{
		DXRelacher(pConstantBuffer);
		DXRelacher(pSampleState);
		DXRelacher(pEffet);
		DXRelacher(pVertexLayout);
		DXRelacher(pVertexBuffer);

	}
	void CAfficheur2D::InitEffet()
	{
		// Compilation et chargement du vertex shader
		ID3D11Device* pD3DDevice = pDispositif->GetD3DDevice();

		// Cr�ation d�un tampon pour les constantes de l�effet
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(ShadersParams);
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = 0;
		pD3DDevice->CreateBuffer(&bd, nullptr, &pConstantBuffer);

		// Pour l�effet
		ID3DBlob* pFXBlob = nullptr;
		DXEssayer(D3DCompileFromFile(L"Sprite1.fx", 0, 0, 0,
			"fx_5_0", 0, 0,
			&pFXBlob, 0),
			DXE_ERREURCREATION_FX);
		D3DX11CreateEffectFromMemory(pFXBlob->GetBufferPointer(),
			pFXBlob->GetBufferSize(), 0, pD3DDevice, &pEffet);
		pFXBlob->Release();
		pTechnique = pEffet->GetTechniqueByIndex(0);
		pPasse = pTechnique->GetPassByIndex(0);

		// Cr�er l�organisation des sommets pour le VS de notre effet
		D3DX11_PASS_SHADER_DESC effectVSDesc;
		pPasse->GetVertexShaderDesc(&effectVSDesc);
		D3DX11_EFFECT_SHADER_DESC effectVSDesc2;
		effectVSDesc.pShaderVariable->GetShaderDesc(effectVSDesc.ShaderIndex,
			&effectVSDesc2);
		const void* vsCodePtr = effectVSDesc2.pBytecode;
		const unsigned vsCodeLen = effectVSDesc2.BytecodeLength;
		//pVertexLayout = nullptr;
		DXEssayer(pD3DDevice->CreateInputLayout(CSommetSprite::layout, CSommetSprite::numElements, vsCodePtr, vsCodeLen, &pVertexLayout), DXE_CREATIONLAYOUT);

		// Initialisation des param�tres de sampling de la texture
		D3D11_SAMPLER_DESC samplerDesc;
		samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 4;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDesc.BorderColor[0] = 0;
		samplerDesc.BorderColor[1] = 0;
		samplerDesc.BorderColor[2] = 0;
		samplerDesc.BorderColor[3] = 0;
		samplerDesc.MinLOD = 0;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		// Cr�ation de l��tat de sampling
		pD3DDevice->CreateSamplerState(&samplerDesc, &pSampleState);
	}

}
