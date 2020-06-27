#pragma once
#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <sstream>

/*
* \brief Generic configuration Class
*
*/
class Config
{
	// Data
	static std::string& trim(std::string&);
	static std::string& toupper(std::string&);
	static std::string& tolower(std::string&);
protected:
	std::string m_Delimiter;  //!< separator between key and value
	std::string m_Comment;    //!< separator between value and comments
	std::map<std::string, std::string> m_Contents;  //!< extracted keys and values

	typedef std::map<std::string, std::string>::iterator mapi;
	typedef std::map<std::string, std::string>::const_iterator mapci;
	// Methods
public:

	Config(const char* filename, std::string delimiter = "=", std::string comment = "#");
	Config(std::string filename, std::string delimiter = "=", std::string comment = "#");
	Config();
	template<class T> T Read(const std::string& in_key) const;  //!<Search for key and read value or optional default value, call as read<T>
	template<class T> T Read(const std::string& in_key, const T& in_value) const;
	template<class T> bool ReadInto(T& out_var, const std::string& in_key) const;
	template<class T>
	bool ReadInto(T& out_var, const std::string& in_key, const T& in_value) const;
	static bool FileExist(std::string filename);
	void ReadFile(std::string filename, std::string delimiter = "=", std::string comment = "#");
	void SaveFile(std::string filename);

	// Check whether key exists in configuration
	bool KeyExists(const std::string& in_key) const;

	// Modify keys and values
	template<class T> void Add(const std::string& in_key, const T& in_value);
	void Remove(const std::string& in_key);

	// Check or change configuration syntax
	std::string GetDelimiter() const { return m_Delimiter; }
	std::string GetComment() const { return m_Comment; }
	std::string SetDelimiter(const std::string& in_s)
	{
		std::string old = m_Delimiter;  m_Delimiter = in_s;  return old;
	}
	std::string SetComment(const std::string& in_s)
	{
		std::string old = m_Comment;  m_Comment = in_s;  return old;
	}

	// Write or read configuration
	friend std::ostream& operator<<(std::ostream& os, const Config& cf);
	friend std::istream& operator>>(std::istream& is, Config& cf);

protected:
	template<class T> static std::string T_as_string(const T& t);
	template<class T> static T string_as_T(const std::string& s);

	// Exception types
public:
	struct File_not_found
	{
		std::string filename;
		File_not_found(const std::string& filename_ = std::string())
			: filename(filename_)
		{}
	};
	struct Key_not_found
	{  // thrown only by T read(key) variant of read()
		std::string key;
		Key_not_found(const std::string& key_ = std::string())
			: key(key_)
		{}
	};
};

/* static */
template<class T>
std::string Config::T_as_string(const T& t)
{
	std::ostringstream ost;
	ost << t;
	return ost.str();
}

/* static */
template<class T>
T Config::string_as_T(const std::string& s)
{
	T t;
	std::istringstream ist(s);
	ist >> t;
	return t;
}

/* static */
template<>
inline std::string Config::string_as_T<std::string>(const std::string& s)
{
	return s;
}

/* static */
template<>
inline bool Config::string_as_T<bool>(const std::string& s)
{
	bool b = true;
	std::string sup(s);
	toupper(sup);
	if (sup == "FALSE" || sup == "F" ||
		sup == "NO" || sup == "N" ||
		sup == "0" || sup == "NONE" ||
		sup == "NULL" || sup == "NUL")
		b = false;
	return b;
}

template<class T>
T Config::Read(const std::string& key) const
{
	mapci p = m_Contents.find(key);
	if (p == m_Contents.end()) throw Key_not_found(key);
	return string_as_T<T>(p->second);
}

template<class T>
T Config::Read(const std::string& key, const T& value) const
{
	mapci p = m_Contents.find(key);
	if (p == m_Contents.end())
		return value;
	return string_as_T<T>(p->second);
}

template<class T>
bool Config::ReadInto(T& var, const std::string& key) const
{
	mapci p = m_Contents.find(key);
	if (p != m_Contents.end())
	{
		var = string_as_T<T>(p->second);
		return true;
	}
	else
	{
		return false;
	}
}

template<class T>
bool Config::ReadInto(T& var, const std::string& key, const T& value) const
{
	mapci p = m_Contents.find(key);
	if (p != m_Contents.end())
	{
		var = string_as_T<T>(p->second);
		return true;
	}
	else
	{
		var = value;
		return false;
	}
}

template<class T>
void Config::Add(const std::string& in_key, const T& value)
{
	std::string v = T_as_string(value);
	std::string key = in_key;
	trim(key);
	trim(v);
	m_Contents[key] = v;
}
