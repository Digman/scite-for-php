// TWL_INI.CPP
/*
 * Steve Donovan, 2003
 * This is GPL'd software, and the usual disclaimers apply.
 * See LICENCE
*/
#include <windows.h>
#include <direct.h>
#include "twl_ini.h"

#define WIN32_INI

EXPORT void output_debug_str(const char *buff)
{
  OutputDebugString(buff);
}

char IniBase::_tmpbuff_[BUFSZ];

void IniBase::write_int(pcchar key, int val)
{
  itoa(val,_tmpbuff_,10);
  write_string(key,_tmpbuff_);
}

int  IniBase::read_int(pcchar key, int def)
{
  char defstr[20];
  itoa(def,defstr,10);
  char* istr = read_string(key,_tmpbuff_,BUFSZ,defstr);
  return atoi(istr);
}

#ifndef WIN32_INI
struct Pair {
	char* key;
	char* value;
};
#define MAP(ptr,section,key,val) (*(IniMap)m_data)[section][key])
#endif

static void trim_end(char* buff)
{
	buff[strlen(buff)-1] = '\0';
}

IniFile::IniFile(pcchar file, bool in_cwd)
{
    if (! in_cwd) m_file = strdup(file);
    else {
        get_app_path(_tmpbuff_,BUFSZ);
        strcat(_tmpbuff_,"/");
        strcat(_tmpbuff_,file);
        m_file = strdup(_tmpbuff_);
    }
#ifndef WIN32_INI
	FILE *in = fopen(file,"r");
	while (! feof(in)) {
		fgets(_tmpbuff_,BUFSZ,in);
		trim_end(_tmpbuff_);
		if (*_tmpbuff_ == ';' || *_tmpbuff_ == '\0')
			continue;
		if (*_tmpbuff_ == '[') {
			char* section = _tmpbuff_ + 1;
			trim_end(section);
		}
	}
	fclose(in);

#endif
}

void IniFile::set_section(pcchar section)
{
	 m_section = section;
}

void IniFile::write_string(pcchar key, pcchar value)
{
    WritePrivateProfileString(m_section,key,value,m_file);
}

char *IniFile::read_string(pcchar key, char *value, int sz, pcchar def)
{
    GetPrivateProfileString(m_section,key,def,value,sz,m_file);
    return value;
}
