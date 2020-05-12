#include "levels.h"

#define LEVEL_ONE_BALL_SPEED 1
#define LEVEL_TWO_BALL_SPEED 1
#define LEVEL_THREE_BALL_SPEED 2
#define LEVEL_FOUR_BALL_SPEED 3
#define LEVEL_FIVE_BALL_SPEED 4

#define LEVEL_ONE_BRICKS_SPEED 1
#define LEVEL_TWO_BRICKS_SPEED 1
#define LEVEL_THREE_BRICKS_SPEED 2
#define LEVEL_FOUR_BRICKS_SPEED 3
#define LEVEL_FIVE_BRICKS_SPEED 4

void configLevelOne(Level *level, Configs configs) {
	level->num_bricks = 10;

	int normal_bricks = 6;
	int resistant_bricks = 2;
	int magic_bricks = 2;

	int i;

	int normal_spacing = 0;
	int resistant_spacing = 0;
	int magic_spacing = 0;

	int spacing_between_bricks = 3;

	int initial_x = 5;

	level->ball_speed = LEVEL_ONE_BALL_SPEED;
	level->bricks_speed = LEVEL_ONE_BRICKS_SPEED;

	for (i = 0; i < normal_bricks; i++) {   // TIJOLOS NORMAIS
		level->bricks[i].type = BRICK_TYPE_NORMAL;
		level->bricks[i].points = configs.brick_normal_points;
		level->bricks[i].width = configs.brick_normal_width;
		level->bricks[i].lifes = configs.brick_normal_max_lives;
		level->bricks[i].coords.x = normal_spacing * (level->bricks[i].width + spacing_between_bricks) + initial_x;
		level->bricks[i].coords.y = 6;

		normal_spacing++;

	}

	for (; i < normal_bricks + resistant_bricks; i++) {      //TIJOLOS RESISTENTES

		level->bricks[i].type = BRICK_TYPE_RESISTANT;
		level->bricks[i].points = configs.brick_resistant_points;
		level->bricks[i].width = configs.brick_resistant_width;
		level->bricks[i].lifes = configs.brick_resistant_max_lives;
		level->bricks[i].coords.x = resistant_spacing * (level->bricks[i].width + spacing_between_bricks) + initial_x;
		level->bricks[i].coords.y = level->bricks[i].coords.y = 4;

		resistant_spacing++;

	}

	for (; i < level->num_bricks; i++) {
		level->bricks[i].type = BRICK_TYPE_MAGIC;
		level->bricks[i].points = configs.brick_magic_points;
		level->bricks[i].width = configs.brick_magic_width;
		level->bricks[i].lifes = configs.brick_magic_max_lives;
		level->bricks[i].coords.x = magic_spacing * (level->bricks[i].width + spacing_between_bricks) + initial_x;
		level->bricks[i].coords.y = level->bricks[i].coords.y = 2;

		magic_spacing++;
	}

}

void configLevelTwo(Level *level, Configs configs) {
	level->num_bricks = 12;

	int normal_bricks = 6;
	int resistant_bricks = 4;
	int magic_bricks = 2;

	int initial_x = 5;

	int i;
	int normal_spacing = 2;
	int resistant_spacing = 2;
	int magic_spacing = 2;

	int spacing_between_bricks = 5;

	level->ball_speed = LEVEL_TWO_BALL_SPEED;
	level->bricks_speed = LEVEL_TWO_BRICKS_SPEED;

	for (i = 0; i < normal_bricks; i++) {   // TIJOLOS NORMAIS
		level->bricks[i].type = BRICK_TYPE_NORMAL;
		level->bricks[i].points = configs.brick_normal_points;
		level->bricks[i].width = configs.brick_normal_width;
		level->bricks[i].lifes = configs.brick_normal_max_lives;
		level->bricks[i].coords.x = normal_spacing * (level->bricks[i].width + spacing_between_bricks) + initial_x;
		level->bricks[i].coords.y = 6;

		normal_spacing++;

	}

	for (; i < normal_bricks + resistant_bricks; i++) {      //TIJOLOS RESISTENTES

		level->bricks[i].type = BRICK_TYPE_RESISTANT;
		level->bricks[i].points = configs.brick_resistant_points;
		level->bricks[i].width = configs.brick_resistant_width;
		level->bricks[i].lifes = configs.brick_resistant_max_lives;
		level->bricks[i].coords.x = resistant_spacing * (level->bricks[i].width + spacing_between_bricks) + initial_x;
		level->bricks[i].coords.y = level->bricks[i].coords.y = 4;

		resistant_spacing++;

	}

	for (; i < level->num_bricks; i++) {
		level->bricks[i].type = BRICK_TYPE_MAGIC;
		level->bricks[i].points = configs.brick_magic_points;
		level->bricks[i].width = configs.brick_magic_width;
		level->bricks[i].lifes = configs.brick_magic_max_lives;
		level->bricks[i].coords.x = magic_spacing * (level->bricks[i].width + spacing_between_bricks) + initial_x;
		level->bricks[i].coords.y = level->bricks[i].coords.y = 2;

		magic_spacing++;
	}

}


void configLevelThree(Level *level, Configs configs) {
	level->num_bricks = 14;

	int normal_bricks = 8;
	int resistant_bricks = 2;
	int magic_bricks = 4;

	int initial_x = 6;

	int i;

	int normal_spacing = 0;
	int resistant_spacing = 0;
	int magic_spacing = 0;

	int spacing_between_bricks = 5;

	level->ball_speed = LEVEL_THREE_BALL_SPEED;
	level->bricks_speed = LEVEL_THREE_BRICKS_SPEED;

	for (i = 0; i < normal_bricks; i++) {   // TIJOLOS NORMAIS
		level->bricks[i].type = BRICK_TYPE_NORMAL;
		level->bricks[i].points = configs.brick_normal_points;
		level->bricks[i].width = configs.brick_normal_width;
		level->bricks[i].lifes = configs.brick_normal_max_lives;
		level->bricks[i].coords.x = normal_spacing * (level->bricks[i].width + spacing_between_bricks) + initial_x;
		level->bricks[i].coords.y = 6;

		normal_spacing++;

	}

	for (; i < normal_bricks + resistant_bricks; i++) {      //TIJOLOS RESISTENTES

		level->bricks[i].type = BRICK_TYPE_RESISTANT;
		level->bricks[i].points = configs.brick_resistant_points;
		level->bricks[i].width = configs.brick_resistant_width;
		level->bricks[i].lifes = configs.brick_resistant_max_lives;
		level->bricks[i].coords.x = resistant_spacing * (level->bricks[i].width + spacing_between_bricks) + initial_x;
		level->bricks[i].coords.y = level->bricks[i].coords.y = 4;

		resistant_spacing++;

	}

	for (; i < level->num_bricks; i++) {
		level->bricks[i].type = BRICK_TYPE_MAGIC;
		level->bricks[i].points = configs.brick_magic_points;
		level->bricks[i].width = configs.brick_magic_width;
		level->bricks[i].lifes = configs.brick_magic_max_lives;
		level->bricks[i].coords.x = magic_spacing * (level->bricks[i].width + spacing_between_bricks) + initial_x;
		level->bricks[i].coords.y = level->bricks[i].coords.y = 2;

		magic_spacing++;
	}

}

void configLevelFour(Level *level, Configs configs) {
	level->num_bricks = 16;

	int normal_bricks = 6;
	int resistant_bricks = 6;
	int magic_bricks = 4;

	int initial_x = 8;

	int i;

	int normal_spacing = 0;
	int resistant_spacing = 0;
	int magic_spacing = 0;

	int spacing_between_bricks = 5;

	level->ball_speed = LEVEL_FOUR_BALL_SPEED;
	level->bricks_speed = LEVEL_FOUR_BRICKS_SPEED;

	for (i = 0; i < normal_bricks; i++) {   // TIJOLOS NORMAIS
		level->bricks[i].type = BRICK_TYPE_NORMAL;
		level->bricks[i].points = configs.brick_normal_points;
		level->bricks[i].width = configs.brick_normal_width;
		level->bricks[i].lifes = configs.brick_normal_max_lives;
		level->bricks[i].coords.x = normal_spacing * (level->bricks[i].width + spacing_between_bricks) + initial_x;
		level->bricks[i].coords.y = 6;

		normal_spacing++;

	}

	for (; i < normal_bricks + resistant_bricks; i++) {      //TIJOLOS RESISTENTES

		level->bricks[i].type = BRICK_TYPE_RESISTANT;
		level->bricks[i].points = configs.brick_resistant_points;
		level->bricks[i].width = configs.brick_resistant_width;
		level->bricks[i].lifes = configs.brick_resistant_max_lives;
		level->bricks[i].coords.x = resistant_spacing * (level->bricks[i].width + spacing_between_bricks) + initial_x;
		level->bricks[i].coords.y = level->bricks[i].coords.y = 4;

		resistant_spacing++;

	}

	for (; i < level->num_bricks; i++) {
		level->bricks[i].type = BRICK_TYPE_MAGIC;
		level->bricks[i].points = configs.brick_magic_points;
		level->bricks[i].width = configs.brick_magic_width;
		level->bricks[i].lifes = configs.brick_magic_max_lives;
		level->bricks[i].coords.x = magic_spacing * (level->bricks[i].width + spacing_between_bricks) + initial_x;
		level->bricks[i].coords.y = level->bricks[i].coords.y = 2;

		magic_spacing++;
	}

}


void configLevelFive(Level *level, Configs configs) {
	level->num_bricks = 20;

	int normal_bricks = 10;
	int resistant_bricks = 6;
	int magic_bricks = 4;

	int initial_x = 5;

	int i;

	int normal_spacing = 0;
	int resistant_spacing = 0;
	int magic_spacing = 0;

	int spacing_between_bricks = 10;

	level->ball_speed = LEVEL_FIVE_BALL_SPEED;
	level->bricks_speed = LEVEL_FIVE_BRICKS_SPEED;

	for (i = 0; i < normal_bricks; i++) {   // TIJOLOS NORMAIS
		level->bricks[i].type = BRICK_TYPE_NORMAL;
		level->bricks[i].points = configs.brick_normal_points;
		level->bricks[i].width = configs.brick_normal_width;
		level->bricks[i].lifes = configs.brick_normal_max_lives;
		level->bricks[i].coords.x = normal_spacing * (level->bricks[i].width + spacing_between_bricks) + initial_x;
		level->bricks[i].coords.y = 6;

		normal_spacing++;

	}

	for (; i < normal_bricks + resistant_bricks; i++) {      //TIJOLOS RESISTENTES

		level->bricks[i].type = BRICK_TYPE_RESISTANT;
		level->bricks[i].points = configs.brick_resistant_points;
		level->bricks[i].width = configs.brick_resistant_width;
		level->bricks[i].lifes = configs.brick_resistant_max_lives;
		level->bricks[i].coords.x = resistant_spacing * (level->bricks[i].width + spacing_between_bricks) + initial_x;
		level->bricks[i].coords.y = level->bricks[i].coords.y = 4;

		resistant_spacing++;

	}

	for (; i < level->num_bricks; i++) {
		level->bricks[i].type = BRICK_TYPE_MAGIC;
		level->bricks[i].points = configs.brick_magic_points;
		level->bricks[i].width = configs.brick_magic_width;
		level->bricks[i].lifes = configs.brick_magic_max_lives;
		level->bricks[i].coords.x = magic_spacing * (level->bricks[i].width + spacing_between_bricks) + initial_x;
		level->bricks[i].coords.y = level->bricks[i].coords.y = 2;

		magic_spacing++;
	}
}