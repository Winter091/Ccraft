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