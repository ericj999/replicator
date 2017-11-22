#include "stdafx.h"
#include "CppUnitTest.h"
#include "StringT.h"
#include "Property.h"

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

	};
}