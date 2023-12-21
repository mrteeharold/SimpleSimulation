/* ------------------------------------------------------------------   */
/*      item            : F16HUDOSGObjectCallback.cxx
	made by         : Olaf Stroosma
        date            : 200328
	category        : body file
        description     : connection of F16HUD to OSG callback
	changes         : 200328 first version
        language        : C++
*/

#ifdef HAVE_OSG_WORLDVIEW

#define F16HUDOSGObjectCallback_cxx
#include "F16HUDOSGObjectCallback.hxx"
#include <OSGViewer.hxx>
#include <dueca/debug.h>
#include "F16HUD.hxx"

USING_DUECA_NS;

F16HUDOSGObjectCallback::F16HUDOSGObjectCallback(const WorldDataSpec& spec):
  hud(800, 600, 20.0),
  initialized(false)
{
  W_MOD("F16HUDOSGObjectCallback " << spec.name);
  this->name = spec.name;
  this->windows = spec.filename;
}

F16HUDOSGObjectCallback::~F16HUDOSGObjectCallback()
{
  //
}

void F16HUDOSGObjectCallback::connect(const GlobalId& master_id,
                                      const NameSet& cname,
                                      entryid_type entry_id,
                                      Channel::EntryTimeAspect time_aspect)
{
  W_MOD("F16HUDOSGObjectCallback connect");
  r_hudinfo.reset(new ChannelReadToken
                 (master_id, cname, HUDData::classname, entry_id,
                  time_aspect, Channel::OneOrMoreEntries,
                  Channel::JumpToMatchTime));
}

void F16HUDOSGObjectCallback::init(const osg::ref_ptr<osg::Group>& root,
                                   OSGViewer* master)
{
  W_MOD("F16HUDOSGObjectCallback init");
  
  // attach the callback, either to specified windows or in general
  if (windows.size()) {
    for (const auto w: windows) {
      master->installPostDrawCallback(this, w);
    }
  }
  else {
    master->installPostDrawCallback(this);
  }
}

void F16HUDOSGObjectCallback::iterate(TimeTickType ts,
                                      const BaseObjectMotion& base,
                                      double late)
{
  if (r_hudinfo->isValid()) {
    try {
      DataReader<HUDData,MatchIntervalStartOrEarlier>
        r(*r_hudinfo, ts);
      F16HUD::fillData(hud, r.data());
    }
    catch (const std::exception& e) {
      W_MOD("Cannot read HUDData data for object " <<
            name << " error " << e.what());
    }
  }
}

/** The actual callback */
void F16HUDOSGObjectCallback::operator()(osg::RenderInfo& renderInfo) const
{
  // initialize when OSG has set up everything and is already calling us
  if(!initialized) {

    // do the initialization here
    init();
    
    initialized = true;
  }

  // render politely
  osg::State* ri = renderInfo.getState();

  // do the rendering here
  draw();

  // clean up
  renderInfo.setState(ri);
}

void F16HUDOSGObjectCallback::draw() const
{
  // I am breaking in on the OSG GL work here. Be very careful, reset
  // everything after doing this draw routine
  glMatrixMode(GL_PROJECTION);
  glPushMatrix(); // saves current (OSG's)

  // define new 2d Projection matrix
  glLoadIdentity();
  gluOrtho2D(0.0, 500.0, 0.0, 500.0);
  
  // define identity model matrix
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix(); // saves current (OSG's)
  glLoadIdentity();  

  // save attributes, a bit coarse, might save space and time by
  // selecting exactly which attributes I change here!
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_FOG);
  glDisable(GL_COLOR_LOGIC_OP);
  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_COLOR_SUM);
  glDisable(GL_CULL_FACE);
  
  // turn alpha blending on
  /*  glDisable(GL_LIGHTING);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);

  // alpha blending off
  glDisable(GL_BLEND);*/

  // draw stuff
  // HUD actually makes its own Matrices
  hud.DrawGL();
  
  // restore the attributes
  glPopAttrib();

  // restore the projection and model matrices
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
}

/** This allows objects of this type to be produced by the OSG Object Factory,
    through the osg-viewer-dueca object and using the syntax 
    'add-object-class-data "AMatchingString" "SomeNameForObject" "f16hud" "ViewPortNameToAttachTo"
    The matching string could be the DCO name (e.g. "HUDData"), or if multiple
    entries exist with that DCO, a more specific name ("HUDData:SomeEntryName")
    Entries will be searched for in the channels supplied to WorldView's
    'add-world-information-channel "TheChannelName://TheEntityName", or
    "ObjectMotion://world" if no channels are defined. Once matched and 
    connected, the iterate function will handle data from the channel
*/
#include "OSGObjectFactory.hxx"
static auto F16HUDOSGObjectCallback_maker = new
  SubContractor<OSGObjectTypeKey,F16HUDOSGObjectCallback>("f16hud");


#endif // HAVE_OSG_WORLDVIEW
