// #pragma once

// using namespace olc::utils::geom2d;

// class Scene
// {
// public:
//     Scene(std::string backgroundPath)
//     {
//         boxes = std::vector<rect<float>>();
//         boxes.push_back({{200, SCREEN_HEIGHT - 50}, {SCREEN_WIDTH, 50}});
//         initialPlayerPosition = new olc::vf2d(10, 250);
//     }

//     void DrawBackground(olc::PixelGameEngine *pge)
//     {
//         pge->SetDrawTarget(0, true);
//         map->Render(pge);
//         isBackgroundDrawn = true;
//     }

//     void DrawMiddleground(olc::PixelGameEngine *pge)
//     {
//         pge->SetDrawTarget(5, true);
//     }

//     void SetWillDrawPlayer(olc::PixelGameEngine *pge)
//     {
//         pge->SetDrawTarget(0, true);
//     }

//     void DrawForeground(olc::PixelGameEngine *pge)
//     {
//         pge->SetDrawTarget(10, true);

//         if (DEBUG)
//         {
//             for (auto box : boxes)
//             {
//                 olc::vf2d pos = {box.pos.x, box.pos.y};
//                 olc::vf2d size = {box.size.x, box.size.y};
//                 pge->DrawRectDecal(pos, size, olc::WHITE);
//             }

//             int fps = pge->GetFPS();
//             pge->DrawStringDecal({0, 0}, "FPS: " + std::to_string(fps), olc::WHITE);
//         }
//     }

//     std::vector<olc::vf2d> IsColliding(ray<float> *ray, float distance)
//     {
//         for (auto &box : boxes)
//         {
//             line<float> line = {ray->origin, ray->origin + ray->direction * distance};
//             std::vector<olc::vf2d> intersection = intersects(line, box);

//             if (intersection.size() > 0)
//             {
//                 return intersection;
//             }
//         }

//         return std::vector<olc::vf2d>();
//     }

//     olc::vf2d *GetInitialPlayerPosition()
//     {
//         return initialPlayerPosition;
//     }

// private:
//     std::vector<rect<float>> boxes;
//     bool isBackgroundDrawn = false;
//     olc::vf2d *initialPlayerPosition;
// };
