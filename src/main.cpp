#include <cstdio>

#include "SDL.h"
#include "SDL_syswm.h"

#include <d3d11.h>

#define ReleaseIfNotNull(ComObj) if(ComObj) ComObj->Release(); ComObj = nullptr

int main(int argc, char** argv)
{
	printf("This program creates a window, then two DirectX devices." "\n");
	printf("The first Direct3D11 Device is headless." "\n");
	printf("The second one has a swapchain on the window" "\n");
	printf("A shared texture is allocated on the first device." "\n");
	printf("The shared texture handle is retreived." "\n");
	printf("The handle is opened as a texture by the second device." "\n");
	printf("The test program is successful if all DirectX call return 0, and if the handle is not null" "\n");

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_Window* window = SDL_CreateWindow(
		"Test",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		640,
		480,
		SDL_WINDOW_SHOWN);

	SDL_Event e{};

	const auto featureLevel = D3D_FEATURE_LEVEL_11_1;
	ID3D11Device*	pDeviceHeadless = nullptr;
	ID3D11DeviceContext* pDeviceHeadlessContext = nullptr;

	const auto deviceCreated = D3D11CreateDevice(nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		&featureLevel,
		1,
		D3D11_SDK_VERSION,
		&pDeviceHeadless,
		nullptr,
		&pDeviceHeadlessContext);

	printf("deviceCreated = %d\n", deviceCreated);

	SDL_SysWMinfo wmInfo{};
	SDL_GetVersion(&wmInfo.version);
	SDL_GetWindowWMInfo(window, &wmInfo);

	DXGI_SWAP_CHAIN_DESC desc{};
	desc.OutputWindow = wmInfo.info.win.window;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 2;
	desc.Windowed = true;

	ID3D11Device* pDeviceWindow = nullptr;
	ID3D11DeviceContext* pDeviceContextWindow = nullptr;
	IDXGISwapChain* pSwapchain = nullptr;

	const auto deviceAndSwapchainCreated =D3D11CreateDeviceAndSwapChain(nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		&featureLevel,
		1,
		D3D11_SDK_VERSION,
		&desc,
		&pSwapchain,
		&pDeviceWindow,
		nullptr,
		&pDeviceContextWindow);
	printf("deviceAndSwapchainCreated = %d\n", deviceAndSwapchainCreated);

	D3D11_TEXTURE2D_DESC texdesc{};
	texdesc.Width = 640;
	texdesc.Height = 480;
	texdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texdesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
	texdesc.Usage = D3D11_USAGE_DEFAULT;
	texdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	texdesc.MipLevels = texdesc.ArraySize = 1;
	texdesc.SampleDesc.Count = 1;

	ID3D11Texture2D* shareSource = nullptr;
	const auto textureHeadlessCreated = pDeviceHeadless->CreateTexture2D(&texdesc,
		nullptr,
		&shareSource);
	printf("textureHeadlessCreated = %d\n", textureHeadlessCreated);

	IDXGIResource* shareSourceResource = nullptr;
	const auto resourceInterfaceObtained = shareSource->QueryInterface(&shareSourceResource);
	printf("resourceInterfaceObtained = %d\n", resourceInterfaceObtained);

	HANDLE shareHandle = nullptr;
	const auto handleObtained = shareSourceResource->GetSharedHandle(&shareHandle);
	printf("handleObtained = %d\n", handleObtained);
	ReleaseIfNotNull(shareSourceResource);

	printf("If the DirectX implementation is patched DXVK, goal is that this handle is a UNIX FD that can be given back to DXVK and openned as a vulkan texture" "\n");
	printf("shareHandle = %llu\n", reinterpret_cast<uintptr_t>(shareHandle));
	printf("shareHandle as int ptr deref = %d\n", *reinterpret_cast<int*>(&shareHandle));
	printf("shareHandle as uint ptr deref = %u\n", *reinterpret_cast<unsigned*>(&shareHandle));

	ID3D11Texture2D* sharedDest = nullptr;
	const auto handleOpenned = pDeviceWindow->OpenSharedResource(shareHandle,
		__uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(&sharedDest));
	printf("handleOpenned = %d\n", handleOpenned);
	printf("sharedDest not null = %s\n", sharedDest ? "TRUE" : "FALSE");

	bool running = true;

	SDL_SetWindowTitle(window, "You can close me.");

	while(running)
	{
		while(SDL_PollEvent(&e)) switch(e.type)
		{
		case SDL_QUIT:
			running = false;
			break;
		default:
			break;
		}
	}

	// Close everything initialized via Direct3D 11
	if(handleObtained == S_OK)
		ReleaseIfNotNull(sharedDest); // This would crash under DXVK where the openning of the texture fails.
	ReleaseIfNotNull(shareSource);
	ReleaseIfNotNull(pDeviceContextWindow);
	ReleaseIfNotNull(pSwapchain);
	ReleaseIfNotNull(pDeviceWindow);
	ReleaseIfNotNull(pDeviceHeadlessContext);
	ReleaseIfNotNull(pDeviceHeadless);

	// Close everything initialized via SDL
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}