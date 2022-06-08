﻿#include "LevelHeader.h"
#include "../../../Scene/LevelScene.h"
#include "../Button.h"
#include "../Image.h"
#include "../UIObject.h"
#include "../Text.h"

// Object virtual methods
void LevelHeader_VM_Destructor(void *self);

// GameObject virtual methods
void LevelHeader_VM_Update(void *self);

static LevelHeaderClass _Class_LevelHeader = { 0 };
const void *const Class_LevelHeader = &_Class_LevelHeader;

void Class_InitLevelHeader()
{
    if (!Class_IsInitialized(Class_LevelHeader))
    {
        Class_InitUIObject();

        void *self = (void *)Class_LevelHeader;
        ClassCtorParams params = {
            .self = self,
            .super = Class_UIObject,
            .name = "LevelHeader",
            .instanceSize = sizeof(LevelHeader),
            .classSize = sizeof(LevelHeaderClass)
        };
        Class_Constructor(params, LevelHeader_VM_Destructor);
        ((GameObjectClass *)self)->Update = LevelHeader_VM_Update;
    }
}

void LevelHeader_Constructor(void *self, void *scene)
{
    UIObject_Constructor(self, scene);
    Object_SetClass(self, Class_LevelHeader);

    LevelHeader *panel = Object_Cast(self, Class_LevelHeader);
    AssetManager *assets = Scene_GetAssetManager(scene);
    TTF_Font *font = AssetManager_GetLargeFont(assets);
    SDL_Color normalColor = AssetManager_GetNormalColor(assets);

    int w, h;
    TTF_SizeUTF8(font, u8"00", &w, &h);
    float imgH = 80.0f;
    float imgW = imgH;
    float numW = (float)w;
    float sep = 20.0f;
    float panelW = imgW + sep + numW;
    float panelH = imgH;

    UIRect rect;
    rect.m_anchorMin = Vec2_Set(0.0f, 0.0f);
    rect.m_anchorMax = Vec2_Set(0.0f, 0.0f);
    rect.m_offsetMin = Vec2_Set(80.0f, 40.0f);
    rect.m_offsetMax = Vec2_Set(80.0f + panelW, 40.0f + panelH);
    UIObject_SetLocalRect(panel, rect);

    // Image du nombre de luciolles
    RE_Atlas *uiAtlas = AssetManager_GetUIAtlas(assets);
    RE_AtlasPart *part = RE_Atlas_GetPart(uiAtlas, "Firefly");
    RE_AtlasPart* Hpart = RE_Atlas_GetPart(uiAtlas, "Heart");
    AssertNew(part);
    AssertNew(Hpart);

    Image *fireflyImg = Scene_AllocateObject(scene, Class_Image);
    AssertNew(fireflyImg);
    Image* heartImg = Scene_AllocateObject(scene, Class_Image);
    AssertNew(heartImg);

    Image_Constructor(fireflyImg, scene, part, 0);
    rect.m_anchorMin = Vec2_Set(0.0f, 0.0f);
    rect.m_anchorMax = Vec2_Set(0.0f, 0.0f);
    rect.m_offsetMin = Vec2_Set(0.0f, 0.0f);
    rect.m_offsetMax = Vec2_Set(imgW, imgH);
    UIObject_SetLocalRect(fireflyImg, rect);
    GameObject_SetParent(fireflyImg, panel);

    Image_Constructor(heartImg, scene, Hpart, 0);
    rect.m_anchorMin = Vec2_Set(0.0f, 1.0f);
    rect.m_anchorMax = Vec2_Set(0.0f, 1.0f);
    rect.m_offsetMin = Vec2_Set(0.0f, 1.0f);
    rect.m_offsetMax = Vec2_Set(imgW, imgH);
    UIObject_SetLocalRect(heartImg, rect);
    GameObject_SetParent(heartImg, panel);

    // Compteur du nombre de luciolles
    Text *fireflyCountText = Scene_AllocateObject(scene, Class_Text);
    AssertNew(fireflyCountText);
    Text_Constructor(fireflyCountText, scene, "0", font, normalColor);
    Text_SetAnchor(fireflyCountText, RE_ANCHOR_LEFT | RE_ANCHOR_MIDDLE);
    rect.m_anchorMin = Vec2_Set(0.0f, 0.0f);
    rect.m_anchorMax = Vec2_Set(0.0f, 0.0f);
    rect.m_offsetMin = Vec2_Set(imgW + sep, 0.0f);
    rect.m_offsetMax = Vec2_Set(imgW + sep + numW, imgH);
    UIObject_SetLocalRect(fireflyCountText, rect);
    GameObject_SetParent(fireflyCountText, panel);
    panel->m_fireflyCountText = fireflyCountText;

    Text* heartCountText = Scene_AllocateObject(scene, Class_Text);
    AssertNew(heartCountText);
    Text_Constructor(heartCountText, scene, "2", font, normalColor);
    Text_SetAnchor(heartCountText, RE_ANCHOR_LEFT | RE_ANCHOR_MIDDLE);
    rect.m_anchorMin = Vec2_Set(0.0f, 1.0f);
    rect.m_anchorMax = Vec2_Set(0.0f, 1.0f);
    rect.m_offsetMin = Vec2_Set(imgW + sep, 1.0f);
    rect.m_offsetMax = Vec2_Set(imgW + sep + numW, imgH);
    UIObject_SetLocalRect(heartCountText, rect);
    GameObject_SetParent(heartCountText, panel);
    panel->m_heartCountText = heartCountText;
}

void LevelHeader_VM_Destructor(void *self)
{
    // Tous les enfants sont supprimés automatiquement par la scène
    LevelHeader *text = Object_Cast(self, Class_LevelHeader);

    // Destructeur de la classe mère
    Object_SuperDestroy(self, Class_LevelHeader);
}

void LevelHeader_VM_Update(void *self)
{
    LevelHeader *panel = Object_Cast(self, Class_LevelHeader);
    Scene *scene = GameObject_GetScene(self);
    Player *player = LevelScene_GetPlayer(scene);
    char str[128] = { 0 };
    int count = 0;
    char strH[128] = { 0 };
    int countH = 0;

    count = Player_GetFireflyCount(player);
    sprintf(str, "%d", count);
    Text_SetString(panel->m_fireflyCountText, str);


    countH = Player_GetHeartCount(player);
    sprintf(strH, "%d", countH);
    Text_SetString(panel->m_heartCountText, strH);
}