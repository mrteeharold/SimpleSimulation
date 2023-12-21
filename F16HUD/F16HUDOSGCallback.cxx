/* ------------------------------------------------------------------   */
/*      item            : F16HUDOSGCallback.cxx
	made by         : Olaf Stroosma
        date            : 200328
	category        : body file
        description     : connection of F16HUD to OSG callback
	changes         : 200328 first version
        language        : C++
*/

#ifdef HAVE_OSG_WORLDVIEW

#define F16HUDOSGCallback_cxx
#include "F16HUDOSGCallback.hxx"

#define DO_INSTANTIATE
#include <VarProbe.hxx>
#include <MemberCall.hxx>
#include <MemberCall2Way.hxx>
#include <CoreCreator.hxx>

USING_DUECA_NS;

F16HUDOSGCallback::F16HUDOSGCallback():
hud(800, 600, 20.0)
{
  //
}

bool F16HUDOSGCallback::complete()
{
  /* All your parameters have been set. You may do extended
     initialisation here. Return false if something is wrong. */
  return true;
}

F16HUDOSGCallback::~F16HUDOSGCallback()
{
  //
}

/** The actual callback */
void F16HUDOSGCallback::operator()(osg::RenderInfo& renderInfo) const
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

void F16HUDOSGCallback::draw() const
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

// script access macro
SCM_FEATURES_NOIMPINH(F16HUDOSGCallback, ScriptCreatable, "f16-hud-osg-callback");

// Make a CoreCreator object for this module, the CoreCreator
// will check in with the scheme-interpreting code, and enable the
// creation of objects of this type
static CoreCreator<F16HUDOSGCallback> a(NULL);

#endif // HAVE_OSG_WORLDVIEW
