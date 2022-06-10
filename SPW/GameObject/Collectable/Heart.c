#include "Heart.h"
#include "../../Scene/LevelScene.h"
#include "../../Utils/Renderer.h"

// Object virtual methods
void Heart_VM_Destructor(void* self);

// GameObject virtual methods
void Heart_VM_OnRespawn(void* self);
void Heart_VM_FixedUpdate(void* self);
void Heart_VM_Render(void* self);
void Heart_VM_Start(void* self);
void Heart_VM_Update(void* self);

// Collectable virtual methods
void Heart_VM_Collect(void* self, void* dst);

// Callbacks de collisions
void Heart_OnCollisionStay(PE_Collision* collision);

static HeartClass _Class_Heart = { 0 };
const void* const Class_Heart = &_Class_Heart;

void Class_InitHeart()
{
    if (!Class_IsInitialized(Class_Heart))
    {
        Class_InitCollectable();

        void* self = (void*)Class_Heart;
        ClassCtorParams params = {
            .self = self,
            .super = Class_Collectable,
            .name = "Heart",
            .instanceSize = sizeof(Heart),
            .classSize = sizeof(HeartClass)
        };
        Class_Constructor(params, Heart_VM_Destructor);
        ((GameObjectClass*)self)->FixedUpdate = Heart_VM_FixedUpdate;
        ((GameObjectClass*)self)->OnRespawn = Heart_VM_OnRespawn;
        ((GameObjectClass*)self)->Render = Heart_VM_Render;
        ((GameObjectClass*)self)->Start = Heart_VM_Start;
        ((GameObjectClass*)self)->Update = Heart_VM_Update;
        ((CollectableClass*)self)->Collect = Heart_VM_Collect;
    }
}

void Heart_CreateAnimator(Heart* heart, void* scene)
{
    AssetManager* assets = Scene_GetAssetManager(scene);
    RE_Atlas* atlas = AssetManager_GetCollectableAtlas(assets);
    RE_AtlasPart* part = NULL;
    void* anim = NULL;

    // Crée l'animateur
    RE_Animator* animator = RE_Animator_New();
    AssertNew(animator);

    heart->m_animator = animator;

    // Animation "Heart"
    part = RE_Atlas_GetPart(atlas, "Heart");
    AssertNew(part);

    anim = RE_Animator_CreateTextureAnim(animator, "Heart", part);
    AssertNew(anim);
    RE_Animation_SetCycleCount(anim, -1);
    RE_Animation_SetCycleTime(anim, 0.3f);
}

void Heart_Constructor(void* self, void* scene, PE_Vec2 startPos)
{
    Collectable_Constructor(self, scene, PE_Vec2_Add(startPos, PE_Vec2_Set(0.5f, 0.5f)));
    Object_SetClass(self, Class_Heart);

    Heart* heart = Object_Cast(self, Class_Heart);
    heart->m_speed = 2.0f;

    Heart_CreateAnimator(heart, scene);
    Scene_SetToRespawn(scene, self, true);
}

void Heart_VM_Start(void* self)
{
    Heart* heart = Object_Cast(self, Class_Heart);
    Scene* scene = GameObject_GetScene(heart);
    PE_World* world = Scene_GetWorld(scene);
    PE_Body* body = NULL;
    PE_BodyDef bodyDef = { 0 };
    PE_ColliderDef colliderDef = { 0 };
    PE_Collider* collider = NULL;

    // Crée le corps
    PE_BodyDef_SetDefault(&bodyDef);
    bodyDef.type = PE_DYNAMIC_BODY;
    bodyDef.position = GameBody_GetStartPosition(heart);
    bodyDef.name = "Heart";
    bodyDef.xDamping = 0.0f;
    bodyDef.yDamping = 0.0f;
    bodyDef.mass = 1.0f;

    body = PE_World_CreateBody(world, &bodyDef);
    AssertNew(body);

    // Crée le collider
    PE_ColliderDef_SetDefault(&colliderDef);
    colliderDef.friction = 0.005f;
    colliderDef.filter.categoryBits = FILTER_COLLECTABLE;
    colliderDef.filter.maskBits = FILTER_TERRAIN | FILTER_PLAYER;
    PE_Shape_SetAsCircle(&colliderDef.shape, PE_Vec2_Set(0.0f, -0.1f), 0.4f);

    collider = PE_Body_CreateCollider(body, &colliderDef);
    AssertNew(collider);

    PE_Collider_SetOnCollisionStay(collider, Heart_OnCollisionStay);
	
    GameBody_SetBody(self, body);

    // Joue l'animation par défaut
    RE_Animator_PlayAnimation(heart->m_animator, "Heart");
}

void Heart_VM_Destructor(void* self)
{
    Heart* heart = Object_Cast(self, Class_Heart);

    RE_Animator_Delete(heart->m_animator);

    // Destructeur de la classe mère
    Object_SuperDestroy(self, Class_Heart);
}

void Heart_OnCollisionStay(PE_Collision* collision)
{
    PE_Manifold manifold = PE_Collision_GetManifold(collision);
    PE_Body* thisBody = PE_Collision_GetBody(collision);
    PE_Body* otherBody = PE_Collision_GetOtherBody(collision);
    PE_Collider* otherCollider = PE_Collision_GetOtherCollider(collision);

    GameBody* thisGameBody = GameBody_GetFromBody(thisBody);
    GameBody* otherGameBody = GameBody_GetFromBody(otherBody);
    Heart* heart = Object_Cast(thisGameBody, Class_Heart);

    // Collision avec le joueur
    if (PE_Collider_CheckCategory(otherCollider, FILTER_PLAYER))
    {
        Player* player = Object_Cast(otherGameBody, Class_Player);
    }

    // Collision avec les mur
    if (PE_Collider_CheckCategory(otherCollider, FILTER_TERRAIN))
    {
        float angle = PE_Vec2_AngleDeg(manifold.normal, PE_Vec2_Left);

        if (angle == 180.0f || angle == 0.0f)
        {
            heart->m_speed *= -1.0f;
        }
    }
	
    if (heart->m_state == HEART_DYING)
    {
        PE_Collision_SetEnabled(collision, false);
    }
}


void Heart_VM_FixedUpdate(void* self)
{
    Heart* heart = Object_Cast(self, Class_Heart);
    PE_Body* body = GameBody_GetBody(self);

    if (!body) return;
	
    PE_Vec2 position = PE_Body_GetPosition(body);
    PE_Vec2 velocity = PE_Body_GetLocalVelocity(body);
	
    // Tue le coeur si elle tombe dans un trou
    if (position.y < -2.0f)
    {
        Scene* scene = GameObject_GetScene(self);
        Scene_DisableObject(scene, self);
        return;
    }
	
    velocity.x = heart->m_speed;
	
	
    PE_Body_SetVelocity(body, velocity);
}

void Heart_VM_Update(void* self)
{
    Heart* heart = Object_Cast(self, Class_Heart);
    RE_Animator_Update(heart->m_animator, g_time);
}

void Heart_VM_OnRespawn(void* self)
{
    Heart* heart = Object_Cast(self, Class_Heart);
	
    Scene* scene = GameObject_GetScene(self);
	

    Scene_SetToRespawn(scene, self, false);

    RE_Animator_StopAnimations(heart->m_animator);
    RE_Animator_PlayAnimation(heart->m_animator, "Heart");
}

void Heart_VM_Render(void* self)
{
    Heart* heart = Object_Cast(self, Class_Heart);
    Scene* scene = GameObject_GetScene(self);
    SDL_Renderer* renderer = Scene_GetRenderer(scene);
    Camera* camera = Scene_GetActiveCamera(scene);
    PE_Vec2 position = GameBody_GetPosition(self);

    SDL_FRect dst = { 0 };
    Camera_WorldToView(camera, position, &(dst.x), &(dst.y));
    float scale = Camera_GetWorldToViewScale(camera);
    dst.w = scale * 1.0f; // Le sprite fait 1 tuile de large
    dst.h = scale * 1.0f; // Le sprite fait 1 tuile de haut

    RE_Animator_RenderCopyF(
        heart->m_animator, renderer, &dst, RE_ANCHOR_CENTER | RE_ANCHOR_MIDDLE
    );
}

void Heart_VM_Collect(void* self, void* dst)
{
    Heart* heart = Object_Cast(self, Class_Heart);
    Scene* scene = GameObject_GetScene(self);

    // Vérifie que le destinataire est bien le joueur
    // On ne souhaite pas qu'une noisette nous vole nos coeur !
    if (Object_IsA(dst, Class_Player))
    {
        Player* player = Object_Cast(dst, Class_Player);

        // Ajoute un coeur au joueur
        Player_AddHeart(player);
    }

    // Indique qu'il faut regénérer l'objet si le joueur meurt
    Scene_SetToRespawn(scene, self, true);

    // Désactive la luciolle
    Scene_DisableObject(scene, heart);
}