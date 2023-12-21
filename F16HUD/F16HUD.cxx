/* ------------------------------------------------------------------   */
/*      item            : F16HUD.cxx
        made by         : DASTUDostroosma
	from template   : DuecaModuleTemplate.cxx
        template made by: Rene van Paassen
        date            : Wed May  4 13:30:35 2011
	category        : body file 
        description     : 
	changes         : Wed May  4 13:30:35 2011 first version
	template changes: 030401 RvP Added template creation comment
	                  060512 RvP Modified token checking code
        language        : C++
*/

static const char c_id[] =
"$Id: F16HUD.cxx,v 1.1.1.1 2022/04/21 14:47:16 ostroosma Exp $";

#define F16HUD_cxx
// include the definition of the module class
#include "F16HUD.hxx"

// include the debug writing header, by default, write warning and 
// error messages
#define W_MOD
#define E_MOD
#include <debug.h>

// include additional files needed for your calculation here

// the standard package for DUSIME, including template source
#define DO_INSTANTIATE
#include <dueca.h>
#include <Entity.hxx>
#include <MemberCall2Way.hxx>

// class/module name
const char* const F16HUD::classname = "f16-hud";

// Parameters to be inserted
const ParameterTable* F16HUD::getMyParameterTable()
{
  static const ParameterTable parameter_table[] = {
    { "set-timing", 
      new MemberCall<F16HUD,TimeSpec>
        (&F16HUD::setTimeSpec), set_timing_description },

    { "check-timing", 
      new MemberCall<F16HUD,vector<int> >
      (&F16HUD::checkTiming), check_timing_description },

#ifdef HAVE_OSG_WORLDVIEW 
    { "set-callback", 
      new MemberCall2Way<F16HUD, ScriptCreatable>
      (&F16HUD::setCallback), 
      "specify a WorldView callback to control" },
#endif

    /* You can extend this table with labels and MemberCall or
       VarProbe pointers to perform calls or insert values into your
       class objects. Please also add a description (c-style string). 

       Note that for efficiency, set_timing_description and
       check_timing_description are pointers to pre-defined strings,
       you can simply enter the descriptive strings in the table. */

    /* The table is closed off with NULL pointers for the variable
       name and MemberCall/VarProbe object. The description is used to
       give an overall description of the module. */
    { NULL, NULL, "stand-alone F16 HUD"} };

  return parameter_table;
}

// constructor
F16HUD::F16HUD(Entity* e, const char* part, const
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
  Module(e, classname, part),

  // initialize the data you need in your simulation
  window(), 
#ifdef HAVE_OSG_WORLDVIEW 
  osg_callback(NULL),
#endif
  
  // initialize the data you need for the trim calculation

  // initialize the channel access tokens
  // example
  // my_token(getId(), NameSet(getEntity(), "MyData", part)),
  hudr_token(getId(), NameSet(getEntity(), "HUDData", part)),
  
  // activity initialization
  myclock(),
  cb1(this, &F16HUD::doCalculation),
  do_calc(getId(), "drawing F16 HUD", &cb1, PrioritySpec(0,0))
{
  // do the actions you need for the simulation
  if(ps.getPriority() != 0) {
    W_MOD(classname << ": GUI modules should run in admin-priority, corrected");
  }

  // connect the triggers for simulation
  do_calc.setTrigger(myclock/* fill in your triggering channels, 
			or enter the clock here */);
}

bool F16HUD::complete()
{
  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */

  // Open the GL window
  //window.openWindow();

  return true;
}

// destructor
F16HUD::~F16HUD()
{
  //
}

// as an example, the setTimeSpec function
bool F16HUD::setTimeSpec(const TimeSpec& ts)
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
bool F16HUD::checkTiming(const vector<int>& i)
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

#ifdef HAVE_OSG_WORLDVIEW
bool F16HUD::setCallback(ScriptCreatable &obj, bool in)
{
  // check direction
  if (!in) return false;

  if (osg_callback) {
    E_CNF(classname << ": can only specify one callback");
    return false;
  }

  // try a dynamic cast
  osg_callback = dynamic_cast<F16HUDOSGCallback*> (&obj);
  if (osg_callback == NULL) {
    E_CNF(classname << ": must supply a F16HUDOSGCallback");
    return false;
  }

  return true;
}
#endif

// tell DUECA you are prepared
bool F16HUD::isPrepared()
{
  bool res = true;

  // Example checking a token:
  // CHECK_TOKEN(w_somedata);
  CHECK_TOKEN(hudr_token);

  // Example checking anything
  // CHECK_CONDITION(myfile.good());

  // return result of checks
  return res;
}

// start the module
void F16HUD::startModule(const TimeSpec &time)
{
  do_calc.switchOn(time);
}

// stop the module
void F16HUD::stopModule(const TimeSpec &time)
{
  do_calc.switchOff(time);
}

// this routine contains the main simulation process of your module. You 
// should read the input channels here, and calculate and write the 
// appropriate output
void F16HUD::doCalculation(const TimeSpec& ts)
{
  // overload protection: return immediately if we're lagging
  if(do_calc.noScheduledBehind()) {
    return;
  }

  // access the input 
  // example:
  // try {
  //   StreamReader<MyInput> u(input_token, ts);
  //   throttle = u.data().throttle;    
  //   de = u.data().de; ....
  // } 
  // catch(Exception& e) {
  //   // strange, there is no input. Should I try to continue or not?
  // }
  /* The above piece of code shows a block in which you try to catch 
     error conditions (exceptions) to handle the case in which the input
     data is lost. This is not always necessary, if you normally do not
     foresee such a condition, and you don t mind being stopped when
     it happens, forget about the try/catch blocks. */

  try {
    StreamReader<HUDData> y(hudr_token, ts);

    // set data on windowed HUD
    fillData(window.getHUD(), y.data());
    
    window.redraw();

#ifdef HAVE_OSG_WORLDVIEW
    // set data on OSG callback HUD
    if(osg_callback) {
      fillData(osg_callback->getHUD(), y.data());
    }
#endif
    
  } catch (Exception& e) {
    W_MOD(classname << " caught " << e << " @ " << ts);
  }
  
  // do the simulation or other calculations, one step

  // DUECA applications are data-driven. From the time a module is switched
  // on, it should produce data, so that modules "downstreams" are 
  // activated
  // access your output channel(s)
  // example
  // StreamWriter<MyOutput> y(output_token, ts);

  // write the output into the output channel, using the stream writer
  // y.data().var1 = something; ...
} 

void F16HUD::fillData(HUD& hud, const HUDData& your_data)
{
    hud.SetSpeedTapeSpeedIAS(your_data.ias);
    hud.SetAltTapeAltitude(your_data.alt);
    hud.SetPitchLadderPitchAngle(your_data.pitch);
    hud.SetPitchLadderRollAngle(your_data.roll);
    hud.SetBankIndicatorRollAngle(your_data.roll);
    hud.SetHeadingTapeHeading(your_data.heading);
    hud.SetAircraftReferenceNz(your_data.loadfactor);
}

// Make a TypeCreator object for this module, the TypeCreator
// will check in with the scheme-interpreting code, and enable the
// creation of modules of this type
static TypeCreator<F16HUD> a(F16HUD::getMyParameterTable());

