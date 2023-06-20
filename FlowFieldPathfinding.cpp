#include "FlowFieldPathfinding.h"
#include "core/vector.h"
#include <math.h>

void FlowFieldPathfinding::Init (const Array& tile_positions) {
    int size = tile_positions.size();
    for (int i = 0; i < size; i++) {
        const Vector2& v = tile_positions[i];
        if (v[0] > m_width)
            m_width = v[0];
        if (v[1] > m_height)
            m_height = v[1];
    }

    m_width++; // 0-based
    m_height++; // 0-based
    int area = m_width * m_height;
	m_tiles = new Tile[area]();
    m_open_tile_indices = new int[area];

    for (int i = 0; i < size; i++) {
        m_tiles[vec2_to_idx(tile_positions[i])].wall = false;
    }
}

void FlowFieldPathfinding::Update (const Vector2& target_pos) {
    int target_idx = vec2_to_idx(target_pos);
    if (!is_valid_idx(target_idx))
        return;
    Tile& target_tile = m_tiles[target_idx];
    target_tile.distance = 0;

    int open_tiles_bot = 0;
    int open_tiles_top = 1;
    m_open_tile_indices[0] = target_idx;
    m_target_pos = target_pos;
    m_visited_integrity = !m_visited_integrity;
    while (open_tiles_bot != open_tiles_top) {
        int tile_idx = m_open_tile_indices[open_tiles_bot++];
        Tile &tile = m_tiles[tile_idx];
        for (int x = -1; x <= 1; x++)
        for (int y = -1; y <= 1; y++) {
            if (x == 0)
            if (y == 0)
                continue;
            int neighbour_idx = tile_idx + xy_to_idx(x, y);
            if (!is_valid_idx(neighbour_idx))
                continue;
            Tile &neighbour_tile = m_tiles[neighbour_idx];
            if (neighbour_tile.wall)
                continue;
            double distance = tile.distance + idx_to_vec2(neighbour_idx).distance_to(idx_to_vec2(tile_idx));
            if (neighbour_tile.visited == m_visited_integrity) { // already visited
                if (distance < neighbour_tile.distance)
                    neighbour_tile.distance = distance;
            }
            else {
                neighbour_tile.distance = distance;
                neighbour_tile.visited = !neighbour_tile.visited;
                m_open_tile_indices[open_tiles_top++] = neighbour_idx;
            }
        }
    }

    for (int i = 0; i < m_width * m_height; i++) {
        if (m_tiles[i].wall)
            continue;
        int closest_neighbour_idx = -1;
        int x_ofs = 0;
        int y_ofs = 0;
        for (int x = -1; x <= 1; x++)
        for (int y = -1; y <= 1; y++) {
            if (x == 0)
            if (y == 0)
                continue;
            int neighbour_idx = i + xy_to_idx(x, y);
            if (!is_valid_idx(neighbour_idx))
                continue;
            Tile &neighbour_tile = m_tiles[neighbour_idx];
            if (neighbour_tile.wall)
                continue;
            if (neighbour_tile.occupied_by != FFP_NO_AGENT)
                continue;
            if (neighbour_tile.visited != m_visited_integrity) // not visited
                continue;
            if ((closest_neighbour_idx < 0) || (neighbour_tile.distance < m_tiles[closest_neighbour_idx].distance)) {
                closest_neighbour_idx = neighbour_idx;
                x_ofs = x;
                y_ofs = y;
            }
        }
        m_tiles[i].dir = xy_to_dir(x_ofs, y_ofs);
        m_tiles[i].dirx = x_ofs;
        m_tiles[i].diry = y_ofs;
    }
}

int FlowFieldPathfinding::RegisterAgent () {
    return m_agents_top++;
}

void FlowFieldPathfinding::RemoveAgent (int id) {
    Agent * agent = get_agent(id);
    if (agent == NULL)
        return;
    set_agent_tiles(*agent, id, FFP_NO_AGENT);
}

void FlowFieldPathfinding::UpdateAgent (int id, int x, int y, int width, int height) {
    Agent * agent = get_agent(id);
    if (agent == NULL)
        return;
        
    RemoveAgent(id);

    agent->x = x;
    agent->y = y;
    agent->width = width;
    agent->height = height;
    set_agent_tiles(*agent, FFP_NO_AGENT, id);
}

Variant FlowFieldPathfinding::GetNextAgentPosition (int id) const {
    const Agent * agent = get_agent(id);
    if (agent == NULL)
        return Variant();
    if (!agent->has_been_updated())
        return Variant();
    const Tile * tile = get_tile(agent->x, agent->y);
    if (tile != NULL)
    if (!tile->wall)
    if (tile->occupied_by == id)
        return Vector2(
            agent->x + tile->dirx,
            agent->y + tile->diry
        );
    return Variant();
}

bool FlowFieldPathfinding::IsTileWall (const Vector2 &pos) const {
    const Tile * tile = get_tile(pos[0], pos[1]);
    if (tile == NULL)
        return false;
    return tile->wall;
}
int FlowFieldPathfinding::GetTileDistance (const Vector2 &pos) const {
    const Tile * tile = get_tile(pos[0], pos[1]);
    if (tile == NULL)
        return -1;
    return tile->distance;
}
FlowFieldPathfinding::Dir FlowFieldPathfinding::GetTileDirection (const Vector2 &pos) const {
    const Tile * tile = get_tile(pos[0], pos[1]);
    if (tile == NULL)
        return Dir::NONE;
    return tile->dir;
}
bool FlowFieldPathfinding::IsTileOccupied (const Vector2 &pos) const {
    const Tile * tile = get_tile(pos[0], pos[1]);
    if (tile == NULL)
        return false;
    return tile->occupied_by != FFP_NO_AGENT;
}

/*
 *
 * Protected
 * 
 */

void FlowFieldPathfinding::_bind_methods () {
    BIND_CONSTANT(FFP_NO_AGENT);

    BIND_ENUM_CONSTANT(NONE);
    BIND_ENUM_CONSTANT(LEFT);
    BIND_ENUM_CONSTANT(RIGHT);
    BIND_ENUM_CONSTANT(UP);
    BIND_ENUM_CONSTANT(DOWN);
    BIND_ENUM_CONSTANT(TOPLEFT);
    BIND_ENUM_CONSTANT(TOPRIGHT);
    BIND_ENUM_CONSTANT(BOTLEFT);
    BIND_ENUM_CONSTANT(BOTRIGHT);

	ClassDB::bind_method(D_METHOD("Init", "tile_positions"), &FlowFieldPathfinding::Init);
	ClassDB::bind_method(D_METHOD("Update", "target_position"), &FlowFieldPathfinding::Update);
	ClassDB::bind_method(D_METHOD("GetWidth"), &FlowFieldPathfinding::GetWidth);
	ClassDB::bind_method(D_METHOD("GetHeight"), &FlowFieldPathfinding::GetHeight);
	ClassDB::bind_method(D_METHOD("GetNextAgentPosition", "id"), &FlowFieldPathfinding::GetNextAgentPosition);
	ClassDB::bind_method(D_METHOD("GetTileDirection", "tile_pos"), &FlowFieldPathfinding::GetTileDirection);
	ClassDB::bind_method(D_METHOD("GetTileDistance", "tile_pos"), &FlowFieldPathfinding::GetTileDistance);
	ClassDB::bind_method(D_METHOD("IsTileWall", "tile_pos"), &FlowFieldPathfinding::IsTileWall);
	ClassDB::bind_method(D_METHOD("IsTileOccupied", "tile_pos"), &FlowFieldPathfinding::IsTileOccupied);
	ClassDB::bind_method(D_METHOD("GetTargetPosition"), &FlowFieldPathfinding::GetTargetPosition);
	ClassDB::bind_method(D_METHOD("RegisterAgent"), &FlowFieldPathfinding::RegisterAgent);
	ClassDB::bind_method(D_METHOD("UpdateAgent", "id", "x", "y", "width", "height"), &FlowFieldPathfinding::UpdateAgent);
	ClassDB::bind_method(D_METHOD("RemoveAgent", "id"), &FlowFieldPathfinding::RemoveAgent);
}

/*
 *
 * Private
 * 
 */

void FlowFieldPathfinding::set_agent_tiles (const Agent & agent, int occupied_by, int to_id) {
    if (!agent.has_been_updated())
        return;
    for (int ix = 0; ix < agent.width; ix++)
    for (int iy = 0; iy < agent.height; iy++) {
        Tile * tile = get_tile(
            agent.x + ix,
            agent.y + iy
        );
        if (tile != NULL)
        if (tile->occupied_by == occupied_by)
            tile->occupied_by = to_id;
    }
}

bool FlowFieldPathfinding::is_valid_idx (int idx) const {
    return (idx >= 0) && (idx < m_width * m_height);
}

Vector2 FlowFieldPathfinding::idx_to_vec2 (int idx) const {
    return Vector2(idx % m_width, idx / m_width);
}

int FlowFieldPathfinding::xy_to_idx (int x, int y) const {
    return x + y * m_width;
}

int FlowFieldPathfinding::vec2_to_idx (const Vector2 &v) const {
    return xy_to_idx(v[0], v[1]);
}

const FlowFieldPathfinding::Tile * FlowFieldPathfinding::get_tile (int x, int y) const {
    int idx = xy_to_idx(x, y);
    if (!is_valid_idx(idx))
        return NULL;
    return &m_tiles[idx];
}

FlowFieldPathfinding::Dir FlowFieldPathfinding::xy_to_dir (int x, int y) const {
    if (x == -1 && y ==  0) return Dir::LEFT;
    if (x ==  1 && y ==  0) return Dir::RIGHT;
    if (x ==  0 && y == -1) return Dir::UP;
    if (x ==  0 && y ==  1) return Dir::DOWN;
    if (x ==  1 && y ==  1) return Dir::BOTRIGHT;
    if (x == -1 && y ==  1) return Dir::BOTLEFT;
    if (x ==  1 && y == -1) return Dir::TOPRIGHT;
    if (x == -1 && y == -1) return Dir::TOPLEFT;
    return Dir::NONE;
}

const FlowFieldPathfinding::Agent * FlowFieldPathfinding::get_agent (int id) const {
    if (id < 0)
    if (id >= FFP_MAX_AGENTS)
        return NULL;
    if (id >= m_agents_top)
        return NULL;
    return &m_agents[id];
}