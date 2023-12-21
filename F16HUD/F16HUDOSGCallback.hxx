/* ------------------------------------------------------------------   */
/*      item            : F16HUDOSGCallback.hxx
        made by         : Olaf Stroosma
        date            : 200328
	category        : header file 
        description     : connection of F16HUD to OSG callback
	changes         : 200328 first version
        language        : C++
*/
#ifdef HAVE_OSG_WORLDVIEW

#ifndef F16HUDOSGCallback_hxx
#define F16HUDOSGCallback_hxx

#include "OSGCallback.hxx"
#include "HUD.hxx"

/** A script-creatable post-draw callback object for e.g. HUDs. */
class F16HUDOSGCallback: public OSGCallback
{
public:
  /** Constructor */
  F16HUDOSGCallback();

  /** Continued construction. This is called after all script
      parameters have been read and filled in, according to the
      parameter table. */
  bool complete();
  
  /** Destructor */
  ~F16HUDOSGCallback();
  
public: // OSGCallback->osg::Camera::DrawCallback
  /** Actual callback function, called by renderer. */
  virtual void operator()(osg::RenderInfo& renderInfo) const;
 
public: // OSGCallback->ScriptCreatable
  /** This is a scheme-level callable class */
  SCM_FEATURES_DEF;
  
public:
  /** Access to the drawing class, for filling data */
  inline HUD& getHUD(){return hud;}

  /** Draw the overlay */
  void draw() const;

  /** Init */
  void init() const { hud.InitGL(); }

private:
  /** Drawing class. */
  mutable HUD hud;
  
};

#endif // F16HUDOSGCallback_hxx

#endif // HAVE_OSG_WORLDVIEW
