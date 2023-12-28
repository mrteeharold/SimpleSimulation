/* ------------------------------------------------------------------   */
/*      item            : UFODynamics.hxx
        made by         : ethung
        from template   : DusimeModuleTemplate.hxx (2022.06)
        date            : Mon Feb 27 14:11:24 2023
        category        : header file
        description     :
        changes         : Mon Feb 27 14:11:24 2023 first version
        language        : C++
        copyright       : (c)
*/

#ifndef UFODynamics_hxx
#define UFODynamics_hxx

// include headers for functions/classes you need in the module

#include <array>
#include <extra/RigidBody.hxx>
#include <extra/integrate_rungekutta.hxx>

// include the dusime header
#include <dusime.h>
USING_DUECA_NS;

// This includes headers for the objects that are sent over the channels
#include "comm-objects.h"


/** A module.

    The instructions to create an module of this class from the Scheme
    script are:

    \verbinclude ufo-dynamics.scm
*/
class UFODynamics: public SimulationModule
{
  /** self-define the module type, to ease writing the parameter table */
  typedef UFODynamics _ThisModule_;

private: // simulation data
  // declare the data you need in your simulation

    // Rigid body dynamics
  RigidBody            body;

  // Workspace for the integration
  RungeKuttaWorkspace  ws;

  // Time constant for following rotational inputs
  double               tau_r;

    // Time constant for following linear inputs
  double               tau_v;


private: // trim calculation data
  // declare the trim calculation data needed for your simulation

private: // snapshot data
  // declare, if you need, the room for placing snapshot data

    // temporary storage for the capturing the state
  //std::array<12,double> snapcopy;
private: // channel access
  // declare access tokens for all the channels you read and write
  // examples:
  ChannelReadToken    r_controls;
  ChannelWriteToken   w_egomotion;
  ChannelWriteToken   hudw_token;
  ChannelWriteToken   w_world;


private: // activity allocation
  /** You might also need a clock. Don't mis-use this, because it is
      generally better to trigger on the incoming channels */
  //PeriodicAlarm        myclock;

  /** Callback object for simulation calculation. */
  Callback<UFODynamics>  cb1;

  /** Activity for simulation calculation. */
  ActivityCallback      do_calc;

public: // class name and trim/parameter tables
  /** Name of the module. */
  static const char* const           classname;

  /** Return the initial condition table. */
  static const IncoTable*            getMyIncoTable();

  /** Return the parameter table. */
  static const ParameterTable*       getMyParameterTable();

public: // construction and further specification
  /** Constructor. Is normally called from scheme/the creation script. */
  UFODynamics(Entity* e, const char* part, const PrioritySpec& ts);

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. Your running environment, e.g. for OpenGL
      drawing, is also prepared. Any lengty initialisations (like
      reading the 4 GB of wind tables) should be done here.
      Return false if something in the parameters is wrong (by
      the way, it would help if you printed what!) May be deleted. */

  bool complete();

  /** Destructor. */
  ~UFODynamics();

  // add here the member functions you want to be called with further
  // parameters. These are then also added in the parameter table
  // The most common one (addition of time spec) is given here.
  // Delete if not needed!

  /** Specify a time specification for the simulation activity. */
  bool setTimeSpec(const TimeSpec& ts);

  /** Request check on the timing. */
  bool checkTiming(const std::vector<int>& i);

public: // member functions for cooperation with DUECA
  /** indicate that everything is ready. */
  bool isPrepared();

  /** start responsiveness to input data. */
  void startModule(const TimeSpec &time);

  /** stop responsiveness to input data. */
  void stopModule(const TimeSpec &time);

public: // the member functions that are called for activities
  /** the method that implements the main calculation. */
  void doCalculation(const TimeSpec& ts);

public: // member functions for cooperation with DUSIME
  /** For the Snapshot capability, fill the snapshot "snap" with the
      data saved at a point in your simulation (if from_trim is false)
      or with the state data calculated in the trim calculation (if
      from_trim is true). */
  void fillSnapshot(const TimeSpec& ts,
                    Snapshot& snap, bool from_trim);

  /** Restoring the state of the simulation from a snapshot. */
  void loadSnapshot(const TimeSpec& t, const Snapshot& snap);

  /** Perform a trim calculation. Should NOT use current state
      uses event channels parallel to the stream data channels,
      calculates, based on the event channel input, the steady state
      output. */
  void trimCalculation(const TimeSpec& ts, const TrimMode& mode);
};

#endif
