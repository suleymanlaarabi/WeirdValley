#ifndef PLAYER_H
    #define PLAYER_H
    #include <flecs.h>


extern ECS_TAG_DECLARE(Player);

void PlayerImport(ecs_world_t *wolrd);

#endif
