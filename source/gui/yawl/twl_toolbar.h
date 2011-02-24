// twl_toolbar.h
#ifndef __TWL_TOOLBAR
#define __TWL_TOOLBAR
#include "twl.h"
#include "twl_menu.h"

class EXPORT TToolbar: public TWin {
   MessageHandler* m_menu_handler;
   TEventWindow* m_form;
   TEventWindow* m_container;
   const char* m_path;
   int m_bwidth,m_bheight;

public:
    TToolbar(TEventWindow* form, int bwidth=16, int bheight=16, TEventWindow* m_container=NULL);
    ~TToolbar();
    void create();
	void set_path(const char* path);
    void add_item(const char* bmp, const char* tooltext, EventHandler eh, void* data=NULL);
	SIZE get_size();
    void release();
};

EXPORT TToolbar& operator<< (TToolbar& tb, Item item);
EXPORT TToolbar& operator<< (TToolbar& tb, Sep sep);

#endif
