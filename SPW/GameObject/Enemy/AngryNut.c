﻿#include "AngryNut.h"
#include "../../Scene/LevelScene.h"
#include "../../Utils/Renderer.h"


// Object virtual methods
void ANut_VM_Destructor(void* self);

// GameObject virtual methods
void ANut_VM_DrawGizmos(void* self);
void ANut_VM_FixedUpdate(void* self);
void ANut_VM_OnRespawn(void* self);
void ANut_VM_Render(void* self);
void ANut_VM_Start(void* self);
void ANut_VM_Update(void* self);

// Enemy virtual methods
void ANut_VM_Damage(void* self, void* damager);


// Callbacks de collisions
void ANut_OnCollisionStay(PE_Collision* collision);

static ANutClass _Class_ANut = { 0 };
const void* const Class_ANut = &_Class_ANut;

void Class_InitANut()
{
    if (!Class_IsInitialized(Class_ANut))
    {
        Class_InitEnemy();

        void* self = (void*)Class_ANut;
        ClassCtorParams params = {
            .self = self,
            .super = Class_Enemy,
            .name = "AngryNut",
            .instanceSize = sizeof(ANut),
            .classSize = sizeof(ANutClass)
        };
        Class_Constructor(params, ANut_VM_Destructor);
        ((GameObjectClass*)self)->DrawGizmos = ANut_VM_DrawGizmos;
        ((GameObjectClass*)self)->FixedUpdate = ANut_VM_FixedUpdate;
        ((GameObjectClass*)self)->OnRespawn = ANut_VM_OnRespawn;
        ((GameObjectClass*)self)->Render = ANut_VM_Render;
        ((GameObjectClass*)self)->Start = ANut_VM_Start;
        ((GameObjectClass*)self)->Update = ANut_VM_Update;
        ((EnemyClass*)self)->Damage = ANut_VM_Damage;
    }
}
void ANut_CreateAnimator(ANut* Angrynut, void* scene)
{
    AssetManager* assets = Scene_GetAssetManager(scene);
    RE_Atlas* atlas = AssetManager_GetEnemyAtlas(assets);
    void* anim = NULL;

    // Cr�e l'animateur
    RE_Animator* animator = RE_Animator_New();
    AssertNew(animator);

    Angrynut->m_animator = animator;

    // Animation "Idle"
    RE_AtlasPart* part = RE_Atlas_GetPart(atlas, "AngryNutIdle");
    AssertNew(part);

    anim = RE_Animator_CreateTextureAnim(animator, "AngryIdle", part);
    AssertNew(anim);
    RE_Animation_SetCycleCount(anim, 0);

    // Animation "Spinning"
    RE_AtlasPart* partS = RE_Atlas_GetPart(atlas, "AngryNutSpinning");
    AssertNew(partS);

    anim = RE_Animator_CreateTextureAnim(animator, "AngrySpinning", partS);
    AssertNew(anim);
    RE_Animation_SetCycleCount(anim, 0);

    // Animation "Dying"
    RE_AtlasPart* partD = RE_Atlas_GetPart(atlas, "AngryNutDying");
    AssertNew(partD);

    anim = RE_Animator_CreateTextureAnim(animator, "AngryDying", partD);
    AssertNew(anim);
    RE_Animation_SetCycleCount(anim, 0);
}

void AngryNut_Constructor(void* self, void* scene, PE_Vec2 startPos)
{
    Enemy_Constructor(self, scene, startPos, 1);
    Object_SetClass(self, Class_ANut);

    ANut* Angrynut = Object_Cast(self, Class_ANut);
    Angrynut->m_state = ANGRYNUT_IDLE;

    ANut_CreateAnimator(Angrynut, scene);
    Scene_SetToRespawn(scene, self, true);
}

void ANut_VM_Start(void* self)
{
    ANut* Angrynut = Object_Cast(self, Class_ANut);
    Scene* scene = GameObject_GetScene(Angrynut);
    PE_World* world = Scene_GetWorld(scene);
    PE_Body* body = NULL;
    PE_BodyDef bodyDef = { 0 };
    PE_ColliderDef colliderDef = { 0 };
    PE_Collider* collider = NULL;

    // Cr�e le corps
    PE_BodyDef_SetDefault(&bodyDef);
    bodyDef.type = PE_DYNAMIC_BODY;
    bodyDef.position = GameBody_GetStartPosition(Angrynut);
    bodyDef.name = "AngryNut";
    bodyDef.xDamping = 0.0f;
    bodyDef.yDamping = 0.0f;
    bodyDef.mass = 1.0f;

    body = PE_World_CreateBody(world, &bodyDef);
    AssertNew(body);

    PE_ColliderDef_SetDefault(&colliderDef);
    colliderDef.friction = 0.005f;
    colliderDef.filter.categoryBits = FILTER_ENEMY;
    colliderDef.filter.maskBits = FILTER_TERRAIN | FILTER_PLAYER;
    PE_Shape_SetAsCircle(
        &colliderDef.shape,
        PE_Vec2_Set(0.0f, 0.45f), 0.45f
    );

    collider = PE_Body_CreateCollider(body, &colliderDef);
    AssertNew(collider);

    PE_Collider_SetOnCollisionStay(collider, ANut_OnCollisionStay);

    GameBody_SetBody(self, body);

    // Endort le corps
    // Permet d'optimiser le calcul de la physique,
    // seuls les corps proches du joueur sont simul�s
    PE_Body_SetAwake(body, false);

    // Joue l'animation par d�faut
    RE_Animator_PlayAnimation(Angrynut->m_animator, "AngrySpinning");
}

void ANut_VM_Damage(void* self, void* damager)
{
    ANut* Angrynut = Object_Cast(self, Class_ANut);
    Scene* scene = GameObject_GetScene(Angrynut);

    PE_Vec2 position = GameBody_GetPosition(Angrynut);

    if (Object_IsA(damager, Class_Player))
    {
        Player_Bounce(damager);
    }

    Angrynut->m_state = ANGRYNUT_DYING;
    RE_Animator_StopAnimations(Angrynut->m_animator);
    RE_Animator_PlayAnimation(Angrynut->m_animator, "AngryDying");
    if (position.y < -2.0f)
    {
        Scene_DisableObject(scene, Angrynut);
    }
}

void ANut_VM_Destructor(void* self)
{
    // Destructeur de la classe m�re
    Object_SuperDestroy(self, Class_ANut);
}

void ANut_OnCollisionStay(PE_Collision* collision)
{
    PE_Manifold manifold = PE_Collision_GetManifold(collision);
    PE_Body* thisBody = PE_Collision_GetBody(collision);
    PE_Body* otherBody = PE_Collision_GetOtherBody(collision);
    PE_Collider* otherCollider = PE_Collision_GetOtherCollider(collision);

    GameBody* thisGameBody = GameBody_GetFromBody(thisBody);
    GameBody* otherGameBody = GameBody_GetFromBody(otherBody);
    ANut* Angrynut = Object_Cast(thisGameBody, Class_ANut);

    // Collision avec le joueur
    if (PE_Collider_CheckCategory(otherCollider, FILTER_PLAYER))
    {
        Player* player = Object_Cast(otherGameBody, Class_Player);

        float angle = PE_Vec2_AngleDeg(manifold.normal, PE_Vec2_Down);
        if (angle > 55.0f)
        {
            Player_Damage(player);
        }
        return;
    }

    if (Angrynut->m_state == ANGRYNUT_DYING)
    {
        PE_Collision_SetEnabled(collision, false);
    }
}

void ANut_VM_DrawGizmos(void* self)
{
    ANut* AngryNut = Object_Cast(self, Class_ANut);
    Scene* scene = GameObject_GetScene(self);
    SDL_Renderer* renderer = Scene_GetRenderer(scene);
    Camera* camera = Scene_GetActiveCamera(scene);

    PE_Vec2 position = GameBody_GetPosition(AngryNut);
    PE_Vec2 velocity = GameBody_GetVelocity(AngryNut);

    // Dessine en blanc le vecteur vitesse du joueur
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    Renderer_DrawVector(renderer, camera, position, velocity);
}

void ANut_VM_FixedUpdate(void* self)
{
    ANut* AngryNut = Object_Cast(self, Class_ANut);
    PE_Body* body = GameBody_GetBody(self);
    PE_Vec2 position = PE_Body_GetPosition(body);
    PE_Vec2 velocity = PE_Body_GetLocalVelocity(body);

    if (PE_Body_IsAwake(body) == false)
    {
        // Ne met pas � jour la noisette si elle est endormie
        // Le joueur est loin d'elle et elle n'est plus visible par la cam�ra
        return;
    }

    // Tue la noisette si elle tombe dans un trou
    if (position.y < -2.0f)
    {
        Scene* scene = GameObject_GetScene(self);
        Scene_DisableObject(scene, self);
        return;
    }

    Scene* scene = GameObject_GetScene(self);
    Player* player = LevelScene_GetPlayer(scene);
    PE_Vec2 playerPos = GameBody_GetPosition(player);

    // Calcule la distance entre le joueur et la noisette
    float dist = PE_Vec2_Distance(position, playerPos);
    float direction = (position.x - playerPos.x);

    if (dist > 24.0f)
    {
        // La distance entre de joueur et la noisette vient de d�passer 24 tuiles.
        // On endort la noisette pour ne plus la simuler dans le moteur physique.
        PE_Body_SetAwake(body, false);
        return;
    }
    else if (dist <= 5.0f && AngryNut->m_state == ANGRYNUT_IDLE)
    {
        // Le joueur est � moins de 5 tuiles de la noisette
        AngryNut->m_state = ANGRYNUT_SPINNING;
        velocity = PE_Vec2_Set(-3.f, 10.f);
        if (direction >= 0.0f) {
            velocity.x = -15.0f;
        }
        else if (direction < 0.0f) {
            velocity.x = 10.f;
        }
    }

    PE_Body_SetVelocity(body, velocity);
}

void ANut_VM_OnRespawn(void* self)
{
    ANut* AngryNut = Object_Cast(self, Class_ANut);
    AngryNut->m_state = ANGRYNUT_IDLE;

    GameBody_EnableBody(self);

    PE_Vec2 startPos = GameBody_GetStartPosition(self);
    PE_Body* body = GameBody_GetBody(self);
    PE_Body_SetPosition(body, startPos);
    PE_Body_SetVelocity(body, PE_Vec2_Zero);
    PE_Body_ClearForces(body);

    RE_Animator_StopAnimations(AngryNut->m_animator);
    RE_Animator_PlayAnimation(AngryNut->m_animator, "AngryIdle");
}

void ANut_VM_Render(void* self)
{
    ANut* Angrynut = Object_Cast(self, Class_ANut);
    Scene* scene = GameObject_GetScene(self);
    SDL_Renderer* renderer = Scene_GetRenderer(scene);
    Camera* camera = Scene_GetActiveCamera(scene);

    PE_Body* body = GameBody_GetBody(self);
    PE_Vec2 position = GameBody_GetPosition(self);

    float scale = Camera_GetWorldToViewScale(camera);
    SDL_FRect rect = { 0 };
    rect.h = 1.0f * scale; // Le sprite fait 1 tuile de haut
    rect.w = 1.0f * scale; // Le sprite fait 1 tuile de large
    Camera_WorldToView(camera, position, &rect.x, &rect.y);

    RE_Animator_RenderCopyF(
        Angrynut->m_animator, renderer, &rect, RE_ANCHOR_CENTER | RE_ANCHOR_BOTTOM
    );
}

void ANut_VM_Update(void* self)
{
    ANut* Angrynut = Object_Cast(self, Class_ANut);

    RE_Animator_Update(Angrynut->m_animator, g_time);
}