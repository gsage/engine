
#include "Engine.h"
#include "EngineEvent.h"
#include "renderers/Factory.hpp"
#include "SDLRenderer.h"

namespace Gsage {
  SDLRenderer::SDLRenderer(WindowPtr window, const DataProxy& params, SDL_Renderer* renderer, SDLCore* core)
    : mRenderer(renderer)
  {
    RendererFactory f;

    for(auto& pair : params.get("render", DataProxy())) {
      RendererPtr r = f.create(pair.second, renderer, core, window);
      if(!r) {
        LOG(ERROR) << "Failed to create renderer";
        continue;
      }
      mRenderers.push_back(r);
    }
  }

  SDLRenderer::~SDLRenderer()
  {
    mRenderers.clear();
    SDL_DestroyRenderer(mRenderer);
  }

  void SDLRenderer::render()
  {
    SDL_SetRenderDrawColor(mRenderer, 255, 255, 255, 0);
    SDL_RenderClear(mRenderer);
    for(auto& v : mRenderers) {
      v->render();
    }
    SDL_RenderPresent(mRenderer);
  }
}
