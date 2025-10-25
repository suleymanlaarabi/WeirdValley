#ifndef CONTROLLER_H
    #define CONTROLLER_H
    #include <flecs.h>


typedef struct {
    int key_up;
    int key_down;
    int key_right;
    int key_left;
    float speed;
} CharacterController;

extern ECS_COMPONENT_DECLARE(CharacterController);

void ControllerImport(ecs_world_t *world);

#endif
