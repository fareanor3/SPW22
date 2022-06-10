#ifndef _ANUT_H_
#define _ANUT_H_

#include "Enemy.h"

void Class_InitANut();

/// @brief Description de la classe "Nut".
extern const void *const Class_ANut;

typedef struct ANutClass_s {
    const EnemyClass base;
} ANutClass;

typedef enum ANutState_e
{
    ANGRYNUT_IDLE,
    ANGRYNUT_SPINNING,
    ANGRYNUT_DYING
} ANutState;

typedef struct ANut_s
{
    struct {
        Enemy base;
    } m_super;

    RE_Animator *m_animator;

    int m_state;

} ANut;

void AngryNut_Constructor(void *self, void *scene, PE_Vec2 startPos);

#endif
