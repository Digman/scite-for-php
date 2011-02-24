#ifndef __TWL_TREEVIEW_H
#define __TWL_TREEVIEW_H
class EXPORT TTreeView: public TNotifyWin {
	SelectionHandler       m_on_select;
	NotifyEventHandler m_on_selection_changing;
	TEventWindow* m_form;
    bool m_has_images;
public:

  void on_select(SelectionHandler handler)
  { m_on_select = handler; }

  void on_selection_changing(NEH handler)
  { m_on_selection_changing = handler; }


   TTreeView(TEventWindow* form, bool has_lines = true, bool editable = false);
   Handle add(Handle parent, const char* caption, int idx1=0, int idx2=-1, bool has_children=false, void* data=0);
   void* get_data(Handle pn);
   void* get_selection();
   void* get_parent(Handle item);
   bool expand(Handle hItem, bool flag=true);
   bool toggle(Handle hItem);
   void select(Handle p);
   bool clear();
   void set_image_list(TImageList* il, bool normal = true);
   void set_foreground(unsigned int colour);
   void set_background(unsigned int colour);

   // override
   int handle_notify(void *p);
   // implement
   virtual void handle_select(int id);
   virtual void handle_double_click(int id);
   virtual void handle_onkey(int id);
};

#ifdef _GCC
typedef struct tagTVKEYDOWN {
  NMHDR hdr;
  WORD  wVKey;
  UINT  flags;
} NMTVKEYDOWN, *LPNMTVKEYDOWN;
#endif

#endif
