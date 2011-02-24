/* gui_ext.cpp
This is a simple set of predefined GUI windows for SciTE,
built using the YAWL library.
Steve Donovan, 2007.
  */
#include <windows.h>
#include "yawl.h"
#include <string.h>
#include <io.h>
#include <direct.h>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

const char* WINDOW_CLASS = "WINDOW*";
const int BUFFER_SIZE = 2*0xFFFF;
static TWin* s_parent = NULL;
static TWin* s_last_parent = NULL;

#define EQ(s1,s2) (strcmp((s1),(s2))==0)

TWin* get_parent()
{
	return s_parent;
}

TWin* get_last_parent()
{
	return s_last_parent;
}

TWin *get_desktop_window()
{
	static TWin *desk = NULL;
	if (desk == NULL) {
		desk = new TWin(GetDesktopWindow());
	}
	return desk;
}


class PromptDlg: public TModalDlg {
public:
	char m_val[256];
    const char *m_field_name;

	PromptDlg(TEventWindow *parent, pchar field_name, const char* val)
		: TModalDlg(parent,"Enter:"), m_field_name(field_name)
	{
		strcpy(m_val,val);
	}

	void layout(Layout& lo)
    {
		lo << Field(m_field_name,m_val);
    }
};

// show a message on the SciTE output window
void OutputMessage(lua_State *L)
{
	if (lua_isstring(L,-1)) {
		size_t len;
		const char *msg = lua_tolstring(L,-1,&len);
		char *buff = new char[len+2];
		strncpy(buff,msg,len);
		buff[len] = '\n';
		buff[len+1] = '\0';
		lua_pop(L,1);
		if (lua_checkstack(L,3)) {
			lua_getglobal(L,"output");
			lua_getfield(L,-1,"AddText");
			lua_insert(L,-2);
			lua_pushstring(L,buff);
			lua_pcall(L,2,0,0);
		}
		delete[] buff;
	}
}

class LuaWindow: public TEventWindow
{
protected:
	lua_State* L;
public:
	LuaWindow(const char* caption,lua_State* l, TWin *parent, int stylex = 0, bool is_child = false, int style = -1)
		: TEventWindow(caption,parent,stylex,is_child,style),L(l)
	{}

	void handler(Item* item)
	{
		char* name = (char*)item->data;
		if (strncmp(name,"IDM_",4) == 0 || (strncmp(name,"11",2) == 0 && strlen(name) == 4)) {
			char buff[128];
			sprintf(buff,"scite.MenuCommand(%s)",name); //
			luaL_dostring(L,buff);
		} else {
			lua_getglobal(L,name);
			if (lua_pcall(L,0,0,0)) {
				OutputMessage(L);
			}
		}
	}

};

class PanelWindow: public LuaWindow
{
public:
	PanelWindow(lua_State* l)
		: LuaWindow("",l,get_parent(),0,true)
	{}
};

class PaletteWindow;
static PaletteWindow* instances[50];
static int n_instances = 0;

class PaletteWindow: public LuaWindow
{
protected:
	bool m_shown;
public:
	PaletteWindow(const char* caption, lua_State* l, int stylex = 0, int style = -1)
		: LuaWindow(caption,l,NULL,stylex,false,style)
	{
		instances[n_instances++] = this;
		instances[n_instances] = NULL;
	}

	void show(int how = SW_SHOW)
	{
		TEventWindow::show(how);
		m_shown = true;
	}

	void hide()
	{
		TEventWindow::hide();
		m_shown = false;
	}

	virtual bool query_close()
	{
		hide();
		return false;
	}

	void really_show()
	{
		TEventWindow::show();
	}

	void really_hide()
	{
		TEventWindow::hide();
	}

	static void set_visibility(bool yesno)
	{
		for (int i = 0; instances[i]; i++) {
			PaletteWindow *w = instances[i];
			if (w->m_shown) {
				if (yesno) w->really_show();
				else w->really_hide();
			}
		}
	}

};

class ToolbarWindow: public PaletteWindow
{
public:
	ToolbarWindow(const char* caption, char** item, int sz, const char* path, lua_State* l)
		: PaletteWindow(caption,l,WS_EX_PALETTEWINDOW, WS_OVERLAPPED)
	{
		TToolbar tbar(this,sz,sz);
		tbar.set_path(path);
		for (;*item; item++) {
			char* img_text = strtok(*item,"|");
			char* fun = strtok(NULL,"");
			tbar << Item(img_text,(EH)&LuaWindow::handler,fun);
		}
		tbar.release();
		SIZE sze = tbar.get_size();
		client_resize(sze.cx, sze.cy + 10);
	}

};

////// This acts as the top-level frame window containing these controls; it supports
////// adding extra buttons and actions.
class ContainerWindow: public PaletteWindow
{
public:
	TListView* listv;
	const char* select_name;
	const char* double_name;

	ContainerWindow(const char* caption, lua_State* l)
		: PaletteWindow(caption,l),select_name(NULL),double_name(NULL)
	{
		set_icon_from_window(s_parent);
	}

	void dispatch(const char* name, int ival)
	{
		if (name != NULL) {
			lua_getglobal(L,name);
			lua_pushnumber(L,ival);
			if (lua_pcall(L,1,0,0)) {
				OutputMessage(L);
			}
		}
	}

	void on_button(Item* item)
	{
		dispatch((char*)item->data,0);  //listv->selected_id()
	}

	void add_buttons(lua_State* L)
	{
		int nargs = lua_gettop(L);
		int i = 2;
		TEW* panel = new TEW(NULL,this,0,true);
		panel->align(alBottom,50);
		Layout layout(panel,this);
		while (i < nargs) {
			layout << Button(luaL_checkstring(L,i),(EH)&ContainerWindow::on_button,(void*)luaL_checkstring(L,i+1));
			i += 2;
		}
		layout.release();
		add(panel);
		size();
	}

};

void dispatch_ref(lua_State* L,int idx, int ival)
{
	if (idx != 0) {
		lua_rawgeti(L,LUA_REGISTRYINDEX,idx);
		lua_pushnumber(L,ival);

		if (lua_pcall(L,1,0,0)) {
			OutputMessage(L);
		}
	}
}


bool function_ref(lua_State* L, int idx, int* pr)
{
	if (*pr != 0) {
		luaL_unref(L,LUA_REGISTRYINDEX,*pr);
	}
	lua_pushvalue(L,idx);
	*pr = luaL_ref(L,LUA_REGISTRYINDEX);
	return true;
}

class LuaControl
{
protected:
	lua_State *L;
	int select_idx;
	int double_idx;
	int onkey_idx;

public:
	LuaControl(lua_State *l)
		: L(l), select_idx(0), double_idx(0), onkey_idx(0)
	{}

	virtual void set_select(int iarg)
	{
		function_ref(L,iarg,&select_idx);
	}

	virtual void set_double_click(int iarg)
	{
		function_ref(L,iarg,&double_idx);
	}

	virtual void set_onkey(int iarg)
	{
		function_ref(L,iarg,&onkey_idx);
	}

};


class TListViewLua: public TListViewB, public LuaControl
{
public:
	TListViewLua(TWin *parent, lua_State *l,bool multiple_columns = false, bool single_select = true)
		: TListViewB(parent, false, multiple_columns, single_select),
		LuaControl(l)
	{
		if (! multiple_columns) {
			add_column("*",200);
		}
	}

	// implement
	virtual void handle_select(int id)
	{
		dispatch_ref(L,select_idx,id);
	}

	virtual void handle_double_click(int id)
	{
		dispatch_ref(L,double_idx,id);
	}

	virtual void handle_onkey(int id)
	{
		dispatch_ref(L,onkey_idx,id);
	}
};

class TTabControlLua: public TTabControlB, public LuaControl
{
	int selection_changing_idx;
	TEventWindow *form;
public:

	TTabControlLua (TEventWindow *parent,lua_State* l)
		: TTabControlB(parent),LuaControl(l),selection_changing_idx(0),form(parent)
	{
	}

	void set_selection_changing(int iarg)
	{
		function_ref(L,iarg,&selection_changing_idx);
	}

  // implement
	virtual void handle_select(int id)
	{
	  TWin *page = (TWin*)get_data(id);
	  form->set_client(page);
	  form->size();
	  dispatch_ref(L,select_idx,id);
	}

	virtual int handle_selection_changing(int id)
	{
	  if (selection_changing_idx) {
		  dispatch_ref(L,selection_changing_idx,id);
		  return 1;
	  } else {
		  return 0;
	  }
	}

};

struct WinWrap {
	TWin *window;
	void *data;

};

static int wrap_window(lua_State* L, TWin* win)
{
	WinWrap *wrp = (WinWrap*)lua_newuserdata(L,sizeof(WinWrap));
	wrp->window = win;
	wrp->data = NULL;
	luaL_getmetatable(L,WINDOW_CLASS);
	lua_setmetatable(L,-2);
	return 1;
}

static void throw_error(lua_State* L, const char *msg)
{
	lua_pushstring(L,msg);
	lua_error(L);
}

static TWin* window_arg(lua_State* L, int idx = 1)
{
	WinWrap *wrp = (WinWrap*)lua_touserdata(L,idx);
	if (! wrp) throw_error(L,"not a window");
	return (PaletteWindow*)wrp->window;
}

static void *& window_data(lua_State* L, int idx = 1)
{
	WinWrap *wrp = (WinWrap*)lua_touserdata(L,idx);
	if (! wrp) throw_error(L,"not a window");
	return wrp->data;
}


static char** table_to_str_array(lua_State *L, int idx, int* psz = NULL)
{
	if (! lua_istable(L,idx)) {
		throw_error(L,"table argument expected");
	}
    char** p = new char*[100];
    int i = 0;
    lua_pushnil(L); // first key
    while (lua_next(L, idx) != 0) {
         /* `key' is at index -2 and `value' at index -1 */
         p[i++] = strdup(lua_tostring(L,-1));
         lua_pop(L, 1);  /* removes `value'; keeps `key' for next iteration */
    }
    p[i] = NULL;  // conventional way of indicating end of string array
    if (psz) *psz = i;
    return p;
}

static int conv(const char* s, int i1)
{
	char args[] = {s[i1],s[i1+1],'\0'};
	int val;
	sscanf(args,"%X",&val);
	return val;
}

static unsigned int convert_colour_spec(const char* clrs)
{
	return RGB(conv(clrs,1),conv(clrs,3),conv(clrs,5));
}


static bool optboolean(lua_State* L, int idx, bool res)
{
	if (lua_isnoneornil(L,idx)) {
		return res;
	} else {
		return lua_toboolean(L,idx);
	}
}


///// Exported Lua functions ///////

/** gui.message(message_string, is_warning)
	@param message_string
	@param is_warning (0 for plain message, 1 for warning box)
	MSG_ERROR=2,MSG_WARNING=1, MSG_QUERY=3;
*/

int do_message(lua_State* L)
{
	const char* msg = luaL_checkstring(L,1);
	const char* kind = luaL_optstring(L,2,"message");
	int type = 0;
	if (EQ(kind,"message")) type = 0; else
	if (EQ(kind,"warning")) type = 1; else
	if (EQ(kind,"error")) type = 2; else
	if (EQ(kind,"query")) type = 3;
	lua_pushboolean(L,get_parent()->message(msg,type));
	return 1;
}


/** gui.prompt_value(prompt_string, default_value)
	@param prompt_string
	@param default_value (must be a string)
	@return string value if OK, nil if Cancel.
*/
int do_prompt_value(lua_State* L)
{
	const char* varname = luaL_checkstring(L,1);
	const char* value = luaL_optstring(L,2,"");
	PromptDlg dlg((TEventWindow*)get_desktop_window(),varname, value);
	if (dlg.show_modal()) {
		lua_pushstring(L,dlg.m_val);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int do_run(lua_State* L)
{
	const char* lpFile = luaL_checkstring(L,1);
	const char* lpParameters = lua_tostring(L,2);
	const char* lpDirectory = lua_tostring(L,3);
	int res = (int)ShellExecute (
		NULL,
		"open",
		lpFile,
		lpParameters,
		lpDirectory,
		SW_SHOWDEFAULT
	);
	if (res <= 32) {
		lua_pushboolean(L,0);
		lua_pushinteger(L,res);
		return 2;
	} else {
		lua_pushinteger(L,res);
		return 1;
	}
}

/** gui.colour_dlg(default_colour)
	@param default_colour  colour either in form '#RRGGBB" or as a 32-bit integer
	@return chosen colour, in same form as default_colour
*/


int do_colour_dlg(lua_State* L)
{
	bool in_rgb = lua_isstring(L,1);
	unsigned int cval;
	if (in_rgb) {
		cval = convert_colour_spec(lua_tostring(L,1));
	} else {
		cval = luaL_optinteger(L,1,0);
	}
	TColourDialog dlg(get_parent(),cval);
	if (dlg.go()) {
		cval = dlg.result();
		if (in_rgb) {
			char buff[12];
			sprintf(buff,"#%02X%02X%02X",GetRValue(cval),GetGValue(cval),GetBValue(cval));
			lua_pushstring(L,buff);
		} else {
			lua_pushnumber(L,cval);
		}
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/** gui.open_dlg(caption,filter)
	@param caption (defaults to "Open File")
	@param filter (defaults to "All (*.*)|*.*")
	@return chosen filename
*/
int do_open_dlg(lua_State* L)
{
	const char* caption = luaL_optstring(L,1,"Open File");
	const char* filter = luaL_optstring(L,2,"All (*.*)|*.*");
	TOpenFile tof (get_parent(),caption,filter);
	if (tof.go()) {
		lua_pushstring(L,tof.file_name());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int do_save_dlg(lua_State* L)
{
	const char* caption = luaL_optstring(L,1,"Save File");
	const char* filter = luaL_optstring(L,2,"All (*.*)|*.*");
	TSaveFile tof (get_parent(),caption,filter);
	if (tof.go()) {
		lua_pushstring(L,tof.file_name());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/** gui.select_dir_dlg(description,initialdir)
	@param description (defaults to empty string)
	@return chosen directory
*/
int do_select_dir_dlg(lua_State* L)
{
	const char* descr = luaL_optstring(L,1,"");
	const char* initdir = luaL_optstring(L,2,"");
	TSelectDir dir(get_parent(), descr, initdir);
	if (dir.go()) {
		lua_pushstring(L, dir.path());
	} else {
		lua_pushnil(L);
	}
	return 1;
}


int new_toolbar(lua_State* L)
{
	const char* caption = luaL_checkstring(L,1);
	char** items = table_to_str_array(L,2);
	int sz = luaL_optinteger(L,3,16);
	const char* path = lua_tostring(L,4);
	ToolbarWindow* ew = new ToolbarWindow(caption,items,sz,path,L);
	ew->show();
	delete[] items;
	return wrap_window(L,ew);
}

int new_window(lua_State* L)
{
	const char* caption = luaL_checkstring(L,1);
	ContainerWindow* cw = new ContainerWindow(caption,L);
	s_last_parent = cw;
	return wrap_window(L,cw);
}

int new_panel(lua_State* L)
{
	PanelWindow* pw = new PanelWindow(L);
	pw->align(alLeft,luaL_checkinteger(L,1));
	s_last_parent = pw;
	return wrap_window(L,pw);
}

int window_client(lua_State* L)
{
	TEventWindow *cw = (TEventWindow*)window_arg(L,1);
	TWin* child = window_arg(L,2);
	if (! child) throw_error(L,"must provide a child window");
	child->set_parent(cw);
	cw->set_client(child);
	return 0;
}

int window_add(lua_State* L)
{
	TEventWindow *cw = (TEventWindow*)window_arg(L,1);
	TWin* child = window_arg(L,2);
	const char *align = luaL_optstring(L,3,"client");
	bool splitter = optboolean(L,5,true);
	int sz = luaL_optinteger(L,4,100);
	child->set_parent(cw);
	if (EQ(align,"top")) {
		child->align(alTop,sz);
    } else
	if (EQ(align,"bottom")) {
		child->align(alBottom,sz);
	} else
	if (EQ(align,"left")) {
		child->align(alLeft,sz);
	} else
	if (EQ(align,"right")) {
		child->align(alRight,sz);
	} else {
		child->align(alClient,sz);
	}
	cw->add(child);
	if (splitter && child->align() != alClient) {
		TSplitter *split = new TSplitter(cw,child);
		cw->add(split);
		window_data(L,2) = split;
	}
	return 0;
}

int window_remove(lua_State* L)
{
	TEventWindow* form = (TEventWindow*)window_arg(L,1);
	form->remove(window_arg(L,2));
	TWin *split = (TWin*)window_data(L,2);
	if (split) {
		form->remove(split);
	}
	return 0;
}

int window_context_menu(lua_State* L)
{
	ContainerWindow *cw = (ContainerWindow*)window_arg(L,1);
	char** items = table_to_str_array(L,2);
	ContextMenu mnu(cw);
	for (;*items; items++) {
		char* text = strtok(*items,"|");
		char* fun = strtok(NULL,"");
		if ( ( text == 0 || *text == 0 )
			 && ( fun == 0 || *fun == 0 ) )
		{
			mnu.add_separator();
		}
		else
		{
			mnu << Item(text,(EH)&ContainerWindow::handler,fun);
		}
	}
	//mnu.release();
	return 0;
}


int new_tabbar(lua_State* L)
{
	TEventWindow* form = (TEventWindow*)window_arg(L,1);
	TTabControlLua *tab = new TTabControlLua(form,L);
	s_last_parent = form;
	tab->align(alTop);
	form->add(tab);
	return wrap_window(L,tab);
}

int tabbar_add(lua_State* L)
{
	TTabControl *tab = (TTabControl*)window_arg(L,1);
	tab->add(luaL_checkstring(L,2),window_arg(L,3));
	return 0;
}

int tabbar_set(lua_State* L)
{
	TTabControlLua *tab = (TTabControlLua*)window_arg(L,1);
	tab->selected(luaL_checkinteger(L,2));
	tab->handle_select(luaL_checkinteger(L,2));
	return 0;
}

int tabbar_selected(lua_State* L)
{
	TTabControl *tab = (TTabControl*)window_arg(L,1);
	lua_pushinteger(L,tab->selected());
	return 1;
}

int tabbar_count(lua_State* L)
{
	TTabControl *tab = (TTabControl*)window_arg(L,1);
	lua_pushinteger(L,tab->count());
	return 1;
}

int new_memo(lua_State* L)
{
	TMemo *m = new TMemo(get_last_parent(),1);
	return wrap_window(L,m);
}

int memo_set(lua_State* L)
{
	TMemo *m = (TMemo*)window_arg(L,1);
	m->set_text(luaL_checkstring(L,2));
	return 0;
}

int memo_set_colour(lua_State* L)
{
	TMemo *m = (TMemo*)window_arg(L,1);
	m->set_text_colour(convert_colour_spec(luaL_checkstring(L,2)));
	m->set_background_colour(convert_colour_spec(luaL_checkstring(L,3)));

	return 0;
}


/** lw:list(multiple_columns,multiple_selection)
	@param multiple_columns (default false)
	@param single_selection (default true)
	@return new ListWindow instance.
*/
int new_list_window(lua_State* L)
{
	bool multiple_columns = optboolean(L,1,false);
	bool single_select = optboolean(L,2,true);
	TListViewLua *lv = new TListViewLua(get_last_parent(),L,multiple_columns,single_select);
	return wrap_window(L,lv);
}

//!-begin-[add treeview]

class TTreeViewlua: public TTreeView, public LuaControl
{
public:
	TTreeViewlua(TEventWindow* parent,lua_State *l,bool has_lines = false, bool editable = false)
		: TTreeView(parent, has_lines, editable),
		LuaControl(l)
	{

	}
	// implement
	virtual void handle_select(int item)
	{
		dispatch_ref(L,select_idx,item);
	}

	virtual void handle_double_click(int item)
	{
		dispatch_ref(L,double_idx,item);
	}

	virtual void handle_onkey(int item)
	{
		dispatch_ref(L,onkey_idx,item);
	}
};

int new_tree_window(lua_State* L)
{
	bool has_lines = optboolean(L,1,false);
	bool editable  = optboolean(L,2,false);
	TTreeViewlua *tv = new TTreeViewlua((TEventWindow*)get_last_parent(),L,has_lines,editable);
	return wrap_window(L,tv);
}

TTreeViewlua* tree_window_arg(lua_State* L)
{
	return (TTreeViewlua*)(window_arg(L));
}

int window_add_tree(lua_State* L)
{
	void* data = NULL;
	int parent = luaL_checkinteger(L,2);
	const char* caption = luaL_checkstring(L,3);
	bool hasChildren = optboolean(L,4,false);
	TTreeViewlua* tv = tree_window_arg(L);
	if (! lua_isnoneornil(L,5)) {
		lua_pushvalue(L,5);
		data = (void*)luaL_ref(L,LUA_REGISTRYINDEX);
	}
	Handle hd = tv->add((Handle)parent,caption,0,-1,hasChildren,data);
	lua_pushinteger(L,(int)hd);
	return 1;
}

int window_get_tree_data(lua_State* L)
{
	Handle item;
	if (lua_isnoneornil(L,2)) {
		item = tree_window_arg(L)->get_selection();
	} else {
		item = (Handle) luaL_checkinteger(L,2);
	}
	if (item) {
		void *data = tree_window_arg(L)->get_data(item);
		lua_rawgeti(L,LUA_REGISTRYINDEX,(int)data);
	}
	return 1;
}

int window_get_selected_tree(lua_State* L)
{
	Handle item = tree_window_arg(L)->get_selection();
 	lua_pushinteger(L,(int)item);
	return 1;
}

int window_set_selected_tree(lua_State* L)
{
	Handle item = (Handle) luaL_checkinteger(L,2);
	tree_window_arg(L)->select(item);
	return 1;
}

int window_get_tree_parent(lua_State* L)
{
	Handle item,hItem;
	if (lua_isnoneornil(L,2)) {
		item = tree_window_arg(L)->get_selection();
	} else {
		item = (Handle) luaL_checkinteger(L,2);
	}
	hItem = tree_window_arg(L)->get_parent(item);
	lua_pushinteger(L,(int)hItem);
	return 1;
}

int window_toggle_tree(lua_State* L)
{
	Handle item;
	if (lua_isnoneornil(L,2)) {
		item = tree_window_arg(L)->get_selection();
	} else {
		item = (Handle) luaL_checkinteger(L,2);
	}
	tree_window_arg(L) ->toggle(item);
	return 1;
}

int window_expand_tree(lua_State* L)
{
	Handle item = (Handle) luaL_checkinteger(L,2);
	bool flag = lua_toboolean(L,3);
    tree_window_arg(L)->expand(item,flag);
	return 1;
}

int window_clear_tree(lua_State* L)
{
	tree_window_arg(L) ->clear();
	return 1;
}

int tree_set_colour(lua_State* L)
{
	tree_window_arg(L)->set_foreground(convert_colour_spec(luaL_checkstring(L,2)));
	tree_window_arg(L)->set_background(convert_colour_spec(luaL_checkstring(L,3)));
	return 0;
}

//!-end-[add treeview]


/** w:show()
	@param self
*/
int window_show(lua_State* L)
{
	window_arg(L)->show();
	return 0;
}

/** w:hide()
	@param self
*/
int window_hide(lua_State* L)
{
	window_arg(L)->hide();
	return 0;
}

/** w:size()
	@param self
	@param width
	@param height
*/
int window_size(lua_State* L)
{
	int w = luaL_checkinteger(L,2);
	int h = luaL_checkinteger(L,3);
	window_arg(L)->resize(w,h);
	return 0;
}

/** w:size()
	@param self
	@param x
	@param y
*/
int window_position(lua_State* L)
{
	int x = luaL_checkinteger(L,2);
	int y = luaL_checkinteger(L,3);
	window_arg(L)->move(x,y);
	return 0;
}

int window_get_bounds(lua_State* L)
{
	TWin *win = (TWin*)window_arg(L);
	Rect rt;
	win->get_rect(rt);
	lua_pushboolean(L,win->visible());
	lua_pushinteger(L,rt.left);
	lua_pushinteger(L,rt.right);
	lua_pushinteger(L,rt.width());
	lua_pushinteger(L,rt.height());
	return 5;
}


TListViewLua* list_window_arg(lua_State* L)
{
//	ListWindow* lw = dynamic_cast<ListWindow*>(window_arg(L));
//	if (! lw) lua_error(L);
//	return lw->listv;
	return (TListViewLua*)(window_arg(L));
}

/** w:add_column()
	@param self
	@param column_title
	@param column_size
*/
int window_add_column(lua_State* L)
{
	list_window_arg(L)->add_column(luaL_checkstring(L,2),luaL_checkinteger(L,3));
	return 0;
}

void window_aux_item(lua_State* L, bool at_index)
{
	TListViewLua* lv = list_window_arg(L);
	int next_arg,ipos;
	void *data = NULL;
	if (at_index) {
		next_arg = 3;
		ipos = luaL_checkinteger(L,2);
	} else {
		next_arg = 2;
		ipos = lv->count();
	}
	if (! lua_isnoneornil(L,next_arg+1)) {
		lua_pushvalue(L,next_arg+1);
		data = (void*)luaL_ref(L,LUA_REGISTRYINDEX);
	}
	if (lua_isstring(L,next_arg)) {
		lv->add_item_at(ipos,luaL_checkstring(L,next_arg),0,data);
	} else {
		char** items = table_to_str_array(L,next_arg);
		int i = 0, ncol = lv->columns();
		int idx = lv->add_item_at(ipos,*items,0,data);
		++items;
		++i;
		for(; *items && i < ncol; ++items) {
			lv->add_subitem(idx,*items,i);
			++i;
		}
	}
}

/** w:add_item()
	@param self
	@param item_string
*/
int window_add_item(lua_State* L)
{
	window_aux_item(L,false);
	return 0;
}

/** w:insert_item()
	@param self
	@param item_string
	@param index
*/
int window_insert_item(lua_State* L)
{
	window_aux_item(L,true);
	return 0;
}

/** w:delete_item()
	@param self
	@param index
*/
int window_delete_item(lua_State* L)
{
	list_window_arg(L)->delete_item(luaL_checkinteger(L,2));
	return 0;
}

int window_count (lua_State* L)
{
	int sz = list_window_arg(L)->count();
	lua_pushinteger(L,sz);
	return 1;
}

int window_set_colour(lua_State* L)
{
	TListViewLua* ls = list_window_arg(L);
	ls->set_foreground(convert_colour_spec(luaL_checkstring(L,2)));
	ls->set_background(convert_colour_spec(luaL_checkstring(L,3)));
	return 0;
}


/** w:get_item_text()
	@param self
	@param index
	@return item string
*/
int window_get_item_text(lua_State* L)
{
	char* buff = new char[BUFFER_SIZE];
	list_window_arg(L)->get_item_text(luaL_checkinteger(L,2),buff,BUFFER_SIZE);
	lua_pushstring(L,buff);
	delete[] buff;
	return 1;
}

int window_get_item_data(lua_State* L)
{
	void *data = list_window_arg(L)->get_item_data(luaL_checkinteger(L,2));
	lua_rawgeti(L,LUA_REGISTRYINDEX,(int)data);
	return 1;
}

int window_selected_item(lua_State* L)
{
	int idx =list_window_arg(L)->selected_id();
	lua_pushinteger(L,idx);
	return 1;
}

int window_select_item(lua_State* L)
{
	list_window_arg(L)->select_item(luaL_checkinteger(L,2));
	return 0;
}

/** w:clear()
	@param self
*/
int window_clear(lua_State* L)
{
	list_window_arg(L)->clear();
	return 0;
}

/** w:autosize()
	@param self
	@param column_index
	@param by_contents (default false)
*/
int window_autosize(lua_State* L)
{
	list_window_arg(L)->autosize_column(luaL_checkinteger(L,2),optboolean(L,3,false));
	return 0;
}


/** w:on_select()
	@param self
	@param select_callback (string representing global function)
*/
int window_on_select(lua_State* L)
{
	LuaControl* lc = dynamic_cast<LuaControl*>(window_arg(L,1));
	lc->set_select(2);
	return 0;
}

/** w:on_double_click()
	@param self
	@param select_callback (string representing global function)
*/
int window_on_double_click(lua_State* L)
{
	//list_window_arg(L)->set_double_click(2);
	LuaControl* lc = dynamic_cast<LuaControl*>(window_arg(L,1));
	lc->set_double_click(2);
	return 0;
}

int window_on_key(lua_State* L)
{
	//list_window_arg(L)->set_onkey(2);
	LuaControl* lc = dynamic_cast<LuaControl*>(window_arg(L,1));
	lc->set_onkey(2);
	return 0;
}

int window_add_buttons(lua_State* L)
{
	ContainerWindow* lw = dynamic_cast<ContainerWindow*>(window_arg(L));
	lw->add_buttons(L);
	return 0;
}


static HWND hSciTE = NULL, hContent = NULL, hCode;
static WNDPROC old_scite_proc, old_scintilla_proc, old_content_proc;
static lua_State *sL;
static TWin *code_window = NULL;
static TWin *extra_window = NULL;
static TWin *content_window = NULL;
static TWin *extra_window_splitter = NULL;
static bool forced_resize = false;
static Rect m, cwb, extra;

static bool eq(const Rect& r1, const Rect& r2)
{
	return r1.left == r2.left && r1.top == r2.top && r1.right == r2.right && r1.bottom == r2.bottom;
}


static void force_contents_resize()
{
	// get the code pane's extents, and don't try to resize it again!
	code_window->get_rect(m,true);
	if (eq(cwb,m)) return;
	int oldw = m.width();
	int w = extra_window->width();
	int h = m.height();
	int sw = extra_window_splitter->width();
	extra = m;
	cwb = m;
	if (extra_window->align() == alLeft) {
		// on the left goes the extra pane, followed by the splitter
		extra.right = extra.left + w;
		extra_window->resize(m.left,m.top,w,h);
		extra_window_splitter->resize(m.left + w,m.top,sw,h);
		cwb.left += w + sw;
	} else {
		int margin = m.right - w;
		extra.left = margin;
		extra_window->resize(margin,m.top, w,h);
		extra_window_splitter->resize(margin-sw,m.top,sw,h);
		cwb.right -= w + sw;
	}
	// and then the code pane; note the hack necessary to prevent a nasty recursion here.
	forced_resize = true;
	code_window->resize(cwb);
	forced_resize = false;
}

static int call_named_function(lua_State* L, const char *name, int arg)
{
	int ret = 0;
	lua_getglobal(L,name);
	if (lua_isfunction(L,-1)) {
		lua_pushinteger(L,arg);
		if (lua_pcall(L,1,1,0)) {
			OutputMessage(L);
		} else {
			ret = lua_toboolean(L,-1);
			lua_pop(L,1);
		}
	}
	lua_pop(L,1);
	return ret;
}

// we subclass the main SciTE window proc mostly because we need to track whether
// SciTE is the active app or not, so that toolwindows can be hidden.
static LRESULT SciTEWndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	if (iMessage == WM_ACTIVATEAPP) {
		PaletteWindow::set_visibility(wParam);
		call_named_function(sL,"OnActivate",wParam);
		if (wParam) { // floating toolbars may grab the focus, so restore it.
			code_window->set_focus();
		}
	} else
	if (iMessage == WM_CLOSE) {
		call_named_function(sL,"OnClosing",0);
	} else
	if (iMessage == WM_COMMAND) {
		if (call_named_function(sL,"OnCommand",LOWORD(wParam))) return TRUE;
	}
	return CallWindowProc(old_scite_proc,hwnd,iMessage,wParam,lParam);
}

// we are interested in any attempts to resize the main code pane, because we
// may wish to place our own pane on the left.
static LRESULT ScintillaWndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	if (iMessage == WM_SIZE) {
		if (extra_window) {
			if (! forced_resize) {
				force_contents_resize();
			}
		}
	}
	if (IsWindowUnicode(hwnd)) {
		return CallWindowProcW(old_scintilla_proc,hwnd,iMessage,wParam,lParam);
	} else {
		return CallWindowProcA(old_scintilla_proc,hwnd,iMessage,wParam,lParam);
	}
}

// the content pane contains the two Scintilla windows (editor and output).
// This subclass prevents SciTE from forcing its dragger cursor onto our left pane.
static LRESULT ContentWndProc(HWND hwnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	if (iMessage == WM_SETCURSOR) {
		Point ptCursor;
		GetCursorPos(&ptCursor);
		Point ptClient = ptCursor;
		ScreenToClient(hContent, &ptClient);
		if (extra.is_inside(ptClient)) {
			return DefWindowProc(hSciTE,iMessage,wParam,lParam);
		}
	}
	return CallWindowProc(old_content_proc,hwnd,iMessage,wParam,lParam);
}

#ifndef GetWindowLongPtrW
#define GetWindowLongPtrW(hwnd,offs) (void*)GetWindowLongW(hwnd,offs)
#define GetWindowLongPtrA(hwnd,offs) (void*)GetWindowLongA(hwnd,offs)
#define SetWindowLongPtrW(hwnd,offs,newv) (void*)SetWindowLongW(hwnd,offs,newv)
#define SetWindowLongPtrA(hwnd,offs,newv) (void*)SetWindowLongA(hwnd,offs,newv)
#endif


static WNDPROC subclass(HWND hwnd, LONG_PTR newproc)
{
	WNDPROC old;
	if (::IsWindowUnicode(hwnd)) {
		old = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hwnd, GWLP_WNDPROC));
		SetWindowLongPtrW(hwnd, GWLP_WNDPROC, newproc);
	} else {
		old = reinterpret_cast<WNDPROC>(GetWindowLongPtrA(hwnd, GWLP_WNDPROC));
		SetWindowLongPtrA(hwnd, GWLP_WNDPROC, newproc);
	}
	return old;
}

static void subclass_scite_window ()
{
	static bool subclassed = false;
	if (!subclassed) {  // to prevent a recursion
		old_scite_proc     = subclass(hSciTE,   (long)SciTEWndProc);
		old_content_proc   = subclass(hContent, (long)ContentWndProc);
		old_scintilla_proc = subclass(hCode,    (long)ScintillaWndProc);
		subclassed = true;
	}
}

// this terrible hack gets a SciTE resize by 'shaking the frame' by a little bit
static void shake_scite_window()
{
	static int delta = 1;
	Rect frt;
	s_parent->get_rect(frt,false);
	frt.right += delta;
	delta = - delta;
	s_parent->resize(frt);
}

// Unfortunately, shake_scite_window() does not work in the full screen mode.
// So here is yet another hack, which seems even more terrible than the first one,
// since its workability completely depends on WM_SIZE handler implementation
// in SciTEWin.
static void shake_scite_descendants()
{
	Rect frt;
	s_parent->get_rect(frt,false);
	s_parent->send_msg(WM_SIZE, SIZE_RESTORED, MAKELONG(frt.width(), frt.height()));
}

class SideSplitter: public TSplitterB
{
public:
	SideSplitter(TEventWindow *form, TWin *control)
		: TSplitterB(form,control,5)
	{}

	void paint(TDC& dc)
	{
		Rect rt(this);
		dc.rectangle(rt);
	}


	void on_resize(const Rect& rt)
	{
		TSplitterB::on_resize(rt);
		shake_scite_descendants();
	}
};

static int do_set_panel(lua_State *L)
{
	if (content_window == NULL) {
		lua_pushstring(L,"Window subclassing was not successful");
		lua_error(L);
	}
	if (! lua_isuserdata(L,1) && extra_window != NULL) {
		extra_window->hide();
		extra_window = NULL;
		extra_window_splitter->close();
		delete extra_window_splitter;
		extra_window_splitter = NULL;
		shake_scite_descendants();
	} else {
		extra_window = window_arg(L);
		const char *align = luaL_optstring(L,2,"left");
		if (EQ(align,"left")) {
			extra_window->align(alLeft);
		} else
		if (EQ(align,"right")) {
			extra_window->align(alRight);
		}
		extra_window->set_parent(content_window);
		extra_window->show();
		extra_window_splitter = new SideSplitter((TEventWindow*)content_window, extra_window);
		extra_window_splitter->show();
		force_contents_resize();
	}

	return 0;
}

static int do_chdir(lua_State *L)
{
	const char *dirname = luaL_checkstring(L,1);
	int res = _chdir(dirname);
	lua_pushboolean(L,res == 0);
	return 1;
}

static int append_file(lua_State *L, int idx,int attrib,bool look_for_dir, const char *value)
{
	if (((attrib & _A_SUBDIR) != 0) == look_for_dir) {
		if (look_for_dir && (EQ(value,".") || EQ(value,".."))) return idx;
		lua_pushinteger(L,idx);
		lua_pushstring(L,value);
		lua_settable(L,-3);
		return idx + 1;
	}
	return idx;
}

static int do_files(lua_State *L)
{
	struct _finddata_t c_file;
	const char *mask = luaL_checkstring(L,1);
	bool look_for_dir = optboolean(L,2,false);
	long hFile = _findfirst(mask,&c_file);
	int i = 1;
	if (hFile == -1L) { lua_pushboolean(L,0); return 1; }
	lua_newtable(L);
	i = append_file(L,i,c_file.attrib,look_for_dir,c_file.name);
	while( _findnext( hFile, &c_file ) == 0) {
		i = append_file(L,i,c_file.attrib,look_for_dir,c_file.name);
	}
	return 1;
}

//  this will allow us to hand keyboard focus over to editor
static int do_pass_focus(lua_State *L)
{
	lua_getglobal(L,"editor");
	lua_pushboolean(L,1);
	lua_setfield(L,-2,"Focus");
	lua_pop(L,1);
	if (code_window) {
		code_window->set_focus();
	}
	return 0;
}

static const struct luaL_reg gui[] = {
	{"message",do_message},
	{"colour_dlg",do_colour_dlg},
	{"open_dlg",do_open_dlg},
	{"save_dlg",do_save_dlg},
	{"select_dir_dlg",do_select_dir_dlg},
	{"toolbar",new_toolbar},
	{"window",new_window},
	{"panel",new_panel},
	{"tabbar",new_tabbar},
	{"list",new_list_window},
	{"tree",new_tree_window},
	{"memo",new_memo},
	{"prompt_value",do_prompt_value},
	{"run",do_run},
	{"chdir",do_chdir},
	{"files",do_files},
	{"set_panel",do_set_panel},
	{"pass_focus",do_pass_focus},
	{NULL, NULL},
};

static const struct luaL_reg window_methods[] = {
	{"show",window_show},
	{"hide",window_hide},
	{"size",window_size},
	{"position",window_position},
	{"bounds",window_get_bounds},
	{"client",window_client},
	{"add",window_add},
	{"remove",window_remove},
	{"context_menu",window_context_menu},
	{"add_column",window_add_column},
	{"add_item",window_add_item},
	{"add_tree",window_add_tree},
	{"insert_item",window_insert_item},
	{"delete_item",window_delete_item},
	{"count",window_count},
	{"get_item_text",window_get_item_text},
	{"get_item_data",window_get_item_data},
	{"get_tree_data",window_get_tree_data},
	{"get_tree_parent",window_get_tree_parent},
	{"toggle",window_toggle_tree},
	{"expand",window_expand_tree},
	{"get_selected_item",window_selected_item},
	{"get_selected_tree",window_get_selected_tree},
	{"set_selected_tree",window_set_selected_tree},
	{"set_selected_item",window_select_item},
	{"on_select",window_on_select},
	{"on_double_click",window_on_double_click},
	{"on_key",window_on_key},
	{"clear",window_clear},
	{"clear_tree",window_clear_tree},
	{"autosize",window_autosize},
	{"add_buttons",window_add_buttons},
	{"add_tab",tabbar_add},
	{"set_tab",tabbar_set},
	{"tab_selected",tabbar_selected},
	{"tab_count",tabbar_count},
	{"set_text",memo_set},
	{"set_memo_colour",memo_set_colour},
	{"set_list_colour",window_set_colour},
	{"set_tree_colour",tree_set_colour},
	{NULL, NULL},
};

BOOL CALLBACK CheckSciteWindow(HWND  hwnd, LPARAM  lParam)
{
	char buff[120];
    GetClassName(hwnd,buff,sizeof(buff));
    if (strcmp(buff,"SciTEWindow") == 0) {
		*(HWND *)lParam = hwnd;
		return FALSE;
    }
    return TRUE;
}

BOOL CALLBACK CheckContainerWindow(HWND  hwnd, LPARAM  lParam)
{
	char buff[120];
    GetClassName(hwnd,buff,sizeof(buff));
    if (strcmp(buff,"SciTEWindowContent") == 0) {
		*(HWND *)lParam = hwnd;
		return FALSE;
    }
    return TRUE;
}

void destroy_windows()
{
	if (extra_window) {
		extra_window->hide();
		extra_window->set_parent(0);
		extra_window->close();
		delete extra_window;
		extra_window = 0;
	}
	if (extra_window_splitter) {
		extra_window_splitter->hide();
		extra_window_splitter->set_parent(0);
		extra_window_splitter->close();
		delete extra_window_splitter;
		extra_window_splitter = 0;
	}
	extra.bottom = extra.top = extra.left = extra.right = 0;
	shake_scite_descendants();
}

extern "C" __declspec(dllexport)
int luaopen_gui(lua_State *L)
{
	// at this point, the SciTE window is available. Can't always assume
	// that it is the foreground window, so we hunt through all windows
	// associated with this thread (the main GUI thread) to find a window
	// matching the appropriate class name
	EnumThreadWindows(GetCurrentThreadId(),CheckSciteWindow,(long)&hSciTE);
	s_parent = new TWin(hSciTE);
	sL = L;

	// Destroy window hierarchy created before.
	// And there are still memory and handle leaks.
	destroy_windows();

	// Its first child shold be the content pane (editor+output),
	// but we check this anyway....
	EnumChildWindows(hSciTE,CheckContainerWindow,(long)&hContent);
	// the first child of the content pane is the editor pane.
	bool subclassed = false;
	if (hContent != NULL) {
		content_window = new TWin(hContent);
		hCode = GetWindow(hContent,GW_CHILD);
		if (hCode != NULL) {
			code_window = new TWin(hCode);
			subclass_scite_window();
			subclassed = true;
		}
	}
	if (! subclassed) {
		get_parent()->message("Cannot subclass SciTE Window",2);
	}
	luaL_openlib (L, "gui", gui, 0);
	luaL_newmetatable(L, WINDOW_CLASS);  // create metatable for window objects
	lua_pushvalue(L, -1);  // push metatable
	lua_setfield(L, -2, "__index");  // metatable.__index = metatable
	luaL_register(L, NULL, window_methods);
	return 1;
}
