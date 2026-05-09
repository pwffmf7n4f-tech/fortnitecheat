// main.cpp
#include <windows.h>
#include <d3d11.h>
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <vector>
#include <cstdint>
#pragma comment(lib,"d3d11.lib")

extern "C" {
#include "../shared.h"
}

HANDLE hDrv;
DWORD g_pid;

struct Vec3 { float x,y,z; };
struct Actor {
	uint64_t base;
	Vec3 pos, head;
	bool bTeammate;
};

#define UWorld 0x10C8A580        // Fortnite 30.10
#define UGameInstance 0x1C0
#define ULocalPlayer 0x38
#define PlayerController 0x30
#define AcknowledgedPawn 0x2A0
#define RootComponent 0x190
#define Mesh 0x310
#define BoneArray 0x5F0
#define ComponentToWorld 0x250
#define CustomTimeDilation 0xD4
#define TeamIndex 0x10FC

template<typename T> T read(uint64_t addr) {
	T val; ReadProcessMemory(g_pid,(void*)addr,&val,sizeof(T),NULL); return val; }

Vec3 getbone(uint64_t mesh, int idx) {
	auto arr = read<uint64_t>(mesh + BoneArray);
	FTransform comp = read<FTransform>(mesh + ComponentToWorld);
	FTransform bone  = read<FTransform>(arr + idx*sizeof(FTransform));
	return Vec3Transform(bone.ToMatrix(), comp.ToMatrix()).W;
}

std::vector<Actor> gather() {
	std::vector<Actor> v;
	auto world = read<uint64_t>(UWorld);
	auto pers = read<uint64_t>(world + 0x160);
	auto actors = read<uint64_t>(pers + 0x98);
	int count = read<int>(pers + 0xA0);
	for (int i=0;i<count;i++) {
		uint64_t a = read<uint64_t>(actors + i*0x8);
		if (!a) continue;
		auto mesh = read<uint64_t>(a + Mesh);
		if (!mesh) continue;
		Actor ac;
		ac.base = a;
		ac.pos  = getbone(mesh,0);
		ac.head = getbone(mesh,6);
		ac.bTeammate = read<uint8_t>(a + TeamIndex) == read<uint8_t>(g_pid + TeamIndex);
		if (!ac.bTeammate) v.push_back(ac);
	}
	return v;
}

void ESP(ImDraw::DrawList* dl, std::vector<Actor>& a, ImVec2 res) {
	for (auto& x: a) {
		ImVec2 head2d, root2d;
		if (!WorldToScreen(x.head, head2d, res)) continue;
		if (!WorldToScreen(x.pos, root2d, res)) continue;
		float h = root2d.y - head2d.y;
		ImRect box(head2d.x - h*0.3f, head2d.y,
			head2d.x + h*0.3f, root2d.y);
		dl->AddRect(box.Min, box.Max, IM_COL32(0,255,0,255));
		dl->AddLine(ImVec2(res.x/2,res.y), head2d, IM_COL32(255,0,0,255));
	}
}

int WINAPI WinMain(HINST,HINST,LPSTR,int) {
	AllocConsole(); freopen("CONOUT$","w",stdout);
	hDrv = CreateFileA("\\\\\\\\.\\\\FortWall",GENERIC_READ|GENERIC_WRITE,
		0,NULL,OPEN_EXISTING,0,NULL);
	if (hDrv==INVALID_HANDLE_VALUE) { puts("no driver"); return 1; }

	// find fortnite window
	HWND tar = NULL;
	while (!(tar = FindWindowA(NULL,"Fortnite  "))) Sleep(500);
	g_pid = GetWindowThreadProcessId(tar,NULL);

	// init D3D11 overlay
	IDXGISwapChain* sc;
	ID3D11Device* dev;
	ID3D11DeviceContext* ctx;
	DXGI_SWAP_CHAIN_DESC sd = {0};
	sd.BufferCount = 2;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = tar;
	sd.SampleDesc.Count = 1;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	D3D11CreateDeviceAndSwapChain(NULL,D3D_DRIVER_TYPE_HARDWARE,NULL,0,
		NULL,0,D3D11_SDK_VERSION,&sd,&sc,&dev,&ctx,NULL);

	ID3D11RenderTargetView* rt;
	ID3D11Texture2D* back;
	sc->GetBuffer(0,__uuidof(ID3D11Texture2D),(void**)&back);
	dev->CreateRenderTargetView(back,NULL,&rt);
	back->Release();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); io.IniFilename = NULL;
	ImGui_ImplWin32_Init(tar);
	ImGui_ImplDX11_Init(dev,ctx);

	MSG msg;
	bool wh=0,aim=0;
	while (!GetAsyncKeyState(VK_END)) {
		MSG msg; while(PeekMessage(&msg,NULL,0,0,PM_REMOVE)) {
			TranslateMessage(&msg); Dispatch(&msg);
		}
		if (GetForegroundWindow()!=tar) { continue; }
		ImGui_ImplDX11_NewFrame(); ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// menu
		ImGui::Begin("NocturAI");
		ImGui::Checkbox("Wallhack",&wh);
		ImGui::Checkbox("Aimbot",&aim);
		ImGui::End();

		// update driver
		GD gd={g_pid,hDrv,wh,aim};
		DeviceIoControl(hDrv,IOCTL_SET_FEATURES,&gd,sizeof(gd),NULL,0,&br,NULL);

		// ESP
		if (wh) {
			auto actors = gather();
			ESP(ImGui::GetBackgroundDrawList(), actors,
				ImVec2((float)io.DisplaySize.x, (float)io.DisplaySize.y));
		}

		ImGui::Render();
		ctx->OMSetRenderTargets(1,&rt,NULL);
		ctx->ClearRenderTargetView(rt,ImVec4(0,0,0,0));
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
		sc->Present(1,0);
	}
	ImGui_ImplDX11_Shutdown(); ImGui_ImplWin32_Shutdown(); ImGui::DestroyContext();
	sc->Release(); dev->Release(); ctx->Release();
	CloseHandle(hDrv);
	return 0;
}
