// twl_toolbar.cpp
// implements the following common controls
//   TToolBar
//   TImageList
//   TTabControl
//   TListView
//   TTreeView
#include <windows.h>
#include <commctrl.h>
#include "twl_toolbar.h"
#define IS_IMPLEMENTATION
#include "twl_imagelist.h"
#include "twl_listview.h"
#include "twl_tab.h"
#include "twl_treeview.h"
#include <string.h>
#include <stdio.h>
#include <io.h>
#include "vc6.h"

// these aren't found in the mingw headers...
#ifndef TBSTYLE_FLAT
#define TBSTYLE_FLAT  0x0800
#endif
#ifndef TB_GETMAXSIZE
#define TB_GETMAXSIZE   (WM_USER + 83)
#endif
#ifndef LVM_SETEXTENDEDLISTVIEWSTYLE
#define LVM_SETEXTENDEDLISTVIEWSTYLE (LVM_FIRST + 54)
#endif
#ifndef LVS_EX_FULLROWSELECT
#define LVS_EX_FULLROWSELECT  0x00000020
#endif
#ifndef CDDS_PREPAINT
#define CDDS_PREPAINT           0x00000001
#define CDRF_NOTIFYITEMDRAW     0x00000020
#define CDDS_ITEM               0x00010000
#define CDDS_ITEMPREPAINT       (CDDS_ITEM | CDDS_PREPAINT)
#define CDRF_NEWFONT            0x00000002
#endif

void *ApplicationInstance();

static int gID = 445560;

static HWND create_common_control(TWin* form, const char* winclass, int style, int height = -1)
{
    int w = CW_USEDEFAULT, h = CW_USEDEFAULT;
    if (height != -1) { w = 100; h = height; }
    return CreateWindowEx( 0L,   // No extended styles.
       winclass,"",WS_CHILD | style,
       0, 0, w, h,
       (HWND)form->handle(),                  // Parent window of the control.
       (HMENU)(void*)gID++,
       (HINSTANCE)ApplicationInstance(),             // Current instance.
       NULL );
}

struct StdItem {
    const char* name;
    int idx;
};
static StdItem std_items[] = {
    {"COPY",STD_COPY},
    {"CUT",STD_CUT},
    {"DELETE",STD_DELETE},
    {"FILENEW",STD_FILENEW},
    {"FILEOPEN",STD_FILEOPEN},
    {"FILESAVE",STD_FILESAVE},
    {"FIND",STD_FIND},
    {"HELP",STD_HELP},
    {"PASTE",STD_PASTE},
    {"PRINT",STD_PRINT},
    {"PRINTPRE",STD_PRINTPRE},
    {"PROPERTIES",STD_PROPERTIES},
    {"REDOW",STD_REDOW},
    {"REPLACE",STD_REPLACE},
    {"UNDO",STD_UNDO},
    {"_DETAILS",VIEW_DETAILS},
    {"_LARGEICONS",VIEW_LARGEICONS},
    {"_LIST",VIEW_LIST},
    {"_SMALLICONS",VIEW_SMALLICONS},
    {"_SORTDATE",VIEW_SORTDATE},
    {"_SORTNAME",VIEW_SORTNAME},
    {"_SORTSIZE",VIEW_SORTSIZE},
    {"_SORTTYPE",VIEW_SORTTYPE},
    {NULL,-1}
};

TToolbar::TToolbar(TEventWindow* form, int bwidth, int bheight, TEventWindow* c)
 : m_form(form),m_bwidth(bwidth),m_bheight(bheight),m_container(c),m_path(NULL)
{
      m_menu_handler = new MessageHandler(form);
	  if (m_container == NULL)
		  m_container = form;
      create();
}

TToolbar::~TToolbar()
{
	if (m_container != NULL)
		release();
}

void TToolbar::create()
{
     int style = WS_BORDER | WS_VISIBLE | TBSTYLE_TOOLTIPS | TBSTYLE_FLAT;
     set(create_common_control(m_container,TOOLBARCLASSNAME,style,150));

     send_msg(TB_BUTTONSTRUCTSIZE,sizeof(TBBUTTON));

     // add the standard bitmaps
     TBADDBITMAP std_bitmaps;
     std_bitmaps.hInst = HINST_COMMCTRL;
     std_bitmaps.nID = IDB_STD_SMALL_COLOR;
     int idx = send_msg(TB_ADDBITMAP,1,(long)&std_bitmaps);

	 send_msg(TB_SETBITMAPSIZE,0,MAKELONG(m_bwidth,m_bheight));

}

static HICON load_icon(const char* file)
{
  return (HICON)LoadImage( (HINSTANCE)ApplicationInstance(),file,IMAGE_ICON,0,0,LR_LOADFROMFILE | LR_LOADTRANSPARENT);
}

static HBITMAP load_bitmap (const char* file)
{
  HBITMAP res = (HBITMAP)LoadImage(0/*ApplicationInstance()*/,file,IMAGE_BITMAP,0,0,LR_LOADFROMFILE | LR_LOADTRANSPARENT);
  if (! res) {
	int res = access(file,0);
	int err = GetLastError();
	err = err - 1;
  }
  return res;
}

SIZE TToolbar::get_size()
{
	SIZE sz;
	send_msg(TB_GETMAXSIZE,0,(int)&sz);
	return sz;
}


void TToolbar::set_path(const char* path)
{
	m_path = path;
}

void TToolbar::add_item(const char* bmp, const char* tooltext, EventHandler eh, void* data)
{
     int idx = 0;
     if (bmp) {
       int std_id = -1;
       StdItem* pi = std_items;
       while (pi->name != NULL) {
         if (strcmp(pi->name,bmp)==0) break;
         pi++;
       }
       idx = pi->idx;
       if (idx == -1) { // wasn't a standard toolbar button
		 char bmpfile[256];
		 if (m_path != NULL) {
			sprintf(bmpfile,"%s\\%s",m_path,bmp);
		 } else {
			 strcpy(bmpfile,bmp);
		 }
		 int sz = strlen(bmpfile);
		 if (strncmp(bmpfile+sz-4,".bmp",4) != 0) {
			 strcat(bmpfile,".bmp");
		 }
         HANDLE hBitmap = load_bitmap(bmpfile);
         TBADDBITMAP bitmap;
         bitmap.hInst = NULL; // i.e we're passing a bitmap handle
         bitmap.nID = (unsigned int)hBitmap;
         idx = send_msg(TB_ADDBITMAP,1,(long)&bitmap);
       }
     }
     Item item(0,eh,data,-1);
     if (bmp)
       m_menu_handler->add(item);

     TBBUTTON btn;
     btn.iBitmap = idx;
     btn.idCommand = bmp ? item.id : 0;
     btn.fsStyle = bmp ? TBSTYLE_BUTTON : TBSTYLE_SEP;
     btn.fsState = TBSTATE_ENABLED;
     btn.dwData = (long)tooltext;
     btn.iString = 0;
     int ret = send_msg(TB_ADDBUTTONS,1,(long)&btn);


}

class TToolBarToolTipNotifyHandler: public TNotifyWin {
  TWin* m_tb;
public:
    TToolBarToolTipNotifyHandler(TWin* tb, Handle tth)
        : m_tb(tb)
    {
       set(tth);
    }

    int handle_notify(void *p)
    {
      LPTOOLTIPTEXT lpToolTipText = (LPTOOLTIPTEXT)p;
      if (lpToolTipText->hdr.code ==  TTN_GETDISPINFO) {
          TBBUTTON btn;
          int id = lpToolTipText->hdr.idFrom;                   // command id
          int index  = m_tb->send_msg(TB_COMMANDTOINDEX,id);    // index in the buttons
          m_tb->send_msg(TB_GETBUTTON,index,(long)&btn);        // get the button
          // and pick up the text we left there
          lpToolTipText->hinst = NULL;
          lpToolTipText->lpszText = lpToolTipText->szText;
		  if (btn.dwData == 0)
			  return 0;
          strcpy(lpToolTipText->szText,(char*)btn.dwData);
      }
      return 1;
    }
};

void TToolbar::release()
{
     if (!m_container) return;
     send_msg(TB_AUTOSIZE);
     HWND hToolTip = (HWND)send_msg(TB_GETTOOLTIPS);
     TWin* tb = new TWin(handle());
     m_container->set_toolbar(tb,new TToolBarToolTipNotifyHandler(tb,hToolTip));
     m_container->add_handler(m_menu_handler);
	 m_container = NULL;
}


EXPORT TToolbar& operator<< (TToolbar& tb, Item item)
{
    char* caption = strdup((char*)item.caption); // hack
    char* bmp_file = strtok(caption,":");
    char* tooltip_text = strtok(NULL,"");
    tb.add_item(bmp_file,tooltip_text,item.handler,item.data);
    return tb;
}

EXPORT TToolbar& operator<< (TToolbar& tb, Sep sep)
{
    tb.add_item(NULL,NULL,NULL);
    return tb;
}


///// TImageList class

static int icon_size(bool s)
{
	return s ? 16 : 32;
}

TImageList::TImageList(int cx, int cy)
{
   create(cx,cy);
   m_small_icons = cx == icon_size(true);
}

TImageList::TImageList(bool s /*= true*/)
: m_small_icons(s)
{
 int cx = icon_size(s);
 int cy = cx;
 create(cx,cy);
}

void TImageList::create(int cx, int cy)
{
	m_handle = ImageList_Create(cx,cy,ILC_COLOR32 | ILC_MASK,0,32);
}

int TImageList::add_icon(const char* iconfile)
{
	  HICON hIcon = load_icon(iconfile);
	  if (! hIcon) return -1;  // can't find icon
	  return ImageList_AddIcon(m_handle,hIcon);
}

int TImageList::add(const char* bitmapfile, long mask_clr)
{
	  HBITMAP hBitmap = load_bitmap(bitmapfile);
	  if (! hBitmap) return -1;  // can't find bitmap
	  if (mask_clr != 1)
	    return ImageList_AddMasked(m_handle,hBitmap,mask_clr);
	  else
        return ImageList_Add(m_handle,hBitmap,NULL);
}

int TImageList::load_icons_from_module(const char* mod)
{
 HINSTANCE hInst = GetModuleHandle(mod);
 HICON hIcon;
 int cx = icon_size(m_small_icons);
 int cy = cx;
 int i = 1;
 while (
     (hIcon = (HICON)LoadImage(hInst, (const char*)(i++), IMAGE_ICON, cx, cy, LR_LOADMAP3DCOLORS))
     != NULL)
	    ImageList_AddIcon(m_handle,hIcon);
 return i;
}

void TImageList::set_back_colour(long clrRef)
{
 ImageList_SetBkColor(m_handle, clrRef);
}

void TImageList::load_shell_icons()
{
 load_icons_from_module("shell32.dll");
 set_back_colour(CLR_NONE);
}

////// TListView

TListViewB::TListViewB(TWin* form, bool large_icons, bool multiple_columns, bool single_select)
{
	int style = WS_CHILD;
	if ( large_icons ) {
		style |= ( LVS_ICON | LVS_AUTOARRANGE );
	}
	else {
		style |= LVS_REPORT;
		if ( single_select ) {
			style |= LVS_SINGLESEL;
		}
		if ( !multiple_columns ) {
			style |= LVS_NOCOLUMNHEADER;
			//add_column("*",1000);
		}
	 }

	// Create the list view control.
	set(create_common_control(form,WC_LISTVIEW,style));
	m_custom_paint = false;
	m_has_images = false;
	m_last_col = 0;
	m_last_row = -1;

	send_msg( LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT ); // Set style
}

void TListViewB::set_image_list(TImageList* il_small, TImageList* il_large)
{
	   if (il_small) send_msg(LVM_SETIMAGELIST, LVSIL_SMALL, (int)il_small->handle());
	   if (il_large) send_msg(LVM_SETIMAGELIST, LVSIL_NORMAL,(int)il_large->handle());
	   m_has_images = true;
}

void TListViewB::add_column(const char* label, int width)
{
     LVCOLUMN lvc;
     lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
     lvc.fmt = LVCFMT_LEFT;   // left-align, by default
     lvc.cx = width;
     lvc.pszText = (char*)label;
	 lvc.iSubItem = m_last_col;

	 ListView_InsertColumn((HWND)m_hwnd, m_last_col, &lvc);
	 m_last_col++;
}

void TListViewB::set_foreground(unsigned int colour)
{
	send_msg(LVM_SETTEXTCOLOR,0,(LPARAM)colour);
	m_fg = colour;
}

void TListViewB::set_background(unsigned int colour)
{
	send_msg(LVM_SETBKCOLOR,0,(LPARAM)colour);
	m_bg = colour;
	m_custom_paint = true;
}


int TListViewB::columns()
{
	return m_last_col;
}

void TListViewB::autosize_column(int col, bool by_contents)
{
  ListView_SetColumnWidth((HWND)m_hwnd,col,by_contents ? LVSCW_AUTOSIZE : LVSCW_AUTOSIZE_USEHEADER);
}

void TListViewB::start_items()
{
	m_last_row = -1;
}

int TListViewB::add_item_at(int i, const char* text, int idx, void* data)
{
     LVITEM lvi;
     lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
	 if (m_has_images) lvi.mask |= LVIF_IMAGE;
	 lvi.state = 0;
	 lvi.stateMask = 0;
     lvi.pszText = (char*)text;
 	 lvi.lParam = (unsigned long)data;
	 lvi.iItem = i;
	 lvi.iImage = idx;                // image list index
	 lvi.iSubItem = 0;

     ListView_InsertItem((HWND)m_hwnd, &lvi);
	 return i;
}

int TListViewB::add_item(const char* text, int idx, void* data)
{
	 m_last_row++;
	 return add_item_at(m_last_row,text,idx,data);
}

void TListViewB::add_subitem(int i, const char* text, int idx)
{
	ListView_SetItemText((HWND)m_hwnd,i,idx,(char*)text);
}

void TListViewB::delete_item(int i)
{
	ListView_DeleteItem((HWND)m_hwnd,i);
}

void TListViewB::select_item(int i)
{
	ListView_SetItemState((HWND)m_hwnd,i,LVIS_SELECTED | LVIS_FOCUSED,LVIS_SELECTED | LVIS_FOCUSED );
}

void TListViewB::get_item_text(int i, char* buff, int buffsize)
{
	ListView_GetItemText((HWND)m_hwnd,i,0,buff,buffsize);
}

void* TListViewB::get_item_data(int i)
{
	LVITEM lvi;
	lvi.mask = LVIF_PARAM;
	lvi.iItem = i;
	lvi.iSubItem = 0;
	ListView_GetItem((HWND)m_hwnd,&lvi);
	return (void*)lvi.lParam;
}

int TListViewB::selected_id()
{
  return send_msg(LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
}

int TListViewB::count()
{
  return send_msg(LVM_GETITEMCOUNT);
}

void TListViewB::clear()
{
  send_msg(LVM_DELETEALLITEMS);
  m_last_row = -1;
}

static int list_custom_draw(void * lParam, COLORREF fg, COLORREF bg)
{
	 LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)lParam;

	 if(lplvcd->nmcd.dwDrawStage == CDDS_PREPAINT)
		// Request prepaint notifications for each item.
		return CDRF_NOTIFYITEMDRAW;

	 if(lplvcd->nmcd.dwDrawStage == CDDS_ITEMPREPAINT){
		lplvcd->clrText = fg;
		lplvcd->clrTextBk = bg;
		return CDRF_NEWFONT;
	}
	return 0;
}



int TListViewB::handle_notify(void *p)
{
 LPNMHDR np  = (LPNMHDR)p;
 int id = selected_id();
 switch(np->code) {
 case LVN_ITEMCHANGED:
	handle_select(id);
	return 1;
 case NM_DBLCLK:
	handle_double_click(id);
	return 1;
 case LVN_KEYDOWN:
	handle_onkey(((LPNMLVKEYDOWN)p)->wVKey);
	return 0;  // ignored, anyway
 case NM_RCLICK:
	send_msg(WM_CHAR,VK_ESCAPE,0);
	return 0;
 case NM_CUSTOMDRAW:
	if (m_custom_paint) {
		return list_custom_draw(p,m_fg,m_bg);
	}
	return 0;
 }
 return 0;
}

TListView::TListView(TEventWindow* form, bool large_icons, bool multiple_columns, bool single_select)
:	TListViewB(form,large_icons,multiple_columns,single_select),
	m_form(form),m_on_select(NULL), m_on_double_click(NULL),m_on_key(NULL)
{

}

void TListView::handle_select(int i)
{
	if (m_on_select) {
		(m_form->*m_on_select)(i);
	}
}

void TListView::handle_double_click(int i)
{
	if (m_on_select) {
		(m_form->*m_on_double_click)(i);
	}
}

void TListView::handle_onkey(int i)
{
	if (m_on_select) {
		(m_form->*m_on_key)(i);
	}
}

/*
class TTabToolTipNotifyHandler: public TNotifyWin {
public:
    TTabToolTipNotifyHandler(Handle h)
    {
       set(h);
    }

    int handle_notify(void *p)
    {
     LPNMTTDISPINFO lpToolTipText = (LPNMTTDISPINFO)p;
	 if (lpToolTipText->hdr.code == TTN_GETDISPINFO)
     {
        lpToolTipText->hinst = NULL;
        lpToolTipText->lpszText = lpToolTipText->szText;
        strcpy(lpToolTipText->szText,"Hello, World!");
     }
     return 0;
  	}
};
*/


TTabControlB::TTabControlB(TWin* form, bool multiline)
{
     // Create the tab control.
	 int style = WS_CHILD; // | TCS_TOOLTIPS;
	 if (multiline) style |= TCS_MULTILINE;

	set(create_common_control(form,WC_TABCONTROL,style,25));
	send_msg(WM_SETFONT,(WPARAM)::GetStockObject(DEFAULT_GUI_FONT),(LPARAM)TRUE);

	 m_index = 0;

     /*
     HWND hToolTip = (HWND)send_msg(TCM_GETTOOLTIPS);
     if (hToolTip) {
         form->add(new TTabToolTipNotifyHandler(hToolTip));
     }
     */
}

void TTabControlB::add(const char* caption, void* data, int image_idx /*= -1*/)
{
	TCITEM item;
	item.mask = TCIF_TEXT | TCIF_PARAM;
	item.pszText = (char*)caption;
	item.lParam = (LPARAM)data;
	send_msg(TCM_INSERTITEM,m_index++,(LPARAM)&item);
}

void* TTabControlB::get_data(int idx)
{
	if (idx == -1) idx = selected();
	TCITEM item;
	item.mask = TCIF_PARAM;
	send_msg(TCM_GETITEM,idx,(LPARAM)&item);
	return (void*)item.lParam;
}

void TTabControlB::remove(int idx /*= -1*/)
{
	send_msg(idx > -1 ? TCM_DELETEITEM : TCM_DELETEALLITEMS, idx);
}

void TTabControlB::selected(int idx /* = -1 */)
{
	send_msg(TCM_SETCURSEL,idx);
}

int TTabControlB::selected()
{
	return send_msg(TCM_GETCURSEL);
}

int TTabControlB::count()
{
	return m_index;
}

int TTabControlB::handle_notify(void *p)
{
    LPNMHDR np  = (LPNMHDR)p;
	int id = selected();
    switch(np->code) {
	case TCN_SELCHANGE:
		handle_select(id);
		return 1;
    case TCN_SELCHANGING:
		return handle_selection_changing(id);
  	}
    return 0;
}

TTabControl::TTabControl(TEventWindow* form, bool multiline /*= false*/)
: TTabControlB(form,multiline),m_form(form),m_on_select(NULL),m_on_selection_changing(NULL)
{
}


void TTabControl::handle_select(int id)
{
	if (m_on_select)
		(m_form->*m_on_select)(id);
}


int TTabControl::handle_selection_changing(int id)
{
	if (m_on_selection_changing)
      return (m_form->*m_on_selection_changing)(id);
	else
	  return 0;
}


TTreeView::TTreeView(TEventWindow* form, bool has_lines, bool editable )
		: m_form(form),m_on_select(NULL),m_on_selection_changing(NULL),m_has_images(false)
{
	int style = TVS_HASBUTTONS;
	if (has_lines) style |= (TVS_HASLINES | TVS_LINESATROOT);
	if (editable) style |= TVS_EDITLABELS;
	set(create_common_control(form,WC_TREEVIEW,style));
	/*TImageList* il = new TImageList(true);
	il -> load_shell_icons();
	set_image_list(il, false);*/
}

void TTreeView::handle_select(int i)
{
	if (m_on_select) {
		(m_form->*m_on_select)(i);
	}
}

void TTreeView::handle_double_click(int i)
{

}

void TTreeView::handle_onkey(int i)
{

}

void TTreeView::set_image_list(TImageList* il, bool normal)
{
	send_msg(TVM_SETIMAGELIST,normal ? TVSIL_NORMAL : TVSIL_STATE,(LPARAM)il->handle());
	m_has_images = true;
}

void TTreeView::set_foreground(unsigned int color)
{
	TreeView_SetTextColor((HWND)m_hwnd,(COLORREF)color);
}

void TTreeView::set_background(unsigned int color)
{
	TreeView_SetBkColor((HWND)m_hwnd,(COLORREF)color);
}

Handle TTreeView::add(Handle parent, const char* caption, int idx1, int idx2, bool has_children, void* data)
{
	TVITEM item;
	item.mask = TVIF_TEXT  | TVIF_PARAM | TVIF_CHILDREN;
	if (m_has_images) {
		 item.mask |= (TVIF_IMAGE | TVIF_SELECTEDIMAGE);
		 if (idx2 == -1) idx2 = idx1;
    }
	item.pszText = (char*)caption;
	//item.ccbTextMax ?
 	item.iImage = idx1;
	item.iSelectedImage = idx2;
	item.lParam = (LPARAM)data;
	item.cChildren = has_children ? 1 : 0;
	TVINSERTSTRUCT tvsi;
	tvsi.hParent = (HTREEITEM)parent;
	tvsi.hInsertAfter = TVI_LAST;
	tvsi.item = item;
	return (Handle)send_msg(TVM_INSERTITEM,0,(LPARAM)&tvsi);
}

void* TTreeView::get_data(Handle pn)
{
	//if (pn == NULL) pn = selected();
	TVITEM item;
	item.mask = TVIF_PARAM | TVIF_HANDLE;
	item.hItem = (HTREEITEM)pn;
	send_msg(TVM_GETITEM,0,(LPARAM)&item);
	return (void*)item.lParam;
}

void* TTreeView::get_selection()
{
	HTREEITEM hItem = TreeView_GetSelection((HWND)m_hwnd);
	return (void*)hItem;
}

void* TTreeView::get_parent(Handle item)
{
	HTREEITEM hItem = TreeView_GetParent((HWND)m_hwnd,(HTREEITEM)item);
	return (void*)hItem;
}

bool TTreeView::expand(Handle hItem, bool flag)
{
	UINT tve = TVE_COLLAPSE;
	if (flag) tve = TVE_EXPAND;
	return TreeView_Expand((HWND)m_hwnd,(HTREEITEM)hItem,tve);
}

bool TTreeView::toggle(Handle hItem)
{
	return TreeView_Expand((HWND)m_hwnd,(HTREEITEM)hItem,TVE_TOGGLE);
}

bool TTreeView::clear()
{
	return TreeView_DeleteAllItems((HWND)m_hwnd);
}

void TTreeView::select(Handle p)
{
	send_msg(TVM_SELECTITEM,TVGN_CARET,(LPARAM)p);
}

int TTreeView::handle_notify(void *p)
{
    NMTREEVIEW* np  = (NMTREEVIEW*)p;
	Handle item = np->itemNew.hItem;
    switch(np->hdr.code) {
	case TVN_SELCHANGED:
		handle_select(int(item));
		return 1;
    case TVN_SELCHANGING:
		if (m_on_selection_changing)
          return (m_form->*m_on_selection_changing)(int(item));
		else return 0;
	case TVN_KEYDOWN:
		handle_onkey(((LPNMTVKEYDOWN)p)->wVKey);
		return 0;  // ignored, anyway
	case TVN_ITEMEXPANDED:
		return 0;
	case TVN_ITEMEXPANDING:
		handle_select(int(item));
		return 0;
	case NM_DBLCLK:
		handle_double_click(int(get_selection()));
		return 0;
	case NM_RCLICK:
		/*TVITEM tv;
		TVHITTESTINFO hti;
		POINT p1;
		HWND hwndTree = (HWND)m_hwnd;
		//char achBuf[100];

		GetCursorPos(&p1);
		hti.flags=TVHT_ONITEM;
		memcpy(&hti.pt, &p1, sizeof(POINT));
		ScreenToClient(hwndTree, &hti.pt);

		ZeroMemory(&tv, sizeof(TVITEM));
		tv.hItem=(HTREEITEM)TreeView_HitTest(hwndTree, &hti);
		//tv.cchTextMax=100;
		//tv.pszText=achBuf;
		//tv.mask=TVIF_TEXT|TVIF_HANDLE;
		TreeView_GetItem(hwndTree,&tv);

		//select(tv.hItem);
		handle_select(int(tv.hItem));*/
		send_msg(WM_CHAR,VK_ESCAPE,0);
		return 0;
	/*case NM_CLICK:
		send_msg(WM_CHAR,VK_ESCAPE,0);
		return 0;*/
	}
	return 0;
}


/*
 int n = (int)ExtractIcon(hInst,mod,-1);
 for (int i = 0; i < n; i++) {
     hIcon = ExtractIcon(hInst,mod,i);
 //while ((hIcon = LoadImage(hInst, (const char*)(i++), IMAGE_ICON, cx, cy, LR_LOADMAP3DCOLORS)) != NULL)
	 ImageList_AddIcon(m_handle,hIcon);
 }
 return n;
*/

