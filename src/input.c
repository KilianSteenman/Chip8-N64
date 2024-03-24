//
// Created by Shadow-Link on 24/03/2024.
//

#include "input.h"

bool is_button_pressed(struct controller_data controllers, int controller_index, Button button) {
    switch(button) {
        case A:
            return controllers.c[controller_index].A;
            break;
        case B:
            return controllers.c[controller_index].B;
            break;
        case L:
            return controllers.c[controller_index].L;
            break;
        case R:
            return controllers.c[controller_index].R;
            break;
        case Z:
            return controllers.c[controller_index].Z;
            break;
        case Up:
            return controllers.c[controller_index].up;
            break;
        case Down:
            return controllers.c[controller_index].down;
            break;
        case Left:
            return controllers.c[controller_index].left;
            break;
        case Right:
            return controllers.c[controller_index].right;
            break;
        case C_Up:
            return controllers.c[controller_index].C_up;
            break;
        case C_Down:
            return controllers.c[controller_index].C_down;
            break;
        case C_Left:
            return controllers.c[controller_index].C_left;
            break;
        case C_Right:
            return controllers.c[controller_index].C_right;
            break;
        default:
            printf("Unknown button");
            exit(0);
            break;
    }
}
