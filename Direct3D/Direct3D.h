#ifndef _DIRECT3D_
#define _DIRECT3D_

//las librerias necesarias
#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3dcompiler")
#pragma comment(lib, "winmm")

//las cabeceras necesarias
#include <string>
#include <vector>
using namespace std;
#include <Windows.h>
#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
using namespace DirectX;

//una pequeña macro que nos ayudara a eliberar memoria
#define X_RELEASE(e) if(e) { e->Release(); e = nullptr; }

//el objeto en si, no vamos a utilizar cosas modernas, es una demo a secas
//iniciar, dibujar y eliberar. ni thread safe ni excepciones
class Direct3D
{
public:
	int init(UINT in_w, UINT in_h, HWND in_hwnd);
	void render();
	void x_it();
private:
	
	//estas variables casi mejor que las vamos explicando en el fuente cpp
	IDXGIFactory2* factory = nullptr;
	IDXGIAdapter2* adapter = nullptr;
	IDXGIOutput1* output = nullptr;
	DXGI_ADAPTER_DESC1 adapter_desc = {};
	D3D_FEATURE_LEVEL feature_level = {};
	ID3D11Device1* device = nullptr;
	IDXGISwapChain1* swapchain = nullptr;
	ID3D11DeviceContext1* context = nullptr;
	const DXGI_FORMAT format_texture = DXGI_FORMAT_R8G8B8A8_UNORM;
	ID3D11RenderTargetView* render_target = nullptr;
	ID3D11Texture2D* render_target_tex = nullptr;
	ID3D11DepthStencilView* depth_stencil = nullptr;
	ID3D11Texture2D* depth_stencil_tex = nullptr;
	ID3D11VertexShader* shader_vertex = nullptr;
	ID3D11PixelShader* shader_pixel = nullptr;
	ID3D11Buffer* buff_viewproj = nullptr;
	ID3D11Buffer* buff_world = nullptr;
	ID3D11InputLayout* input_layout = nullptr;
	ID3D11Buffer* vb = nullptr;
	ID3D11Buffer* ib = nullptr;
	XMMATRIX viewproj;
};

#endif