
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
    cart_pole \
    cart_pole_ui \
    double_cart_pole \
    double_cart_pole_ui \
    unicycle \
    unicycle_ui \
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
