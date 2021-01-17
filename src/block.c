#include "block.h"

unsigned char block_textures[][6] = 
{
    //lft  rgt  top  bot  bck  frt
    {  0,   0,   0,   0,   0,   0},      // 0   BLOCK_AIR              
    {239, 239, 239, 239, 239, 239},      // 1   BLOCK_PLAYER_HAND     
    {241, 241, 241, 241, 241, 241},      // 2   BLOCK_STONE            
    {242, 242, 242, 242, 242, 242},      // 3   BLOCK_DIRT             
    {243, 243, 254, 242, 243, 243},      // 4   BLOCK_GRASS            
    {244, 244, 244, 244, 244, 244},      // 5   BLOCK_WOODEN_PLANKS    
    {246, 246, 246, 246, 246, 246},      // 6   BLOCK_POLISHED_STONE   
    {247, 247, 247, 247, 247, 247},      // 7   BLOCK_BRICKS           
    {224, 224, 224, 224, 224, 224},      // 8   BLOCK_COBBLESTONE      
    {225, 225, 225, 225, 225, 225},      // 9   BLOCK_BEDROCK          
    {226, 226, 226, 226, 226, 226},      // 10  BLOCK_SAND             
    {227, 227, 227, 227, 227, 227},      // 11  BLOCK_GRAVEL           
    {228, 228, 229, 229, 228, 228},      // 12  BLOCK_WOOD             
    {230, 230, 230, 230, 230, 230},      // 13  BLOCK_IRON             
    {231, 231, 231, 231, 231, 231},      // 14  BLOCK_GOLD             
    {232, 232, 232, 232, 232, 232},      // 15  BLOCK_DIAMOND          
    {233, 233, 233, 233, 233, 233},      // 16  BLOCK_EMERALD          
    {234, 234, 234, 234, 234, 234},      // 17  BLOCK_REDSTONE         
    {212, 212, 212, 212, 212, 212},      // 18  BLOCK_MOSSY_COBBLESTONE
    {213, 213, 213, 213, 213, 213},      // 19  BLOCK_OBSIDIAN         
    {198, 198, 198, 198, 198, 198},      // 20  BLOCK_STONE_BRICKS     
    {178, 178, 178, 178, 178, 178},      // 21  BLOCK_SNOW             
    {180, 180, 178, 242, 180, 180},      // 22  BLOCK_SNOW_GRASS       
    {193, 193, 193, 193, 193, 193},      // 23  BLOCK_GLASS            
    { 61,  61,  61,  61,  61,  61},      // 24  BLOCK_WATER   
    {223, 223, 223, 223, 223, 223},      // 25  BLOCK_LEAVES      
    {215, 215, 215, 215, 215, 215},      // 26  BLOCK_GRASS_PLANT     
    {252, 252, 252, 252, 252, 252},      // 27  BLOCK_FLOWER_ROSE             
    {253, 253, 253, 253, 253, 253},      // 28  BLOCK_FLOWER_DANDELION       
    {237, 237, 237, 237, 237, 237},      // 29  BLOCK_MUSHROOM_BROWN            
    {236, 236, 236, 236, 236, 236},      // 30  BLOCK_MUSHROOM_RED       
    {199, 199, 199, 199, 199, 199},      // 31  BLOCK_DEAD_PLANT      
    {182, 182, 181, 183, 182, 182},      // 32  BLOCK_CACTUS      
    { 48,  48,  64,  32,  48,  48},      // 33  BLOCK_SANDSTONE      
    { 21,  21,  64,  32,  21,  21},      // 34  BLOCK_SANDSTONE_CHISELED      
};

/*

x, y, z: cube's world coordinates
faces:   determine whether to draw face or not
ao:      ao level for each vertex of each face

*/
void gen_cube_vertices(
    Vertex* vertices, int* curr_vertex_count, int x, int y, int z,
    int block_type, float block_size, int faces[6], float ao[6][4]
)
{
    // row = face (6 faces), each face has 4 points forming a square
    static const float positions[6][4][3] = 
    {
        { {0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1} }, // left
        { {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1} }, // right
        { {0, 1, 0}, {0, 1, 1}, {1, 1, 0}, {1, 1, 1} }, // top
        { {0, 0, 0}, {0, 0, 1}, {1, 0, 0}, {1, 0, 1} }, // bottom
        { {0, 0, 0}, {0, 1, 0}, {1, 0, 0}, {1, 1, 0} }, // back
        { {0, 0, 1}, {0, 1, 1}, {1, 0, 1}, {1, 1, 1} }  // front
    };

    static const float a = 0.0625f, b = 1.0f - 0.0625f;
    static const float positions_cactus[6][4][3] = 
    {
        { {a, 0, 0}, {a, 0, 1}, {a, 1, 0}, {a, 1, 1} }, // left
        { {b, 0, 0}, {b, 0, 1}, {b, 1, 0}, {b, 1, 1} }, // right
        { {0, 1, 0}, {0, 1, 1}, {1, 1, 0}, {1, 1, 1} }, // top
        { {0, 0, 0}, {0, 0, 1}, {1, 0, 0}, {1, 0, 1} }, // bottom
        { {0, 0, a}, {0, 1, a}, {1, 0, a}, {1, 1, a} }, // back
        { {0, 0, b}, {0, 1, b}, {1, 0, b}, {1, 1, b} }  // front
    };

    static const int indices[6][6] = 
    {
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3},
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3},
        {0, 3, 2, 0, 1, 3},
        {0, 3, 1, 0, 2, 3}
    };

    static const int indices_flipped[6][6] = {
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1},
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1},
        {0, 1, 2, 1, 3, 2},
        {0, 2, 1, 2, 3, 1}
    };

    static const float uvs[6][4][2] = 
    {
        {{0, 0}, {1, 0}, {0, 1}, {1, 1}},
        {{1, 0}, {0, 0}, {1, 1}, {0, 1}},
        {{0, 1}, {0, 0}, {1, 1}, {1, 0}},
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        {{0, 0}, {0, 1}, {1, 0}, {1, 1}},
        {{1, 0}, {1, 1}, {0, 0}, {0, 1}}
    };

    // for each face
    for (int f = 0; f < 6; f++)
    {
        if (!faces[f]) continue;
        
        // for each vertex
        for (int v = 0; v < 6; v++)
        {
            int index = ao[f][0] + ao[f][3] > ao[f][1] + ao[f][2] ? 
                indices_flipped[f][v] :
                indices[f][v];

            int vert_index = (*curr_vertex_count)++;

            // cactus is a bit smaller that other blocks
            if (block_type == BLOCK_CACTUS)
            {
                vertices[vert_index].pos[0] = (positions_cactus[f][index][0] + x) * block_size;
                vertices[vert_index].pos[1] = (positions_cactus[f][index][1] + y) * block_size;
                vertices[vert_index].pos[2] = (positions_cactus[f][index][2] + z) * block_size;
            }
            else
            {
                vertices[vert_index].pos[0] = (positions[f][index][0] + x) * block_size;
                vertices[vert_index].pos[1] = (positions[f][index][1] + y) * block_size;
                vertices[vert_index].pos[2] = (positions[f][index][2] + z) * block_size;
            }

            // make water block shorter
            if (block_type == BLOCK_WATER && positions[f][index][1] > 0)
            {
                vertices[vert_index].pos[1] -= 0.125f * block_size;
            }

            vertices[vert_index].tex_coord[0] = uvs[f][index][0];
            vertices[vert_index].tex_coord[1] = uvs[f][index][1];

            vertices[vert_index].ao = ao[f][index];

            vertices[vert_index].tile = block_textures[block_type][f];
        }
    }
}

void gen_plant_vertices(
    Vertex* vertices, int* curr_vertex_count, int x, int y, int z,
    int block_type, float block_size
)
{
    static const float positions[2][4][3] = 
    {
        { {0.5f, 0.0f, 0.0f}, {0.5f, 0.0f, 1.0f}, {0.5f, 1.0f, 1.0f}, {0.5f, 1.0f, 0.0f} },
        { {0.0f, 0.0f, 0.5f}, {1.0f, 0.0f, 0.5f}, {1.0f, 1.0f, 0.5f}, {0.0f, 1.0f, 0.5f} }
    };

    static const int indices[2][6] = 
    {
        {0, 1, 2, 2, 3, 0},
        {0, 3, 2, 2, 1, 0}
    };

    static const float uvs[4][2] = 
    {
        {0, 0}, {1, 0}, {1, 1}, {0, 1}
    };

    // There are 2 faces for a plant, but we are
    // generating 4 to eliminate face culling effect
    for (int f = 0; f < 4; f++)
    { 
        for (int v = 0; v < 6; v++)
        {
            int index = indices[f % 2][v];
            int vert_index = (*curr_vertex_count)++;

            vertices[vert_index].pos[0] = (positions[f / 2][index][0] + x) * block_size;
            vertices[vert_index].pos[1] = (positions[f / 2][index][1] + y) * block_size;
            vertices[vert_index].pos[2] = (positions[f / 2][index][2] + z) * block_size;

            vertices[vert_index].tex_coord[0] = uvs[index][0];
            vertices[vert_index].tex_coord[1] = uvs[index][1];

            vertices[vert_index].ao = 0.0f;

            vertices[vert_index].tile = block_textures[block_type][f];
        }
    }
}

void block_gen_aabb(int x, int y, int z, vec3 aabb[2])
{
    aabb[0][0] = x * BLOCK_SIZE;
    aabb[0][1] = y * BLOCK_SIZE;
    aabb[0][2] = z * BLOCK_SIZE;
    aabb[1][0] = aabb[0][0] + BLOCK_SIZE;
    aabb[1][1] = aabb[0][1] + BLOCK_SIZE;
    aabb[1][2] = aabb[0][2] + BLOCK_SIZE;
}

int block_is_solid(unsigned char block)
{
    switch (block)
    {
        case BLOCK_AIR:
        case BLOCK_WATER:
            return 0;
        default:
            return 1;
    }
}

int block_is_transparent(unsigned char block)
{
    switch (block)
    {
        case BLOCK_AIR:
        case BLOCK_GLASS:
        case BLOCK_WATER:
        case BLOCK_LEAVES:
        case BLOCK_GRASS_PLANT:
        case BLOCK_FLOWER_ROSE:
        case BLOCK_FLOWER_DANDELION:
        case BLOCK_MUSHROOM_BROWN:
        case BLOCK_MUSHROOM_RED:
        case BLOCK_DEAD_PLANT:
        case BLOCK_CACTUS:
            return 1;
        default:
            return 0;
    }
}

int block_is_plant(unsigned char block)
{
    switch (block)
    {
        case BLOCK_GRASS_PLANT:
        case BLOCK_FLOWER_ROSE:
        case BLOCK_FLOWER_DANDELION:
        case BLOCK_MUSHROOM_BROWN:
        case BLOCK_MUSHROOM_RED:
        case BLOCK_DEAD_PLANT:
            return 1;
        default:
            return 0;
    }
}

static int get_block_from_chunk(Chunk* c, int bx, int by, int bz)
{
    if (!c) return BLOCK_AIR;
    return c->blocks[XYZ(bx, by, bz)];
}

/*

Neighs array layout (-Y view direction): 

----> +X
|
v +Z

Top layer (blocks above):
18 21 24
19 22 25
20 23 26

Middle layer (13 is the block we are getting neighs of):
9  12 15
10 13 16
11 14 17

Bottom layer (blocks below):
0 3 6
1 4 7
2 5 8

*/
void block_get_neighs(Chunk* c, Chunk* neighs[8], int x, int y, int z, unsigned char b_neighs[27])
{
    int index = 0;
    const int last = CHUNK_WIDTH - 1;
    
    for (int dy = -1; dy <= 1; dy++)
    for (int dx = -1; dx <= 1; dx++)
    for (int dz = -1; dz <= 1; dz++)
    {
        int bx = x + dx;
        int by = y + dy;
        int bz = z + dz;

        // too high or too low
        if (by < 0 || by >= CHUNK_HEIGHT)
            b_neighs[index++] = BLOCK_AIR;

        // completely in bounds
        else if (bx >= 0 && bx < CHUNK_WIDTH && by >= 0 && by < CHUNK_HEIGHT && bz >= 0 && bz < CHUNK_WIDTH)
            b_neighs[index++] = get_block_from_chunk(c, bx, by, bz);

        else if (bx < 0)
        {
            if (bz < 0)
                b_neighs[index++] = get_block_from_chunk(neighs[CHUNK_NEIGH_BL], last, by, last);
            else if (bz >= CHUNK_WIDTH)
                b_neighs[index++] = get_block_from_chunk(neighs[CHUNK_NEIGH_FL], last, by, 0);
            else
                b_neighs[index++] = get_block_from_chunk(neighs[CHUNK_NEIGH_L], last, by, bz);
        }

        else if (bx >= CHUNK_WIDTH)
        {
            if (bz < 0)
                b_neighs[index++] = get_block_from_chunk(neighs[CHUNK_NEIGH_BR], 0, by, last);
            else if (bz >= CHUNK_WIDTH)
                b_neighs[index++] = get_block_from_chunk(neighs[CHUNK_NEIGH_FR], 0, by, 0);
            else
                b_neighs[index++] = get_block_from_chunk(neighs[CHUNK_NEIGH_R], 0, by, bz);
        }

        else if (bz < 0)
            b_neighs[index++] = get_block_from_chunk(neighs[CHUNK_NEIGH_B], bx, by, last);
        
        else if (bz >= CHUNK_WIDTH)
            b_neighs[index++] = get_block_from_chunk(neighs[CHUNK_NEIGH_F], bx, by, 0);
    }
}

static int should_be_visible(unsigned char block_type, Chunk* c, int neigh_x, int neigh_y, int neigh_z)
{
    if (!c) return 1;

    unsigned char neigh_type = c->blocks[XYZ(neigh_x, neigh_y, neigh_z)];
    return block_is_transparent(neigh_type) && block_type != neigh_type;
}

int block_set_visible_faces(Chunk* c, int x, int y, int z, Chunk* neighs[8], int faces[6])
{
    unsigned char block_type = c->blocks[XYZ(x, y, z)];
    
    // left
    if (x == 0)
        faces[0] = should_be_visible(block_type, neighs[CHUNK_NEIGH_L], CHUNK_WIDTH - 1, y, z);
    else
        faces[0] = should_be_visible(block_type, c, x - 1, y, z);

    // right
    if (x == CHUNK_WIDTH - 1)
        faces[1] = should_be_visible(block_type, neighs[CHUNK_NEIGH_R], 0, y, z);
    else
        faces[1] = should_be_visible(block_type, c, x + 1, y, z);
 
    // top
    if (y == CHUNK_HEIGHT - 1)
        faces[2] = 1;
    else
        faces[2] = should_be_visible(block_type, c, x, y + 1, z);

    // bottom
    if (y == 0)
        faces[3] = 0;
    else
        faces[3] = should_be_visible(block_type, c, x, y - 1, z);

    // back
    if (z == 0)
        faces[4] = should_be_visible(block_type, neighs[CHUNK_NEIGH_B], x, y, CHUNK_WIDTH - 1);
    else
        faces[4] = should_be_visible(block_type, c, x, y, z - 1);

    // front
    if (z == CHUNK_WIDTH - 1)
        faces[5] = should_be_visible(block_type, neighs[CHUNK_NEIGH_F], x, y, 0);
    else
        faces[5] = should_be_visible(block_type, c, x, y, z + 1);

    // return amount of visible faces
    int sum = 0;
    for (int i = 0; i < 6; i++)
        sum += faces[i];
    return sum;
}

void block_set_ao(unsigned char neighs[27], float ao[6][4])
{       
    // neighbours indices for each vertex for each face
    static const unsigned char lookup[6][4][3] = 
    {
        { { 0,  1,  9}, { 2,  1, 11}, {18,  9, 19}, {20, 19, 11} }, // left
        { { 6, 15,  7}, { 8, 17,  7}, {24, 25, 15}, {26, 17, 25} }, // right
        { {18, 19, 21}, {20, 19, 23}, {24, 21, 25}, {26, 23, 25} }, // top
        { { 0,  1,  3}, { 2,  1,  5}, { 6,  3,  7}, { 8,  5,  7} }, // bottom
        { { 0,  3,  9}, {18,  9, 21}, { 6,  3, 15}, {24, 15, 21} }, // back
        { { 2,  5, 11}, {20, 23, 11}, { 8,  5, 17}, {26, 17, 23} }, // front
    };

    static const float curve[4] = 
    {
        0.0f, 0.33f, 0.66f, 1.0f
    };

    for (int f = 0; f < 6; f++)
    {
        for (int v = 0; v < 4; v++)
        {
            int corner = block_is_transparent(neighs[lookup[f][v][0]]) ? 0 : 1;
            int side1  = block_is_transparent(neighs[lookup[f][v][1]]) ? 0 : 1;
            int side2  = block_is_transparent(neighs[lookup[f][v][2]]) ? 0 : 1;

            ao[f][v] = side1 && side2 ? curve[3] : curve[corner + side1 + side2];
        }
    }
}