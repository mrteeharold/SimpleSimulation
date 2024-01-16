## -*-python-*-
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
    
    for e in ("team1",):
        DUECA_mods.append(
            dueca.Module("initials-inventory", e, admin_priority).param(
                # reference_file=f"initials-{e}.toml",
                store_file=f"initials-{e}-%Y%m%d_%H%M.toml"))
        DUECA_mods.append(
            dueca.Module("replay-master", e, admin_priority).param(
                # reference_files=f"recordings-{e}.ddff",
                store_files=f"recordings-{e}-%Y%m%d_%H%M%S.ddff"))
    

    # create the DUECA entity with that list
    DUECA_entity = dueca.Entity("dueca", DUECA_mods)

## ---------------------------------------------------------------------
# modules for your project (example)
mymods = []

if this_node_id == ecs_node:
    mymods.append(dueca.Module(
        "flexi-stick", "", sim_priority).param(
            set_timing = sim_timing,
            enable_record_replay = False,
            check_timing = (1000, 2000)).param(
            # logitech stick, first SDL device
            ('add_device', "logi:0"),
 
            # by default, axes go from -1 to 1, convert throttle to
            # run from -1 to 5 (slow back-up to forward 5m/s), with
            # a polynomial. The throttle is on axis 2 of the stick
            ('create_poly', ('throttle', 'logi.a[2]')),
            ('poly_params', (2, -3)),
            ('create_poly', ('roll', 'logi.a[0]')),
            ('poly_params', (0, -1)),
            ('create_poly', ('pitch', 'logi.a[1]')),
            ('poly_params', (0, -1)),
            ('create_poly', ('yaw', 'logi.a[2]')),
            ('poly_params', (0, -1)),
 
            # define that we write a channel
            ('add_channel',
             ('controls',              # variable
              'ControlInput://team1', # channel name
              'ControlInput',          # data type
              'control input')),       # label
 
            # link axis 0 to control roll, etc, etc
            ("add_link", ("controls.roll", "roll")),
            ("add_link", ("controls.pitch", "pitch")),
            ("add_link", ("controls.yaw", "yaw")),
            ("add_link", ("controls.throttle", "throttle"))
            )
    )
    # our new dynamics module
    mymods.append(dueca.Module(
    "ufo-dynamics", "", sim_priority).param(
        set_timing = sim_timing,
        check_timing = (1000, 2000)))
    # the HUD
    #mymods.append(dueca.Module(
    #    "f16-hud", "", admin_priority).param(
    #    set_timing = display_timing))
    # replicator
    mymods.append(dueca.Module(
    'channel-replicator-peer', "", log_priority).param(
        config_url="ws://127.0.0.1:8032/config"))
    # the visual output
    mymods.append(dueca.Module(
        "world-view", "", admin_priority).param(
        set_timing = display_timing,
        check_timing = (8000, 9000),
        claim_thread = False,
        restore_context = False,
        predict_dt = 0.0,
        predict_dt_max = 0.0,
        initial_camera = (0.0, 0.0, -10.0, 0.0, 0.0, 0.0),
        add_world_information_channel =
        #("ObjectMotion://world","HUDData://team1"),
                  ("BaseObjectMotion://world",),
        set_viewer =
        dueca.OSGViewer().param(
            # set up window
            ('add_window', 'front'),
            ('window_size+pos', (800, 600, 10, 10)),
            ('add_viewport', 'front'),
            ('viewport_window', 'front'),
            ('viewport_pos+size', (0, 0, 800, 600)),

            # add visual objects (classes, then instantiation)
            ('add-object-class-data',
             ("static:sunlight", "sunlight", "static-light")),
            ('add-object-class-coordinates',
             (0.48, 0.48, 0.48, 1,   # ambient
              0.48, 0.48, 0.48, 1,   # diffuse
              0.0, 0.0, 0.0, 1,      # specular
              0.4, 0.0, 1.0, 0,      # south??
              0, 0, 0,               # direction not used
              0.2, 0, 0)),           # no attenuation for sun
            # create an object class for the terrain, to be represented
            # as a static (not position controlled) object, with
            # terrain.obj as the file defining it
            ('add-object-class-data',
             ("static:terrain", "terrain", "static", "terrain.obj")),
            # same for skydome
            ('add-object-class-data',
             ("centered:skydome", "skydome", "centered", "skydome.obj")),
            # move the skydome default position a bit down
            ('add-object-class-coordinates',
             (0.0, 0.0, 50.0)),

            # make static objects through the configuration.
            # These match the creation keys (static:sunlight etc), to
            # find the right object class.
            ('static-object', ('static:sunlight', 'sunlight')),
            ('static-object', ('static:terrain', 'terrain')),
            ('static-object', ('centered:skydome', 'skydome')),
            #object class for teams
#            ('add-object-class-data',
#            ('BaseObjectMotion:team1', 'Team One', 'moving',
#             'platillo.obj')),
#            ('add-object-class-data',
#            ('BaseObjectMotion:team2', 'Team Two', 'moving',
#             'platillo.obj')),
            ('add-object-class-data',
            ('BaseObjectMotion', 'Team #', 'moving',
             'mav1.ac')),

            # HUD on main viewport, looking for HUDData
            #add_object_class_data = ("HUDData", "hud", "f16hud", "front"),
            #set_frustum = (0.5, 10000, 30),
            #set_bg_color = (0,0,1),
            #set_fog = (2, 0.0, 0.0, 0.0, 0.5, 1.0, 10000.0, 100000.0),
            #use_compositeviewer = False,
            #allow_unknown = True)
        )
        )
)


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
    #             ('info', ("endpoint", "MyData://PHLAB")),
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
    #             ('log_entry', ("MyData://PHLAB",
    #                            "MyData", "/data/mydata"))))
    #     )

# etc, each node can have modules in its mymods list

    # add a filer in this node for replay support
    # filer = dueca.ReplayFiler("PLHLAB")

# then combine in an entity (one "copy" per node)
if mymods:
    myentity = dueca.Entity("team1", mymods)
