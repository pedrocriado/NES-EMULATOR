#include <stdio.h>
#include <SDL.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>
#include <SDL_syswm.h>
#else
#include <errno.h>
#include <unistd.h>
#if defined(GRAPHICS_HAS_GTK)
#include <gtk/gtk.h>
#endif
#endif

#include "Graphics.h"

#ifdef _WIN32
static void Graphics_windows_menu(Graphics* grap);
#endif

#ifdef __linux__
static bool Graphics_prompt_with_gtk(char* outPath, size_t outSize);
static bool Graphics_prompt_with_zenity(char* outPath, size_t outSize);
#endif

void Graphics_init(Graphics* grap)
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO |
        SDL_INIT_EVENTS) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

    // Create a window
    grap->window = SDL_CreateWindow(
        "NES Emulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        PIXEL_WIDTH * PIXEL_SCALE,
        PIXEL_HEIGHT * PIXEL_SCALE,
        SDL_WINDOW_SHOWN
    );

#ifdef _WIN32
    Graphics_windows_menu(grap);
#endif

    grap->renderer = SDL_CreateRenderer(grap->window, -1, SDL_RENDERER_ACCELERATED);
    if(!grap->renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(grap->window);
        SDL_Quit();
        grap->window = NULL;
        return;
    }

    SDL_RenderSetLogicalSize(grap->renderer,
        PIXEL_WIDTH * PIXEL_SCALE,
        PIXEL_HEIGHT * PIXEL_SCALE);

    grap->texture = SDL_CreateTexture(
        grap->renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        PIXEL_WIDTH,
        PIXEL_HEIGHT
    );
}

void Graphics_render(Graphics* grap, uint32_t* screen)
{
    SDL_UpdateTexture(grap->texture, NULL, screen, PIXEL_WIDTH * sizeof(uint32_t));
    SDL_RenderClear(grap->renderer);
    SDL_RenderCopy(grap->renderer, grap->texture, NULL, NULL);
    SDL_RenderPresent(grap->renderer);
}

void Graphics_free(Graphics* grap)
{
    SDL_DestroyTexture(grap->texture);
    SDL_DestroyRenderer(grap->renderer);
    SDL_DestroyWindow(grap->window);
    SDL_Quit();
}

bool Graphics_prompt_rom_selection(Graphics* grap, char* outPath, size_t outSize)
{
    if(!outPath || outSize == 0)
        return false;

#ifdef _WIN32
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    HWND owner;
    if(SDL_GetWindowWMInfo(grap->window, &wmInfo)) {
        owner = wmInfo.info.win.window;
    }

    char buffer[GRAPHICS_DIALOG_PATH_MAX] = {0};
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = "NES ROMs (*.nes)\0*.nes\0All Files\0*.*\0";
    ofn.lpstrFile = buffer;
    ofn.nMaxFile = GRAPHICS_DIALOG_PATH_MAX;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    ofn.lpstrDefExt = "nes";

    if(GetOpenFileNameA(&ofn) == TRUE) {
        strncpy(outPath, buffer, outSize);
        outPath[outSize - 1] = '\0';
        return true;
    }
    return false;
#else
#if defined(GRAPHICS_HAS_GTK)
    if(Graphics_prompt_with_gtk(outPath, outSize))
        return true;
#endif
    return Graphics_prompt_with_zenity(outPath, outSize);
#endif
}

#ifdef _WIN32
static void Graphics_windows_menu(Graphics* grap)
{
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if(!SDL_GetWindowWMInfo(grap->window, &wmInfo)) {
        SDL_Log("SDL_GetWindowWMInfo failed: %s", SDL_GetError());
        return;
    }

    HWND hwnd = wmInfo.info.win.window;
    if(!hwnd)
        return;

    HMENU menuBar = CreateMenu();
    HMENU fileMenu = CreatePopupMenu();
    if(!menuBar || !fileMenu) {
        SDL_Log("Failed to create Win32 menu.");
        return;
    }

    AppendMenuW(fileMenu, MF_STRING, GRAPHICS_MENU_CMD_OPEN_ROM, L"&Open ROM...");
    AppendMenuW(menuBar, MF_POPUP, (UINT_PTR)fileMenu, L"&File");

    if(!SetMenu(hwnd, menuBar)) {
        SDL_Log("Failed to set Win32 menu.");
        DestroyMenu(fileMenu);
        DestroyMenu(menuBar);
        return;
    }

    SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
}
#endif

#if defined(GRAPHICS_HAS_GTK)
static bool Graphics_prompt_with_gtk(char* outPath, size_t outSize)
{
    if(!gtk_init_check(NULL, NULL))
        return false;

    GtkWidget* dialog = gtk_file_chooser_dialog_new(
        "Select NES ROM",
        NULL,
        GTK_FILE_CHOOSER_ACTION_OPEN,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Open", GTK_RESPONSE_ACCEPT,
        NULL
    );
    if(!dialog)
        return false;

    GtkFileChooser* chooser = GTK_FILE_CHOOSER(dialog);

    GtkFileFilter* nesFilter = gtk_file_filter_new();
    gtk_file_filter_set_name(nesFilter, "NES ROMs (*.nes)");
    gtk_file_filter_add_pattern(nesFilter, "*.nes");
    gtk_file_chooser_add_filter(chooser, nesFilter);

    GtkFileFilter* allFilter = gtk_file_filter_new();
    gtk_file_filter_set_name(allFilter, "All Files");
    gtk_file_filter_add_pattern(allFilter, "*");
    gtk_file_chooser_add_filter(chooser, allFilter);

    bool accepted = false;
    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(chooser);
        if(filename) {
            strncpy(outPath, filename, outSize);
            outPath[outSize - 1] = '\0';
            g_free(filename);
            accepted = true;
        }
    }

    gtk_widget_destroy(dialog);
    while(gtk_events_pending())
        gtk_main_iteration_do(FALSE);

    return accepted;
}
#endif

static bool Graphics_prompt_with_zenity(char* outPath, size_t outSize)
{
    FILE* pipe = popen("zenity --file-selection --title=\"Select NES ROM\" --file-filter=\"NES ROMs (*.nes) | *.nes\" --file-filter=\"All files | *\"", "r");

    char buffer[GRAPHICS_DIALOG_PATH_MAX];
    bool result = false;

    if(fgets(buffer, sizeof(buffer), pipe) != NULL) {
        size_t len = strlen(buffer);
        if(len > 0 && buffer[len - 1] == '\n')
            buffer[len - 1] = '\0';
        strncpy(outPath, buffer, outSize);
        outPath[outSize - 1] = '\0';
        result = true;
    }

    pclose(pipe);
    return result;
}
