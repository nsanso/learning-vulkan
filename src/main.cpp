#include "engine/engine.h"

int main(int argc, char *argv[]) {
    GraphicsEngine engine;
    engine.run();
    return 0;
}

#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                   _In_ LPSTR lpCmdLine, _In_ int nShowCmd) {
    return main(__argc, __argv);
}
#endif
