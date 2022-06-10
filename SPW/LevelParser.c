#include "LevelParser.h"

#include "Scene/Scene.h"
#include "Scene/LevelScene.h"

#include "GameObject/Player.h"
#include "GameObject/Collectable/Firefly.h"
#include "GameObject/Collectable/Heart.h"
#include "GameObject/Enemy/Nut.h"
#include "GameObject/Enemy/AngryNut.h"
#include "GameObject/Terrain/Brick.h"
#include "GameObject/Terrain/Bonus.h"
#include "GameObject/Terrain/MovingPlatform.h"
#include "GameObject/Terrain/Checkpoint.h"
#include "GameObject/Terrain/LevelEnd.h"
#include "GameObject/Terrain/StaticMap.h"

static char validChar[] = {
    '\n', '\\', '/', 'L', 'l', 'R', 'r' , '.', '#', '=','q',
    'W', 'C', 'e', 'S', 'X', 'o', '?', 'F', '+', 'M', 'B', 'A','E','p','P'
};

LevelParser* LevelParser_New(char* path)
{
    FILE* levelFile = fopen(path, "rb");
    AssertNew(levelFile);

    bool isValidChar[128] = { 0 };
    for (int i = 0; i < sizeof(validChar); ++i)
    {
        isValidChar[(int)validChar[i]] = true;
    }

    fseek(levelFile, 0, SEEK_END);
    long fileSize = ftell(levelFile);
    rewind(levelFile);

    char* buffer = (char*)calloc(fileSize, sizeof(char));
    AssertNew(buffer);

    fread(buffer, 1, fileSize, levelFile);
    fclose(levelFile);
    levelFile = NULL;

    int height = 0;
    int width = 0;
    int i;
    for (i = 0; i < fileSize; i++)
    {
        char c = buffer[i];
        if (isValidChar[(int)c])
        {
            if (c == '\n')
            {
                height = 1;
                break;
            }
            else
            {
                width++;
            }
        }
        else
        {
            printf("INFO - Invalid char (%c)\n", c);
        }
    }
    if (height == 0)
    {
        printf("ERROR - Incorrect level file\n");
        assert(false);
        abort();
    }
    int w = 0;
    for (i = i + 1; i < fileSize; ++i)
    {
        char c = buffer[i];
        if (isValidChar[(int)c])
        {
            if (c == '\n')
            {
                if (w == 0)
                {
                    break;
                }
                height++;

                if (w != width)
                {
                    printf("ERROR - Incorrect level file\n");
                    assert(false);
                    abort();
                }
                w = 0;
            }
            else
            {
                w++;
            }
        }
        else
        {
            printf("INFO - Invalid char (%c)\n", c);
        }
    }

    // Initialisation du parser
    LevelParser* parser = calloc(1, sizeof(LevelParser));
    AssertNew(parser);

    parser->m_width = width;
    parser->m_heigth = height;

    parser->m_matrix = calloc(width, sizeof(char*));
    AssertNew(parser->m_matrix);

    for (int x = 0; x < width; ++x)
    {
        parser->m_matrix[x] = calloc(height, sizeof(char));
        AssertNew(parser->m_matrix[x]);
    }

    // Remplissage de la matrice du niveau
    int x = 0;
    int y = height - 1;
    for (i = 0; i < fileSize; ++i)
    {
        char c = buffer[i];
        if (isValidChar[(int)c])
        {
            if (c == '\n')
            {
                x = 0;
                y--;
                if (y < 0)
                {
                    break;
                }
            }
            else
            {
                parser->m_matrix[x][y] = c;
                x++;
            }
        }
    }

    free(buffer);

    return parser;
}

void LevelParser_Delete(LevelParser* parser)
{
    if (!parser)
        return;

    if (parser->m_matrix)
    {
        int width = parser->m_width;
        for (int i = 0; i < width; ++i)
        {
            if (parser->m_matrix)
            {
                free(parser->m_matrix[i]);
            }
        }
        free(parser->m_matrix);
    }

    free(parser);
}

void LevelParser_InitScene(LevelParser* parser, void* scene)
{
    assert(parser && scene);

    char** matrix = parser->m_matrix;
    int width = parser->m_width;
    int height = parser->m_heigth;

    // Cr�e la TileMap
    StaticMap* map = (StaticMap*)Scene_AllocateObject(scene, Class_StaticMap);
    AssertNew(map);

    StaticMap_Constructor(map, scene, width, height);

    for (int x = 0; x < width; ++x)
    {
        for (int y = 0; y < height; ++y)
        {
            switch (matrix[x][y])
            {
            case '#':
                StaticMap_SetTile(map, x, y, TILE_GROUND);
                break;
            case 'W':
                StaticMap_SetTile(map, x, y, TILE_WOOD);
                break;
            case '=':
                StaticMap_SetTile(map, x, y, TILE_ONE_WAY);
                break;
            case 'A':
                StaticMap_SetTile(map, x, y, TILE_SPIKE);
                break;
            case '/':
                StaticMap_SetTile(map, x, y, TILE_STEEP_SLOPE_L);
                break;
            case '\\':
                StaticMap_SetTile(map, x, y, TILE_STEEP_SLOPE_R);
                break;
            case 'l':
                StaticMap_SetTile(map, x, y, TILE_GENTLE_SLOPE_L1);
                break;
            case 'L':
                StaticMap_SetTile(map, x, y, TILE_GENTLE_SLOPE_L2);
                break;
            case 'r':
                StaticMap_SetTile(map, x, y, TILE_GENTLE_SLOPE_R1);
                break;
            case 'R':
                StaticMap_SetTile(map, x, y, TILE_GENTLE_SLOPE_R2);
                break;

            case 'S':
                if (Object_IsA(scene, Class_LevelScene))
                {
                    Player* player = LevelScene_GetPlayer(scene);
                    GameBody_SaveStartPosition(player, PE_Vec2_Set((float)x + 0.5f, (float)y));
                }
                break;
            case 'o':
            {
                Firefly* firefly = Scene_AllocateObject(scene, Class_Firefly);
                AssertNew(firefly);
                Firefly_Constructor(firefly, scene, PE_Vec2_Set((float)x, (float)y));
                break;
            }
            case 'F':
            {
                LevelEnd* lvlEnd = Scene_AllocateObject(scene, Class_LevelEnd);
                AssertNew(lvlEnd);
                LevelEnd_Constructor(lvlEnd, scene, PE_Vec2_Set((float)x, (float)y));
                break;
            }
            case 'e':
            {
                Nut* nut = Scene_AllocateObject(scene, Class_Nut);
                AssertNew(nut);
                Nut_Constructor(nut, scene, PE_Vec2_Set((float)x + 0.5f, (float)y));
                break;
            }
            case 'E':
            {
                ANut* AngryNut = Scene_AllocateObject(scene, Class_ANut);
                AssertNew(AngryNut);
                AngryNut_Constructor(AngryNut, scene, PE_Vec2_Set((float)x + 0.5f, (float)y));
                break;
            }
            case 'C':
            {
                Checkpoint* checkpoint = Scene_AllocateObject(scene, Class_Checkpoint);
                AssertNew(checkpoint);
                Checkpoint_Constructor(checkpoint, scene, PE_Vec2_Set((float)x, (float)y));
                break;
            }
            case 'B':
            {
                Brick* brick = Scene_AllocateObject(scene, Class_Brick);
                AssertNew(brick);
                Brick_Constructor(brick, scene, PE_Vec2_Set((float)x, (float)y));
                break;
            }
            case 'q':
            {
                MovingPlatform* platformHeight = Scene_AllocateObject(scene, Class_MovingPlatform);
                AssertNew(platformHeight);

                PE_ColliderDef colliderDef = { 0 };
                PE_ColliderDef_SetDefault(&colliderDef);
                colliderDef.filter.categoryBits = FILTER_TERRAIN;
                colliderDef.isOneWay = true;

                PE_Shape_SetAsBox(&colliderDef.shape, -2.0f, -0.25f, 2.0f, 0.25f);
                MovingPlatform_Constructor(platformHeight, scene, PE_Vec2_Set((float)x, (float)y), &colliderDef);

                MovingPlatform_SetPointCount(platformHeight, 2); //nb point total pour les chemins des platformes
                PE_Vec2* points = MovingPlatform_GetPoints(platformHeight); //tableau points

                points[0] = PE_Vec2_Set((float)x, (float)y + 2.0f); //point de départ
                points[1] = PE_Vec2_Set((float)x, (float)y + 7.0f);
                MovingPlatform_SetSpeed(platformHeight, 3.0f); //vitesse de la platforme
                break;
            }
            case 'p':
            {
                MovingPlatform* platformWidth = Scene_AllocateObject(scene, Class_MovingPlatform);
                AssertNew(platformWidth);

                PE_ColliderDef colliderDef = { 0 };
                PE_ColliderDef_SetDefault(&colliderDef);
                colliderDef.filter.categoryBits = FILTER_TERRAIN;
                colliderDef.isOneWay = true;

                PE_Shape_SetAsBox(&colliderDef.shape, -2.0f, -0.25f, 2.0f, 0.25f);
                MovingPlatform_Constructor(platformWidth, scene, PE_Vec2_Set((float)x, (float)y), &colliderDef);

                MovingPlatform_SetPointCount(platformWidth, 2); //nb point total pour les chemins des platformes
                PE_Vec2* points = MovingPlatform_GetPoints(platformWidth); //tableau points

                points[0] = PE_Vec2_Set((float)x, (float)y + 2.0f); //point de départ
                points[1] = PE_Vec2_Set((float)x + 5.0f, (float)y + 2.0f);
                MovingPlatform_SetSpeed(platformWidth, 3.0f); //vitesse de la platforme
                break;
            }
            case 'P':
            {
                MovingPlatform* PlatformTriangleRec = Scene_AllocateObject(scene, Class_MovingPlatform);
                AssertNew(PlatformTriangleRec);

                PE_ColliderDef colliderDef = { 0 };
                PE_ColliderDef_SetDefault(&colliderDef);
                colliderDef.filter.categoryBits = FILTER_TERRAIN;
                colliderDef.isOneWay = true;

                PE_Shape_SetAsBox(&colliderDef.shape, -2.0f, -0.25f, 2.0f, 0.25f);
                MovingPlatform_Constructor(PlatformTriangleRec, scene, PE_Vec2_Set((float)x, (float)y), &colliderDef);

                MovingPlatform_SetPointCount(PlatformTriangleRec, 3); //nb point total pour les chemins des platformes
                PE_Vec2* points = MovingPlatform_GetPoints(PlatformTriangleRec); //tableau points

                points[0] = PE_Vec2_Set((float)x, (float)y + 2.0f); //point de départ
                points[1] = PE_Vec2_Set((float)x + 5.0f, (float)y + 2.0f);
                points[2] = PE_Vec2_Set((float)x + 0.0f, (float)y + 5.0f); //point d'arrivé
                MovingPlatform_SetSpeed(PlatformTriangleRec, 2.0f); //vitesse de la platforme
                break;
            }
            case '+':
            {
                Heart* heart = Scene_AllocateObject(scene, Class_Heart);
                AssertNew(heart);
                Heart_Constructor(heart, scene, PE_Vec2_Set((float)x, (float)y));
                break;
            }
            case '?':
            {
                Bonus* bonus = Scene_AllocateObject(scene, Class_Bonus);
                AssertNew(bonus);
                Bonus_Constructor(bonus, scene, PE_Vec2_Set((float)x, (float)y));
                break;
            }
            default:
                break;
            }
        }
    }
    StaticMap_InitTiles(map);
}