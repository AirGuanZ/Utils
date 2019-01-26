#pragma once

#include <cstring>
#include <filesystem>
#include <string>
#include <string_view>

#include "../Misc/Common.h"
#include "../String/StdStr.h"

namespace AGZ
{

/**
 * @cond
 */

#ifdef AGZ_OS_WIN32
	namespace fs = std::experimental::filesystem;
#else
	namespace fs = std::filesystem;
#endif

/**
 * @endcond
 */

/**
 * @brief 路径类，主要用于封装各平台对std::filesystem实现的不同。内部统一采用utf-8编码
 */
class Path
{
public:

	/**
	 * @brief 默认初始化为空路径
	 */
	Path() = default;

	/**
	 * @brief 以给定的字符串初始化
	 */
	explicit Path(std::string_view path)
		: path_(WIDEN(path))
	{
		
	}

	/**
	 * @brief 该路径是否指向一个regular file
	 */
	bool IsRegular() const
	{
		return fs::is_regular_file(path_);
	}

	/**
	 * @brief 该路径是否指向一个文件目录
	 */
	bool IsDirectory() const
	{
		return fs::is_directory(path_);
	}

	/**
	 * @brief 将该路径转换为utf-8编码的字符串
	 */
	std::string GetString() const
	{
		return path_.u8string();
	}

	/**
	 * @brief 在该路径后方追加一个子路径
	 */
	Path &Append(const Path &rhs)
	{
		path_.append(rhs);
		return *this;
	}

	/**
	 * @brief 设置文件名
	 */
	Path &SetFilename(std::string_view filename)
	{
		path_.replace_filename(filename);
		return *this;
	}

	/**
	 * @brief 设置文件名扩展
	 */
	Path &SetExtension(std::string_view ext)
	{
		path_.replace_extension(ext);
		return *this;
	}

	/**
	 * @brief 是否是一个绝对路径
	 */
	bool IsAbsolute() const
	{
		return path_.is_absolute();
	}

	/**
	 * @brief 是否是一个相对路径
	 */
	bool IsRelative() const
	{
		return path_.is_relative();
	}

private:

	fs::path path_;
};

/**
 * @brief 文件系统时间戳
 */
struct FileTime
{
	int32_t year   = 0;
	int32_t month  = 0;
	int32_t day    = 0;
	int32_t hour   = 0;
	int32_t minute = 0;
	int32_t second = 0;

	bool operator==(const FileTime &rhs) const { std::memcmp(this, &rhs, sizeof(FileTime)) == 0; }
};

} // namespace AGZ
