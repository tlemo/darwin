
TEMPLATE = subdirs

SUBDIRS += \
    pong \
    pong_ui \
    conquest \
    conquest_ui \
    harvester \
    harvester_ui \
    max \
    tic_tac_toe \
    tic_tac_toe_ui \
    car_track \
    car_track_ui \
    cart_pole \
    cart_pole_ui \
    double_cart_pole \
    double_cart_pole_ui \
    unicycle \
    unicycle_ui \
    drone_follow \
    drone_follow_ui \
    drone_vision \
    drone_vision_ui \
    drone_track \
    drone_track_ui \
    ballistics \
    ballistics_ui \
    test_domain

pong_ui.depends = pong
tic_tac_toe_ui.depends = tic_tac_toe
conquest_ui.depends = conquest
harvester_ui.depends = harvester
cart_pole_ui.depends = cart_pole
double_cart_pole_ui.depends = double_cart_pole
unicycle_ui.depends = unicycle
ballistics_ui.depends = ballistics
drone_vision_ui.depends = drone_vision
drone_follow_ui.depends = drone_follow
drone_track_ui.depends = drone_track
car_track_ui.depends = car_track
