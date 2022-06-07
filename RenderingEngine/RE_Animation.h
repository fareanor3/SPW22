#ifndef _RE_ANIMATION_H_
#define _RE_ANIMATION_H_

#include "RE_Settings.h"
#include "RE_Utils.h"

typedef struct RE_Animator_s RE_Animator;

void Class_InitAnimation();

/// @brief Description de la classe "RE_Animation".
extern const void *const Class_Animation;

typedef struct AnimationClass_s {
    const Class base;

    // Messages
    void (*Update)(void *self, float dt);
} AnimationClass;

/// @ingroup RE_Animator
/// @brief Type d'une courbe d'acc�l�ration.
/// Ces fonctions sont d�finies sur [0,1] et v�rifient g�n�ralement f(0) = 0 et f(1) = 1.
typedef float (*RE_EasingFct)(float);

/// @ingroup RE_Animator
/// @brief Courbe d'acc�l�ration lin�aire.
/// @param[in] x ratio entre 0.f et 1.f.
/// @return Le ratio de progression entre 0.f et 1.f.
float RE_EasingFct_Linear(float x);

/// @ingroup RE_Animator
/// @brief Courbe d'acc�l�ration douce en entr�e.
/// @param[in] x ratio entre 0.f et 1.f.
/// @return Le ratio de progression entre 0.f et 1.f.
float RE_EasingFct_In(float x);

/// @ingroup RE_Animator
/// @brief Courbe d'acc�l�ration douce en sortie.
/// @param[in] x ratio entre 0.f et 1.f.
/// @return Le ratio de progression entre 0.f et 1.f.
float RE_EasingFct_Out(float x);

/// @ingroup RE_Animator
/// @brief Courbe d'acc�l�ration avec douce en entr�e et en sortie.
/// @param[in] x ratio entre 0.f et 1.f.
/// @return Le ratio de progression entre 0.f et 1.f.
float RE_EasingFct_InOut(float x);

/// @ingroup RE_Animator
/// @brief Courbe d'acc�l�ration d�finie � partir du cosinus.
/// @param[in] x ratio entre 0.f et 1.f.
/// @return Le ratio de progression entre 0.f et 1.f.
float RE_EasingFct_Cos(float x);

/// @ingroup RE_Animator
/// @brief Enum�ration contenant les flags pouvant �tre utilis�s
/// pour contr�ler l'animation d'une texture ou d'un param�tre.
typedef enum RE_AnimFlags_e
{
    /// @brief Met l'animation en pause.
    RE_ANIM_PAUSED = 1 << 1,
    /// @brief Joue l'animation dans le sens inverse.
    RE_ANIM_REVERSED = 1 << 2,
    /// @brief Joue l'animation en avant puis en arri�re.
    RE_ANIM_ALTERNATE = 1 << 3,
    /// @brief Arr�te automatiquement l'animation � la fin.
    RE_ANIM_STOP_AT_END = 1 << 4,
    RE_ANIM_STOPPED = 1 << 5
} RE_AnimFlags;

typedef struct RE_Animation_s
{
    struct {
        Object base;
    } m_super;

    char *m_name;

    RE_Animator *m_animator;

    /// @brief Courbe d'acc�l�ration de l'animation.
    RE_EasingFct m_easing;

    /// @brief Phase de l'animation.
    float m_phase;

    /// @brief Le coefficient multiplicateur de la vitesse de lecture.
    float m_speed;

    /// @brief La dur�e d'un cycle.
    float m_cycleTime;

    /// @brief D�lai avant le d�but de l'animation.
    float m_delay;

    /// @brief Accumulateur.
    float m_accu;

    /// @brief Accumulateur pour le d�lai.
    float m_delayAccu;

    /// @brief Les flags de l'animation.
    /// Les valeurs possibles des flags sont donn�es dans l'�num�ration RE_AnimFlags.
    int m_flags;

    /// @brief Nombre de cycles de l'animation.
    /// L'animation tourne en boucle si ce membre vaut -1.
    int m_cycleCount;

    /// @brief Indice du cycle courant. 
    int m_cycleIdx;
} RE_Animation;

void RE_Animation_Constructor(void *self, RE_Animator *animator, char *name);

/// @brief D�finit la dur�e d'un cycle de l'animation d'une texture.
/// @param[in,out] self l'animation.
/// @param[in] cycleTime la dur�e d'un cycle.
RE_INLINE void RE_Animation_SetCycleTime(void *self, float cycleTime)
{
    RE_Animation *anim = (RE_Animation *)Object_Cast(self, Class_Animation);
    anim->m_cycleTime = cycleTime;
}

/// @brief D�finit le nombre de cycle de l'animation d'une texture.
/// L'animation se r�p�te ind�finiment si le nombre de cycle vaut -1.
/// @param[in,out] self l'animation.
/// @param[in] cycleCount le nombre de cycle de l'animation.
RE_INLINE void RE_Animation_SetCycleCount(void *self, int cycleCount)
{
    RE_Animation *anim = (RE_Animation *)Object_Cast(self, Class_Animation);
    anim->m_cycleCount = cycleCount;
    anim->m_cycleIdx = 0;
    if (cycleCount == 0)
    {
        anim->m_flags |= RE_ANIM_PAUSED;
    }
}

/// @ingroup PRE_Animator
/// @brief D�finit le d�lai avant le d�but de l'animation d'une texture.
/// @param[in,out] self l'animation.
/// @param[in] delay le d�lai de l'animation.
RE_INLINE void RE_Animation_SetDelay(void *self, float delay)
{
    RE_Animation *anim = (RE_Animation *)Object_Cast(self, Class_Animation);
    anim->m_delay = delay;
}

/// @ingroup PRE_Animator
/// @brief D�finit la vitesse de lecture d'une texture anim�e. La valeur par d�faut est 1.f.
/// Si elle vaut par exemple 2.f, l'animation se d�roulera deux fois plus vite.
/// @param[in,out] self l'animation.
/// @param[in] speed coefficient multiplicateur de la vitesse de lecture.
RE_INLINE void RE_Animation_SetSpeed(void *self, float speed)
{
    RE_Animation *anim = (RE_Animation *)Object_Cast(self, Class_Animation);
    anim->m_speed = speed;
}

RE_INLINE void RE_Animation_SetPhase(void *self, float phase)
{
    RE_Animation *anim = (RE_Animation *)Object_Cast(self, Class_Animation);
    anim->m_phase = phase;
}

RE_INLINE void RE_Animation_SetEasing(void *self, RE_EasingFct easing)
{
    RE_Animation *anim = (RE_Animation *)Object_Cast(self, Class_Animation);
    anim->m_easing = easing;
}

/// @ingroup PRE_Animator
/// @brief Ajoute des flags � une texture anim�e.
/// Les valeurs possibles des flags sont donn�es dans l'�num�ration RE_AnimFlags.
/// @param[in,out] self l'animation.
/// @param[in] flags les flags � ajouter.
RE_INLINE void RE_Animation_AddFlags(void *self, int flags)
{
    RE_Animation *anim = (RE_Animation *)Object_Cast(self, Class_Animation);
    anim->m_flags |= flags;
}

/// @ingroup PRE_Animator
/// @brief Enl�ve des flags � une texture anim�e.
/// Les valeurs possibles des flags sont donn�es dans l'�num�ration RE_AnimFlags.
/// @param[in,out] self l'animation.
/// @param[in] flags les flags � enlever.
RE_INLINE void RE_Animation_RemoveFlags(void *self, int flags)
{
    RE_Animation *anim = (RE_Animation *)Object_Cast(self, Class_Animation);
    anim->m_flags &= ~flags;
}

/// @brief Progression avant courbe d'acc�l�ration et flags
/// @param self 
/// @return 
float RE_Animation_GetRawProgression(void *self);
float RE_Animation_GetProgression(void *self);

RE_INLINE RE_Animator *RE_Animation_GetAnimator(void *self)
{
    RE_Animation *anim = (RE_Animation *)Object_Cast(self, Class_Animation);
    return anim->m_animator;
}

void RE_Animation_OnPlay(void *self);
void RE_Animation_OnResume(void *self);
void RE_Animation_OnStop(void *self);

RE_INLINE void RE_Animation_Update(void *self, float dt)
{
    assert(Object_IsA(self, Class_Animation));
    const AnimationClass *clsDesc = (AnimationClass *)Object_GetClass(self);
    (clsDesc->Update)(self, dt);
}

RE_INLINE bool RE_Animation_IsStopped(void *self)
{
    RE_Animation *anim = (RE_Animation *)Object_Cast(self, Class_Animation);
    return (anim->m_flags & RE_ANIM_STOPPED) == RE_ANIM_STOPPED;
}
RE_INLINE char *RE_Animation_GetName(void *self)
{
    RE_Animation *anim = (RE_Animation *)Object_Cast(self, Class_Animation);
    return anim->m_name;
}

#endif