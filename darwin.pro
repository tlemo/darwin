
TEMPLATE = subdirs

SUBDIRS += \
    core \
    core_ui \
    populations \
    domains \
    registry \
    darwin_studio \
    tests \
    experimental \
    third_party

core.depends = third_party
core_ui.depends = core
populations.depends = core core_ui
domains.depends = core core_ui
registry.depends = core populations domains
darwin_studio.depends = core core_ui registry
tests.depends = core registry third_party
experimental.depends = core registry third_party
