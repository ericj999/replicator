#include "stdafx.h"
#include "CppUnitTest.h"
#include "StringT.h"
#include "Property.h"
#include "util.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace utReplicator
{		
	TEST_CLASS(StringTTest)
	{
	public:
		TEST_METHOD(PropertyCopyCtorTest)
		{
			Database::Property prop{ _T("Name"), _T("NameValue"), 0 };
			Database::Property prop2{ prop };
			Assert::AreEqual(prop.m_name.c_str(), prop2.m_name.c_str());
			Assert::AreEqual(prop.m_str.c_str(), prop2.m_str.c_str());
			Assert::AreEqual(prop.m_index, prop2.m_index);
		}

		TEST_METHOD(PropertyListCopyCtorTest)
		{
			Database::PropertyList props1;
			int i = 1;

			props1.push_back(Database::Property(_T("Name"), _T("NameValue"), 0));
			props1.push_back(Database::Property(_T("Integer"), i, 1));

			Database::PropertyList props2(props1);

			Assert::AreEqual(props2[0].m_name.c_str(), _T("Name"));
			Assert::AreEqual(props2[1].m_name.c_str(), _T("Integer"));
			Assert::AreEqual(props2[0].m_str.c_str(), _T("NameValue"));
			Assert::AreEqual(props2[1].m_i, i);
			Assert::AreEqual(props2[0].m_index, 0);
			Assert::AreEqual(props2[1].m_index, 1);
		}

		TEST_METHOD(ReplaceTest)
		{
			Assert::AreEqual(String::replace(_T("This is a test"), _T("'"), _T("''")), StringT(_T("This is a test")), L"No replacement.", LINE_INFO());
			Assert::AreEqual(String::replace(_T(""), _T("'"), _T("''")), StringT(_T("")), L"Empty string.", LINE_INFO());
			Assert::AreEqual(String::replace(_T("'"), _T("'"), _T("''")), StringT(_T("''")), L"Single apostrophe.", LINE_INFO());
			Assert::AreEqual(String::replace(_T("'this is a test"), _T("'"), _T("''")), StringT(_T("''this is a test")), L"Start with apostrophe.", LINE_INFO());
			Assert::AreEqual(String::replace(_T("this is a test'"), _T("'"), _T("''")), StringT(_T("this is a test''")), L"End with apostrophe.", LINE_INFO());
			Assert::AreEqual(String::replace(_T("'a test'"), _T("'"), _T("''")), StringT(_T("''a test''")), L"Quoted with apostrophe.", LINE_INFO());
		}

		TEST_METHOD(ParseParsingPathTest)
		{
			wchar_t* local[] = { L"C:", L"Users", L"eric_2", L"Documents" };
			std::vector<std::wstring> localPath = Util::ParseParsingPath(L"C:\\Users\\eric_2\\Documents");
			Assert::AreEqual(localPath.size(), static_cast<size_t>(4));
			for (size_t index = 0; index < localPath.size(); ++index)
			{
				Assert::AreEqual(localPath[index].c_str(), local[index]);
			}

			std::vector<std::wstring> localPath2 = Util::ParseParsingPath(L"C:\\Users\\eric_2\\Documents\\");
			Assert::AreEqual(localPath2.size(), static_cast<size_t>(4));
			for (size_t index = 0; index < localPath2.size(); ++index)
			{
				Assert::AreEqual(localPath2[index].c_str(), local[index]);
			}

			wchar_t* wpd[] =
			{
				L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}",
				L"\\\\?\\usb#vid_054c&pid_03fe#0e49a634437417#{6ac27878-a6fa-4155-ba85-f98f491d4f33}",
				L"SID-{10001,00000000000000000000000004437417,15846604800}",
				L"{00000000-0000-0000-B86B-754800000000}",
				L"{0001CF32-01E0-0000-4301-05D700000000}"
			};

			std::vector<std::wstring> wpdPath = Util::ParseParsingPath(
				L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\\\\\?\\usb#vid_054c&pid_03fe#0e49a634437417#{6ac27878-a6fa-4155-ba85-f98f491d4f33}\\SID-{10001,00000000000000000000000004437417,15846604800}\\{00000000-0000-0000-B86B-754800000000}\\{0001CF32-01E0-0000-4301-05D700000000}");

			Assert::AreEqual(wpdPath.size(), static_cast<size_t>(5));
			for (size_t index = 0; index < wpdPath.size(); ++index)
			{
				Assert::AreEqual(wpdPath[index].c_str(), wpd[index]);
			}

			std::vector<std::wstring> wpdPath2 = Util::ParseParsingPath(
				L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}\\\\\\?\\usb#vid_054c&pid_03fe#0e49a634437417#{6ac27878-a6fa-4155-ba85-f98f491d4f33}\\SID-{10001,00000000000000000000000004437417,15846604800}\\{00000000-0000-0000-B86B-754800000000}\\{0001CF32-01E0-0000-4301-05D700000000}\\");

			Assert::AreEqual(wpdPath2.size(), static_cast<size_t>(5));
			for (size_t index = 0; index < wpdPath2.size(); ++index)
			{
				Assert::AreEqual(wpdPath2[index].c_str(), wpd[index]);
			}

			wchar_t* network[] = { L"\\\\ERIC-PC", L"Users", L"eric_2", L"Documents" };
			std::vector<std::wstring> networkPath = Util::ParseParsingPath(L"\\\\ERIC-PC\\Users\\eric_2\\Documents");
			Assert::AreEqual(networkPath.size(), static_cast<size_t>(4));
			for (size_t index = 0; index < networkPath.size(); ++index)
			{
				Assert::AreEqual(networkPath[index].c_str(), network[index]);
			}

			std::vector<std::wstring> networkPath2 = Util::ParseParsingPath(L"\\\\ERIC-PC\\Users\\eric_2\\Documents\\");
			Assert::AreEqual(networkPath2.size(), static_cast<size_t>(4));
			for (size_t index = 0; index < networkPath2.size(); ++index)
			{
				Assert::AreEqual(networkPath2[index].c_str(), network[index]);
			}
		}

	};
}