
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
    cart_pole_ui

pong_ui.depends = pong
tic_tac_toe_ui.depends = tic_tac_toe
conquest_ui.depends = conquest
harvester_ui.depends = harvester
cart_pole_ui.depends = cart_pole
