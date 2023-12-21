/* ------------------------------------------------------------------   */
/*      item            : UFODynamics.cxx
        made by         : ethung
        from template   : DusimeModuleTemplate.cxx (2022.06)
        date            : Mon Feb 27 14:11:24 2023
        category        : body file
        description     :
        changes         : Mon Feb 27 14:11:24 2023 first version
        language        : C++
        copyright       : (c)
*/


#define UFODynamics_cxx
// include the definition of the module class
#include "UFODynamics.hxx"

// include additional files needed for your calculation here

// the standard package for DUSIME, including template source
#define DO_INSTANTIATE
#include <dusime.h>

// include the debug writing header. Warning and error messages
// are on by default, debug and info can be selected by
// uncommenting the respective defines
//#define D_MOD
//#define I_MOD
#include <debug.h>

// class/module name
const char* const UFODynamics::classname = "ufo-dynamics";

// initial condition/trim table
const IncoTable* UFODynamics::getMyIncoTable()
{
  static IncoTable inco_table[] = {
    // enter pairs of IncoVariable and VarProbe pointers (i.e.
    // objects made with new), in this table.
    // For example
//    {(new IncoVariable("example", 0.0, 1.0, 0.01))
//     ->forMode(FlightPath, Constraint)
//     ->forMode(Speed, Control),
//     new VarProbe<_ThisModule_,double>
//       (REF_MEMBER(&_ThisModule_::i_example))}

    // always close off with:
    { NULL, NULL} };

  return inco_table;
}

// parameters to be inserted
const ParameterTable* UFODynamics::getMyParameterTable()
{
  static const ParameterTable parameter_table[] = {
    { "set-timing",
      new MemberCall<_ThisModule_,TimeSpec>
        (&_ThisModule_::setTimeSpec), set_timing_description },

    { "check-timing",
      new MemberCall<_ThisModule_,std::vector<int> >
      (&_ThisModule_::checkTiming), check_timing_description },

    /* You can extend this table with labels and MemberCall or
       VarProbe pointers to perform calls or insert values into your
       class objects. Please also add a description (c-style string).

       Note that for efficiency, set_timing_description and
       check_timing_description are pointers to pre-defined strings,
       you can simply enter the descriptive strings in the table. */

    /* The table is closed off with NULL pointers for the variable
       name and MemberCall/VarProbe object. The description is used to
       give an overall description of the module. */
    { NULL, NULL, "please give a description of this module"} };

  return parameter_table;
}

// constructor
UFODynamics::UFODynamics(Entity* e, const char* part, const
                       PrioritySpec& ps) :
  /* The following line initialises the SimulationModule base class.
     You always pass the pointer to the entity, give the classname and the
     part arguments.
     If you give a NULL pointer instead of the inco table, you will not be
     called for trim condition calculations, which is normal if you for
     example implement logging or a display.
     If you give 0 for the snapshot state, you will not be called to
     fill a snapshot, or to restore your state from a snapshot. Only
     applicable if you have no state. */
  SimulationModule(e, classname, part, getMyIncoTable(), 1),

  // initialize the data you need in your simulation
  body(1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0),
  ws(13),
  tau_r(0.5),
  tau_v(2.0),

  // set the UFO at a reasonable height (not in the ice, above it!)
//  body.initialize(0.0, 0.0, -3.0, 0.0, 0.0, 0.0,
//               0.0, 0.0, 0.0, 0.0, 0.0, 0.0);


  // initialize the data you need for the trim calculation

  // initialize the channel access tokens, check the documentation for the
  // various parameters. Some examples:
  // r_mytoken(getId(), NameSet(getEntity(), MyData::classname, part),
  //           MyData::classname, 0, Channel::Events, Channel::ReadAllData),
  // w_mytoken(getId(), NameSet(getEntity(), MyData::classname, part),
  //           MyData::classname, "label", Channel::Continuous),
  r_controls(getId(), NameSet(getEntity(), "ControlInput", part),
             "ControlInput", 0, Channel::Continuous, Channel::OnlyOneEntry),
  w_egomotion(getId(), NameSet(getEntity(), "ObjectMotion", part),
              "BaseObjectMotion", "ufo movement", Channel::Continuous,
              Channel::OnlyOneEntry),
  hudw_token(getId(), NameSet(getEntity(), "HUDData", part),
	      "HUDData","",Channel::Continuous,
	      Channel::OnlyOneEntry),

  // activity initialization
  // myclock(),
  cb1(this, &_ThisModule_::doCalculation),
  do_calc(getId(), "update ufo dynamics", &cb1, ps)
{
  // do the actions you need for the simulation
 // set the UFO at a reasonable height (not in the ice, above it!)
  body.initialize(0.0, 0.0, -3.0, 0.0, 0.0, 0.0,
               0.0, 0.0, 0.0, 0.0, 0.0, 0.0);

  // connect the triggers for simulation
  do_calc.setTrigger(r_controls);

  // connect the triggers for trim calculation. Leave this out if you
  // don not need input for trim calculation
  // trimCalculationCondition(/* fill in your trim triggering channels */);
}

bool UFODynamics::complete()
{
  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */
  return true;
}

// destructor
UFODynamics::~UFODynamics()
{
  //
}

// as an example, the setTimeSpec function
bool UFODynamics::setTimeSpec(const TimeSpec& ts)
{
  // a time span of 0 is not acceptable
  if (ts.getValiditySpan() == 0) return false;

  // specify the timespec to the activity
  do_calc.setTimeSpec(ts);
  // or do this with the clock if you have it (don't do both!)
  // myclock.changePeriodAndOffset(ts);

  // do whatever else you need to process this in your model
  // hint: ts.getDtInSeconds()

  // return true if everything is acceptable
  return true;
}

// and the checkTiming function
bool UFODynamics::checkTiming(const std::vector<int>& i)
{
  if (i.size() == 3) {
    new TimingCheck(do_calc, i[0], i[1], i[2]);
  }
  else if (i.size() == 2) {
    new TimingCheck(do_calc, i[0], i[1]);
  }
  else {
    return false;
  }
  return true;
}

// tell DUECA you are prepared
bool UFODynamics::isPrepared()
{
  bool res = true;

  // Example checking a token:
  CHECK_TOKEN(r_controls);
  CHECK_TOKEN(w_egomotion);

  // Example checking anything
  // CHECK_CONDITION(myfile.good());
  // CHECK_CONDITION2(sometest, "some test failed");

  // return result of checks
  return res;
}

// start the module
void UFODynamics::startModule(const TimeSpec &time)
{
  do_calc.switchOn(time);
}

// stop the module
void UFODynamics::stopModule(const TimeSpec &time)
{
  do_calc.switchOff(time);
}

// fill a snapshot with state data. You may remove this method (and the
// declaration) if you specified to the SimulationModule that the size of
// state snapshots is zero
void UFODynamics::fillSnapshot(const TimeSpec& ts,
                              Snapshot& snap, bool from_trim)
{
  // The most efficient way of filling a snapshot is with an AmorphStore
  // object.
  AmorphStore s(snap.accessData(), snap.getDataSize());
  // assert(snap.getDataSize() == sizeof(snapcopy));

  // set the right format
  snap.coding = Snapshot::Doubles;

  if (from_trim) {
    // use packData(s, trim_state_variable1); ... to pack your state into
    // the snapshot
  }
  else {
    // this is a snapshot from the running simulation. Dusime takes care
    // that no other snapshot is taken at the same time, so you can safely
    // pack the data you copied into (or left into) the snapshot state
    // variables in here
    // use packData(s, snapshot_state_variable1); ...
    // for (const auto &xs: snapcopy) {
    //   packData(s, xs);
    // }
  }
}

// reload from a snapshot. You may remove this method (and the
// declaration) if you specified to the SimulationModule that the size of
// state snapshots is zero
void UFODynamics::loadSnapshot(const TimeSpec& t, const Snapshot& snap)
{
  // access the data in the snapshot with an AmorphReStore object
  AmorphReStore s(snap.data, snap.getDataSize());
  double x(s), y(s), z(s), u(s), v(s), w(s);
  double phi(s), theta(s), psi(s), p(s), q(s), r(s);
  body.initialize(x, y, z, u, v, w, phi, theta, psi, p, q, r);

  // use unPackData(s, real_state_variable1 ); ... to unpack the data
  // from the snapshot.
  // You can safely do this, while snapshot loading is going on the
  // simulation is in HoldCurrent or the activity is stopped.
}

// this routine contains the main simulation process of your module. You
// should read the input channels here, and calculate and write the
// appropriate output
void UFODynamics::doCalculation(const TimeSpec& ts)
{
  // check the state we are supposed to be in
  switch (getAndCheckState(ts)) {
  case SimulationState::HoldCurrent: {
    // only repeat the output, do not change the model state

    break;
    }

  case SimulationState::Replay:
  case SimulationState::Advance: {
    /* The above piece of code shows a block in which you try to catch
       error conditions (exceptions) to handle the case in which the input
       data is lost. This is not always necessary, if you normally do not
       foresee such a condition, and you don t mind being stopped when
       it happens, forget about the try/catch blocks. */

    // do the simulation calculations, one step
    try {
      DataReader<ControlInput> u(r_controls, ts);
          // apply forces on the body
      body.zeroForces();
      Vector3 moms { -u.data().roll, -u.data().pitch, -u.data().yaw };
      body.applyBodyMoment((moms - body.X().segment(6,3))/tau_r);
      Vector3 forces { u.data().throttle, 0.0, 0.0 };
      static Vector3 cg {0.0, 0.0, 0.0};
      body.applyBodyForce((forces - body.X().segment(0,3))/tau_v, cg);
      // printing, for now
      // std::cout << u.data() << std::endl;
    }
    catch(std::exception& e) {
      W_MOD("Could not read control input at " << ts);
    }
    // do the simulation calculations, one step
    integrate_rungekutta(body, ws, ts.getDtInSeconds());
    break;
    }
  default:
    // other states should never be entered for a SimulationModule,
    // HardwareModules on the other hand have more states. Throw an
    // exception if we get here,
    throw CannotHandleState(getId(),GlobalId(), "state unhandled");
  }

  // DUECA applications are data-driven. From the time a module is switched
  // on, it should produce data, so that modules "downstreams" are
  // activated
  // access your output channel(s)
  // example
  // DataWriter<MyOutput> y(output_token, ts);

  // write the output into the output channel, using the stream writer
  // y.data().var1 = something; ...
    body.output();
    DataWriter<BaseObjectMotion> y(w_egomotion, ts);
    for (unsigned ii = 3; ii--; ) {
      y.data().xyz[ii] = body.X()[3+ii];
      y.data().uvw[ii] = body.X()[ ii];
      y.data().omega[ii] = body.X()[6+ii];
    }
    y.data().setquat(body.phi(), body.theta(), body.psi());
 
    cout << "x  = " <<  body.X()[2] << endl;
    cout << "y  = " <<  body.X()[1] << endl;
    cout << "z  = " <<  body.X()[0] << endl;
    cout << "u  = " << body.X()[5] << endl;
    cout << "v  = " << body.X()[4] << endl;
    cout << "w  = " << body.X()[3] << endl;
    cout << "omega 2 = " << body.X()[8] << endl;
    cout << "omega 1 = " << body.X()[7] << endl;
    cout << "omega 0  = " << body.X()[6] << endl;
    DataWriter<HUDData> hud(hudw_token, ts);
       hud.data().ias = body.X()[6];
       hud.data().alt = body.X()[2];
       hud.data().pitch = body.X()[3];
       hud.data().roll = body.X()[4];
       hud.data().heading = body.X()[2];
       hud.data().loadfactor = body.X()[5];
    //   hud.data().xpos = body.X()[3];
    //   hud.data().ypos = body.X()[2];
    //   hud.data().zpos = body.X()[1];
    //   hud.data().u = body.X()[3];
     //  hud.data().v = body.X()[2];
    //   hud.data().w = body.X()[1];
    //   hud.data().vel0 = body.X()[6];
    //   hud.data().vel0 = body.X()[5];
    //   hud.data().vel0 = body.X()[4];
    // set our viewpoint high enough to see something
    // y.data().xyz[2] = -3.0;
 
  if (snapshotNow()) {
    // keep a copy of the model state. Snapshot sending is done in the
    // sendSnapshot routine, later, and possibly at lower priority
    // e.g.
    // snapshot_state_variable1 = state_variable1; ...
    // (or maybe if your state is very large, there is a cleverer way ...)
      // keep a copy of the current state
    for (unsigned ii = 3; ii--; ) {
      // snapcopy[ii] = body.X()[3+ii]; // copy xyz
      // snapcopy[3+ii] = body.X()[ii]; // uvw
      // snapcopy[9+ii] = body.X()[6+ii]; // pqr
    }
    // snapcopy[6] = body.phi();
    // snapcopy[7] = body.theta();
    // snapcopy[8] = body.psi();
  }
}


void UFODynamics::trimCalculation(const TimeSpec& ts, const TrimMode& mode)
{
  // read the event equivalent of the input data
  // example
  // DataReader<MyData> u(i_input_token, ts);

  // using the input, and the data put into your trim variables,
  // calculate the derivative of the state. DO NOT use the state
  // vector of the normal simulation here, because it might be that
  // this is done while the simulation runs!
  // Some elements in this state derivative are needed as target, copy
  // these out again into trim variables (see you TrimTable

  // trim calculation
  switch(mode) {
  case FlightPath: {
    // one type of trim calculation, find a power setting and attitude
    // belonging to a flight path angle and speed
  }
  break;

  case Speed: {
    // find a flightpath belonging to a speed and power setting (also
    // nice for gliders)
  }
  break;

  case Ground: {
    // find an altitude/attitude belonging to standing still on the
    // ground, power/speed 0
  }
  break;

  default:
    W_MOD(getId() << " cannot calculate inco mode " << mode);
  break;
  }

  // This works just like a normal calculation, only you provide the
  // steady state value (if your system is stable anyhow). So, if you
  // have other modules normally depending on your output, you should
  // also produce the equivalent output here.
  // DataWriter<MyOutput> y(output_token, ts);

  // write the output into the output channel, using the DataWriter

  // now return. The real results from the trim calculation, as you
  // specified them in the TrimTable, will now be collected and sent
  // off for processing.
}

// Make a TypeCreator object for this module, the TypeCreator
// will check in with the scheme-interpreting code, and enable the
// creation of modules of this type
static TypeCreator<UFODynamics> a(UFODynamics::getMyParameterTable());

