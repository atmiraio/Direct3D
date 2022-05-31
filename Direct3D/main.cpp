//las cabeceras con las funcionalidades de windows
#include <Windows.h>

//A
#include "Direct3D.h"

//el prototipo de la funcion que donde se procesan los eventos de la ventana
LRESULT CALLBACK prv_window_proc(_In_ HWND in_hwnd, _In_ UINT in_msg, _In_ WPARAM in_wparam, _In_ LPARAM in_lparam);

//el punto de partida de programa
int WINAPI WinMain(_In_ HINSTANCE in_hInstance, _In_opt_ HINSTANCE in_hprev_instance, _In_ LPSTR in_cmd, _In_ int in_show)
{
	//mediante esta estructura definimos las propiedades de la ventana
	//para luego registrarla en el sistema
	WNDCLASSEX t_wcex;
	t_wcex.cbSize = sizeof(WNDCLASSEX);
	t_wcex.style = CS_HREDRAW | CS_VREDRAW;
	t_wcex.lpfnWndProc = &prv_window_proc;
	t_wcex.cbClsExtra = 0;
	t_wcex.cbWndExtra = 0;
	t_wcex.hInstance = in_hInstance;
	t_wcex.hIcon = LoadIcon(in_hInstance, IDI_APPLICATION);
	t_wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	t_wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_DESKTOP);
	t_wcex.lpszMenuName = NULL;
	t_wcex.lpszClassName = L"Direct3D";
	t_wcex.hIconSm = LoadIcon(t_wcex.hInstance, IDI_APPLICATION);

	//registramos la ventana en el sistema
	if (!RegisterClassEx(&t_wcex))
	{
		return -2;
	}

	//creamos la ventana. esta funcion nos devolvera un handler para
	//tener una referencia a la ventana
	HWND hwnd = CreateWindow(L"Direct3D", L"Direct3D",
		WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
		50, 50, 800, 600,
		NULL, NULL, in_hInstance, NULL);

	//si no hemos conseguido crear la ventana, salimos
	if (!hwnd)
	{
		return -3;
	}
	else {

		//si se ha logrado crear la ventana, la mostramos y
		//enviamos una señal para que el sistema actualice la ventana
		ShowWindow(hwnd, SW_SHOW);
		UpdateWindow(hwnd);
	}

	//B
	Direct3D* d3d = new Direct3D();
	if (int ret = d3d->init(800, 600, hwnd) != 0)
	{
		return ret;
	}

	//la estructura donde vamos a recibir los mensajes que la ventana nos envia
	MSG t_msg = { 0 };

	//un bucle hasta que el mensaje que nos envia la ventana no sea el de salir
	while (t_msg.message != WM_QUIT)
	{
		//si hay algun mensaje asociado con la ventana
		if (PeekMessage(&t_msg, NULL, 0, 0, PM_REMOVE))
		{
			//traduce los mensajes de tecla virtuales a chars
			TranslateMessage(&t_msg);
			//envia el mensaje recuperado por GetMessage al procesador de la ventana
			DispatchMessage(&t_msg);
		}
		else {
			//C
			d3d->render();
		}
	}

	//hemos terminado

	//D
	d3d->x_it();
	d3d = nullptr;

	//quitamos la ventana del sistema
	UnregisterClass(L"Direct3D", in_hInstance);

	return 0;
}

//->functions
LRESULT CALLBACK prv_window_proc(_In_ HWND in_hwnd, _In_ UINT in_msg, _In_ WPARAM in_wparam, _In_ LPARAM in_lparam)
{
	switch (in_msg)
	{
		//si se destruye la ventana (cuando le damos a la X para cerrar por ejemplo), 
		//avisar al sistema que cierre la aplicacion
		//(publica el mensaje WM_QUIT para la ventana)
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_KEYDOWN:
		switch (in_wparam)
		{
			//si pulsamos escape, avisar al sistema que cierre la aplicacion
			//(publica el mensaje WM_QUIT para la ventana)
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}
	default:
		//devuelve los demas mensajes no procesados (si los hay)
		return DefWindowProc(in_hwnd, in_msg, in_wparam, in_lparam);
	}
}