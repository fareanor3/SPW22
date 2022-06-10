#include "Player.h"
#include "../Scene/LevelScene.h"
#include "Camera/MainCamera.h"
#include "../Utils/Renderer.h"
#include "Enemy/Enemy.h"
#include "Collectable/Collectable.h"

// Object virtual methods
void Player_VM_Destructor(void *self);

// GameObject virtual methods
void Player_VM_DrawGizmos(void *self);
void Player_VM_FixedUpdate(void *self);
void Player_VM_OnRespawn(void *self);
void Player_VM_Render(void *self);
void Player_VM_Start(void *self);
void Player_VM_Update(void *self);

static PlayerClass _Class_Player = { 0 };
const void *const Class_Player = &_Class_Player;

void Class_InitPlayer()
{
    if (!Class_IsInitialized(Class_Player))
    {
        Class_InitGameBody();

        void *self = (void *)Class_Player;
        ClassCtorParams params = {
            .self = self,
            .super = Class_GameBody,
            .name = "Player",
            .instanceSize = sizeof(Player),
            .classSize = sizeof(PlayerClass)
        };
        Class_Constructor(params, Player_VM_Destructor);
        ((GameObjectClass *)self)->DrawGizmos = Player_VM_DrawGizmos;
        ((GameObjectClass *)self)->FixedUpdate = Player_VM_FixedUpdate;
        ((GameObjectClass *)self)->OnRespawn = Player_VM_OnRespawn;
        ((GameObjectClass *)self)->Render = Player_VM_Render;
        ((GameObjectClass *)self)->Start = Player_VM_Start;
        ((GameObjectClass *)self)->Update = Player_VM_Update;
    }
}

void Player_OnCollisionEnter(PE_Collision *collision)
{
    PE_Manifold manifold = PE_Collision_GetManifold(collision);
    PE_Body *thisBody = PE_Collision_GetBody(collision);
    PE_Body *otherBody = PE_Collision_GetOtherBody(collision);
    PE_Collider *otherCollider = PE_Collision_GetOtherCollider(collision);

    Player *player = (Player *)GameBody_GetFromBody(thisBody);
    GameBody *otherGameBody = GameBody_GetFromBody(otherBody);

    // Collision avec un ennemi
    if (PE_Collider_CheckCategory(otherCollider, FILTER_ENEMY))
    {
        Enemy *enemy = Object_Cast(otherGameBody, Class_Enemy);

        // Calcule l'angle entre la normale de contact et le vecteur "UP"
        // L'angle vaut :
        // * 0 si le joueur est parfaitement au dessus de l'ennemi,
        // * 90 s'il est à gauche ou à droite
        // * 180 s'il est en dessous
        float angleUp = PE_Vec2_AngleDeg(manifold.normal, PE_Vec2_Up);
        if (angleUp < 55.0f)
        {
            Enemy_Damage(enemy, player);
        }
        return;
    }

    // Collision avec un objet
    if (PE_Collider_CheckCategory(otherCollider, FILTER_COLLECTABLE))
    {
        Collectable *collectable = Object_Cast(otherGameBody, Class_Collectable);

        // Collecte l'objet
        // C'est ensuite l'objet qui affecte un bonus au joueur,
        // en appellant Player_AddFirefly() par exemple.
        Collectable_Collect(collectable, player);
    }
}

void Player_OnCollisionStay(PE_Collision *collision)
{
    PE_Manifold manifold = PE_Collision_GetManifold(collision);
    PE_Body *thisBody = PE_Collision_GetBody(collision);
    PE_Body *otherBody = PE_Collision_GetOtherBody(collision);
    PE_Collider *otherCollider = PE_Collision_GetOtherCollider(collision);
    Player *player = (Player *)GameBody_GetFromBody(thisBody);
	
    if (player->m_state == PLAYER_DYING)
    {
        PE_Collision_SetEnabled(collision, false);
        return;
    }
	
    if (PE_Collider_CheckCategory(otherCollider, FILTER_COLLECTABLE))
    {
        // Désactive la collision avec un objet
        // Evite d'arrêter le joueur quand il prend un coeur
        PE_Collision_SetEnabled(collision, false);
        return;
    }
    else if (PE_Collider_CheckCategory(otherCollider, FILTER_TERRAIN))
    {
        float angleUp = PE_Vec2_AngleDeg(manifold.normal, PE_Vec2_Up);

        if (angleUp <= 55.0f)
        {
            // Résoud la collision en déplaçant le joueur vers le haut
            // Evite de "glisser" sur les pentes si le joueur ne bouge pas
            PE_Collision_ResolveUp(collision);
        }
    }
}

void Player_CreateAnimator(Player *player, void *scene)
{
    AssetManager *assets = Scene_GetAssetManager(scene);
    RE_Atlas *atlas = AssetManager_GetPlayerAtlas(assets);
    RE_AtlasPart *part = NULL;
    void *anim = NULL;

    // Crée l'animateur
    RE_Animator *animator = RE_Animator_New();
    AssertNew(animator);

    player->m_animator = animator;

    // Animation "Falling"
    part = RE_Atlas_GetPart(atlas, "Falling");
    AssertNew(part);

    anim = RE_Animator_CreateTextureAnim(animator, "Falling", part);
    AssertNew(anim);
    RE_Animation_SetCycleCount(anim, -1);
    RE_Animation_SetCycleTime(anim, 0.2f);

    // Animation "Idle"
    part = RE_Atlas_GetPart(atlas, "Idle");
    AssertNew(part);

    anim = RE_Animator_CreateTextureAnim(animator, "Idle", part);
    AssertNew(anim);
    RE_Animation_SetCycleCount(anim, 0);
    RE_Animation_SetCycleTime(anim, 0.2f);

    // Animation "Run"
    part = RE_Atlas_GetPart(atlas, "Running");
    AssertNew(part);

    anim = RE_Animator_CreateTextureAnim(animator, "Running", part);
    AssertNew(anim);
    RE_Animation_SetCycleCount(anim, -1);
    RE_Animation_SetCycleTime(anim, 0.3f);

    // Animation "Skidding"
    part = RE_Atlas_GetPart(atlas, "Skidding");
    AssertNew(part);

    anim = RE_Animator_CreateTextureAnim(animator, "Skidding", part);
    AssertNew(anim);
    RE_Animation_SetCycleCount(anim, 0);
	
    // Animation "Dying"
    part = RE_Atlas_GetPart(atlas, "Dying");
    AssertNew(part);

    anim = RE_Animator_CreateTextureAnim(animator, "Dying", part);
    AssertNew(anim);
    RE_Animation_SetCycleCount(anim, 0);

	// Animation "landing down"
    Vec2 vectDown1 = { .x = 1.0f, . y = 1.0f };
    Vec2 vectDown2 = { .x = 1.1f, . y = 1.1f };
    anim = RE_Animator_CreateScaleAnim(animator, "Landing Down", vectDown1, vectDown2);
    AssertNew(anim);
    RE_Animation_SetCycleCount(anim, 1);
	RE_Animation_SetCycleTime(anim, 0.2f);
    RE_Animation_AddFlags(anim, RE_ANIM_ALTERNATE);

	// Animation "Landing Up"
	Vec2 vectUp1 = { .x = 1.0f, . y = 1.0f };
	Vec2 vectUp2 = { .x = 0.9f, . y = 0.9f };
	anim = RE_Animator_CreateScaleAnim(animator, "Landing Up", vectUp1, vectUp2);
	AssertNew(anim);
	RE_Animation_SetCycleCount(anim, 1);
	RE_Animation_SetCycleTime(anim, 0.2f);
	RE_Animation_AddFlags(anim, RE_ANIM_ALTERNATE);

    // Animation "Invincible"
    anim = RE_Animator_CreateAlphaAnim(animator, "Invincible", 0.3f, 2.0f);
    AssertNew(anim);
    RE_Animation_SetCycleCount(anim, 20);
    RE_Animation_SetCycleTime(anim, 0.2f);
}

void Player_Constructor(void *self, void *scene)
{
    GameBody_Constructor(self, scene, LAYER_PLAYER);
    Object_SetClass(self, Class_Player);

    Player *player = Object_Cast(self, Class_Player);

    player->m_state = PLAYER_IDLE;
    player->m_hDirection = 0.0f;
    player->m_facingRight = true;
    player->m_jump = false;
    player->m_JumpTime = 0.0f;

    player->m_lifeCount = 5;
    player->m_fireflyCount = 0;
    player->m_heartCount = 2;

    // Le joueur doit être réinitialliser à chaque fois qu'il meurt
    Scene_SetToRespawn(scene, player, true);

    Player_CreateAnimator(player, scene);
}

void Player_VM_Start(void *self)
{
    Player *player = Object_Cast(self, Class_Player);
    Scene *scene = GameObject_GetScene(self);

    LevelScene_SetStartPosition(scene, GameBody_GetStartPosition(player));

    PE_World *world = Scene_GetWorld(scene);
    PE_Body *body = NULL;
    PE_BodyDef bodyDef = { 0 };
    PE_ColliderDef colliderDef = { 0 };
    PE_Collider *collider = NULL;

    // Crée le corps
    PE_BodyDef_SetDefault(&bodyDef);
    bodyDef.type = PE_DYNAMIC_BODY;
    bodyDef.position = LevelScene_GetStartPosition(scene);
    bodyDef.name = "Player";
    bodyDef.xDamping = 0.0f;
    bodyDef.yDamping = 0.0f;
    bodyDef.mass = 1.0f;

    body = PE_World_CreateBody(world, &bodyDef);
    AssertNew(body);
    GameBody_SetBody(player, body);

    // Crée le collider
    PE_ColliderDef_SetDefault(&colliderDef);
    colliderDef.friction = 1.0f;
    colliderDef.filter.categoryBits = FILTER_PLAYER;
    PE_Shape_SetAsCapsule(
        &colliderDef.shape,
        PE_Vec2_Set(0.0f, 0.25),
        PE_Vec2_Set(0.0f, 0.9f),
        0.3f
    );

    collider = PE_Body_CreateCollider(body, &colliderDef);
    AssertNew(collider);

    // Définit les callbacks de collisions
    PE_Collider_SetOnCollisionEnter(collider, Player_OnCollisionEnter);
    PE_Collider_SetOnCollisionStay(collider, Player_OnCollisionStay);

    // Joue l'animation par défaut
    RE_Animator_PlayAnimation(player->m_animator, "Idle");
}

void Player_Damage(Player* player)
{
    // Méthode appellée par un ennemi qui touche le joueur
    player->m_heartCount--;
    if (player->m_heartCount <= 0)
    {
        player->m_state = PLAYER_DYING;
    }
    player->unstoppable = true;
    RE_Animator_PlayAnimation(player->m_animator, "Invincible");
    return;
}

void Player_Kill(Player* player)
{
    Scene* scene = GameObject_GetScene((GameObject*)player);
    player->m_lifeCount--;
    if (player->m_lifeCount <= 0)
    {
        Scene_Quit(scene);
    }
    Scene_Respawn(scene);
}

void Player_AddFirefly(void *self)
{
    Player *player = Object_Cast(self, Class_Player);
    player->m_fireflyCount++;
}

void Player_SubFirefly(void* self)
{
    Player* player = Object_Cast(self, Class_Player);
    player->m_fireflyCount--;
}

void Player_AddHeart(void *self)
{
    Player *player = Object_Cast(self, Class_Player);
    if (player->m_heartCount < 2)
	{
		player->m_heartCount++;
	}
}

static bool Player_WakeUpCallback(PE_Collider *collider, void *data)
{
    PE_Body *body = PE_Collider_GetBody(collider);
    PE_Body_SetAwake(body, true);

    return true;
}

void Player_WakeUpSurroundings(Player *player)
{
    // Fonction utilisée pour réveiller les corps autour du joueur.
    // Cela permet d'optimiser le jeu pour ne simuler que les corps
    // proche du joueur.
    PE_Body *body = GameBody_GetBody((GameBody *)player);
    PE_Vec2 position = PE_Body_GetPosition(body);

    Scene *scene = GameObject_GetScene((GameObject *)player);
    PE_World *world = Scene_GetWorld(scene);
    PE_AABB aabb = PE_AABB_Set(
        position.x - 20.0f, position.y - 10.0f,
        position.x + 20.0f, position.y + 10.0f
    );

    PE_World_OverlapAreaExpert(world, &aabb, Player_WakeUpCallback, NULL);
}

void Player_VM_FixedUpdate(void *self)
{
    Player *player = Object_Cast(self, Class_Player);
    Scene *scene = GameObject_GetScene(self);

    InputManager* inputManager = Scene_GetInputManager(scene);
    ControlsInput* controls = InputManager_GetControls(inputManager);
	
    PE_World* world = Scene_GetWorld(scene);
    PE_Body* body = GameBody_GetBody(self);
    PE_Vec2 velocity = PE_Body_GetLocalVelocity(body);
    PE_Vec2 position = PE_Body_GetPosition(body);

    // Réveille les corps autour du joueur
    Player_WakeUpSurroundings(player);

    // Tue le joueur s'il tombe dans un trou
    if (position.y < -2.0f)
    {
        Player_Kill(player);
        return;
    }
    if (player->m_state == PLAYER_DYING)
    {
        RE_Animator_StopAnimations(player->m_animator);
        RE_Animator_PlayAnimation(player->m_animator, "Dying");
        return;
    }
    //--------------------------------------------------------------------------
    // Détection du sol

    bool onGround = false;
    PE_Vec2 gndNormal = PE_Vec2_Up;

    // Lance deux rayons vers le bas ayant pour origines
    // les coins gauche et droit du bas du collider du joueur
    // Ces deux rayons sont dessinés en jaune dans DrawGizmos()
    PE_Vec2 originL = PE_Vec2_Add(position, PE_Vec2_Set(-0.15f, 0.0f));
    PE_Vec2 originR = PE_Vec2_Add(position, PE_Vec2_Set(+0.15f, 0.0f));

    // Les rayons ne touchent que des colliders solides (non trigger)
    // ayant la catégorie FILTER_TERRAIN
    PE_RayCastHit hitL = PE_World_RayCast(
        world, originL, PE_Vec2_Down, 0.1f, FILTER_TERRAIN, true
    );
    PE_RayCastHit hitR = PE_World_RayCast(
        world, originR, PE_Vec2_Down, 0.1f, FILTER_TERRAIN, true
    );

    if (hitL.collider != NULL)
    {
        // Le rayon gauche à touché le sol
        onGround = true;
        gndNormal = hitL.normal;
    }
    if (hitR.collider != NULL)
    {
        // Le rayon droit à touché le sol
        onGround = true;
        gndNormal = hitR.normal;
    }

    //--------------------------------------------------------------------------
    // Etat du joueur

    // Détermine l'état du joueur et change l'animation si nécessaire
    if (onGround)
    {
        if (velocity.x != 0.0f) {
            if ((velocity.x > 0.0f && player->m_hDirection > 0.0f) || (velocity.x < 0.0f && player->m_hDirection < 0.0f)) {
                if (player->m_state != PLAYER_RUNNING) {
                    player->m_state = PLAYER_RUNNING;
                    RE_Animator_PlayAnimation(player->m_animator, "Running");
                }
            }
            else if (player->m_state == PLAYER_RUNNING) {
                player->m_state = PLAYER_SKIDDING;
                RE_Animator_PlayAnimation(player->m_animator, "Skidding");
            }
        }
        else if (player->m_state != PLAYER_IDLE) {
			player->m_state = PLAYER_IDLE;
			RE_Animator_PlayAnimation(player->m_animator, "Idle");
		}
    }
    else
    {
        if (player->m_state != PLAYER_FALLING)
        {
            player->m_state = PLAYER_FALLING;
            RE_Animator_PlayAnimation(player->m_animator, "Falling");
        }
        RE_Animator_PlayAnimation(player->m_animator, "Landing Down");
    }

    // Orientation du joueur
    // Utilisez player->m_hDirection qui vaut :
    // *  0.0f si le joueur n'accélère pas ;
    // * +1.0f si le joueur accélère vers la droite ;
    // * -1.0f si le joueur accélère vers la gauche.
    player->m_facingRight = true;

    // s'orriende la direction gauche quand on touche a fleche
	if (player->m_hDirection < 0.0f)
	{
		player->m_facingRight = false;
	}
    //--------------------------------------------------------------------------
    // Modification de la vitesse et application des forces

    // Application des forces
    // Définit la force d'accélération horizontale du joueur
    PE_Vec2 force = PE_Vec2_Scale(PE_Vec2_Right, 15.0f * player->m_hDirection);
    PE_Body_ApplyForce(body, force);

    // Limite la vitesse horizontale
    float maxHSpeed = 9.f;
    velocity.x = Float_Clamp(velocity.x, -maxHSpeed, maxHSpeed);

    // Saut
	
	bool jumpflag = false;
    float MaxTimeFlyInput = 0.1f;
	
    if (!onGround ) {
        player->m_JumpTime += RE_Timer_GetUnscaledDelta(g_time);
        if (player->m_JumpNumber > 2) {
            if (player->m_jump) {
                velocity.y = 15.0f;
                RE_Animator_PlayAnimation(player->m_animator, "Landing Up");
                player->m_jump = false;
            }
		}
    }
    if (onGround) {
 
		player->m_JumpNumber = 0;
        if (player->m_JumpTime < MaxTimeFlyInput && player-> m_jump) {
            velocity.y = 15.0f;
            RE_Animator_PlayAnimation(player->m_animator, "Landing Up");
			player->m_JumpNumber++;
			player->m_JumpTime = 0.0f;
        }
        if (player->m_jump) {
            velocity.y = 15.0f;
            RE_Animator_PlayAnimation(player->m_animator, "Landing Up");
            player->m_JumpNumber++;    
        }
    }
    player->m_jump = false;

	// saut modéré
    if (controls->jumpDown ) {
        PE_Body_SetGravityScale(body, 0.6f);
    }
    else {
        PE_Body_SetGravityScale(body, 1.0f);
    }
		
    // Rebond sur les ennemis
    if (player->m_bounce)
    {
        velocity.y = 15.0f;
        RE_Animator_PlayAnimation(player->m_animator, "Landing Up");
        player->m_bounce = false;
    }

    // Remarques :
    // Le facteur de gravité peut être modifié avec l'instruction
    // -> PE_Body_SetGravityScale(body, 1.0f);
    // pour faire des sauts de hauteurs différentes.
    // La physique peut être différente si le joueur touche ou non le sol.

    PE_Body_SetVelocity(body, velocity);
}

void Player_VM_OnRespawn(void *self)
{
    Player *player = Object_Cast(self, Class_Player);
    PE_Body *body = GameBody_GetBody(self);
    Scene *scene = GameObject_GetScene(self);

    AssertNew(body);
    PE_Body_SetPosition(body, LevelScene_GetStartPosition(scene));
    PE_Body_SetVelocity(body, PE_Vec2_Zero);

    player->m_heartCount = 2;
    player->m_state = PLAYER_IDLE;
    player->m_hDirection = 0.0f;

    player->m_facingRight = true;
    player->m_bounce = false;
    player->m_jump = false;

    RE_Animator_StopAnimations(player->m_animator);
    RE_Animator_PlayAnimation(player->m_animator, "Idle");
}

void Player_VM_Render(void *self)
{
    Player *player = Object_Cast(self, Class_Player);
    Scene *scene = GameObject_GetScene(self);
    SDL_Renderer *renderer = Scene_GetRenderer(scene);
    Camera *camera = Scene_GetActiveCamera(scene);
    PE_Vec2 position = GameBody_GetPosition(self);
    
    int flip = player->m_facingRight ? 0 : SDL_FLIP_HORIZONTAL;

    SDL_FRect rect = { 0 };
    Camera_WorldToView(camera, position, &rect.x, &rect.y);
    float scale = Camera_GetWorldToViewScale(camera);
    rect.h = 1.375f * scale; // Le sprite fait 1.375 tuile de haut
    rect.w = 1.000f * scale; // Le sprite fait 1 tuile de large

    // Dessine l'animateur du joueur
    Vec2 center = Vec2_Set(0.5f, 0.5f);
    RE_Animator_RenderCopyExF(
        player->m_animator, renderer, &rect, RE_ANCHOR_CENTER | RE_ANCHOR_BOTTOM,
        0.0f, center, flip
    );
}

void Player_VM_Destructor(void *self)
{
    Player *player = Object_Cast(self, Class_Player);

    RE_Animator_Delete(player->m_animator);

    // Destructeur de la classe mère
    Object_SuperDestroy(self, Class_Player);
}

void Player_VM_DrawGizmos(void *self)
{
    Player *player = Object_Cast(self, Class_Player);
    Scene *scene = GameObject_GetScene(self);
    SDL_Renderer *renderer = Scene_GetRenderer(scene);
    Camera *camera = Scene_GetActiveCamera(scene);

    PE_Vec2 position = GameBody_GetPosition(player);
    PE_Vec2 velocity = GameBody_GetVelocity(player);

    // Dessine en blanc le vecteur vitesse du joueur
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    Renderer_DrawVector(renderer, camera, position, velocity);

    // Dessine en jaune les rayons pour la détection du sol
    PE_Vec2 originL = PE_Vec2_Add(position, PE_Vec2_Set(-0.15f, 0.0f));
    PE_Vec2 originR = PE_Vec2_Add(position, PE_Vec2_Set(+0.15f, 0.0f));
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    Renderer_DrawVector(renderer, camera, originL, PE_Vec2_Set(0.0f, -0.1f));
    Renderer_DrawVector(renderer, camera, originR, PE_Vec2_Set(0.0f, -0.1f));
}

void Player_VM_Update(void *self)
{
    Player *player = Object_Cast(self, Class_Player);
    Scene *scene = GameObject_GetScene(self);
    InputManager *inputManager = Scene_GetInputManager(scene);
    ControlsInput *controls = InputManager_GetControls(inputManager);

    // Met à jour les animations du joueur
    RE_Animator_Update(player->m_animator, g_time);
    if (player->unstoppable)
    {
        player->invincible_T += RE_Timer_GetUnscaledDelta(g_time);
        if (player->invincible_T > 3.0f)
        {
            player->unstoppable = false;
            player->invincible_T = 0;
            RE_Animator_StopAnimation(player->m_animator, "Invincible");
        }
    }
    // Sauvegarde les contrôles du joueur pour modifier
    // sa physique au prochain FixedUpdate()
        if (controls->jumpPressed){
            player->m_jump = true;
        }
    player->m_hDirection = controls->hAxis;
}