//Incluir nuestra cabecera
#include "Direct3D.h"

//Esta funcion se llamara para arrancar el Direct3D y setear los estados que no hara falta
//cambiarlos mas. Como entrada tenemos el tamaño de la ventana y el vinculo hacia ella.
int Direct3D::init(UINT in_w, UINT in_h, HWND in_hwnd)
{
	//DirectX tiene un componente llamado DXGI que se ocupa de la comunicacion con el driver.
	HRESULT result = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	if (FAILED(result)) return -3001;

	//Los feture level se utilizan para validar el minimo requerimito que necesitamos
	//para nuestra aplicacion. Si un dispositivo es compatible con el que pedimos, sabemos que
	//cumple con todas las funcionalidades que vamos a utilizar mas adelante.
	D3D_FEATURE_LEVEL t_feature_levels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	//Vamos por todos los dispositivos hardware que disponemos en el sistema.
	//Intentamos crear el dispositivo y si se logra, en este caso, nos quedamos 
	//con el que mas memoria video disponga y sea compatible con nuestro feature level pedido.
	size_t t_mem = 0;
	IDXGIAdapter1* t_adapter_final = nullptr;
	IDXGIAdapter1* t_adapter = nullptr;
	for (UINT i = 0; factory->EnumAdapters1(i, &t_adapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		D3D_FEATURE_LEVEL t_max_feature_level = D3D_FEATURE_LEVEL_9_1;

		if (SUCCEEDED(D3D11CreateDevice(NULL,
			D3D_DRIVER_TYPE_HARDWARE,
			NULL,
			0,
			t_feature_levels,
			ARRAYSIZE(t_feature_levels),
			D3D11_SDK_VERSION,
			NULL,
			&t_max_feature_level,
			NULL)))
		{
			DXGI_ADAPTER_DESC1 t_desc;
			t_adapter->GetDesc1(&t_desc);

			if (t_desc.DedicatedVideoMemory > t_mem && t_max_feature_level >= D3D_FEATURE_LEVEL_11_0)
			{
				t_mem = t_desc.DedicatedVideoMemory;
				t_adapter_final = t_adapter;
				adapter_desc = t_desc;
				feature_level = t_max_feature_level;
			}
		}
		++i;
	}

	//Si hay dispositivo hardware reinterpretamos la interfaz a un nivel superior para disponer de mas
	//funcionalidades
	if (t_adapter_final)
	{
		result = t_adapter_final->QueryInterface(__uuidof(IDXGIAdapter2), reinterpret_cast<void**>(&adapter));
		if (FAILED(result)) return -3002;

		t_adapter_final->Release();
		t_adapter_final = nullptr;
	}
	else {
		return -3003;
	}
	
	//Una vez que tenemos el dispositivo harware hay que mirar sus salidas de video y elegir la que mas
	//nos convenga. En este caso, que soporte el formato de textura DXGI_FORMAT_R8G8B8A8_UNORM y
	//la que mas resolucion tenga.
	IDXGIOutput* t_output_final = nullptr;
	IDXGIOutput* t_output = nullptr;
	UINT t_screen_space = 0;
	for (UINT i = 0; adapter->EnumOutputs(i, &t_output) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		UINT t_num = 0;
		t_output->GetDisplayModeList(format_texture, 0, &t_num, 0);

		DXGI_MODE_DESC* t_desc_mode = new DXGI_MODE_DESC[t_num];
		t_output->GetDisplayModeList(format_texture, 0, &t_num, t_desc_mode);

		if (t_num > 0 && (t_desc_mode[t_num - 1].Width * t_desc_mode[t_num - 1].Height) > t_screen_space)
		{
			t_output_final = t_output;
			t_screen_space = t_desc_mode[t_num - 1].Width * t_desc_mode[t_num - 1].Height;
		}
	}

	//Si hemos encontrado una, reinterpretar su interfaz a un nivel superior para disponer de mas funcionalidades.
	if (t_output_final)
	{
		result = t_output_final->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&output));
		if (FAILED(result)) return -3004;

		X_RELEASE(t_output_final);
		X_RELEASE(t_output);
	}
	else {
		return -3005;
	}
	
	//Ya tenemos todo listo para iniciar el dispositivo.
	DXGI_SWAP_CHAIN_DESC t_swapchain_desc;
	ZeroMemory(&t_swapchain_desc, sizeof(DXGI_SWAP_CHAIN_DESC));

	t_swapchain_desc.BufferCount = 2;
	t_swapchain_desc.BufferDesc.Format = format_texture;
	t_swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	t_swapchain_desc.OutputWindow = in_hwnd;
	t_swapchain_desc.SampleDesc.Count = 1;
	t_swapchain_desc.SampleDesc.Quality = 0;
	t_swapchain_desc.Windowed = true;
	t_swapchain_desc.BufferDesc.Width = in_w;
	t_swapchain_desc.BufferDesc.Height = in_h;
	t_swapchain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	t_swapchain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	t_swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	IDXGISwapChain* t_swapchain = nullptr;
	ID3D11Device* t_device = nullptr;
	ID3D11DeviceContext* t_context = nullptr;
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL,
		&feature_level, 1,
		D3D11_SDK_VERSION,
		&t_swapchain_desc,
		&t_swapchain,
		&t_device,
		NULL,
		&t_context);
	if (FAILED(result)) return -3006;
	
	//Si hemos creado el dispositivo reinterpretamos las interfaces a unas versiones superiores
	//para disponer de mas funcionalidades.
	result = t_device->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&device));
	if (FAILED(result)) return -3007;

	result = t_swapchain->QueryInterface(__uuidof(IDXGISwapChain1), reinterpret_cast<void**>(&swapchain));
	if (FAILED(result)) return -3008;

	result = t_context->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&context));
	if (FAILED(result)) return -3009;

	X_RELEASE(t_context);
	X_RELEASE(t_swapchain);
	X_RELEASE(t_device);

	//Creamos el backbuffer donde el Direct3D va a dibujar nuestra escena
	result = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&render_target_tex));
	if (FAILED(result)) return -3010;

	result = device->CreateRenderTargetView(render_target_tex, NULL, &render_target);
	if (FAILED(result)) return -3011;

	D3D11_TEXTURE2D_DESC t_depth_stencil_texture_desc;
	ZeroMemory(&t_depth_stencil_texture_desc, sizeof(t_depth_stencil_texture_desc));
	t_depth_stencil_texture_desc.Width = in_w;
	t_depth_stencil_texture_desc.Height = in_h;
	t_depth_stencil_texture_desc.MipLevels = 1;
	t_depth_stencil_texture_desc.ArraySize = 1;
	t_depth_stencil_texture_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	t_depth_stencil_texture_desc.SampleDesc.Count = 1;
	t_depth_stencil_texture_desc.SampleDesc.Quality = 0;
	t_depth_stencil_texture_desc.Usage = D3D11_USAGE_DEFAULT;
	t_depth_stencil_texture_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	t_depth_stencil_texture_desc.CPUAccessFlags = 0;
	t_depth_stencil_texture_desc.MiscFlags = 0;
	result = device->CreateTexture2D(&t_depth_stencil_texture_desc, NULL, &depth_stencil_tex);
	if (FAILED(result)) return -3012;

	//De esto no hemos hablado asi que un poco de detalle no vendra mal.
	//El depth/stencil es otro buffer igual en tamaño que el backbuffer pero con diferente formato.
	//Para que sirve? Pues para poner orden en el backbuffer. Cada pixel tiene su profundidad, entonces
	//antes de lograr entrar en el backbuffer primero se testea contra el depth/stencil.
	//Si en la misma posicion ya hay un pixel con mas profundidad, entonces pasara el nuevo pixel, si no,
	//el nuevo sera descartado. Tambien pasara si no hay ninguno. Esto es el depth.
	//El stencil, es igual que el depth unicamente que este lo podemos manipular. Tenemos la oportunidad
	//de decidir si un pixel que ha pasado el depth pasa al backbuffer o no.
	D3D11_DEPTH_STENCIL_VIEW_DESC t_depth_stencil_desc;
	ZeroMemory(&t_depth_stencil_desc, sizeof(t_depth_stencil_desc));
	t_depth_stencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	t_depth_stencil_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	t_depth_stencil_desc.Texture2D.MipSlice = 0;
	result = device->CreateDepthStencilView(depth_stencil_tex, &t_depth_stencil_desc, &depth_stencil);
	if (FAILED(result)) return -3013;

	//Y ya seteamos el primer estado, el backbuffer y el depth
	context->OMSetRenderTargets(1, &render_target, depth_stencil);

	//El viewport es un rectangulo que define la proyeccion del backbuffer.
	//En este caso, en su totalidad.
	D3D11_VIEWPORT view_port;
	view_port.Width = static_cast<FLOAT>(in_w);
	view_port.Height = static_cast<FLOAT>(in_h);
	view_port.MinDepth = 0.0f;
	view_port.MaxDepth = 1.0f;
	view_port.TopLeftX = 0;
	view_port.TopLeftY = 0;

	//Y lo seteamos
	context->RSSetViewports(1, &view_port);

	//SHADERS. La parte programable del pipeline.
	ID3D10Blob* t_blob = nullptr;
	ID3D10Blob* t_blob_err = nullptr;

	//->Vertex Shader. Se porcesa por cada vertice de la geometria.
	//Tiene dos buffers de entrada. Esta informacion se la pasamos nosotros.
	//Una es la ya multiplicacion entre las matrices de vista y proyeccion.
	//La otra es la matriz del objeto a dibujar.
	//Le tenemos que definir tambien, dos estructuras. Una de entrada, que coincide con el formato del vertice.
	//En este caso, la posicion y el color.
	//Y una estructura de salida que contiene la informacion que sale del shader hacia el siguiente paso del pipeline.
	//Dentro transforma nuestro vertice al mundo que vemos, y pasa el color a salida.
	string t_shader_vertex =
		"cbuffer BUFFER_VIEW_PROJ : register(b0)               \n"
		"{                                                     \n"
		"   matrix mat_viewproj;                               \n"
		"};                                                    \n"
		"                                                      \n"
		"cbuffer BUFFER_PROJ : register(b1)                    \n"
		"{                                                     \n"
		"   matrix mat_world;                                  \n"
		"};                                                    \n"
		"                                                      \n"
		"                                                      \n"
		"struct VS_IN                                          \n"
		"{                                                     \n"
		"   float3 pos : POSITION;                             \n"
		"   float4 col : COLOR;                                \n"
		"};                                                    \n"
		"                                                      \n"
		"struct PS_IN                                          \n"
		"{                                                     \n"
		"   float4 pos : SV_POSITION;                          \n"
		"   float4 col : COLOR;                                \n"
		"};                                                    \n"
		"                                                      \n"
		"PS_IN VS(VS_IN IN)                                    \n"
		"{                                                     \n"
		"   PS_IN OUT = (PS_IN)0;                              \n"
		"                                                      \n"
		"   OUT.pos = mul(float4(IN.pos, 1.0f), mat_world);    \n"
		"   OUT.pos = mul(OUT.pos, mat_viewproj);              \n"
		"   OUT.col = IN.col;                                  \n"
		"                                                      \n"
		"    return OUT;                                       \n"
		"}                                                     \n";

	result = D3DCompile(t_shader_vertex.c_str(), t_shader_vertex.length(), "VS", NULL, NULL, "VS",
		"vs_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&t_blob, &t_blob_err);
	if (FAILED(result))
	{
		if (t_blob_err)
		{
			const char* str = static_cast<const char*>(t_blob_err->GetBufferPointer());
			int size = static_cast<int>(t_blob_err->GetBufferSize());

			wstring t_err_msg;
			int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)size, NULL, 0);
			t_err_msg.resize(size_needed);
			MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)size, &t_err_msg[0], size_needed);

			MessageBox(in_hwnd, t_err_msg.c_str(), L"vertex shader", 0);
		}

		X_RELEASE(t_blob);
		X_RELEASE(t_blob_err);

		return -3014;
	}

	result = device->CreateVertexShader(t_blob->GetBufferPointer(),
		t_blob->GetBufferSize(),
		NULL, &shader_vertex);
	if (FAILED(result)) return -3015;

	//Indicamos la composicion de un vertice al sistema para que sepa donde empieza el siguiente y su contenido.
	//En este caso la posicion y el color.
	D3D11_INPUT_ELEMENT_DESC t_input[2] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	result = device->CreateInputLayout(t_input,
		2,
		t_blob->GetBufferPointer(),
		t_blob->GetBufferSize(),
		&input_layout);
	if (FAILED(result)) return -3016;

	//->Pixel Shader. Se procesa una vaz por cada pixel.
	//Recibe en una estructura de entrada, igual a la del anterior paso, en este caso el vertex shader.
	//De aqui devolvemos el color a salida.
	string t_ps =
		"struct PS_IN                                                \n"
		"{                                                           \n"
		"   float4 pos : SV_POSITION;                                \n"
		"   float4 col : COLOR;                                      \n"
		"};                                                          \n"
		"                                                            \n"
		"float4 PS(PS_IN IN): SV_TARGET                              \n"
		"{                                                           \n"
		"   return float4(IN.col.r, IN.col.g, IN.col.b, IN.col.a);   \n"
		"}                                                           \n";
	result = D3DCompile(t_ps.c_str(), t_ps.length(), "PS", NULL, NULL, "PS",
		"ps_4_0", D3D10_SHADER_ENABLE_STRICTNESS, 0,
		&t_blob, &t_blob_err);
	if (FAILED(result))
	{
		if (t_blob_err)
		{
			const char* str = static_cast<const char*>(t_blob_err->GetBufferPointer());
			int size = static_cast<int>(t_blob_err->GetBufferSize());

			wstring t_err_msg;
			int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(size), NULL, 0);
			t_err_msg.resize(size_needed);
			MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(size), &t_err_msg[0], size_needed);

			MessageBox(in_hwnd, t_err_msg.c_str(), L"pixel shader", 0);
		}

		X_RELEASE(t_blob);
		X_RELEASE(t_blob_err);
		

		return -3017;
	}

	result = device->CreatePixelShader(t_blob->GetBufferPointer(),
		t_blob->GetBufferSize(),
		NULL, &shader_pixel);
	if (FAILED(result)) return -3018;

	X_RELEASE(t_blob);
	X_RELEASE(t_blob_err);

	//->Creamos los buffers para las matrices que vamos a pasarle al vertex shader.
	D3D11_BUFFER_DESC t_buffer_desc;
	ZeroMemory(&t_buffer_desc, sizeof(t_buffer_desc));
	t_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	t_buffer_desc.ByteWidth = sizeof(XMMATRIX);
	t_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	result = device->CreateBuffer(&t_buffer_desc, 0, &buff_viewproj);
	if (FAILED(result)) return -3019;

	result = device->CreateBuffer(&t_buffer_desc, 0, &buff_world);
	if (FAILED(result)) return -3020;

	//Y seteamos los shaders y sus buffers.
	context->VSSetShader(shader_vertex, NULL, 0);
	context->PSSetShader(shader_pixel, NULL, 0);
	context->IASetInputLayout(input_layout);
	context->VSSetConstantBuffers(0, 1, &buff_viewproj);
	context->VSSetConstantBuffers(1, 1, &buff_world);

	//A continuacion creamos el vertex buffer.
	//Como sabemos se trata de poner en un buffer todos los vertices unicos.
	//En este caso, vamos a crear un cubo donde cada vertice tiene su propio color.
	//Cada vertice tiene 7 floats. 3 para posicion y 4 para el color.
	//Que se podrian agrupar en una estructura Vertex o cualquier cosa asi? Si, se podria.
	//Pero intentamos enseñar como se ve el buffer en si, un flujo de floats, uno tras otro.
	vector<float> t_verts;
	t_verts.reserve(8 * 7);

	t_verts.emplace_back(-0.5f);
	t_verts.emplace_back(0.5f);
	t_verts.emplace_back(-0.5f);
	t_verts.emplace_back(0.7f);
	t_verts.emplace_back(0.9f);
	t_verts.emplace_back(0.11f);
	t_verts.emplace_back(1.0f);

	t_verts.emplace_back(0.5f);
	t_verts.emplace_back(0.5f);
	t_verts.emplace_back(-0.5f);
	t_verts.emplace_back(0.7f);
	t_verts.emplace_back(0.9f);
	t_verts.emplace_back(0.11f);
	t_verts.emplace_back(1.0f);

	t_verts.emplace_back(0.5f);
	t_verts.emplace_back(-0.5f);
	t_verts.emplace_back(-0.5f);
	t_verts.emplace_back(0.7f);
	t_verts.emplace_back(0.9f);
	t_verts.emplace_back(0.11f);
	t_verts.emplace_back(1.0f);

	t_verts.emplace_back(-0.5f);
	t_verts.emplace_back(-0.5f);
	t_verts.emplace_back(-0.5f);
	t_verts.emplace_back(0.7f);
	t_verts.emplace_back(0.9f);
	t_verts.emplace_back(0.11f);
	t_verts.emplace_back(1.0f);

	t_verts.emplace_back(-0.5f);
	t_verts.emplace_back(0.5f);
	t_verts.emplace_back(0.5f);
	t_verts.emplace_back(0.11f);
	t_verts.emplace_back(0.7f);
	t_verts.emplace_back(0.9f);
	t_verts.emplace_back(1.0f);

	t_verts.emplace_back(0.5f);
	t_verts.emplace_back(0.5f);
	t_verts.emplace_back(0.5f);
	t_verts.emplace_back(0.11f);
	t_verts.emplace_back(0.7f);
	t_verts.emplace_back(0.9f);
	t_verts.emplace_back(1.0f);

	t_verts.emplace_back(0.5f);
	t_verts.emplace_back(-0.5f);
	t_verts.emplace_back(0.5f);
	t_verts.emplace_back(0.11f);
	t_verts.emplace_back(0.7f);
	t_verts.emplace_back(0.9f);
	t_verts.emplace_back(1.0f);

	t_verts.emplace_back(-0.5f);
	t_verts.emplace_back(-0.5f);
	t_verts.emplace_back(0.5f);
	t_verts.emplace_back(0.11f);
	t_verts.emplace_back(0.7f);
	t_verts.emplace_back(0.9f);
	t_verts.emplace_back(1.0f);

	D3D11_BUFFER_DESC t_vb_desc;
	t_vb_desc.Usage = D3D11_USAGE_DEFAULT;
	t_vb_desc.ByteWidth = static_cast<UINT>(sizeof(float) * t_verts.size());
	t_vb_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	t_vb_desc.CPUAccessFlags = 0;
	t_vb_desc.MiscFlags = 0;
	t_vb_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA t_vb_data;
	t_vb_data.pSysMem = t_verts.data();
	t_vb_data.SysMemPitch = 0;
	t_vb_data.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&t_vb_desc, &t_vb_data, &vb);
	if (FAILED(result)) return -3021;

	//Despues creamos el index buffer donde construimos nuestro objeto.
	//Cada valor apunta al vertice del vertex buffer a la posicion indicada.
	vector<short> t_indexs;
	t_indexs.reserve(static_cast<size_t>(6 * 6));
	
	t_indexs.emplace_back(0);
	t_indexs.emplace_back(1);
	t_indexs.emplace_back(2);
	t_indexs.emplace_back(0);
	t_indexs.emplace_back(2);
	t_indexs.emplace_back(3);

	t_indexs.emplace_back(6);
	t_indexs.emplace_back(5);
	t_indexs.emplace_back(4);
	t_indexs.emplace_back(7);
	t_indexs.emplace_back(6);
	t_indexs.emplace_back(4);

	t_indexs.emplace_back(3);
	t_indexs.emplace_back(7);
	t_indexs.emplace_back(6);
	t_indexs.emplace_back(6);
	t_indexs.emplace_back(2);
	t_indexs.emplace_back(3);

	t_indexs.emplace_back(0);
	t_indexs.emplace_back(4);
	t_indexs.emplace_back(5);
	t_indexs.emplace_back(5);
	t_indexs.emplace_back(1);
	t_indexs.emplace_back(0);

	t_indexs.emplace_back(7);
	t_indexs.emplace_back(4);
	t_indexs.emplace_back(0);
	t_indexs.emplace_back(3);
	t_indexs.emplace_back(7);
	t_indexs.emplace_back(0);

	t_indexs.emplace_back(1);
	t_indexs.emplace_back(5);
	t_indexs.emplace_back(2);
	t_indexs.emplace_back(5);
	t_indexs.emplace_back(6);
	t_indexs.emplace_back(2);

	D3D11_BUFFER_DESC t_ib_desc;
	t_ib_desc.Usage = D3D11_USAGE_DEFAULT;
	t_ib_desc.ByteWidth = static_cast<UINT>(sizeof(short) * t_indexs.size());
	t_ib_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	t_ib_desc.CPUAccessFlags = 0;
	t_ib_desc.MiscFlags = 0;
	t_ib_desc.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA t_ib_data;
	t_ib_data.pSysMem = t_indexs.data();
	t_ib_data.SysMemPitch = 0;
	t_ib_data.SysMemSlicePitch = 0;

	result = device->CreateBuffer(&t_ib_desc, &t_ib_data, &ib);
	if (FAILED(result)) return -3022;

	//Y seteamos los bufferes
	UINT t_stride[1];
	t_stride[0] = sizeof(float) * 7;

	UINT t_offset[1];
	t_offset[0] = 0;

	ID3D11Buffer* buffs[1];
	buffs[0] = vb;

	context->IASetVertexBuffers(0, 1, buffs, t_stride, t_offset);
	context->IASetIndexBuffer(ib, DXGI_FORMAT_R16_UINT, 0);

	//Indicamos que el tipo de geometria utilizada va a ser una lista de triangulos.
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Como no vamos a mover mas ni la camara para ver en nuestro mundo ni la proyeccion.
	//Las creamos en el inicio, las multiplicamos y seteamos el valor al shader.
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XMConvertToRadians(45.0f), static_cast<float>(in_w) / static_cast<float>(in_h), 1.0f, 100.0f);
	XMVECTOR view_pos {-7.0f, 2.0f, 0.0f};
	XMVECTOR view_target {0.0f, 0.0f, 0.0f};
	XMVECTOR view_up {0.0f, 1.0f, 0.0f};
	XMMATRIX view = XMMatrixLookAtLH(view_pos, view_target, view_up);

	XMMATRIX viewproj = XMMatrixMultiply(view, proj);
	viewproj = XMMatrixTranspose(viewproj);

	context->UpdateSubresource(buff_viewproj, 0, 0, &viewproj, 0, 0);

	return 0;
}

//Aqui es donde vamos a dibujar nuestra escena.
//Esta funcion hay que llamara todas las veces que se actualiza la ventana.
void Direct3D::render()
{
	//Limpiamos los backbuffers.
	float color[4] = { 0.47f, 0.47f, 0.47f, 1.0f };
	context->ClearRenderTargetView(render_target, color);
	context->ClearDepthStencilView(depth_stencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

	//Creamos la matriz del objeto a dibujar y la actualizamos en el shader.
	XMMATRIX mat = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	XMMATRIX mat_rot = XMMatrixRotationY(::timeGetTime() / 1000.0f);
	mat = XMMatrixMultiply(mat, mat_rot);
	mat = XMMatrixTranspose(mat);
	context->UpdateSubresource(buff_world, 0, 0, &mat, 0, 0);

	//Dibujamos el objeto.
	context->DrawIndexed(36, 0, 0);

	//Indicamos que hemos terminado la escena y que se puede pasar a su presentacion.
	swapchain->Present(0, 0);
}

//Eliberamos la memoria de los objetos utilizados.
//Esta funcion se deberia llamar al terminar la aplicacion.
void Direct3D::x_it()
{
	X_RELEASE(ib);
	X_RELEASE(vb);
	X_RELEASE(input_layout);
	X_RELEASE(buff_world);
	X_RELEASE(buff_viewproj);
	X_RELEASE(shader_pixel);
	X_RELEASE(shader_vertex);
	X_RELEASE(depth_stencil_tex);
	X_RELEASE(depth_stencil);
	X_RELEASE(render_target_tex);
	X_RELEASE(render_target);
	X_RELEASE(context);
	X_RELEASE(swapchain);
	X_RELEASE(adapter);
	X_RELEASE(factory);
	X_RELEASE(device);
}