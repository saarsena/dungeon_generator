#pragma once
#include <vector>
#include <random>
#include <cmath>

// --- Data Structures (Pure C++) ---

enum class Shape { Rect, Circle };

struct Point { int x, y; };
struct Vec2 { float x, y; };

struct RoomObj {
    int id;
    float x, y; // Center
    float w, h; // For rect: width/height. For circle: w = diameter
    Shape shape;
    bool isMain;
    
    // Physics internal
    float vx, vy;
};

struct Link {
    int u, v; // Room indices
    bool isMST;
};

struct WalkerAgent {
    int x, y;
    int life;
};

struct GenSettings {
    int roomCount = 150;
    float spreadRadius = 50.0f;
    int walkerCount = 400;
    int gridWidth = 200;  
    int gridHeight = 150; 
    int tileW = 4;
    int tileH = 4;
    unsigned int seed = 0; // 0 = Random
};

// --- The API Class ---

class DungeonBuilder {
public:
    enum class Phase {
        Physics,
        Graph,
        Raster,
        Walkers,
        Automata,
        Complete
    };
    
    enum class Tile : unsigned char { Empty, Floor, Wall };

    DungeonBuilder();
    
    void init(const GenSettings& settings);
    void step(); 
    bool isComplete() const { return phase == Phase::Complete; }
    Phase getPhase() const { return phase; }

    const std::vector<RoomObj>& getRooms() const { return rooms; }
    const std::vector<Link>& getLinks() const { return links; }
    const std::vector<WalkerAgent>& getWalkers() const { return walkers; }
    
    const std::vector<Point>& getFloors() const { return floors; }
    const std::vector<Point>& getWalls() const { return walls; }
    int getTotalTiles() const { return static_cast<int>(floors.size() + walls.size()); }

    int getGridWidth() const { return cfg.gridWidth; }
    int getGridHeight() const { return cfg.gridHeight; }
    int getTileW() const { return cfg.tileW; }
    int getTileH() const { return cfg.tileH; }

private:
    GenSettings cfg;
    std::mt19937 rng;
    Phase phase;
    
    std::vector<RoomObj> rooms;
    std::vector<int> mainRoomIndices;
    std::vector<Link> links;
    std::vector<WalkerAgent> walkers;
    
    std::vector<Tile> grid;
    
    std::vector<Point> floors;
    std::vector<Point> walls;
    
    void updatePhysics();
    void computeGraph(); 
    void rasterizeBase(); 
    void updateWalkers();
    void runAutomataPass();
    void despeckleWalls();
    void pruneDeadEnds();
    void floodFillPrune();
    void rebuildTileLists(); 
    
    void setTile(int x, int y, Tile t);
    Tile getTile(int x, int y) const;
    int toGridX(float v) const { return static_cast<int>(v / cfg.tileW); }
    int toGridY(float v) const { return static_cast<int>(v / cfg.tileH); }
};