#include "minigolf_levels.h"

// hardcode the parameters of the different levels

minigolf_course_t level1(scene_t *scene) {
    list_t *wall_coordinates = list_init(4, free);
    vector_t *v1 = malloc(sizeof(vector_t));
    vector_t *v2 = malloc(sizeof(vector_t));
    vector_t *v3 = malloc(sizeof(vector_t));
    vector_t *v4 = malloc(sizeof(vector_t));

    *v1 = (vector_t) {-300, -200};
    *v2 = (vector_t) {-300, 200};
    *v3 = (vector_t) {300, 200};
    *v4 = (vector_t) {300, -200};

    list_add(wall_coordinates, v4);
    list_add(wall_coordinates, v3);
    list_add(wall_coordinates, v2);
    list_add(wall_coordinates, v1);

    vector_t ball_center = (vector_t) {-150, 0};
    vector_t hole_center = (vector_t) {150, 0};

    return make_minigolf_course(scene, wall_coordinates, ball_center, hole_center, 10);
}

minigolf_course_t level2(scene_t *scene) {
    list_t *wall_coordinates = list_init(16, free);
    vector_t *v = malloc(sizeof(vector_t));
    *v = (vector_t) {-450, 200}; // 1
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-300, 200}; // 2
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-300, 100}; // 3
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {200, 100}; // 4
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {200, 200}; // 5
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {350, 200}; // 6
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {350, 50}; // 7
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {450, 50}; // 8
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {450, -100}; // 9
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {350, -100}; // 10
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {350, -200}; // 11
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {200, -200}; // 12
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {200, -50}; // 13
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-300, -50}; // 14
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-300, -150}; // 15
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-450, -150}; // 16
    list_add(wall_coordinates, v);

    vector_t ball_center = (vector_t) {-375, 150};
    vector_t hole_center = (vector_t) {275, -150};

    minigolf_course_t course = make_minigolf_course(scene, wall_coordinates, ball_center, hole_center, 2);

    list_t *obs_shape = list_init(3, free);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {200, 25};
    list_add(obs_shape, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {300, 75};
    list_add(obs_shape, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {300, -25};
    list_add(obs_shape, v);

    make_obstacle(scene, obs_shape, course);

    return course;
}

minigolf_course_t level3(scene_t *scene) {
    list_t *wall_coordinates = list_init(8, free);
    vector_t *v = malloc(sizeof(vector_t));
    *v = (vector_t) {-450, 0}; // 1
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-150, 150}; // 2
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {0, 25}; // 3
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {150, 150}; // 4
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {450, 0}; // 5
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {150, -150}; // 6
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {0, -25}; // 7
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-150, -150}; // 8
    list_add(wall_coordinates, v);

    vector_t ball_center = (vector_t) {-300, 0};
    vector_t hole_center = (vector_t) {300, 0};

    minigolf_course_t course = make_minigolf_course(scene, wall_coordinates, ball_center, hole_center, 15);

    list_t *obs_shape = list_init(4, free);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-200, 50};
    list_add(obs_shape, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-100, 50};
    list_add(obs_shape, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-100, -50};
    list_add(obs_shape, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-200, -50};
    list_add(obs_shape, v);

    make_obstacle(scene, obs_shape, course);

    list_t *obs_shape_2 = list_init(4, free);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {200, 50};
    list_add(obs_shape_2, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {100, 50};
    list_add(obs_shape_2, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {100, -50};
    list_add(obs_shape_2, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {200, -50};
    list_add(obs_shape_2, v);

    make_obstacle(scene, obs_shape_2, course);

    return course;
}

minigolf_course_t level4(scene_t *scene) {
    list_t *wall_coordinates = list_init(8, free);
    vector_t *v = malloc(sizeof(vector_t));
    *v = (vector_t) {-450, 0}; // 1
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-450, 200}; // 2
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {200, 200}; // 3
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {200, 0}; // 4
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {450, 0}; // 5
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {450, -200}; // 6
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-200, -200}; // 7
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-200, 0}; // 8
    list_add(wall_coordinates, v);

    vector_t ball_center = (vector_t) {-350, 100};
    vector_t hole_center = (vector_t) {350, -100};

    minigolf_course_t course = make_minigolf_course(scene, wall_coordinates, ball_center, hole_center, 15);

    list_t *obs_shape = list_init(3, free);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {0, -50};
    list_add(obs_shape, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {50, -150};
    list_add(obs_shape, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-50, -150};
    list_add(obs_shape, v);

    make_obstacle(scene, obs_shape, course);

    list_t *obs_shape_2 = list_init(3, free);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-150, 150};
    list_add(obs_shape_2, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-50, 150};
    list_add(obs_shape_2, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-100, 50};
    list_add(obs_shape_2, v);

    make_obstacle(scene, obs_shape_2, course);

    list_t *obs_shape_3 = list_init(3, free);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {150, 150};
    list_add(obs_shape_3, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {50, 150};
    list_add(obs_shape_3, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {100, 50};
    list_add(obs_shape_3, v);

    make_obstacle(scene, obs_shape_3, course);

    return course;
}

minigolf_course_t level5(scene_t *scene) {
    list_t *wall_coordinates = list_init(8, free);
    vector_t *v = malloc(sizeof(vector_t));
    *v = (vector_t) {-400, 0}; // 1
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-200, 200}; // 2
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {0, 0}; // 3
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {200, 200}; // 4
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {400, 0}; // 5
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {400, -200}; // 6
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {200, 0}; // 7
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {0, -200}; // 8
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-200, 0}; // 9
    list_add(wall_coordinates, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-400, -200}; // 10
    list_add(wall_coordinates, v);

    vector_t ball_center = (vector_t) {-350, -75};
    vector_t hole_center = (vector_t) {350, -75};

    minigolf_course_t course = make_minigolf_course(scene, wall_coordinates, ball_center, hole_center, 15);

    list_t *obs_shape = list_init(3, free);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-250, 50};
    list_add(obs_shape, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-200, 150};
    list_add(obs_shape, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {-150, 50};
    list_add(obs_shape, v);

    make_obstacle(scene, obs_shape, course);

    list_t *obs_shape_2 = list_init(3, free);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {150, 50};
    list_add(obs_shape_2, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {200, 150};
    list_add(obs_shape_2, v);
    v = malloc(sizeof(vector_t));
    *v = (vector_t) {250, 50};
    list_add(obs_shape_2, v);

    make_obstacle(scene, obs_shape_2, course);

    return course;
}


minigolf_course_t get_level(scene_t *scene, int level) {
    switch (level) {
        case 1:
            return level1(scene);
        case 2:
            return level2(scene);
        case 3:
            return level3(scene);
        case 4:
            return level4(scene);
        case 5:
            return level5(scene);
        default:
            return level1(scene);
    }
}
