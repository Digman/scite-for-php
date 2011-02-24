// twl_dialogs.h

#ifndef __TWL_DIALOGS_H
#define __TWL_DIALOGS_H

class EXPORT TOpenFile {
protected:
  void *m_ofn;
  char *m_filename;
  bool m_prompt;
  char *m_file;
  char *m_file_out;
  char *m_path;
public:
  TOpenFile(TWin *parent,const char *caption,const char *filter,bool do_prompt=true);
  ~TOpenFile();
  virtual bool go();
  void initial_dir(const char *dir);
  bool next();
  const char *file_name();
  void file_name(const char *buff);
};

class TSelectDir {
	TWin *parent;
	char *descr;
	char *dirPath;
	char *lpszInitialDir;
	HWND m_hWndTreeView;
	TSelectDir();
public:
	TSelectDir(TWin *_parent, const char *_description="",const char *_initialdir="");
	virtual ~TSelectDir();
	virtual bool go();
	const char *path() const
		{ return dirPath; }
private:
	static int WINAPI SHBrowseCallBack( HWND hWnd, UINT uMsg, LPARAM lParam, LPARAM lpData );
};

class EXPORT TSaveFile: public TOpenFile {
	public:
	TSaveFile(TWin *parent, const char *caption, const char *filter,bool do_prompt=true)
		: TOpenFile(parent,caption,filter,false)
	{}
	bool go();
};

class EXPORT TColourDialog {
protected:
	void *m_choose_color;
public:
	TColourDialog(TWin *parent, unsigned int clr);
	virtual bool go();
	int result();
};
#endif
