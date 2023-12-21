/* ------------------------------------------------------------------   */
/*      item            : F16HUDOSGObjectCallback.hxx
        made by         : Olaf Stroosma
        date            : 200328
	category        : header file 
        description     : connection of F16HUD to OSG callback
	changes         : 200328 first version
        language        : C++
*/
#ifdef HAVE_OSG_WORLDVIEW

#ifndef F16HUDOSGObjectCallback_hxx
#define F16HUDOSGObjectCallback_hxx

#include "OSGObjectCallback.hxx"
#include "HUD.hxx"
#include "comm-objects.h"

/** A script-creatable post-draw callback object for e.g. HUDs. */
class F16HUDOSGObjectCallback: public OSGObjectCallback
{
  /** Channel read token for hud data */
  boost::scoped_ptr<ChannelReadToken>     r_hudinfo;

  /** vector of windows */
  std::vector<std::string>                windows;
  
public:
  /** Constructor */
  F16HUDOSGObjectCallback(const WorldDataSpec& spec);
  
  /** Destructor */
  ~F16HUDOSGObjectCallback();
  
public: // OSGObjectCallback->osg::Camera::DrawObjectCallback
  /** Actual callback function, called by renderer. */
  void operator()(osg::RenderInfo& renderInfo) const override;

  /** Connect to a channel entry 

      @param master_id ID for opening a channel reader
      @param cname     Channel with object data
      @param entry_id  Entry in the channel
  */
  void connect(const GlobalId& master_id, const NameSet& cname,
               entryid_type entry_id,
               Channel::EntryTimeAspect time_aspect);
  
  /** Init for the object, overriding OSGObject init 

      This init attaches the callback
  */
  void init(const osg::ref_ptr<osg::Group>& root, OSGViewer* master);
  
  /** Play, update, recalculate, etc. */
  void iterate(TimeTickType ts, const BaseObjectMotion& base, double late);

public:
  /** Access to the drawing class, for filling data */
  inline HUD& getHUD(){return hud;}

  /** Draw the overlay */
  void draw() const;

  /** Simple Init */
  void init() const { hud.InitGL(); }

private:
  /** Drawing class. */
  mutable HUD hud;

  /** Init done */
  mutable bool initialized;
  
};

#endif // F16HUDOSGCallback_hxx

#endif // HAVE_OSG_WORLDVIEW
