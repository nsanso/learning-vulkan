#include "graphics/engine.h"

int main(int argc, char *argv[]) {
    GraphicsEngine engine;
    engine.run();
    return 0;
}

#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                   _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
#ifndef NDEBUG
    AllocConsole();
    freopen_s((FILE **)stdout, "CONOUT$", "w", stdout);
    freopen_s((FILE **)stderr, "CONOUT$", "w", stderr);
    printf("Console attached!\n");
#endif
    int out = main(__argc, __argv);
    Sleep(1000);
    return out;
}
#endif
