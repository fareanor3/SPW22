#include "MainCamera.h"
#include "../../Scene/LevelScene.h"
#include "../../Utils/Renderer.h"

// Object virtual methods
void MainCamera_VM_Destructor(void *self);

// GameObject virtual methods
void MainCamera_VM_OnRespawn(void *self);
void MainCamera_VM_Render(void *self);
void MainCamera_VM_Update(void *self);

static MainCameraClass _Class_MainCamera = { 0 };
const void *const Class_MainCamera = &_Class_MainCamera;

void Class_InitMainCamera()
{
    if (!Class_IsInitialized(Class_MainCamera))
    {
        Class_InitCamera();

        void *self = (void *)Class_MainCamera;
        ClassCtorParams params = {
            .self = self,
            .super = Class_Camera,
            .name = "MainCamera",
            .instanceSize = sizeof(MainCamera),
            .classSize = sizeof(MainCameraClass)
        };
        Class_Constructor(params, MainCamera_VM_Destructor);
        ((GameObjectClass *)self)->Render = MainCamera_VM_Render;
        ((GameObjectClass *)self)->Update = MainCamera_VM_Update;
        ((GameObjectClass *)self)->OnRespawn = MainCamera_VM_OnRespawn;
    }
}

void MainCamera_Constructor(void *self, void *scene, int width, int height)
{
    Camera_Constructor(self, scene, width, height);
    Object_SetClass(self, Class_MainCamera);

    MainCamera *mainCam = Object_Cast(self, Class_MainCamera);

    Camera_SetWorldView(mainCam, PE_AABB_Set(0.0f, 0.0f, 24.0f, 13.5f));
    Camera_SetWorldBounds(mainCam, PE_AABB_Set(0.0f, 0.0f, 1000.0f, 1000.0f));
}

void MainCamera_VM_Destructor(void *self)
{
    // Destructeur de la classe m�re
    Object_SuperDestroy(self, Class_MainCamera);
}

void MainCamera_VM_OnRespawn(void *self)
{
    MainCamera *mainCam = Object_Cast(self, Class_MainCamera);
    Scene *scene = GameObject_GetScene(self);
    Player *player = LevelScene_GetPlayer(scene);

    mainCam->m_smoothedTarget = LevelScene_GetStartPosition(scene);
    mainCam->m_centerVelocity = PE_Vec2_Zero;

    Camera_SetWorldView(mainCam, PE_AABB_Set(0.0f, 0.0f, 24.0f, 13.5f));
}

void MainCamera_VM_Render(void *self)
{
    MainCamera *mainCam = Object_Cast(self, Class_MainCamera);
    Scene *scene = GameObject_GetScene(self);
    SDL_Renderer *renderer = scene->m_renderer;
    Camera *sceneCam = Scene_GetActiveCamera(scene);

    // Permet de voir o� est la cam�ra principale
    // lorsqu'on utilise la cam�ra de d�bug
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    PE_AABB worldView = Camera_GetWorldView(mainCam);
    Renderer_DrawAABB(renderer, sceneCam, &worldView);
}

void MainCamera_VM_Update(void *self)
{
    MainCamera *mainCam = Object_Cast(self, Class_MainCamera);
    Scene *scene = GameObject_GetScene(self);
    Player *player = LevelScene_GetPlayer(scene);

    // D�finit la cible (position souhait�e)
    PE_Vec2 target = GameBody_GetPosition(player);

    // La fonction SmoothDamp() lisse le d�plancement de la cam�ra
    mainCam->m_smoothedTarget = PE_Vec2_SmoothDamp(
        // Position courante
        mainCam->m_smoothedTarget,
        // Position souhait�e
        target,
        // Vitesse courante (vecteur modifi� par la fonction)
        &(mainCam->m_centerVelocity),
        // Approximativement le temps n�cessaire pour atteindre la position souhait�e
        0.15f,
        // Vitesse maximale autoris�e
        1000.0f,
        // Temps �coul� depuis le dernier appel
        RE_Timer_GetDelta(g_time)
    );

    // Calcule la position du coin inf�rieur gauche visible par la cam�ra
    // On voit ici :
    // * 10 tuiles � gauche du joueur
    // * 6.75 tuiles en dessous du joueur
    PE_Vec2 lower = PE_Vec2_Sub(mainCam->m_smoothedTarget, PE_Vec2_Set(10.0f, 6.75f));

    // Fixe la position verticale minimale de la cam�ra pour ne pas voir sous le sol
    lower.y = fmaxf(lower.y, 0.0f);

    // D�place la cam�ra vers la nouvelle position
    PE_AABB worldView = Camera_GetWorldView(mainCam);
    PE_Vec2 displacement = PE_Vec2_Sub(lower, worldView.lower);
    Camera_TranslateWorldView(mainCam, displacement);

    // Rend la cam�ra visible en debug
    Camera *activeCam = Scene_GetActiveCamera(scene);
    if (activeCam != self)
    {
        Scene_SetVisible(scene, self);
    }
}
