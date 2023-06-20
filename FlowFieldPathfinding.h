#ifndef FLOW_FIELD_PATHFINDING_H
#define FLOW_FIELD_PATHFINDING_H

#include "core/reference.h"
#include "core/array.h"

#define FFP_MAX_AGENT_SIZE (20 * 20)
#define FFP_MAX_AGENTS (1000)
#define FFP_NO_AGENT (-1)

class FlowFieldPathfinding : public Reference {
	GDCLASS(FlowFieldPathfinding, Reference);

public:
    enum Dir : uint8_t {
        NONE,
        LEFT,
        RIGHT,
        UP,
        DOWN,
        TOPLEFT,
        TOPRIGHT,
        BOTLEFT,
        BOTRIGHT
    };
    
private:
    struct TileVector2 {
        int x;
        int y;

        TileVector2 (int x_, int y_) {
            x = x_;
            y = y_;
        }
    };

    struct Agent {
        int x;
        int y;
        int width;
        int height;

        Agent () {
            x = -1;
        }

        inline bool has_been_updated () const {
            return x >= 0;
        }
    };

    struct Tile {
        bool wall;
        bool visited;
        Dir dir;
        double distance;
        int dirx;
        int diry;
        int occupied_by;

        Tile () {
            wall = true;
            visited = false;
            occupied_by = FFP_NO_AGENT;
            dir = Dir::NONE;
            dirx = -1;
            diry = -1;
            distance = 0;
        }
    };

public:
    FlowFieldPathfinding () {
        m_width = -1;
        m_height = -1;
        m_tiles = NULL;
        m_open_tile_indices = NULL;
        m_visited_integrity = false;
        m_agents_top = 0;
    }

    ~FlowFieldPathfinding () {
        delete[] m_tiles;
        delete[] m_open_tile_indices;
    }

	void Init (const Array& tile_positions);
    void Update (const Vector2& target_pos);

    int RegisterAgent ();
    void UpdateAgent (int id, int x, int y, int width, int height);
    void RemoveAgent (int id);

    int GetWidth  () const { return m_width; }
    int GetHeight () const { return m_height; }
    const Vector2& GetTargetPosition () const { return m_target_pos; }
    Variant GetNextAgentPosition (int id) const; // NULL or Vector2

    bool IsTileWall      (const Vector2 & pos) const;
    int GetTileDistance  (const Vector2 & pos) const;
    Dir GetTileDirection (const Vector2 & pos) const;
    bool IsTileOccupied  (const Vector2 & pos) const;

protected:
	static void _bind_methods ();

private:
           void          set_agent_tiles (const Agent & agent, int occupied_by, int to_id);
    inline bool          is_valid_idx    (int idx)          const;
    inline Vector2       idx_to_vec2     (int idx)          const;
    inline int           xy_to_idx       (int x, int y)     const;
    inline int           vec2_to_idx     (const Vector2 &v) const;
    inline Dir           xy_to_dir       (int x, int y)     const;
    inline const Tile *  get_tile        (int x, int y)     const;
    inline const Agent * get_agent       (int id)           const;
    inline Tile *        get_tile        (int x, int y) { return const_cast<Tile*>(const_cast<const FlowFieldPathfinding*>(this)->get_tile(x, y)); }
    inline Agent *       get_agent       (int id)       { return const_cast<Agent*>(const_cast<const FlowFieldPathfinding*>(this)->get_agent(id)); }


    Vector2 m_target_pos;
    int m_width;
    int m_height;
    
    Tile * m_tiles;
    bool m_visited_integrity;
    int m_agents_top;
    Agent m_agents[FFP_MAX_AGENTS];
    int * m_open_tile_indices;
};

VARIANT_ENUM_CAST(FlowFieldPathfinding::Dir);

#endif // FLOW_FIELD_PATHFINDING_H