#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <map>
#include <string>
#include <chrono>

#include "../bogo.h"

#ifdef USE_IMGUI
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_sdl2.h"
#include "../imgui/imgui_impl_sdlrenderer2.h"
#endif


class UI
{
public:
    UI(int w, int h);
    ~UI();

    void update();
    void draw();

    void rect(int pos, int size, int total_bars);
    void render_number(const char* num);
    void text(std::string text, SDL_Rect dest);
    bool success;
    bool running;

    char* current_iteration;
    int total_iterations;
    std::chrono::steady_clock::time_point start_time;
#ifdef USE_IMGUI
    ImGuiIO io;
#endif // USE_IMGUI

private:
    int screen_w;
    int screen_h;

    void render_metadata();

    SDL_Window* m_window;
    SDL_Renderer* m_window_renderer;
    SDL_Event    m_window_event;
    std::map<int, SDL_Rect> m_rects;

    Uint32 startTicks;
    Uint64 startPerf;
    Uint64 totalFrameTicks;
    Uint64 totalFrames;
    TTF_Font* font;
#ifdef USE_IMGUI
    bool show_tool_metrics = true;
    bool show_tool_debug_log = false;
#endif // USE_IMGUI

};