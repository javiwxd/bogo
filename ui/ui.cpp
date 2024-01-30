#include "ui.h"

UI::UI(int w, int h)
{

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cout << "Error initializing SDL: " << SDL_GetError() << std::endl;
        return;
    }

#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    TTF_Init();

    screen_w = w;
    screen_h = h;
    running = true;

    totalFrameTicks = 0;
    totalFrames = 0;

    m_window = SDL_CreateWindow("Bogo",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        w, h,
        0);

    if (!m_window)
    {
        std::cout << "Failed to initialize the SDL2 window" << std::endl;
        std::cout << "SDL2 Error: " << SDL_GetError() << std::endl;
        return;
    }

    m_window_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (!m_window_renderer)
    {
		std::cout << "Failed to create renderer" << std::endl;
		std::cout << "SDL2 Error: " << SDL_GetError() << std::endl;
		return;
	}

    font = TTF_OpenFont("OpenSans.ttf", 60);
    if (!font)
    {
		std::cout << "Failed to load font" << std::endl;
		std::cout << "SDL2 Error: " << SDL_GetError() << std::endl;
		return;
	}

#ifdef USE_IMGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& imgui_io = ImGui::GetIO(); (void)imgui_io;
    imgui_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    imgui_io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    io = imgui_io;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(m_window, m_window_renderer);
    ImGui_ImplSDLRenderer2_Init(m_window_renderer);
#endif

}

UI::~UI()
{
#ifdef USE_IMGUI
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
#endif
    SDL_DestroyRenderer(m_window_renderer);
    SDL_DestroyWindow(m_window);
    TTF_Quit();
}

void UI::update()
{
    while (running)
    {
        totalFrames++;
        startTicks = SDL_GetTicks();
        startPerf = SDL_GetPerformanceCounter();

        while (SDL_PollEvent(&m_window_event) > 0)
        {
#ifdef USE_IMGUI
            ImGui_ImplSDL2_ProcessEvent(&m_window_event);
#endif
            switch (m_window_event.type)
            {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_WINDOWEVENT:
            {
                switch (m_window_event.window.event)
                {
                case SDL_WINDOWEVENT_CLOSE:
                    if (m_window_event.window.windowID == SDL_GetWindowID(m_window))
                        running = false;
                    break;
                }
            }
            case SDL_KEYDOWN:
            {
                switch (m_window_event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    running = false;
                    break;
                    // Q key
                case SDLK_q:
                    running = false;
                    break;
#ifdef USE_IMGUI
                    // D key
                case SDLK_d:
                    show_tool_debug_log = !show_tool_debug_log;
                    break;
                    // M key
                case SDLK_m:
                    show_tool_metrics = !show_tool_metrics;
                    break;
#endif // USE_IMGUI

                }
            }
            }
        }
        if (running)
            draw();
    }
}

void UI::render_metadata() {

    Uint32 endTicks = SDL_GetTicks();
    Uint64 endPerf = SDL_GetPerformanceCounter();
    Uint64 framePerf = endPerf - startPerf;
    float frameTime = (endTicks - startTicks) / 1000.0f;
    totalFrameTicks += endTicks - startTicks;

    std::string fps = "Current FPS: " + std::to_string(1.0f / frameTime);
    std::string avg = "Average FPS: " + std::to_string(1000.0f / ((float)totalFrameTicks / totalFrames));
    std::string perf = "Current Perf: " + std::to_string(framePerf);

    std::string current_num = "Current Number: " + std::string(current_iteration);
    std::string total_num = "Total Iterations: " + std::to_string(total_iterations);

    double interations_per_second = static_cast<double>(total_iterations) / std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start_time).count();
    std::string ips = "Iterations Per Second: " + std::to_string(interations_per_second);
    ips = ips.substr(0, ips.find(".") + 1);

    SDL_Rect metadata_rect;
    metadata_rect.x = 0;
    metadata_rect.y = 0;
    metadata_rect.w = screen_w / 10;
    metadata_rect.h = 20;
    text(fps, metadata_rect);

    metadata_rect.y = 20;
    text(avg, metadata_rect);

    metadata_rect.y = 40;
    text(perf, metadata_rect);

    metadata_rect.x = screen_w / 10 * 8;
    metadata_rect.y = 0;
    metadata_rect.w = screen_w / 10 * 2;
    metadata_rect.h = 20;
    text(current_num, metadata_rect);


    metadata_rect.y = 20;
    text(total_num, metadata_rect);

    metadata_rect.y = 40;
    text(ips, metadata_rect);

}

void UI::draw()
{

    SDL_RenderClear(m_window_renderer);

#ifdef USE_IMGUI
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
#endif

    if (success)
        SDL_SetRenderDrawColor(m_window_renderer, 0, 255, 0, 255);
    else 
        SDL_SetRenderDrawColor(m_window_renderer, 255, 255, 255, 255);

    for (int i = 0; i < m_rects.size(); ++i)
    {
        auto rect = m_rects[i];
        SDL_RenderDrawRect(m_window_renderer, &rect);
        SDL_RenderFillRect(m_window_renderer, &rect);
    }

    render_metadata();

    SDL_SetRenderDrawColor(m_window_renderer, 0, 0, 0, 255);

#ifdef USE_IMGUI

    if (show_tool_metrics)
        ImGui::ShowMetricsWindow(&show_tool_metrics);
    if (show_tool_debug_log)
        ImGui::ShowDebugLogWindow(&show_tool_debug_log);

    ImGui::Render();
    SDL_RenderSetScale(m_window_renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
#endif

    SDL_RenderPresent(m_window_renderer);

}

void UI::rect(int pos, int size, int total_bars)
{

    if (size > 10)
        return;

    SDL_Rect rect;
    rect.x = (screen_w / total_bars) * pos;
    rect.y = screen_h - 10;
    rect.w = (screen_w / total_bars) - 1;
    rect.h = - ((screen_h / 10) * size - 60);

    m_rects[pos] = rect;

    /*SDL_RenderClear(m_window_renderer);

    SDL_Rect rect;
    rect.x = (screen_w / total_bars) * pos + 5;
    rect.y = 5;
    rect.w = (screen_w / total_bars) - total_bars - 10;
    rect.h = (screen_h / 10) * size - 50;

    SDL_SetRenderDrawColor(m_window_renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(m_window_renderer, &rect);
    SDL_RenderFillRect(m_window_renderer, &rect);
    SDL_SetRenderDrawColor(m_window_renderer, 0, 0, 0, 255);

    SDL_RenderPresent(m_window_renderer);*/
}

void UI::render_number(const char* num)
{
    int length = std::strlen(num);

    for (int i = 0; i < length; ++i) {
        int digit = num[i] - '0';
        rect(i, digit, length);
    }
}

void UI::text(std::string text, SDL_Rect dest)
{
    SDL_Surface* surface = TTF_RenderText_Solid(font, text.c_str(), { 255, 255, 255, 255 });
    SDL_Texture* texture = SDL_CreateTextureFromSurface(m_window_renderer, surface);
    SDL_RenderCopy(m_window_renderer, texture, NULL, &dest);

    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}
