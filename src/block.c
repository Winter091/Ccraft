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
        int block_type, float block_size, int is_short, int faces[6], float ao[6][4]
)
{
    // 6 faces, each face has 4 points forming a square
    static const float positions[6][4][3] = 
    {
        { {0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1} }, // left
        { {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1} }, // right
        { {0, 1, 0}, {0, 1, 1}, {1, 1, 0}, {1, 1, 1} }, // top
        { {0, 0, 0}, {0, 0, 1}, {1, 0, 0}, {1, 0, 1} }, // bottom
        { {0, 0, 0}, {0, 1, 0}, {1, 0, 0}, {1, 1, 0} }, // back
        { {0, 0, 1}, {0, 1, 1}, {1, 0, 1}, {1, 1, 1} }  // front
    };

    // Cactus is a bit smaller than other blocks
#define a 0.0625f
#define b 1.0f - a
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
        { {0, 0}, {1, 0}, {0, 1}, {1, 1} },
        { {1, 0}, {0, 0}, {1, 1}, {0, 1} },
        { {0, 1}, {0, 0}, {1, 1}, {1, 0} },
        { {0, 0}, {0, 1}, {1, 0}, {1, 1} },
        { {0, 0}, {0, 1}, {1, 0}, {1, 1} },
        { {1, 0}, {1, 1}, {0, 0}, {0, 1} }
    };

    // for each face
    for (int f = 0; f < 6; f++)
    {
        if (!faces[f]) continue;
        
        // for each vertex
        for (int v = 0; v < 6; v++)
        {
            // Flip some quads to eliminate ao unevenness
            int index = ao[f][0] + ao[f][3] > ao[f][1] + ao[f][2] ?
                indices_flipped[f][v] : indices[f][v];

            int i = (*curr_vertex_count)++;

            // cactus is a bit thinner than other blocks
            if (block_type == BLOCK_CACTUS)
            {
                vertices[i].pos[0] = (positions_cactus[f][index][0] + (float)x) * block_size;
                vertices[i].pos[1] = (positions_cactus[f][index][1] + (float)y) * block_size;
                vertices[i].pos[2] = (positions_cactus[f][index][2] + (float)z) * block_size;
            }
            else
            {
                vertices[i].pos[0] = (positions[f][index][0] + (float)x) * block_size;
                vertices[i].pos[1] = (positions[f][index][1] + (float)y) * block_size;
                vertices[i].pos[2] = (positions[f][index][2] + (float)z) * block_size;
            }

            // Shorten only a top of a block
            if (is_short && positions[f][index][1] > 0)
            {
                vertices[i].pos[1] -= 0.125f * block_size;
            }

            vertices[i].tex_coord[0] = uvs[f][index][0];
            vertices[i].tex_coord[1] = uvs[f][index][1];
            vertices[i].ao = ao[f][index];
            vertices[i].tile = block_textures[block_type][f];
        }
    }
}

void gen_plant_vertices(
    Vertex* vertices, int* curr_vertex_count, int x, int y, int z,
    int block_type, float block_size
)
{
    // A cross made up of 2 faces
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
            int i = (*curr_vertex_count)++;

            vertices[i].pos[0] = (positions[f / 2][index][0] + (float)x) * block_size;
            vertices[i].pos[1] = (positions[f / 2][index][1] + (float)y) * block_size;
            vertices[i].pos[2] = (positions[f / 2][index][2] + (float)z) * block_size;

            vertices[i].tex_coord[0] = uvs[index][0];
            vertices[i].tex_coord[1] = uvs[index][1];
            vertices[i].ao = 0.0f;
            vertices[i].tile = block_textures[block_type][f];
        }
    }
}

void block_gen_aabb(int x, int y, int z, vec3 aabb[2])
{
    aabb[0][0] = (float)x * BLOCK_SIZE;
    aabb[0][1] = (float)y * BLOCK_SIZE;
    aabb[0][2] = (float)z * BLOCK_SIZE;
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
