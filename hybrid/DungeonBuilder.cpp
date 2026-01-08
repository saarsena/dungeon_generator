#include "DungeonBuilder.h"
#include <algorithm>
#include <map>
#include <set>
#include <iostream>

// --- Helper Math (Global) ---
static float distSq(float x1, float y1, float x2, float y2) {
    return (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2);
}

// Minimal Delaunay Helper (Global)
static void getCircumcircle(Vec2 p1, Vec2 p2, Vec2 p3, Vec2& center, float& rSq) {
    float d = 2 * (p1.x * (p2.y - p3.y) + p2.x * (p3.y - p1.y) + p3.x * (p1.y - p2.y));
    if (std::abs(d) < 0.001f) { center={0,0}; rSq=1e9; return; }
    float ux = ((p1.x*p1.x + p1.y*p1.y)*(p2.y-p3.y) + (p2.x*p2.x+p2.y*p2.y)*(p3.y-p1.y) + (p3.x*p3.x+p3.y*p3.y)*(p1.y-p2.y))/d;
    float uy = ((p1.x*p1.x + p1.y*p1.y)*(p3.x-p2.x) + (p2.x*p2.x+p2.y*p2.y)*(p1.x-p3.x) + (p3.x*p3.x+p3.y*p3.y)*(p2.x-p1.x))/d;
    center = {ux, uy};
    rSq = distSq(center.x, center.y, p1.x, p1.y);
}

// DSU Helper (Global)
struct DSU {
    std::vector<int> p;
    DSU(int n) { p.resize(n); std::iota(p.begin(), p.end(), 0); }
    int find(int i) { return (p[i]==i) ? i : p[i]=find(p[i]); }
    void unite(int i, int j) { int r1=find(i), r2=find(j); if(r1!=r2) p[r1]=r2; }
};

// --- Implementation ---

DungeonBuilder::DungeonBuilder() : rng(std::random_device{}()) {}

void DungeonBuilder::init(const GenSettings& settings) {
    cfg = settings;
    
    if (cfg.seed == 0) {
        rng.seed(std::random_device{}());
    } else {
        rng.seed(cfg.seed);
    }

    phase = Phase::Physics;
    rooms.clear();
    links.clear();
    walkers.clear();
    grid.clear();
    floors.clear();
    walls.clear();
    
    // Resize grid immediately
    grid.assign(cfg.gridWidth * cfg.gridHeight, Tile::Empty);
    
    // Spawn Rooms centered in the grid world
    float worldCX = (cfg.gridWidth * cfg.tileW) / 2.0f;
    float worldCY = (cfg.gridHeight * cfg.tileH) / 2.0f;
    
    std::normal_distribution<float> sizeDist(12.0f, 6.0f);
    std::uniform_real_distribution<float> angleDist(0, 6.2831f);
    std::uniform_real_distribution<float> radDist(0, 1.0f);
    
    for(int i=0; i<cfg.roomCount; ++i) {
        RoomObj r;
        r.id = i;
        float angle = angleDist(rng);
        float rad = std::sqrt(radDist(rng)) * cfg.spreadRadius; 
        r.x = worldCX + std::cos(angle)*rad;
        r.y = worldCY + std::sin(angle)*rad;
        
        float dim = std::max(8.0f, sizeDist(rng));
        bool big = std::uniform_int_distribution<int>(0,10)(rng) > 8;
        if(big) dim *= 3.0f;
        
        r.w = dim;
        r.h = dim * std::uniform_real_distribution<float>(0.8f, 1.2f)(rng);
        r.shape = (std::uniform_int_distribution<int>(0,1)(rng)==0) ? Shape::Rect : Shape::Circle;
        if(r.shape == Shape::Circle) r.h = r.w; 
        
        r.vx = 0; r.vy = 0;
        r.isMain = false;
        rooms.push_back(r);
    }
}

void DungeonBuilder::step() {
    if (phase == Phase::Physics) {
        updatePhysics();
    }
    else if (phase == Phase::Graph) {
        computeGraph();
        phase = Phase::Raster;
    }
    else if (phase == Phase::Raster) {
        rasterizeBase();
        rebuildTileLists();
        phase = Phase::Walkers;
        
        // Init Walkers
        std::vector<Point> spawnPoints = floors;
        if (!spawnPoints.empty()) {
            for(int i=0; i<cfg.walkerCount; ++i) {
                Point p = spawnPoints[std::uniform_int_distribution<int>(0, spawnPoints.size()-1)(rng)];
                walkers.push_back({p.x, p.y, std::uniform_int_distribution<int>(30, 100)(rng)});
            }
        }
    }
    else if (phase == Phase::Walkers) {
        updateWalkers();
        rebuildTileLists(); // Sync for rendering
        if (walkers.empty()) {
            phase = Phase::Automata;
        }
    }
    else if (phase == Phase::Automata) {
        static int caSteps = 0;
        runAutomataPass();
        rebuildTileLists();
        caSteps++;
        if (caSteps >= 4) {
            caSteps = 0;
            despeckleWalls(); // New cleaning pass
            pruneDeadEnds();
            floodFillPrune(); // Then ensure connectivity
            rebuildTileLists();
            phase = Phase::Complete;
        }
    }
}

void DungeonBuilder::updatePhysics() {
    bool active = false;
    float totalE = 0;
    
    for(size_t i=0; i<rooms.size(); ++i) {
        float fx = 0, fy = 0;
        for(size_t j=0; j<rooms.size(); ++j) {
            if(i==j) continue;
            RoomObj& A = rooms[i];
            RoomObj& B = rooms[j];
            
            float nx = 0, ny = 0;
            
            float rA = A.w * 0.55f; 
            float rB = B.w * 0.55f;
            float minD = rA + rB + 2.0f; // Padding
            
            float dx = A.x - B.x;
            float dy = A.y - B.y;
            float d2 = dx*dx + dy*dy;
            
            if (d2 < minD*minD) {
                float d = std::sqrt(d2);
                if (d < 0.1f) { nx = 1; ny = 0; } 
                else { nx = dx/d; ny = dy/d; }
                
                float force = (minD - d) * 5.0f; 
                fx += nx * force;
                fy += ny * force;
            }
        }
        
        rooms[i].vx += fx;
        rooms[i].vy += fy;
        if (fx != 0 || fy != 0) active = true;
    }
    
    // Integration
    for(auto& r : rooms) {
        r.x += r.vx * 0.1f; // dt fixed
        r.y += r.vy * 0.1f;
        totalE += (r.vx*r.vx + r.vy*r.vy);
        r.vx *= 0.5f;
        r.vy *= 0.5f;
    }
    
    if (totalE < 1.0f && !active) {
        float avg = 0;
        for(auto& r : rooms) avg += r.w;
        avg /= rooms.size();
        for(auto& r : rooms) r.isMain = (r.w > avg * 1.3f);
        
        phase = Phase::Graph;
    }
}

void DungeonBuilder::computeGraph() {
    mainRoomIndices.clear();
    for(size_t i=0; i<rooms.size(); ++i) if(rooms[i].isMain) mainRoomIndices.push_back(i);
    if(mainRoomIndices.size() < 3) return;
    
    std::vector<Vec2> pts;
    for(int idx : mainRoomIndices) pts.push_back({rooms[idx].x, rooms[idx].y});
    
    struct Tri { int p1,p2,p3; bool bad; };
    std::vector<Tri> tris;
    
    float minX=1e9, maxX=-1e9, minY=1e9, maxY=-1e9;
    for(auto& p: pts) { minX=std::min(minX,p.x); maxX=std::max(maxX,p.x); minY=std::min(minY,p.y); maxY=std::max(maxY,p.y); }
    float dx=maxX-minX, dy=maxY-minY;
    
    float margin = 100.0f;
    Vec2 center={(minX+maxX)/2, (minY+maxY)/2};
    pts.push_back({center.x-margin*dx, center.y-dy});
    pts.push_back({center.x, center.y+margin*dy});
    pts.push_back({center.x+margin*dx, center.y-dy});
    tris.push_back({(int)pts.size()-3, (int)pts.size()-2, (int)pts.size()-1, false});
    
    for(int i=0; i<(int)mainRoomIndices.size(); ++i) {
        std::vector<Tri> bad;
        for(auto& t: tris) {
            Vec2 c; float r;
            getCircumcircle(pts[t.p1], pts[t.p2], pts[t.p3], c, r);
            if(distSq(pts[i].x, pts[i].y, c.x, c.y) < r) { t.bad=true; bad.push_back(t); }
        }
        std::map<std::pair<int,int>, int> edges;
        auto addE = [&](int u, int v){ if(u>v) std::swap(u,v); edges[{u,v}]++; };
        for(auto& t: bad) { addE(t.p1,t.p2); addE(t.p2,t.p3); addE(t.p3,t.p1); }
        tris.erase(std::remove_if(tris.begin(), tris.end(), [](const Tri& t){ return t.bad; }), tris.end());
        for(auto& [e,n] : edges) if(n==1) tris.push_back({e.first, e.second, i, false});
    }
    
    // Fix: Filter out any triangles that use the super-triangle vertices (indices >= N)
    int N = (int)mainRoomIndices.size();
    tris.erase(std::remove_if(tris.begin(), tris.end(), [&](const Tri& t){ 
        return t.p1 >= N || t.p2 >= N || t.p3 >= N; 
    }), tris.end());
    
    std::set<std::pair<int,int>> uniqueEdges;
    for(const auto& t : tris) {
        int u = mainRoomIndices[t.p1];
        int v = mainRoomIndices[t.p2];
        int w = mainRoomIndices[t.p3];
        
        auto add = [&](int a, int b) { if(a>b) std::swap(a,b); uniqueEdges.insert({a,b}); };
        add(u,v); add(v,w); add(w,u);
    }
    
    std::vector<std::pair<int,int>> allEdges;
    std::vector<float> weights;
    for(auto& e : uniqueEdges) {
        allEdges.push_back(e);
        float d = std::sqrt(distSq(rooms[e.first].x, rooms[e.first].y, rooms[e.second].x, rooms[e.second].y));
        weights.push_back(d);
    }
    
    std::vector<int> idx(allEdges.size());
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(), [&](int a, int b){ return weights[a] < weights[b]; });
    
    DSU dsu(rooms.size());
    links.clear();
    for(int i : idx) {
        int u = allEdges[i].first;
        int v = allEdges[i].second;
        if(dsu.find(u) != dsu.find(v)) {
            dsu.unite(u, v);
            links.push_back({u, v, true}); 
        } else {
            if(std::uniform_int_distribution<int>(0,10)(rng) > 8) {
                links.push_back({u, v, false}); 
            }
        }
    }
}

void DungeonBuilder::rasterizeBase() {
    std::fill(grid.begin(), grid.end(), Tile::Empty);
    
    for(int idx : mainRoomIndices) {
        RoomObj& r = rooms[idx];
        if(r.shape == Shape::Rect) {
            int sx = toGridX(r.x - r.w/2); int sy = toGridY(r.y - r.h/2);
            int w = toGridX(r.w); int h = toGridY(r.h);
            if(w < 1) w=1; if(h < 1) h=1;
            for(int y=sy; y<sy+h; ++y) for(int x=sx; x<sx+w; ++x) setTile(x,y,Tile::Floor);
        } else {
            int cx = toGridX(r.x); int cy = toGridY(r.y);
            int radX = toGridX(r.w/2); int radY = toGridY(r.w/2);
            for(int y=cy-radY; y<=cy+radY; ++y) for(int x=cx-radX; x<=cx+radX; ++x) {
                float dx = (float)(x-cx); float dy = (float)(y-cy);
                if((dx*dx)/(radX*radX) + (dy*dy)/(radY*radY) <= 1.0f) setTile(x,y,Tile::Floor);
            }
        }
    }
    
    for(auto& l : links) {
        int x1 = toGridX(rooms[l.u].x); int y1 = toGridY(rooms[l.u].y);
        int x2 = toGridX(rooms[l.v].x); int y2 = toGridY(rooms[l.v].y);
        
        auto hline = [&](int xa, int xb, int y) {
            int s = (xb>xa)?1:-1;
            for(int x=xa; x!=xb+s; x+=s) { setTile(x,y,Tile::Floor); setTile(x,y+1,Tile::Floor); }
        };
        auto vline = [&](int ya, int yb, int x) {
            int s = (yb>ya)?1:-1;
            for(int y=ya; y!=yb+s; y+=s) { setTile(x,y,Tile::Floor); setTile(x+1,y,Tile::Floor); }
        };
        
        if(std::uniform_int_distribution<int>(0,1)(rng)) {
            int mx = (x1+x2)/2; hline(x1,mx,y1); vline(y1,y2,mx); hline(mx,x2,y2);
        } else {
            int my = (y1+y2)/2; vline(y1,my,x1); hline(x1,x2,my); vline(my,y2,x2);
        }
    }
}

void DungeonBuilder::updateWalkers() {
    for(auto& w : walkers) {
        if(w.life <= 0) continue;
        int d = std::uniform_int_distribution<int>(0,3)(rng);
        if(d==0) w.x--; else if(d==1) w.x++; else if(d==2) w.y--; else w.y++;
        w.life--;
        
        if(w.x<1||w.x>=cfg.gridWidth-1||w.y<1||w.y>=cfg.gridHeight-1) { w.life=0; continue; }
        
        bool sanctuary = false;
        float wx = w.x * cfg.tileW; float wy = w.y * cfg.tileH;
        
        for(int idx : mainRoomIndices) {
            RoomObj& r = rooms[idx];
            if(r.shape==Shape::Rect) {
                if(wx > r.x-r.w/2+4 && wx < r.x+r.w/2-4 && wy > r.y-r.h/2+4 && wy < r.y+r.h/2-4) { sanctuary=true; break; }
            } else {
                if(distSq(wx,wy,r.x,r.y) < (r.w/2-2)*(r.w/2-2)) { sanctuary=true; break; }
            }
        }
        
        if(!sanctuary) setTile(w.x, w.y, Tile::Floor);
        else w.life = 0; 
    }
    walkers.erase(std::remove_if(walkers.begin(), walkers.end(), [](auto& w){return w.life<=0;}), walkers.end());
}

void DungeonBuilder::runAutomataPass() {
    std::vector<Tile> next = grid;
    for(int y=1; y<cfg.gridHeight-1; ++y) {
        for(int x=1; x<cfg.gridWidth-1; ++x) {
            int n=0;
            for(int dy=-1; dy<=1; ++dy) for(int dx=-1; dx<=1; ++dx) {
                if(dx==0&&dy==0) continue;
                if(getTile(x+dx, y+dy) != Tile::Floor) n++;
            }
            if(n > 4) next[y*cfg.gridWidth+x] = Tile::Wall;
            else if(n < 4) next[y*cfg.gridWidth+x] = Tile::Floor;
        }
    }
    grid = next;
}

void DungeonBuilder::despeckleWalls() {
    std::vector<int> wallRegions(grid.size(), 0);
    int currentRegion = 0;
    std::vector<std::vector<int>> regions;

    for (int i = 0; i < (int)grid.size(); ++i) {
        if (grid[i] == Tile::Wall && wallRegions[i] == 0) {
            currentRegion++;
            std::vector<int> q;
            q.reserve(64); 
            q.push_back(i);
            wallRegions[i] = currentRegion;
            
            int head = 0;
            while(head < (int)q.size()) {
                int curr = q[head++];
                int cx = curr % cfg.gridWidth;
                int cy = curr / cfg.gridWidth;
                
                std::vector<int> candidates;
                if(cx > 0) candidates.push_back(curr-1);
                if(cx < cfg.gridWidth-1) candidates.push_back(curr+1);
                if(cy > 0) candidates.push_back(curr-cfg.gridWidth);
                if(cy < cfg.gridHeight-1) candidates.push_back(curr+cfg.gridWidth);

                for(int n : candidates) {
                    if(grid[n] == Tile::Wall && wallRegions[n] == 0) {
                        wallRegions[n] = currentRegion;
                        q.push_back(n);
                    }
                }
            }
            regions.push_back(q);
        }
    }

    for (const auto& tiles : regions) {
        if (tiles.size() < 10) { // Increased threshold to remove small blobs
            for (int idx : tiles) {
                int cx = idx % cfg.gridWidth; 
                int cy = idx / cfg.gridWidth;
                if (cx > 0 && cx < cfg.gridWidth-1 && cy > 0 && cy < cfg.gridHeight-1) {
                    grid[idx] = Tile::Floor;
                } else {
                    grid[idx] = Tile::Empty;
                }
            }
        }
    }
}

void DungeonBuilder::pruneDeadEnds() {
    // Iteratively remove floor tiles that have 0 or 1 floor neighbors
    bool changed = true;
    int k = 0;
    while(changed && k < 100) { // Safety limit
        changed = false;
        std::vector<int> toRemove;
        
        for(int y=1; y<cfg.gridHeight-1; ++y) {
            for(int x=1; x<cfg.gridWidth-1; ++x) {
                if(getTile(x,y) != Tile::Floor) continue;
                
                int n = 0;
                if(getTile(x+1, y) == Tile::Floor) n++;
                if(getTile(x-1, y) == Tile::Floor) n++;
                if(getTile(x, y+1) == Tile::Floor) n++;
                if(getTile(x, y-1) == Tile::Floor) n++;
                
                if(n <= 1) {
                    toRemove.push_back(y*cfg.gridWidth + x);
                }
            }
        }
        
        if(!toRemove.empty()){
            changed = true;
            for(int idx : toRemove) grid[idx] = Tile::Empty; // or Wall? Empty helps cleanup
        }
        k++;
    }
}

void DungeonBuilder::floodFillPrune() {
    std::vector<int> regions(grid.size(), 0);
    int rID = 0;
    std::map<int,int> sizes;
    
    for(int i=0; i<(int)grid.size(); ++i) {
        if(grid[i]==Tile::Floor && regions[i]==0) {
            rID++;
            int size=0;
            std::vector<int> q; q.reserve(cfg.gridWidth * cfg.gridHeight / 4);
            q.push_back(i);
            regions[i]=rID;
            size++;
            int head=0;
            while(head<(int)q.size()){
                int curr = q[head++];
                int cx = curr%cfg.gridWidth; int cy = curr/cfg.gridWidth;
                
                int candidates[4];
                int cCount = 0;
                if(cx > 0) candidates[cCount++] = curr-1;
                if(cx < cfg.gridWidth-1) candidates[cCount++] = curr+1;
                if(cy > 0) candidates[cCount++] = curr-cfg.gridWidth;
                if(cy < cfg.gridHeight-1) candidates[cCount++] = curr+cfg.gridWidth;

                for(int k=0; k<cCount; ++k) {
                    int n = candidates[k];
                    if(grid[n]==Tile::Floor && regions[n]==0) {
                        regions[n]=rID; q.push_back(n); size++;
                    }
                }
            }
            sizes[rID]=size;
        }
    }
    
    int bestR = -1, maxS = 0;
    for(auto [r,s] : sizes) if(s>maxS) { maxS=s; bestR=r; }
    
    for(int i=0; i<(int)grid.size(); ++i) {
        if(grid[i]==Tile::Floor && regions[i]!=bestR) grid[i]=Tile::Empty;
    }
    
    // Wall Gen
    for(int i=0; i<(int)grid.size(); ++i) {
        if(grid[i]!=Tile::Floor) {
            int cx = i%cfg.gridWidth; int cy = i/cfg.gridWidth;
            bool touch=false;
            for(int dy=-1; dy<=1; ++dy) for(int dx=-1; dx<=1; ++dx) {
                if(getTile(cx+dx, cy+dy)==Tile::Floor) touch=true;
            }
            grid[i] = touch ? Tile::Wall : Tile::Empty;
        }
    }
}

void DungeonBuilder::rebuildTileLists() {
    floors.clear();
    walls.clear();
    for(int y=0; y<cfg.gridHeight; ++y) {
        for(int x=0; x<cfg.gridWidth; ++x) {
            Tile t = getTile(x,y);
            if(t == Tile::Floor) floors.push_back({x,y});
            else if(t == Tile::Wall) walls.push_back({x,y});
        }
    }
}

void DungeonBuilder::setTile(int x, int y, Tile t) {
    if(x>=0 && x<cfg.gridWidth && y>=0 && y<cfg.gridHeight) {
        // Prevent floors on the very edge to avoid leaks/renderer issues
        // Increased margin to 2 to ensure a solid wall seal + void boundary
        if (t == Tile::Floor && (x<=1 || x>=cfg.gridWidth-2 || y<=1 || y>=cfg.gridHeight-2)) return;
        grid[y*cfg.gridWidth+x] = t;
    }
}

DungeonBuilder::Tile DungeonBuilder::getTile(int x, int y) const {
    if(x>=0 && x<cfg.gridWidth && y>=0 && y<cfg.gridHeight) return grid[y*cfg.gridWidth+x];
    return Tile::Empty;
}
