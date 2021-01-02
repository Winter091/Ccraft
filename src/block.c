#include "block.h"

unsigned char block_textures[][6] = 
{
    //lft  rgt  top  bot  bck  frt
    {  0,   0,   0,   0,   0,   0},      // 0   BLOCK_AIR              
    {241, 241, 241, 241, 241, 241},      // 1   BLOCK_STONE            
    {242, 242, 242, 242, 242, 242},      // 2   BLOCK_DIRT             
    {243, 243, 254, 242, 243, 243},      // 3   BLOCK_GRASS            
    {244, 244, 244, 244, 244, 244},      // 4   BLOCK_WOODEN_PLANKS    
    {246, 246, 246, 246, 246, 246},      // 5   BLOCK_POLISHED_STONE   
    {247, 247, 247, 247, 247, 247},      // 6   BLOCK_BRICKS           
    {224, 224, 224, 224, 224, 224},      // 7   BLOCK_COBBLESTONE      
    {225, 225, 225, 225, 225, 225},      // 8   BLOCK_BEDROCK          
    {226, 226, 226, 226, 226, 226},      // 9   BLOCK_SAND             
    {227, 227, 227, 227, 227, 227},      // 10  BLOCK_GRAVEL           
    {228, 228, 229, 229, 228, 228},      // 11  BLOCK_WOOD             
    {230, 230, 230, 230, 230, 230},      // 12  BLOCK_IRON             
    {231, 231, 231, 231, 231, 231},      // 13  BLOCK_GOLD             
    {232, 232, 232, 232, 232, 232},      // 14  BLOCK_DIAMOND          
    {233, 233, 233, 233, 233, 233},      // 15  BLOCK_EMERALD          
    {234, 234, 234, 234, 234, 234},      // 16  BLOCK_REDSTONE         
    {212, 212, 212, 212, 212, 212},      // 17  BLOCK_MOSSY_COBBLESTONE
    {213, 213, 213, 213, 213, 213},      // 18  BLOCK_OBSIDIAN         
    {198, 198, 198, 198, 198, 198},      // 19  BLOCK_STONE_BRICKS     
    {178, 178, 178, 178, 178, 178},      // 20  BLOCK_SNOW             
    {180, 180, 178, 242, 180, 180},      // 21  BLOCK_SNOW_GRASS       
    {193, 193, 193, 193, 193, 193},      // 22  BLOCK_GLASS            
    {239, 239, 239, 239, 239, 239}       // 23  BLOCK_PLAYER_HAND     
};

int block_is_transparent(unsigned char block)
{
    switch (block)
    {
        case BLOCK_AIR:
        case BLOCK_GLASS:
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

/*

x, y, z: cube's world coordinates
faces:   determine whether to draw face or not
ao:      ao level for each vertex of each face

*/
void block_gen_vertices(
    Vertex* vertices, int curr_vertex_count, int x, int y, int z,
    int block_type, int center_align, float block_size, int faces[6], float ao[6][4]
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

    int curr_vertex = 0;

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

            int vert_index = curr_vertex_count + curr_vertex++;

            vertices[vert_index].pos[0] = (positions[f][index][0] + x) * block_size;
            vertices[vert_index].pos[1] = (positions[f][index][1] + y) * block_size;
            vertices[vert_index].pos[2] = (positions[f][index][2] + z) * block_size;

            if (center_align)
            {
                vertices[vert_index].pos[0] -= 0.5f * block_size;
                vertices[vert_index].pos[1] -= 0.5f * block_size;
                vertices[vert_index].pos[2] -= 0.5f * block_size;
            }

            vertices[vert_index].tex_coord[0] = uvs[f][index][0];
            vertices[vert_index].tex_coord[1] = uvs[f][index][1];

            vertices[vert_index].ao = ao[f][index];

            vertices[vert_index].tile = block_textures[block_type][f];
        }
    }
}

static int get_block_transparency(Chunk* c, int bx, int by, int bz)
{
    if (!c) return 1;
    return block_is_transparent(c->blocks[XYZ(bx, by, bz)]);
}

int block_set_visible_faces(Chunk* c, int x, int y, int z, Chunk* neighs[8], int faces[6])
{
    // left
    if (x == 0)
        faces[0] = get_block_transparency(neighs[CHUNK_NEIGH_L], CHUNK_WIDTH - 1, y, z);
    else
        faces[0] = get_block_transparency(c, x - 1, y, z);

    // right
    if (x == CHUNK_WIDTH - 1)
        faces[1] = get_block_transparency(neighs[CHUNK_NEIGH_R], 0, y, z);
    else
        faces[1] = get_block_transparency(c, x + 1, y, z);
 
    // top
    if (y == CHUNK_HEIGHT - 1)
        faces[2] = 1;
    else
        faces[2] = get_block_transparency(c, x, y + 1, z);

    // bottom
    if (y == 0)
        faces[3] = 0;
    else
        faces[3] = get_block_transparency(c, x, y - 1, z);

    // back
    if (z == 0)
        faces[4] = get_block_transparency(neighs[CHUNK_NEIGH_B], x, y, CHUNK_WIDTH - 1);
    else
        faces[4] = get_block_transparency(c, x, y, z - 1);

    // front
    if (z == CHUNK_WIDTH - 1)
        faces[5] = get_block_transparency(neighs[CHUNK_NEIGH_F], x, y, 0);
    else
        faces[5] = get_block_transparency(c, x, y, z + 1);

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