#include "block.h"

// left right top bottom back front
unsigned char block_textures[][6] = 
{
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
    {193, 193, 193, 193, 193, 193}       // 22  BLOCK_GLASS            
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

// x, y, z = cube's world coordinates
// faces = determine whether to draw face or not
void block_gen_vertices(
    Vertex* vertices, int curr_vertex_count, 
    int x, int y, int z, int block_type, int faces[6]
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
            int index = indices[f][v];
            int vert_index = curr_vertex_count + curr_vertex++;

            vertices[vert_index].pos[0] = (positions[f][index][0] + x) * BLOCK_SIZE;
            vertices[vert_index].pos[1] = (positions[f][index][1] + y) * BLOCK_SIZE;
            vertices[vert_index].pos[2] = (positions[f][index][2] + z) * BLOCK_SIZE;

            vertices[vert_index].tex_coord[0] = uvs[f][index][0];
            vertices[vert_index].tex_coord[1] = uvs[f][index][1];

            vertices[vert_index].tile = block_textures[block_type][f];
        }
    }
}

void block_set_visible_faces(
    Chunk* c, int x, int y, int z, 
    Chunk* left, Chunk* right, Chunk* front, Chunk* back, int faces[6]
)
{
    // left
    if (x == 0)
    {
        if (!left)
            faces[0] = 1;
        else if (block_is_transparent(left->blocks[XYZ(CHUNK_WIDTH - 1, y, z)]))
            faces[0] = 1;
        else
            faces[0] = 0;
    }
    else
    {
        if (block_is_transparent(c->blocks[XYZ(x - 1, y, z)]))
            faces[0] = 1;
        else
            faces[0] = 0;
    }

    // right
    if (x == CHUNK_WIDTH - 1)
    {
        if (!right)
            faces[1] = 1;
        else if (block_is_transparent(right->blocks[XYZ(0, y, z)]))
            faces[1] = 1;
        else
            faces[1] = 0;
    }
    else
    {
        if (block_is_transparent(c->blocks[XYZ(x + 1, y, z)]))
            faces[1] = 1;
        else
            faces[1] = 0;
    }

    // top
    if (y == CHUNK_HEIGHT - 1)
    {
        faces[2] = 1;
    }
    else
    {
        if (block_is_transparent(c->blocks[XYZ(x, y + 1, z)]))
            faces[2] = 1;
        else
            faces[2] = 0;
    }

    // bottom
    if (y == 0)
    {
        faces[3] = 0;
    }
    else
    {
        if (block_is_transparent(c->blocks[XYZ(x, y - 1, z)]))
            faces[3] = 1;
        else
            faces[3] = 0;
    }

    // back
    if (z == 0)
    {
        if (!back)
            faces[4] = 1;
        else if (block_is_transparent(back->blocks[XYZ(x, y, CHUNK_WIDTH - 1)]))
            faces[4] = 1;
        else
            faces[4] = 0;
    }
    else
    {
        if (block_is_transparent(c->blocks[XYZ(x, y, z - 1)]))
            faces[4] = 1;
        else
            faces[4] = 0;
    }

    // front
    if (z == CHUNK_WIDTH - 1)
    {
        if (!front)
            faces[5] = 1;
        else if (block_is_transparent(front->blocks[XYZ(x, y, 0)]))
            faces[5] = 1;
        else
            faces[5] = 0;
    }
    else
    {
        if (block_is_transparent(c->blocks[XYZ(x, y, z + 1)]))
            faces[5] = 1;
        else
            faces[5] = 0;
    }
}