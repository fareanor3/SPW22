#ifndef _HEART_H_
#define _HEART_H_

#include "Collectable.h"

void Class_InitHeart();

/// @brief Description de la classe "Heart".
extern const void *const Class_Heart;

typedef struct HeartClass_s {
    const CollectableClass base;
} HeartClass;

typedef enum HeartState_e
{
    HEART_IDLE,
    HEART_SPINNING,
    HEART_DYING
} HeartState;

typedef struct Heart_s
{
    struct {
        Collectable base;
    } m_super;

    RE_Animator *m_animator;

    bool m_bonus;
    int m_state;
    float m_speed;
	
} Heart;

void Heart_Constructor(void *self, void *scene, PE_Vec2 startPos);

#endif
