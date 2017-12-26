#include "stdafx.h"

#include "WPDPortableDevice.h"
#include "WPDDefinitions.h"

namespace WPD
{
	struct WPDIdName
	{
		GUID guid;
		LPCWSTR name;
	};

	enum class WPDContentType : int
	{
		funtionalObject = 0,
		folder,
		image,
		document,
		contact,
		contactGroup,
		audio,
		video,
		television,
		playlist,
		mixedContentAlbum,
		audioAlbum,
		imageAlbum,
		vidioAlbum,
		memo,
		email,
		appointment,
		task,
		program,
		genericFile,
		clander,
		genericMessage,
		networkAssociation,
		certificate,
		wirelessProfile,
		mediaCast,
		section,
		unspecified,
		max
	};

	enum class WPDObjectFormatType : int
	{
		icon = 0,
		m4a,
		networkAssociation,
		x509v3certificate,
		mswfc,
		_3gpa,
		_3g2a,
		propertiesOnly,
		unspecified,
		script,
		executable,
		text,
		html,
		dpof,
		aiff,
		wave,
		mp3,
		avi,
		mpeg,
		asf,
		exif,	// jpg
		tiffep,
		flashpix,
		bmp,
		ciff,
		gif,
		jfif,
		pcd,
		pict,
		png,
		tiff,
		tiffit,
		jp2,
		jpx,
		wbmp,
		jpegxr,
		windowsImageFormat,
		wma,
		wmv,
		wplplaylist,
		m3uplaylist,
		mplplaylist,
		asxplaylist,
		plsplaylist,
		absContectGroup,
		absMediaCast,
		vcalender1,
		icalender,
		absContact,
		vcard2,
		vcard3,
		xml,
		aac,
		audible,
		flac,
		qcelp,
		amr,
		ogg,
		mp4,
		mp2,
		msWord,
		mhtCompiledHtml,
		msExcel,
		msPowerPoint,
		_3gp,
		_3g2,
		avchd,
		atscts,
		dvbts,
		mkv,
		max
	};

	struct FileExtFormatMap
	{
		wchar_t* ext;
		WPDObjectFormatType formatIndex;
		WPDContentType typeIndex;
	};

	constexpr int maxContentType = static_cast<int>(WPDContentType::max);

	WPDIdName g_ContentType[maxContentType] =
	{
		{ WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT , L"WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT" },
		{ WPD_CONTENT_TYPE_FOLDER, L"WPD_CONTENT_TYPE_FOLDER" },
		{ WPD_CONTENT_TYPE_IMAGE, L"WPD_CONTENT_TYPE_IMAGE" },
		{ WPD_CONTENT_TYPE_DOCUMENT, L"WPD_CONTENT_TYPE_DOCUMENT" },
		{ WPD_CONTENT_TYPE_CONTACT, L"WPD_CONTENT_TYPE_CONTACT" },
		{ WPD_CONTENT_TYPE_CONTACT_GROUP, L"WPD_CONTENT_TYPE_CONTACT_GROUP" },
		{ WPD_CONTENT_TYPE_AUDIO, L"WPD_CONTENT_TYPE_AUDIO" },
		{ WPD_CONTENT_TYPE_VIDEO, L"WPD_CONTENT_TYPE_VIDEO" },
		{ WPD_CONTENT_TYPE_TELEVISION, L"WPD_CONTENT_TYPE_TELEVISION" },
		{ WPD_CONTENT_TYPE_PLAYLIST, L"WPD_CONTENT_TYPE_PLAYLIST" },
		{ WPD_CONTENT_TYPE_MIXED_CONTENT_ALBUM, L"WPD_CONTENT_TYPE_MIXED_CONTENT_ALBUM" },
		{ WPD_CONTENT_TYPE_AUDIO_ALBUM, L"WPD_CONTENT_TYPE_AUDIO_ALBUM" },
		{ WPD_CONTENT_TYPE_IMAGE_ALBUM, L"WPD_CONTENT_TYPE_IMAGE_ALBUM" },
		{ WPD_CONTENT_TYPE_VIDEO_ALBUM, L"WPD_CONTENT_TYPE_VIDEO_ALBUM" },
		{ WPD_CONTENT_TYPE_MEMO, L"WPD_CONTENT_TYPE_MEMO" },
		{ WPD_CONTENT_TYPE_EMAIL, L"WPD_CONTENT_TYPE_EMAIL" },
		{ WPD_CONTENT_TYPE_APPOINTMENT, L"WPD_CONTENT_TYPE_APPOINTMENT" },
		{ WPD_CONTENT_TYPE_TASK, L"WPD_CONTENT_TYPE_TASK" },
		{ WPD_CONTENT_TYPE_PROGRAM, L"WPD_CONTENT_TYPE_PROGRAM" },
		{ WPD_CONTENT_TYPE_GENERIC_FILE, L"WPD_CONTENT_TYPE_GENERIC_FILE" },
		{ WPD_CONTENT_TYPE_CALENDAR, L"WPD_CONTENT_TYPE_CALENDAR" },
		{ WPD_CONTENT_TYPE_GENERIC_MESSAGE, L"WPD_CONTENT_TYPE_GENERIC_MESSAGE" },
		{ WPD_CONTENT_TYPE_NETWORK_ASSOCIATION, L"WPD_CONTENT_TYPE_NETWORK_ASSOCIATION" },
		{ WPD_CONTENT_TYPE_CERTIFICATE, L"WPD_CONTENT_TYPE_CERTIFICATE" },
		{ WPD_CONTENT_TYPE_WIRELESS_PROFILE, L"WPD_CONTENT_TYPE_WIRELESS_PROFILE" },
		{ WPD_CONTENT_TYPE_MEDIA_CAST, L"WPD_CONTENT_TYPE_MEDIA_CAST" },
		{ WPD_CONTENT_TYPE_SECTION, L"WPD_CONTENT_TYPE_SECTION" },
		{ WPD_CONTENT_TYPE_UNSPECIFIED, L"WPD_CONTENT_TYPE_UNSPECIFIED" }
	};

	constexpr int maxObjectFormat = static_cast<int>(WPDObjectFormatType::max);

	WPDIdName g_ObjectFormat[maxObjectFormat] =
	{
		{ WPD_OBJECT_FORMAT_ICON, L"WPD_OBJECT_FORMAT_ICON" },
		{ WPD_OBJECT_FORMAT_M4A, L"WPD_OBJECT_FORMAT_M4A" },
		{ WPD_OBJECT_FORMAT_NETWORK_ASSOCIATION, L"WPD_OBJECT_FORMAT_NETWORK_ASSOCIATION" },
		{ WPD_OBJECT_FORMAT_X509V3CERTIFICATE, L"WPD_OBJECT_FORMAT_X509V3CERTIFICATE" },
		{ WPD_OBJECT_FORMAT_MICROSOFT_WFC, L"WPD_OBJECT_FORMAT_MICROSOFT_WFC" },
		{ WPD_OBJECT_FORMAT_3GPA, L"WPD_OBJECT_FORMAT_3GPA" },
		{ WPD_OBJECT_FORMAT_3G2A, L"WPD_OBJECT_FORMAT_3G2A" },
		// legacy formats
		{ WPD_OBJECT_FORMAT_PROPERTIES_ONLY, L"WPD_OBJECT_FORMAT_PROPERTIES_ONLY" },
		{ WPD_OBJECT_FORMAT_UNSPECIFIED, L"WPD_OBJECT_FORMAT_UNSPECIFIED" },
		{ WPD_OBJECT_FORMAT_SCRIPT, L"WPD_OBJECT_FORMAT_SCRIPT" },
		{ WPD_OBJECT_FORMAT_EXECUTABLE, L"WPD_OBJECT_FORMAT_EXECUTABLE" },
		{ WPD_OBJECT_FORMAT_TEXT, L"WPD_OBJECT_FORMAT_TEXT" },
		{ WPD_OBJECT_FORMAT_HTML, L"WPD_OBJECT_FORMAT_HTML" },
		{ WPD_OBJECT_FORMAT_DPOF, L"WPD_OBJECT_FORMAT_DPOF" },
		{ WPD_OBJECT_FORMAT_AIFF, L"WPD_OBJECT_FORMAT_AIFF" },
		{ WPD_OBJECT_FORMAT_WAVE, L"WPD_OBJECT_FORMAT_WAVE" },
		{ WPD_OBJECT_FORMAT_MP3, L"WPD_OBJECT_FORMAT_MP3" },
		{ WPD_OBJECT_FORMAT_AVI, L"WPD_OBJECT_FORMAT_AVI" },
		{ WPD_OBJECT_FORMAT_MPEG, L"WPD_OBJECT_FORMAT_MPEG" },
		{ WPD_OBJECT_FORMAT_ASF, L"WPD_OBJECT_FORMAT_ASF" },
		{ WPD_OBJECT_FORMAT_EXIF, L"WPD_OBJECT_FORMAT_EXIF" },	// jpeg
		{ WPD_OBJECT_FORMAT_TIFFEP, L"WPD_OBJECT_FORMAT_TIFFEP" },
		{ WPD_OBJECT_FORMAT_FLASHPIX, L"WPD_OBJECT_FORMAT_FLASHPIX" },
		{ WPD_OBJECT_FORMAT_BMP, L"WPD_OBJECT_FORMAT_BMP" },
		{ WPD_OBJECT_FORMAT_CIFF, L"WPD_OBJECT_FORMAT_CIFF" },
		{ WPD_OBJECT_FORMAT_GIF, L"WPD_OBJECT_FORMAT_GIF" },
		{ WPD_OBJECT_FORMAT_JFIF, L"WPD_OBJECT_FORMAT_JFIF" },
		{ WPD_OBJECT_FORMAT_PCD, L"WPD_OBJECT_FORMAT_PCD" },
		{ WPD_OBJECT_FORMAT_PICT, L"WPD_OBJECT_FORMAT_PICT" },
		{ WPD_OBJECT_FORMAT_PNG, L"WPD_OBJECT_FORMAT_PNG" },
		{ WPD_OBJECT_FORMAT_TIFF, L"WPD_OBJECT_FORMAT_TIFF" },
		{ WPD_OBJECT_FORMAT_TIFFIT, L"WPD_OBJECT_FORMAT_TIFFIT" },
		{ WPD_OBJECT_FORMAT_JP2, L"WPD_OBJECT_FORMAT_JP2" },
		{ WPD_OBJECT_FORMAT_JPX, L"WPD_OBJECT_FORMAT_JPX" },
		{ WPD_OBJECT_FORMAT_WBMP, L"WPD_OBJECT_FORMAT_WBMP" },
		{ WPD_OBJECT_FORMAT_JPEGXR, L"WPD_OBJECT_FORMAT_JPEGXR" },
		{ WPD_OBJECT_FORMAT_WINDOWSIMAGEFORMAT, L"WPD_OBJECT_FORMAT_WINDOWSIMAGEFORMAT" },
		{ WPD_OBJECT_FORMAT_WMA, L"WPD_OBJECT_FORMAT_WMA" },
		{ WPD_OBJECT_FORMAT_WMV, L"WPD_OBJECT_FORMAT_WMV" },
		{ WPD_OBJECT_FORMAT_WPLPLAYLIST, L"WPD_OBJECT_FORMAT_WPLPLAYLIST" },
		{ WPD_OBJECT_FORMAT_M3UPLAYLIST, L"WPD_OBJECT_FORMAT_M3UPLAYLIST" },
		{ WPD_OBJECT_FORMAT_MPLPLAYLIST, L"WPD_OBJECT_FORMAT_MPLPLAYLIST" },
		{ WPD_OBJECT_FORMAT_ASXPLAYLIST, L"WPD_OBJECT_FORMAT_ASXPLAYLIST" },
		{ WPD_OBJECT_FORMAT_PLSPLAYLIST, L"WPD_OBJECT_FORMAT_PLSPLAYLIST" },
		{ WPD_OBJECT_FORMAT_ABSTRACT_CONTACT_GROUP, L"WPD_OBJECT_FORMAT_ABSTRACT_CONTACT_GROUP" },
		{ WPD_OBJECT_FORMAT_ABSTRACT_MEDIA_CAST, L"WPD_OBJECT_FORMAT_ABSTRACT_MEDIA_CAST" },
		{ WPD_OBJECT_FORMAT_VCALENDAR1, L"WPD_OBJECT_FORMAT_VCALENDAR1" },
		{ WPD_OBJECT_FORMAT_ICALENDAR, L"WPD_OBJECT_FORMAT_ICALENDAR" },
		{ WPD_OBJECT_FORMAT_ABSTRACT_CONTACT, L"WPD_OBJECT_FORMAT_ABSTRACT_CONTACT" },
		{ WPD_OBJECT_FORMAT_VCARD2, L"WPD_OBJECT_FORMAT_VCARD2" },
		{ WPD_OBJECT_FORMAT_VCARD3, L"WPD_OBJECT_FORMAT_VCARD3" },
		{ WPD_OBJECT_FORMAT_XML, L"WPD_OBJECT_FORMAT_XML" },
		{ WPD_OBJECT_FORMAT_AAC, L"WPD_OBJECT_FORMAT_AAC" },
		{ WPD_OBJECT_FORMAT_AUDIBLE, L"WPD_OBJECT_FORMAT_AUDIBLE" },
		{ WPD_OBJECT_FORMAT_FLAC, L"WPD_OBJECT_FORMAT_FLAC" },
		{ WPD_OBJECT_FORMAT_QCELP, L"WPD_OBJECT_FORMAT_QCELP" },
		{ WPD_OBJECT_FORMAT_AMR, L"WPD_OBJECT_FORMAT_AMR" },
		{ WPD_OBJECT_FORMAT_OGG, L"WPD_OBJECT_FORMAT_OGG" },
		{ WPD_OBJECT_FORMAT_MP4, L"WPD_OBJECT_FORMAT_MP4" },
		{ WPD_OBJECT_FORMAT_MP2, L"WPD_OBJECT_FORMAT_MP2" },
		{ WPD_OBJECT_FORMAT_MICROSOFT_WORD, L"WPD_OBJECT_FORMAT_MICROSOFT_WORD" },
		{ WPD_OBJECT_FORMAT_MHT_COMPILED_HTML, L"WPD_OBJECT_FORMAT_MHT_COMPILED_HTML" },
		{ WPD_OBJECT_FORMAT_MICROSOFT_EXCEL, L"WPD_OBJECT_FORMAT_MICROSOFT_EXCEL" },
		{ WPD_OBJECT_FORMAT_MICROSOFT_POWERPOINT, L"WPD_OBJECT_FORMAT_MICROSOFT_POWERPOINT" },
		{ WPD_OBJECT_FORMAT_3GP, L"WPD_OBJECT_FORMAT_3GP" },
		{ WPD_OBJECT_FORMAT_3G2, L"WPD_OBJECT_FORMAT_3G2" },
		{ WPD_OBJECT_FORMAT_AVCHD, L"WPD_OBJECT_FORMAT_AVCHD" },
		{ WPD_OBJECT_FORMAT_ATSCTS, L"WPD_OBJECT_FORMAT_ATSCTS" },
		{ WPD_OBJECT_FORMAT_DVBTS, L"WPD_OBJECT_FORMAT_DVBTS" },
		{ WPD_OBJECT_FORMAT_MKV, L"WPD_OBJECT_FORMAT_MKV" }
	};

	FileExtFormatMap g_FileExtFormatMap[] =
	{
		{ L"wav", WPDObjectFormatType::wave, WPDContentType::audio },
		{ L"mp3", WPDObjectFormatType::mp3, WPDContentType::audio },
		{ L"wma", WPDObjectFormatType::wma, WPDContentType::audio },
		{ L"ogg", WPDObjectFormatType::wave, WPDContentType::audio },
		{ L"mp4", WPDObjectFormatType::mp4, WPDContentType::video },
		{ L"wmv", WPDObjectFormatType::wmv, WPDContentType::video },
		{ L"avi", WPDObjectFormatType::avi, WPDContentType::video },
		{ L"mpg", WPDObjectFormatType::mpeg, WPDContentType::video },
		{ L"mpeg", WPDObjectFormatType::mpeg, WPDContentType::video },
		{ L"asf", WPDObjectFormatType::asf, WPDContentType::audio },
		{ L"jpg", WPDObjectFormatType::exif, WPDContentType::image },
		{ L"jpeg", WPDObjectFormatType::exif, WPDContentType::image },
		{ L"jfif", WPDObjectFormatType::jfif, WPDContentType::image },
		{ L"tif", WPDObjectFormatType::tiff, WPDContentType::image },
		{ L"tiff", WPDObjectFormatType::tiff, WPDContentType::image },
		{ L"bmp", WPDObjectFormatType::bmp, WPDContentType::image },
		{ L"gif", WPDObjectFormatType::gif, WPDContentType::image },
		{ L"pic", WPDObjectFormatType::pict, WPDContentType::image },
		{ L"pict", WPDObjectFormatType::pict, WPDContentType::image },
		{ L"png", WPDObjectFormatType::png, WPDContentType::image },
		{ L"wmf", WPDObjectFormatType::windowsImageFormat, WPDContentType::image },
		{ L"ics", WPDObjectFormatType::icalender, WPDContentType::clander },
		{ L"aac", WPDObjectFormatType::aac, WPDContentType::audio },
		{ L"mp2", WPDObjectFormatType::mp2, WPDContentType::audio },
		{ L"flac", WPDObjectFormatType::flac, WPDContentType::audio },
		{ L"m4a", WPDObjectFormatType::m4a, WPDContentType::audio },
		{ L"xml", WPDObjectFormatType::xml, WPDContentType::document },
		{ L"doc", WPDObjectFormatType::msWord, WPDContentType::document },
		{ L"xls", WPDObjectFormatType::msExcel, WPDContentType::document },
		{ L"ppt", WPDObjectFormatType::msPowerPoint, WPDContentType::document },
		{ L"mht", WPDObjectFormatType::mhtCompiledHtml, WPDContentType::document },
		{ L"txt", WPDObjectFormatType::text, WPDContentType::document },
		{ L"text", WPDObjectFormatType::text, WPDContentType::document },
		{ L"jp2", WPDObjectFormatType::jp2, WPDContentType::image },
		{ L"jpx", WPDObjectFormatType::jpx, WPDContentType::image },
		{ L"vcf", WPDObjectFormatType::vcard3, WPDContentType::contact },
		{ L"mt2s", WPDObjectFormatType::avchd, WPDContentType::video },
		{ L"mkv", WPDObjectFormatType::mkv, WPDContentType::video },
		{ L"jxr", WPDObjectFormatType::jpegxr, WPDContentType::image },
		{ L"hdp", WPDObjectFormatType::jpegxr, WPDContentType::image },
		{ L"wdp", WPDObjectFormatType::jpegxr, WPDContentType::image },
		{ nullptr, WPDObjectFormatType::unspecified, WPDContentType::unspecified }
	};

	void GetFileContentType(const std::wstring& ext, GUID& contentType, GUID& format)
	{
		for (size_t index = 0; ; ++index)
		{
			if (!g_FileExtFormatMap[index].ext || (_wcsicmp(ext.c_str(), g_FileExtFormatMap[index].ext) == 0))
			{
				contentType = g_ContentType[static_cast<int>(g_FileExtFormatMap[index].typeIndex)].guid;
				format = g_ObjectFormat[static_cast<int>(g_FileExtFormatMap[index].formatIndex)].guid;
				return;
			}
		}
	}

}