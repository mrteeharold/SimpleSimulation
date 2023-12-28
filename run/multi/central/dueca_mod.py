## -*-python-*-
## dueca.cnf: created with DUECA version 4.0.5
## Created on: 28-Dec-2023

## this is an example dueca_mod.py file, for you to start out with and adapt
## according to your needs. Note that you need a dueca_mod.py file only for the
## node with number 0

## in general, it is a good idea to clearly document your set up
## this is an excellent place.

## node set-up
ecs_node = 0   # dutmms1, send order 3
#aux_node = 1   # dutmms3, send order 1
#pfd_node = 2   # dutmms5, send order 2
#cl_node = 3    # dutmms4, send order 0

## priority set-up
# normal nodes: 0 administration
#               1 hdf5 logging
#               2 simulation, unpackers
#               3 communication
#               4 ticker

# administration priority. Run the interface and logging here
admin_priority = dueca.PrioritySpec(0, 0)

# logging prio. Keep this free from time-critical other processes
log_priority = dueca.PrioritySpec(1, 0)

# priority of simulation, just above log
sim_priority = dueca.PrioritySpec(2, 0)

# nodes with a different priority scheme
# control loading node has 0, 1, 2 and 3 as above and furthermore
#               4 stick priority
#               5 ticker priority
# priority of the stick. Higher than prio of communication
# stick_priority = dueca.PrioritySpec(4, 0)

# timing set-up
# timing of the stick calculations. Assuming 100 usec ticks, this gives 2500 Hz
# stick_timing = dueca.TimeSpec(0, 4)

# this is normally 100, giving 100 Hz timing
sim_timing = dueca.TimeSpec(0, 100)

## for now, display on 50 Hz
display_timing = dueca.TimeSpec(0, 200)

## log a bit more economical, 25 Hz
log_timing = dueca.TimeSpec(0, 400)

## the name for the main entity to create. Note that you can create as
## many entities as you want, usually one is enough. Adjust as appropriate
entity_name = "central"

## ---------------------------------------------------------------------
### the modules needed for dueca itself
if this_node_id == ecs_node:

    # create a list of modules:
    DUECA_mods = []
    DUECA_mods.append(dueca.Module("dusime", "", admin_priority))
    DUECA_mods.append(dueca.Module("dueca-view", "", admin_priority))
    DUECA_mods.append(dueca.Module("activity-view", "", admin_priority))
    DUECA_mods.append(dueca.Module("timing-view", "", admin_priority))
    DUECA_mods.append(dueca.Module("log-view", "", admin_priority))
    DUECA_mods.append(dueca.Module("channel-view", "", admin_priority))
    # uncomment for web-based graph, see DUECA documentation
    # DUECA_mods.append(dueca.Module("config-storage", "", admin_priority))

    if no_of_nodes > 1 and not classic_ip:
        DUECA_mods.append(dueca.Module("net-view", "", admin_priority))

    # remove the quotes to enable DUSIME initial condition recording and
    # setting, and simulation recording and replay
    '''
    for e in (entity_name,):
        DUECA_mods.append(
            dueca.Module("initials-inventory", e, admin_priority).param(
                reference_file=f"initials-{e}.toml",
                store_file=f"initials-{e}-%Y%m%d_%H%M.toml"))
        DUECA_mods.append(
            dueca.Module("replay-master", e, admin_priority).param(
                reference_files=f"recordings-{e}.ddff",
                store_files=f"recordings-{e}-%Y%m%d_%H%M%S.ddff"))
    '''

    # create the DUECA entity with that list
    DUECA_entity = dueca.Entity("dueca", DUECA_mods)

## ---------------------------------------------------------------------
# modules for your project (example)
mymods = []

if this_node_id == ecs_node:
#    mymods.append(dueca.Module(
#        "some-module-i-created", "", sim_priority).param(
#            set_timing = sim_timing,
#            check_timing = (10000, 20000)))
#
    # this simply prints joining of teams and current position
    mymods.append(dueca.Module(
        "monitor-teams", "", sim_priority).param(
            set_timing = log_timing,
            check_timing = (10000, 20000)))
        # this is a standard DUECA module (from the "inter" library), that
    # can connect to other DUECA processes, and replicate given channels
    mymods.append(dueca.Module(
        'channel-replicator-master', "", log_priority).param(
        timing_gain=0.00001,
        set_timing=sim_timing,
        watch_channels=("BaseObjectMotion://world", ),
        message_size=1450,
        replicator_information_channel="ReplicatorInfo://central",
        data_url="ws://127.0.0.1:8032/data",
        config_url="ws://127.0.0.1:8032/config"))




    # Uncomment and adapt for web-based graph, see DUECA documentation.
    # This also serves the static files for the default plotting application
    # over http.
    # adjust the priority if you need this for other, time-critical, data
    #
    # mymods.append(
    #     dueca.Module(
    #         "web-sockets-server", "", admin_priority).param(
    #             ('set-timing', sim_timing),
    #             ('check-timing', (5000, 9000)),
    #             ('port', 8001),
    #             ('info', ("endpoint", "MyData://"+entity_name)),
    #             ('write-and-read', ("plotconfig",
    #                                 "ConfigFileRequest://dueca",
    #                                 "ConfigFileData://dueca")),
    #             ('http-port', 8000),
    #             ('document-root', '/usr/share/dplotter/dist')))

    # uncomment and adapt for HDF5 logging, see DUECA documentation
    # mymods.append(
    #     dueca.Module(
    #         "hdf5-logger", "", log_priority).param(
    #             ('set_timing', log_timing),
    #             ('chunksize', 3000),
    #             ('log_entry', ("MyData://"+entity_name,
    #                            "MyData", "/data/mydata"))))
    #     )

# etc, each node can have modules in its mymods list

    # add a filer in this node for replay support
    # filer = dueca.ReplayFiler(entity_name)

# then combine in an entity
if mymods:
    myentity = dueca.Entity(entity_name, mymods)
